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

You need to install `llvm` firstly.

## Build

`$ cmake -b build && cd build && make`

## Test example

To compile and emit assembly code:

`$ ./Kaleidoscope --filetype=asm fib.kpe`

To compile and emit LLVM IR:

`$ ./Kaleidoscope --filetype=asm --emit-llvm fib.kpe`

To compile and emit object Code:

`$ ./Kaleidoscope --filetype=obj fib.kpe`

To optimize code with `-O1`:

`$ ./kaleidoscope -O1 fib.kpe`

Enter JIT REPL:

`$ ./Kaleidoscope`


