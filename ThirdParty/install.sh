#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
LUA_DIR="$ROOT/lua54"
SOL2_DIR="$ROOT/sol2"
SQLITE_DIR="$ROOT/sqlite3"
PLUGINS_DIR="$ROOT/../Plugins"

echo "====================================================="
echo "  InfinityTable ThirdParty Dependency Installer"
echo "====================================================="
echo
echo "This script scaffolds the expected folder structure and fetches"
echo "what it can over the network. Some libraries (Lua binaries) are"
echo "easiest to build from source on Linux; this script builds Lua 5.4"
echo "from the official tarball if a C compiler is available."
echo

mkdir -p "$LUA_DIR/include" "$LUA_DIR/lib" "$SOL2_DIR/include" "$SQLITE_DIR"

echo "[1/4] Lua 5.4"
if command -v curl >/dev/null && command -v make >/dev/null && command -v cc >/dev/null; then
    TMP=$(mktemp -d)
    echo "  Downloading lua-5.4.6.tar.gz..."
    curl -fsSL "https://www.lua.org/ftp/lua-5.4.6.tar.gz" -o "$TMP/lua.tar.gz"
    tar -xzf "$TMP/lua.tar.gz" -C "$TMP"
    pushd "$TMP/lua-5.4.6" >/dev/null
    make linux MYCFLAGS="-fPIC" >/dev/null
    popd >/dev/null
    cp "$TMP/lua-5.4.6/src/"*.h "$LUA_DIR/include/"
    cp "$TMP/lua-5.4.6/src/liblua.a" "$LUA_DIR/lib/lua54.lib"
    rm -rf "$TMP"
    echo "  Built and installed to $LUA_DIR"
else
    echo "  curl/make/cc not all available — download manually from:"
    echo "  https://www.lua.org/ftp/lua-5.4.6.tar.gz"
    echo "  and place headers in $LUA_DIR/include, liblua.a as $LUA_DIR/lib/lua54.lib"
fi

echo "[2/4] sol2 (header-only)"
if command -v curl >/dev/null; then
    curl -fsSL "https://raw.githubusercontent.com/ThePhD/sol2/develop/single/include/sol/sol.hpp" \
        -o "$SOL2_DIR/include/sol.hpp" || echo "  Could not auto-fetch; clone https://github.com/ThePhD/sol2 manually."
else
    echo "  curl not available — clone https://github.com/ThePhD/sol2 manually into $SOL2_DIR/include/sol/"
fi

echo "[3/4] SQLite3 amalgamation"
echo "  Download the amalgamation zip from:"
echo "  https://sqlite.org/download.html"
echo "  and place sqlite3.c / sqlite3.h into $SQLITE_DIR/"

echo "[4/4] glTFRuntime plugin"
mkdir -p "$PLUGINS_DIR"
if command -v git >/dev/null; then
    if [ ! -d "$PLUGINS_DIR/glTFRuntime" ]; then
        echo "  Cloning glTFRuntime into $PLUGINS_DIR/glTFRuntime ..."
        git clone --depth 1 "https://github.com/rdeioris/glTFRuntime.git" "$PLUGINS_DIR/glTFRuntime" \
            || echo "  Clone failed — clone manually from https://github.com/rdeioris/glTFRuntime"
    else
        echo "  Already present at $PLUGINS_DIR/glTFRuntime"
    fi
else
    echo "  git not available — clone https://github.com/rdeioris/glTFRuntime manually into $PLUGINS_DIR/glTFRuntime"
fi
echo "  Required because .uproject enables this plugin by default; if you"
echo "  don't need glTF asset import, remove the glTFRuntime entry from"
echo "  InfinityTable.uproject's Plugins list instead."

echo
echo "Once all four are in place, run ./build_linux.sh from the project root."
