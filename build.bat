@echo off

CALL "%~dp0setup_for_dev.bat"

@echo Setting up build directory
set BUILD_DIR=%~dp0build\
if not exist %BUILD_DIR%\ (
    mkdir %BUILD_DIR%
)

@echo Compiling
clang -std=c++20 ^
    src\main.cpp ^
    src\token.cpp ^
    src\tokenizer.cpp ^
    src\error.cpp ^
    src\expression.cpp ^
    src\ast_printer.cpp ^
    -O0 ^
    -g ^
    -o %BUILD_DIR%\main.exe

@echo Running
@REM %BUILD_DIR%\main.exe
%BUILD_DIR%\main.exe F:\Projects\lox\samples\text_simple.l