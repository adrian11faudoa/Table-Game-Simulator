@echo off
setlocal EnableDelayedExpansion

echo =====================================================
echo   InfinityTable — Build Setup Script
echo   Unreal Engine 5.3+  ^|  Windows x64
echo =====================================================
echo.

:: ── Check UE5 ─────────────────────────────────────────
set "UE5_ROOT="
for %%D in (
    "C:\Program Files\Epic Games\UE_5.3"
    "C:\Program Files\Epic Games\UE_5.4"
    "D:\Epic Games\UE_5.3"
    "D:\UE_5.3"
) do (
    if exist "%%~D\Engine\Build\BatchFiles\Build.bat" (
        set "UE5_ROOT=%%~D"
        goto :found_ue5
    )
)
echo ERROR: Unreal Engine 5.3+ not found.
echo Please install via the Epic Games Launcher.
pause & exit /b 1
:found_ue5
echo [OK] Found Unreal Engine at: %UE5_ROOT%

:: ── Check VS2022 ──────────────────────────────────────
where cl.exe >nul 2>&1
if !errorlevel! neq 0 (
    echo WARNING: cl.exe not found. Ensure Visual Studio 2022 with
    echo          "Game Development with C++" is installed.
)

:: ── Generate project files ────────────────────────────
echo.
echo [1/4] Generating Visual Studio project files...
"%UE5_ROOT%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" ^
    -projectfiles ^
    -project="%~dp0InfinityTable.uproject" ^
    -game -rocket -progress
if !errorlevel! neq 0 (
    echo ERROR: Project file generation failed.
    pause & exit /b 1
)
echo [OK] Project files generated.

:: ── Build Development Editor ──────────────────────────
echo.
echo [2/4] Building Development Editor...
"%UE5_ROOT%\Engine\Build\BatchFiles\Build.bat" ^
    InfinityTableEditor ^
    Win64 ^
    Development ^
    -Project="%~dp0InfinityTable.uproject" ^
    -WaitMutex -FromMsBuild
if !errorlevel! neq 0 (
    echo ERROR: Editor build failed. Check Output window.
    pause & exit /b 1
)
echo [OK] Editor built.

:: ── Build Game Client ─────────────────────────────────
echo.
echo [3/4] Building Game Client (Development)...
"%UE5_ROOT%\Engine\Build\BatchFiles\Build.bat" ^
    InfinityTable ^
    Win64 ^
    Development ^
    -Project="%~dp0InfinityTable.uproject" ^
    -WaitMutex -FromMsBuild
if !errorlevel! neq 0 (
    echo ERROR: Client build failed.
    pause & exit /b 1
)
echo [OK] Client built.

:: ── Open Solution ─────────────────────────────────────
echo.
echo [4/4] Opening Visual Studio solution...
start "" "InfinityTable.sln"

echo.
echo =====================================================
echo   Build complete! 
echo   Open InfinityTable.sln to continue development.
echo   Press F5 in VS to launch with the UE5 Editor.
echo =====================================================
pause
