#!/usr/bin/env bash
set -euo pipefail
echo "=== InfinityTable Linux Build ==="

UE5_ROOT="${UE5_ROOT:-$HOME/UnrealEngine}"
if [ ! -f "$UE5_ROOT/Engine/Build/BatchFiles/Linux/Build.sh" ]; then
    echo "ERROR: Set UE5_ROOT to your UE5 install path."
    exit 1
fi

PROJECT="$(cd "$(dirname "$0")" && pwd)/InfinityTable.uproject"

echo "[1/2] Generating Makefiles..."
"$UE5_ROOT/GenerateProjectFiles.sh" -project="$PROJECT" -game -engine

echo "[2/2] Building..."
"$UE5_ROOT/Engine/Build/BatchFiles/Linux/Build.sh" InfinityTable Linux Development -Project="$PROJECT"

echo "Done! Run: ./Binaries/Linux/InfinityTable"
