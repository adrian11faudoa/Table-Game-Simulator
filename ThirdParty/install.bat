@echo off
setlocal enabledelayedexpansion
echo =====================================================
echo   InfinityTable ThirdParty Dependency Installer
echo =====================================================

set "ROOT=%~dp0"
set "LUA_DIR=%ROOT%lua54"
set "SOL2_DIR=%ROOT%sol2"
set "SQLITE_DIR=%ROOT%sqlite3"
set "PLUGINS_DIR=%ROOT%..\Plugins"

echo.
echo This script does not redistribute third-party binaries directly
echo (licensing/size). It scaffolds the expected folder structure and
echo opens the download pages so you can place the files manually.
echo.

if not exist "%LUA_DIR%\include" mkdir "%LUA_DIR%\include"
if not exist "%LUA_DIR%\lib"     mkdir "%LUA_DIR%\lib"
if not exist "%SOL2_DIR%\include" mkdir "%SOL2_DIR%\include"
if not exist "%SQLITE_DIR%"      mkdir "%SQLITE_DIR%"

echo [1/4] Lua 5.4
echo   Download the Windows binaries from:
echo   https://luabinaries.sourceforge.net
echo   Place lua.h, luaconf.h, lualib.h, lauxlib.h into:
echo     %LUA_DIR%\include\
echo   Place lua54.lib into:
echo     %LUA_DIR%\lib\
start "" "https://luabinaries.sourceforge.net"

echo [2/4] sol2 (header-only)
echo   Clone or download from:
echo   https://github.com/ThePhD/sol2
echo   Copy the single_include/sol/sol.hpp (or full include/sol/) into:
echo     %SOL2_DIR%\include\sol\
start "" "https://github.com/ThePhD/sol2"

echo [3/4] SQLite3 amalgamation
echo   Download the "amalgamation" zip from:
echo   https://sqlite.org/download.html
echo   Place sqlite3.c and sqlite3.h into:
echo     %SQLITE_DIR%\
start "" "https://sqlite.org/download.html"

echo [4/4] glTFRuntime plugin
if not exist "%PLUGINS_DIR%" mkdir "%PLUGINS_DIR%"
echo   Clone or download from:
echo   https://github.com/rdeioris/glTFRuntime
echo   Place the full plugin folder (containing glTFRuntime.uplugin) into:
echo     %PLUGINS_DIR%\glTFRuntime\
echo   This plugin is required because the .uproject enables it by default;
echo   if you don't need glTF asset import, you can instead remove the
echo   glTFRuntime entry from InfinityTable.uproject's Plugins list.
start "" "https://github.com/rdeioris/glTFRuntime"

echo.
echo Once all four are in place, run setup_and_build.bat from the
echo project root to generate project files and build.
echo.
pause
