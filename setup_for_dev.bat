@echo off

REM Setup to use MSVC cl.exe
if not defined DevEnvDir (
	CALL "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)
