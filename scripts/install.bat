@echo off
setlocal

echo ==========================================
echo  Installing Jack Compiler (Windows)
echo ==========================================

:: 1. Configuration
set "SOURCE_DIR=%~dp0"
set "INSTALL_DIR=%USERPROFILE%\.jack_toolchain"

:: 2. Create Folders
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"
if not exist "%INSTALL_DIR%\bin" mkdir "%INSTALL_DIR%\bin"
if not exist "%INSTALL_DIR%\tools" mkdir "%INSTALL_DIR%\tools"
if not exist "%INSTALL_DIR%\os" mkdir "%INSTALL_DIR%\os"

:: 3. Install Executable
if exist "%SOURCE_DIR%bin\NAND2TETRIS_win.exe" (
    copy /Y "%SOURCE_DIR%bin\NAND2TETRIS_win.exe" "%INSTALL_DIR%\bin\jack.exe" >nul
    echo [OK] Installed jack.exe
) else (
    echo [ERROR] Could not find bin\NAND2TETRIS_win.exe
    pause
    exit /b 1
)

:: 4. Install Extras (Tools + OS)
xcopy /Y /S /Q "%SOURCE_DIR%tools\*" "%INSTALL_DIR%\tools\" >nul
if exist "%SOURCE_DIR%os" (
    xcopy /Y /S /Q "%SOURCE_DIR%os\*" "%INSTALL_DIR%\os\" >nul
)

:: 5. Install Python Dependencies
echo.
echo [INFO] Checking Python environment...
python --version >nul 2>&1
if %errorlevel% equ 0 (
    echo    -> Python detected. Installing libraries...
    pip install -r "%INSTALL_DIR%\tools\requirements.txt" >nul 2>&1
    if %errorlevel% neq 0 (
        echo    [WARN] Automatic library install failed.
        echo           Please run: pip install -r "%INSTALL_DIR%\tools\requirements.txt"
    )
) else (
    echo.
    echo    [!] WARNING: Python is NOT installed.
    echo        You can compile code, but the Visualizer will not work.
    echo.
    echo        Please install Python from the Microsoft Store or:
    echo        https://www.python.org/downloads/
)

echo.
echo ==========================================
echo  SUCCESS!
echo ==========================================
echo To run 'jack', add this folder to your PATH environment variable:
echo.
echo    %INSTALL_DIR%\bin
echo.
pause