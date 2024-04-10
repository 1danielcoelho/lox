@echo off

CALL "%~dp0setup_for_dev.bat"

@REM Setup the build directory
set BUILD_DIR=%~dp0build\
if not exist %BUILD_DIR%\ (
    mkdir %BUILD_DIR%
)

@REM Compile everything
clang -std=c++20 ^
    src\main.cpp ^
    src\token.cpp ^
    src\tokenizer.cpp ^
    -o %BUILD_DIR%\main.exe

@REM Run
@REM %BUILD_DIR%\main.exe
%BUILD_DIR%\main.exe F:\Projects\lox\samples\text_newlines.l