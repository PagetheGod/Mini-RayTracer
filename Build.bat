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
if %errorlevel% neq 0 (
	rem This prints a new line, separating the error output from CMake
	echo.
	echo MSVC build FAILED! See errors above.
	pause
) else (
	echo MSVC build succeeded!
)

exit /b %errorlevel%

:build_GCC

rem Try to find gcc on PATH, exit early if not found, the following commands suppress its output
where gcc >nul 2>nul
if %errorlevel% neq 0 (
	echo GCC not found on PATH! Make sure gcc and g++ are installed and added to your PATH.
	pause
	exit /b 1
)

echo Building the project with GCC...
cmake -S . -B build-GCC -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
cmake --build build-GCC -- -j
if %errorlevel% neq 0 (
	echo.
	echo GCC build FAILED! See errors above.
	pause
) else (
	echo GCC build succeeded!
)

exit /b %errorlevel%

:build_Clang

where clang >nul 2>nul
if %errorlevel% neq 0 (
	echo Clang not found on PATH! Make sure clang and clang++ are installed and added to your PATH.
	pause
	exit /b 1
)

echo Building the project with Clang...
cmake -S . -B build-Clang -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build build-Clang -- -j
if %errorlevel% neq 0 (
	echo.
	echo Clang build FAILED! See errors above.
	pause
) else (
	echo Clang build succeeded!
)

exit /b %errorlevel%

:build_ALL
call :build_MSVC
call :build_GCC
call :build_Clang
exit /b 0




