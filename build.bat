@echo off
setlocal
rem ============================================================================
rem build.bat - one-shot dependency fetch + build + test runner for snvoxeng
rem Usage:
rem   build.bat              | fetch deps, build lib + tests (Debug), run tests
rem   build.bat Release      | same in Release
rem   build.bat fetch        | only fetch dependencies (googletest)
rem ============================================================================

set "ROOT=%~dp0"
set "GTEST_TAG=v1.17.0"
set "GTEST_DIR=%ROOT%tests\thirdparty\googletest"

set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Debug"

echo === snvoxeng build ===
echo Configuration: %CONFIG%

rem ---- 1. Dependencies ------------------------------------------------------
if exist "%GTEST_DIR%\CMakeLists.txt" (
    echo [deps] GoogleTest already present: %GTEST_DIR%
    goto :msbuild_lookup
)

echo [deps] Fetching GoogleTest %GTEST_TAG% ...
git clone --depth 1 --branch %GTEST_TAG% https://github.com/google/googletest.git "%GTEST_DIR%"
if errorlevel 1 (
    echo [deps] ERROR: failed to clone googletest. Check your internet connection.
    exit /b 1
)
echo [deps] GoogleTest fetched OK.

:msbuild_lookup
if /i "%CONFIG%"=="fetch" exit /b 0

rem ---- 2. Locate MSBuild via vswhere ----------------------------------------
rem (-prerelease is required to see Visual Studio Insiders installations)
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo [build] ERROR: vswhere.exe not found.
    echo         Install Visual Studio 2022/2026 with the C++ workload,
    echo         or run this script from the Developer Command Prompt.
    exit /b 1
)

set "MSBUILD="
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do set "MSBUILD=%%i"
if not defined MSBUILD (
    echo [build] ERROR: MSBuild not found via vswhere.
    exit /b 1
)

set "SOLUTION_DIR=%ROOT%"
rem NOTE: passed UNQUOTED on purpose - the trailing backslash is required
rem ($(SolutionDir)build\ concatenation) and \" inside quotes would break parsing.


rem ---- 3. Build engine library -----------------------------------------------
echo [build] Engine library...
"%MSBUILD%" "%ROOT%snvoxeng\snvoxeng.vcxproj" /p:Configuration=%CONFIG% /p:Platform=x64 /p:SolutionDir=%SOLUTION_DIR%\ /m /v:m /nologo
if errorlevel 1 (
    echo [build] FAILED: engine library.
    exit /b 1
)

rem ---- 4. Build tests ----------------------------------------------------------
echo [build] Tests...
"%MSBUILD%" "%ROOT%tests\tests.vcxproj" /p:Configuration=%CONFIG% /p:Platform=x64 /p:SolutionDir=%SOLUTION_DIR%\ /m /v:m /nologo
if errorlevel 1 (
    echo [build] FAILED: tests.
    exit /b 1
)

rem ---- 5. Run tests --------------------------------------------------------------
set "EXE=%ROOT%build\tests-d.exe"
if /i not "%CONFIG%"=="Debug" set "EXE=%ROOT%build\tests.exe"

if not exist "%EXE%" (
    echo [run] WARNING: "%EXE%" not found, skipping test run.
    exit /b 0
)

echo [run] %EXE%
"%EXE%"
exit /b %errorlevel%
