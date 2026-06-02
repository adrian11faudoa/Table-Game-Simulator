@echo off
echo =====================================================
echo   InfinityTable Build Setup
echo   Unreal Engine 5.3+ required
echo =====================================================

set "UE5_ROOT="
for %%D in ("C:\Program Files\Epic Games\UE_5.3" "C:\Program Files\Epic Games\UE_5.4" "D:\UE_5.3") do (
    if exist "%%~D\Engine\Build\BatchFiles\Build.bat" (
        set "UE5_ROOT=%%~D"
        goto found
    )
)
echo ERROR: UE5 not found. Install via Epic Games Launcher.
pause & exit /b 1
:found
echo [OK] UE5 at: %UE5_ROOT%

echo [1/3] Generating project files...
"%UE5_ROOT%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" ^
    -projectfiles -project="%~dp0InfinityTable.uproject" -game -rocket -progress

echo [2/3] Building editor...
"%UE5_ROOT%\Engine\Build\BatchFiles\Build.bat" InfinityTableEditor Win64 Development ^
    -Project="%~dp0InfinityTable.uproject" -WaitMutex

echo [3/3] Opening solution...
start "" "InfinityTable.sln"
echo Done! Press F5 in VS to launch the editor.
pause
