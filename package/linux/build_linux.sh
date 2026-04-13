#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$SCRIPT_DIR/../.."
BINARY="$ROOT/build/conr"
STAGE="$ROOT/dist/linux"
TARBALL="$ROOT/dist/CastleOfNoReturn-linux-x86_64.tar.gz"

echo "==> Cleaning staging directory..."
rm -rf "$STAGE"
mkdir -p "$STAGE/lib"

echo "==> Copying binary..."
cp "$BINARY" "$STAGE/conr"

echo "==> Copying assets..."
cp -r "$ROOT/assets" "$STAGE/assets"

echo "==> Copying SDL2 shared libraries..."
for LIB in libSDL2-2.0.so.0 libSDL2_ttf-2.0.so.0 libSDL2_mixer-2.0.so.0; do
    SRC="$(ldconfig -p | awk -v lib="$LIB" '$1 == lib { print $NF; exit }')"
    if [ -z "$SRC" ]; then
        echo "ERROR: could not locate $LIB via ldconfig" >&2
        exit 1
    fi
    cp "$SRC" "$STAGE/lib/$LIB"
done

echo "==> Writing launcher script..."
cat > "$STAGE/run.sh" <<'EOF'
#!/bin/bash
DIR="$(cd "$(dirname "$0")" && pwd)"
export LD_LIBRARY_PATH="$DIR/lib:$LD_LIBRARY_PATH"
exec "$DIR/conr"
EOF
chmod +x "$STAGE/run.sh"

echo "==> Creating tarball..."
tar -czf "$TARBALL" -C "$ROOT/dist" linux

echo "==> Done: $TARBALL"
