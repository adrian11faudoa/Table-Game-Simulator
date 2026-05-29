@echo off
echo Installing InfinityTable Third-Party Dependencies...
echo.

:: ── Create directories ────────────────────────────────
mkdir ThirdParty\lua54\include 2>nul
mkdir ThirdParty\lua54\lib     2>nul
mkdir ThirdParty\sol2\include  2>nul
mkdir ThirdParty\sqlite3       2>nul

echo [INFO] Please download the following manually:
echo.
echo  1. Lua 5.4 Windows binaries:
echo     https://luabinaries.sourceforge.net/
echo     → Extract lua54.lib to ThirdParty\lua54\lib\
echo     → Extract lua.h, lualib.h, lauxlib.h to ThirdParty\lua54\include\
echo.
echo  2. sol2 (header-only, C++17):
echo     https://github.com/ThePhD/sol2/releases
echo     → Copy include/sol/ folder to ThirdParty\sol2\include\sol\
echo.
echo  3. SQLite3 amalgamation:
echo     https://www.sqlite.org/download.html
echo     → Download sqlite-amalgamation-*.zip
echo     → Copy sqlite3.h and sqlite3.c to ThirdParty\sqlite3\
echo.
echo  4. glTFRuntime UE5 Plugin:
echo     https://github.com/rdeioris/glTFRuntime
echo     → Clone into Plugins\glTFRuntime\
echo.

echo After downloading all dependencies, run setup_and_build.bat
pause
