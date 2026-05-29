#!/usr/bin/env bash
set -euo pipefail

echo "====================================================="
echo "  InfinityTable — Linux Build Script"
echo "  Unreal Engine 5.3+  |  Linux x64"
echo "====================================================="

# ── Find UE5 ─────────────────────────────────────────────
UE5_PATHS=(
    "$HOME/UnrealEngine"
    "/opt/UnrealEngine"
    "/usr/local/UnrealEngine"
)

UE5_ROOT=""
for P in "${UE5_PATHS[@]}"; do
    if [ -f "$P/Engine/Build/BatchFiles/Linux/Build.sh" ]; then
        UE5_ROOT="$P"
        break
    fi
done

if [ -z "$UE5_ROOT" ]; then
    echo "ERROR: UE5 not found. Set UE5_ROOT manually."
    echo "  export UE5_ROOT=/path/to/UnrealEngine"
    exit 1
fi

echo "[OK] UE5 found at: $UE5_ROOT"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT="$SCRIPT_DIR/InfinityTable.uproject"

# ── Generate project files ────────────────────────────────
echo ""
echo "[1/3] Generating Makefiles..."
"$UE5_ROOT/GenerateProjectFiles.sh" \
    -project="$PROJECT" \
    -game -engine

echo "[OK] Makefiles generated."

# ── Build ─────────────────────────────────────────────────
echo ""
echo "[2/3] Building InfinityTable (Development)..."
"$UE5_ROOT/Engine/Build/BatchFiles/Linux/Build.sh" \
    InfinityTable \
    Linux \
    Development \
    -Project="$PROJECT" \
    -WaitMutex

echo "[OK] Client build complete."

# ── Build Server ──────────────────────────────────────────
echo ""
echo "[3/3] Building Dedicated Server..."
"$UE5_ROOT/Engine/Build/BatchFiles/Linux/Build.sh" \
    InfinityTableServer \
    Linux \
    Development \
    -Project="$PROJECT" \
    -WaitMutex

echo ""
echo "====================================================="
echo "  Build complete!"
echo "  Binaries: Binaries/Linux/"
echo "  Run server: ./InfinityTableServer /Game/Maps/TableMap -server -port=7777 -log"
echo "====================================================="
