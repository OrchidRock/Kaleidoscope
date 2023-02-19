cmake_minimum_required(VERSION 3.4.3)


set(VERSION_STRING "1.0")
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/version.inc.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/version.inc
)

project(Kaleidoscope)

find_package(LLVM REQUIRED HINTS "${LLVM_CMAKE_PATH}")
message("-- Found LLVM ${LLVM_PACKAGE_VERSION}, build type ${LLVM_BUILD_TYPE}")
list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})

include(AddLLVM)
include(HandleLLVMOptions)
include_directories("${LLVM_BINARY_DIR}/include" "${LLVM_INCLUDE_DIR}")
link_directories("${LLVM_LIBRARY_DIR}")

set(CMAKE_CXX_FLAGS "-rdynamic ${CMAKE_CXX_FLAGS}")

add_subdirectory(codegen)
add_subdirectory(lexer)
add_subdirectory(parser)
add_subdirectory(JIT)

add_llvm_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE lexer parser codegen jit)
