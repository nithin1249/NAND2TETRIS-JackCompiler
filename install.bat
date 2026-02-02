@echo off
setlocal

:: --- CONFIGURATION ---
set "INSTALL_DIR=%USERPROFILE%\.jack_toolchain"
set "TOOLS_SOURCE=tools"
set "BUILD_DIR=cmake-build-debug"
set "EXE_NAME=NAND2TETRIS.exe"

echo ==========================================
echo  Installing Jack Compiler (Windows)
echo ==========================================

:: 1. Check if Build Exists
if not exist "%BUILD_DIR%\%EXE_NAME%" (
    echo [ERROR] Could not find compiled binary: %BUILD_DIR%\%EXE_NAME%
    echo Please build the project using CMake first!
    pause
    exit /b 1
)

:: 2. Create Directories
echo [1/3] Creating installation folder...
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"
if not exist "%INSTALL_DIR%\tools" mkdir "%INSTALL_DIR%\tools"
if not exist "%INSTALL_DIR%\bin" mkdir "%INSTALL_DIR%\bin"

:: 3. Copy Binary
echo [2/3] Copying executable...
copy /Y "%BUILD_DIR%\%EXE_NAME%" "%INSTALL_DIR%\bin\jack.exe" >nul
if %errorlevel% neq 0 (
    echo [ERROR] Failed to copy binary.
    pause
    exit /b 1
)

:: 4. Copy Tools
echo [3/3] Copying visualization tools...
xcopy /Y /S "%TOOLS_SOURCE%\*" "%INSTALL_DIR%\tools\" >nul

:: 5. Success
echo.
echo ==========================================
echo  SUCCESS!
echo ==========================================
echo To run 'jack' from anywhere, add this path to your Environment Variables:
echo.
echo    %INSTALL_DIR%\bin
echo.
pause