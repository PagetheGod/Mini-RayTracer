@echo off

rem Check if the user typed nothing
if "%1"=="" (
	echo Usage: ./Build.bat [MSVC/GCC/Clang/ALL]
	exit /b 1
)

rem Execute different compiler build commands based on user input
if /i "%1"=="MSVC" goto :build_MSVC
if /i "%1"=="GCC" goto :build_GCC
if /i "%1"=="Clang" goto :build_Clang
if /i "%1"=="ALL" goto :build_ALL

rem We get an unknown option, prompt the user and quit
echo Unknown option: %1
echo Usage: ./Build.bat [MSVC/GCC/Clang/ALL]
exit /b 1

:build_MSVC
rem Be very careful with the white spaces, batch scripts require spaces before ( in if/else
rem See below for examples
echo Building the project with MSVC...
cmake -S . -B build-MSVC
cmake --build build-MSVC --config Release
rem Use "if errorlevel 1" instead of "if %errorlevel% neq 0"
rem %errorlevel% is expanded at parse time, not at execution time
rem Inside a parenthesized block, it would always see the OLD value
rem "if errorlevel 1" checks the actual runtime value (means: errorlevel >= 1)
if errorlevel 1 (
	echo.
	echo MSVC build FAILED! See errors above.
	pause
	exit /b 1
)
echo MSVC build succeeded!
exit /b 0

:build_GCC

rem Try to find gcc on PATH, exit early if not found, the following commands suppress its output
where gcc >nul 2>nul
if errorlevel 1 (
	echo GCC not found on PATH! Make sure gcc and g++ are installed and added to your PATH.
	pause
	exit /b 1
)

echo Building the project with GCC...
cmake -S . -B build-GCC -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
cmake --build build-GCC -- -j
if errorlevel 1 (
	echo.
	echo GCC build FAILED! See errors above.
	pause
	exit /b 1
)
echo GCC build succeeded!
exit /b 0

:build_Clang

where clang >nul 2>nul
if errorlevel 1 (
	echo Clang not found on PATH! Make sure clang and clang++ are installed and added to your PATH.
	pause
	exit /b 1
)

echo Building the project with Clang...
cmake -S . -B build-Clang -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build build-Clang -- -j
if errorlevel 1 (
	echo.
	echo Clang build FAILED! See errors above.
	pause
	exit /b 1
)
echo Clang build succeeded!
exit /b 0

:build_ALL
set BUILD_FAILED=0
call :build_MSVC
if errorlevel 1 set BUILD_FAILED=1
call :build_GCC
if errorlevel 1 set BUILD_FAILED=1
call :build_Clang
if errorlevel 1 set BUILD_FAILED=1
exit /b %BUILD_FAILED%
