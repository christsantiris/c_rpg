#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$SCRIPT_DIR/../.."
STAGE="$ROOT/dist/linux"
TARBALL="$ROOT/dist/CastleOfNoReturn-linux-x86_64.tar.gz"

echo "==> Cleaning staging directory..."
rm -rf "$STAGE"
mkdir -p "$STAGE/lib"

if [ "$(uname)" = "Darwin" ]; then
    if ! command -v docker &>/dev/null; then
        echo "ERROR: docker is required to build the Linux installer on macOS" >&2
        exit 1
    fi

    echo "==> Building binary and copying SDL2 libs inside Docker..."
    docker run --rm \
        -v "$ROOT:/src" \
        -w /src \
        ubuntu:22.04 bash -c "
            set -e
            apt-get update -qq
            apt-get install -y -qq cmake gcc pkg-config libsdl2-dev libsdl2-ttf-dev libsdl2-mixer-dev
            cmake -B build-linux -DCMAKE_BUILD_TYPE=Release
            cmake --build build-linux
            cp build-linux/conr dist/linux/conr
            cp /usr/lib/x86_64-linux-gnu/libSDL2-2.0.so.0       dist/linux/lib/
            cp /usr/lib/x86_64-linux-gnu/libSDL2_ttf-2.0.so.0   dist/linux/lib/
            cp /usr/lib/x86_64-linux-gnu/libSDL2_mixer-2.0.so.0 dist/linux/lib/
        "

    echo "==> Converting icon to PNG..."
    sips -s format png "$ROOT/package/AppIcon.icns" --out "$STAGE/AppIcon.png" --resampleWidth 256 >/dev/null
else
    echo "==> Building binary..."
    cmake -B "$ROOT/build" -DCMAKE_BUILD_TYPE=Release
    cmake --build "$ROOT/build"

    echo "==> Copying binary..."
    cp "$ROOT/build/conr" "$STAGE/conr"

    echo "==> Copying SDL2 shared libraries..."
    for LIB in libSDL2-2.0.so.0 libSDL2_ttf-2.0.so.0 libSDL2_mixer-2.0.so.0; do
        SRC="$(ldconfig -p | awk -v lib="$LIB" '$1 == lib { print $NF; exit }')"
        if [ -z "$SRC" ]; then
            echo "ERROR: could not locate $LIB via ldconfig" >&2
            exit 1
        fi
        cp "$SRC" "$STAGE/lib/$LIB"
    done

    echo "==> Converting icon to PNG..."
    if command -v convert &>/dev/null; then
        convert "$ROOT/package/AppIcon.icns[0]" -resize 256x256 "$STAGE/AppIcon.png"
    else
        echo "WARNING: ImageMagick 'convert' not found — AppIcon.png will be missing" >&2
    fi
fi

echo "==> Copying assets..."
cp -r "$ROOT/assets" "$STAGE/assets"

echo "==> Writing launcher script..."
cat > "$STAGE/run.sh" <<'EOF'
#!/bin/bash
DIR="$(cd "$(dirname "$0")" && pwd)"
export LD_LIBRARY_PATH="$DIR/lib:$LD_LIBRARY_PATH"
exec "$DIR/conr"
EOF
chmod +x "$STAGE/run.sh"

echo "==> Writing install script..."
cat > "$STAGE/install.sh" <<'EOF'
#!/bin/bash
set -e

GAME_DIR="$(cd "$(dirname "$0")" && pwd)"
INSTALL_DIR="$HOME/.local/share/castleofnoreturn"
APPS_DIR="$HOME/.local/share/applications"
ICONS_DIR="$HOME/.local/share/icons"

echo "==> Installing to $INSTALL_DIR..."
mkdir -p "$INSTALL_DIR"
cp -r "$GAME_DIR"/. "$INSTALL_DIR/"
chmod +x "$INSTALL_DIR/run.sh"

echo "==> Installing icon..."
mkdir -p "$ICONS_DIR"
cp "$INSTALL_DIR/AppIcon.png" "$ICONS_DIR/castleofnoreturn.png"

echo "==> Creating desktop entry..."
mkdir -p "$APPS_DIR"
cat > "$APPS_DIR/castleofnoreturn.desktop" <<DESKTOP
[Desktop Entry]
Name=Castle of No Return
Exec=$INSTALL_DIR/run.sh
Icon=$ICONS_DIR/castleofnoreturn.png
Type=Application
Categories=Game;
Terminal=false
DESKTOP

echo "==> Done. Launch from your app menu or double-click the desktop entry."
EOF
chmod +x "$STAGE/install.sh"

echo "==> Creating tarball..."
tar -czf "$TARBALL" -C "$ROOT/dist" linux

echo "==> Done: $TARBALL"
