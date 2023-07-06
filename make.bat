@echo off
set name=lsassdump

:: llvm clang x64 exe

x86_64-w64-mingw32-clang -o %name%.x64.exe -s -O2 -fpermissive -static -municode -Wall -Wextra -ldbghelp entry.c util.c common.c

llvm-strip --strip-all %name%.x64.exe