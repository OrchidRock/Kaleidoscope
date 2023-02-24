#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/CodeGen/CommandFlags.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/WithColor.h"
//#include "version.inc"
#include "include/lexer.h"
#include "include/parser.h"
#include "include/codegen.h"
#include "include/JIT.h"

#include <memory>
#include <string>

using namespace llvm;

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

/// putchard - putchar that takes a double and returns 0.
extern "C" DLLEXPORT double putchard(double X) {
  fputc((char)X, stderr);
  return 0;
}

/// printd - printf that takes a double prints it as "%f\n", returning 0.
extern "C" DLLEXPORT double printd(double X) {
  fprintf(stderr, "%f\n", X);
  return 0;
}

static codegen::RegisterCodeGenFlags CGF;

static llvm::cl::opt<std::string>
    InputFilename(llvm::cl::Positional,
          llvm::cl::desc("<input source file>"),
          llvm::cl::init(""));

static llvm::cl::opt<signed char> OptLevel(
        llvm::cl::desc("Setting the optimization level:"),
        llvm::cl::ZeroOrMore,
        llvm::cl::values (
            clEnumValN(3, "O", "Equivalent to -O3"),
            clEnumValN(0, "O0", "Optimization level 0"),
            clEnumValN(1, "O1", "Optimization level 1"),
            clEnumValN(2, "O2", "Optimization level 2"),
            clEnumValN(3, "O3", "Optimization level 3"),
            clEnumValN(-1, "Os", "Like -O2 with extra optimization for size"),
            clEnumValN(-2, "Oz", "Like -Os but reduces code size further")
        ),
        llvm::cl::init(0));

static llvm::cl::opt<std::string> 
    OutputFilename("o", 
                    llvm::cl::desc("Specify output filename"), 
                    llvm::cl::value_desc("filename"),
                    llvm::cl::init(""));

static llvm::cl::opt<std::string>
    MTriple("mtriple",
            llvm::cl::desc("Override target triple for module"));

static llvm::cl::opt<bool>
    EmitLLVM("emit-llvm",
             llvm::cl::desc("Emit IR code instead of assembler"),
             llvm::cl::init(false));

llvm::TargetMachine *createTargetMachine(const char *Argv0) {
  llvm::Triple Triple = llvm::Triple(
      !MTriple.empty()
          ? llvm::Triple::normalize(MTriple)
          : llvm::sys::getDefaultTargetTriple());
    
  
  llvm::TargetOptions TargetOptions =
      codegen::InitTargetOptionsFromCodeGenFlags(Triple);
  std::string CPUStr = codegen::getCPUStr();
  std::string FeatureStr = codegen::getFeaturesStr();
  
  std::string Error;
  const llvm::Target *Target =
      llvm::TargetRegistry::lookupTarget(codegen::getMArch(), Triple,
                                         Error);

  if (!Target) {
    llvm::WithColor::error(llvm::errs(), Argv0) << Error;
    return nullptr;
  }
    
  llvm::TargetMachine *TM = Target->createTargetMachine(
      Triple.getTriple(), CPUStr, FeatureStr, TargetOptions,
      llvm::Optional<llvm::Reloc::Model>(codegen::getRelocModel()));
  return TM;
}

int emit(StringRef Argv0, llvm::Module& M, llvm::TargetMachine& TM,
                StringRef InputFilename){
  CodeGenFileType FileType = codegen::getFileType();
  std::string OutputFilename;
  if (InputFilename == "-") {
    OutputFilename = "-";
  } else {
    if (InputFilename.endswith(".kpe") ||
        InputFilename.endswith(".kpe"))
      OutputFilename = InputFilename.drop_back(4).str();
    else
      OutputFilename = InputFilename.str();
    switch (FileType) {
    case CGFT_AssemblyFile:
      OutputFilename.append(EmitLLVM ? ".ll" : ".s");
      break;
    case CGFT_ObjectFile:
      OutputFilename.append(".o");
      break;
    case CGFT_Null:
      OutputFilename.append(".null");
      break;
    }
  }
  
  // Open the file.
  std::error_code EC;
  sys::fs::OpenFlags OpenFlags = sys::fs::OF_None;
  if (FileType == CGFT_AssemblyFile)
    OpenFlags |= sys::fs::OF_Text;
  auto Out = std::make_unique<llvm::ToolOutputFile>(
      OutputFilename, EC, OpenFlags);
  if (EC) {
    WithColor::error(llvm::errs(), Argv0) << EC.message() << '\n';
    return false;
  }

  legacy::PassManager PM;
  if (FileType == CGFT_AssemblyFile && EmitLLVM) {
    PM.add(createPrintModulePass(Out->os()));
  } else {
    if (TM.addPassesToEmitFile(PM, Out->os(), nullptr,
                                FileType)) {
      WithColor::error() << "No support for file type\n";
      return false;
    }
  }

  PM.run(M);
  Out->keep();
  return true;
}


int main(int argc, char *argv[])
{
    llvm::InitLLVM X(argc, argv); 
    llvm::cl::ParseCommandLineOptions(
        argc, argv, "Kaleidoscope - the Kaleidoscope language compiler\n");
    
    auto TheContext = std::make_unique<LLVMContext>();
    auto TheModule = std::make_unique<Module>("my cool jit", *TheContext);

    if(InputFilename.size()) { // compiler
        
        llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
            FileOrErr = llvm::MemoryBuffer::getFile(InputFilename);

        if (std::error_code BufferError = FileOrErr.getError()) {
            llvm::errs() << "Error reading " << InputFilename << ": "
                   << BufferError.message() << "\n";
        }    
        
        llvm::SourceMgr SrcMgr;
         
        // Tell SrcMgr about this buffer, which is what the
        // parser will pick up.
        SrcMgr.AddNewSourceBuffer(std::move(*FileOrErr), llvm::SMLoc());
        
        auto &MyModule = *TheModule;
        
        Lexer* lexer = new LexerFile(SrcMgr);
        auto cg = new CodeGenVisitor(&SrcMgr,std::move(TheContext), std::move(TheModule),
                        OptLevel?1:0);
        auto parser = Parser(lexer, cg, false);
        parser.parse();
         
        delete lexer; 

        InitializeAllTargetInfos();
        InitializeAllTargets();
        InitializeAllTargetMCs();
        InitializeAllAsmParsers();
        InitializeAllAsmPrinters();
  
        auto TheTargetMachine = createTargetMachine(argv[0]); 
        if(!TheTargetMachine){
            delete cg;
            return 1;
        }
        
        MyModule.setDataLayout(TheTargetMachine->createDataLayout());
        
        if(!emit(argv[0], MyModule, *TheTargetMachine, InputFilename)){
          llvm::WithColor::error(llvm::errs(), argv[0]) << "Error writing output\n";
        }
        
        delete cg;
    } else{ // JIT
        InitializeNativeTarget();
        InitializeNativeTargetAsmPrinter();
        InitializeNativeTargetAsmParser();
        
        //auto &MyModule = *TheModule;
        
        Lexer* lexer = new LexerSimple();
        auto jit = new JITVisitor(std::move(TheContext), std::move(TheModule),
                        OptLevel?1:0);
        auto parser = Parser(lexer, jit, true);
        parser.parse(); 

        delete jit;
        delete lexer;
    }
    return 0;
}
