@echo off
CALL "%~dp0setup_for_dev.bat"

setlocal

set BUILD_DIR=%~dp0build\

@REM @echo Clearing previous executable
@REM if exist %BUILD_DIR%\main.exe (
@REM     del %BUILD_DIR%\main.exe
@REM )

@echo Setting up build directory
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
    /nologo ^
    /EHsc ^
    /W4 /diagnostics:caret^
    /Od /Ob0 /DEBUG /Zi ^
    /Fe: main.exe

@echo Running
main.exe
@REM main.exe F:\Projects\lox\samples\text_simple.l

popd
endlocal