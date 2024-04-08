@echo off

CALL "%~dp0setup_for_dev.bat"

set BUILD_DIR=%~dp0build\
if not exist %BUILD_DIR%\ (
    mkdir %BUILD_DIR%
)

clang -std=c++20 src\main.cpp -o %BUILD_DIR%\main.exe
%BUILD_DIR%\main.exe