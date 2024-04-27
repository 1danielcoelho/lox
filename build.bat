@echo off
CALL "%~dp0setup_for_dev.bat"

setlocal

set BUILD_DIR=%~dp0build\
set ENTRY_FILE=..\src\jlox\unity.cpp

@echo Setting up build directory
if not exist %BUILD_DIR%\ (
    mkdir %BUILD_DIR%
)
pushd %BUILD_DIR%

@REM @echo Compiling with Clang
@REM clang   %ENTRY_FILE% ^
@REM         -std=c++20 ^
@REM         -O0 ^
@REM         -g ^
@REM         -Wall -Wpedantic ^
@REM         -o main.exe

@echo Compiling with MSVC
cl  %ENTRY_FILE% ^
    /std:c++20 ^
    /nologo ^
    /EHsc ^
    /W4 /diagnostics:caret^
    /Od /Ob0 /DEBUG /Zi ^
    /Fe: main.exe

@echo Running
@REM main.exe
main.exe F:\Projects\lox\samples\basic.l

popd
endlocal