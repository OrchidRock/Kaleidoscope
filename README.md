# Kaleidoscope
This is a extent project for Kaleidoscope compiler which comes from [LLVM tutorial](https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html).

## Kaleidoscope Language example:
``` 
# fib.kpe
#
def fib(x)
    if x < 3 then 1
    else fib(x-1) + fib(x-2);
```

## Depends

You need to instart `llvm` firstly.

## Build

`$ cmake -b build && cd build && make`

## Test example

Create assembly code:

`$ ./Kaleidoscope --filetype=asm fib.kpe`


Create LLVM IR:

`$ ./Kaleidoscope --filetype=asm --emit-llvm fib.kpe`


Create Object Code:

`$ ./Kaleidoscope --filetype=obj fib.kpe`

Enter JIT REPL:

`$ ./Kaleidoscope`


