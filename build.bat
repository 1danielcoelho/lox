@echo off
setlocal
CALL "%~dp0setup_for_dev.bat"

@echo Setting up build directory
set BUILD_DIR=%~dp0build\
if not exist %BUILD_DIR%\ (
    mkdir %BUILD_DIR%
)
pushd %BUILD_DIR%

@REM @echo Compiling with Clang
@REM clang   ..\src\unity.cpp ^
@REM         -std=c++20 ^
@REM         -O0 ^
@REM         -g ^
@REM         -Wall -Wpedantic ^
@REM         -o main.exe

@echo Compiling with MSVC
cl  ..\src\unity.cpp ^
    /std:c++20 ^
    /EHsc ^
    /W4 /diagnostics:caret^
    /Od /Ob0 /DEBUG ^
    /Fe: main.exe

@echo Running
main.exe
@REM main.exe F:\Projects\lox\samples\text_simple.l

popd
endlocal