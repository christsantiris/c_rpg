#!/bin/bash
set -e

APP_NAME="Castle of No Return"
APP_DIR="dist/${APP_NAME}.app"
DMG_NAME="CastleOfNoReturn.dmg"
BINARY="build/conr"

SDL2_LIB="/usr/local/opt/sdl2/lib/libSDL2-2.0.0.dylib"
TTF_LIB="/usr/local/opt/sdl2_ttf/lib/libSDL2_ttf-2.0.0.dylib"
MIXER_LIB="/usr/local/opt/sdl2_mixer/lib/libSDL2_mixer-2.0.0.dylib"

echo "==> Cleaning dist..."
rm -rf dist
mkdir -p "${APP_DIR}/Contents/MacOS"
mkdir -p "${APP_DIR}/Contents/Frameworks"
mkdir -p "${APP_DIR}/Contents/Resources"

echo "==> Copying binary..."
cp "${BINARY}" "${APP_DIR}/Contents/MacOS/conr"

echo "==> Copying assets..."
cp -r assets "${APP_DIR}/Contents/Resources/assets"
cp -r saves  "${APP_DIR}/Contents/Resources/saves" 2>/dev/null || mkdir -p "${APP_DIR}/Contents/Resources/saves"

echo "==> Copying Info.plist..."
cp package/macos/Info.plist "${APP_DIR}/Contents/Info.plist"

echo "==> Copying icon..."
cp package/macos/AppIcon.icns "${APP_DIR}/Contents/Resources/AppIcon.icns"

echo "==> Copying dylibs..."
cp "${SDL2_LIB}"  "${APP_DIR}/Contents/Frameworks/libSDL2-2.0.0.dylib"
cp "${TTF_LIB}"   "${APP_DIR}/Contents/Frameworks/libSDL2_ttf-2.0.0.dylib"
cp "${MIXER_LIB}" "${APP_DIR}/Contents/Frameworks/libSDL2_mixer-2.0.0.dylib"

echo "==> Fixing dylib paths in binary..."
install_name_tool -change \
    "/usr/local/opt/sdl2/lib/libSDL2-2.0.0.dylib" \
    "@executable_path/../Frameworks/libSDL2-2.0.0.dylib" \
    "${APP_DIR}/Contents/MacOS/conr"

install_name_tool -change \
    "/usr/local/opt/sdl2_ttf/lib/libSDL2_ttf-2.0.0.dylib" \
    "@executable_path/../Frameworks/libSDL2_ttf-2.0.0.dylib" \
    "${APP_DIR}/Contents/MacOS/conr"

install_name_tool -change \
    "/usr/local/opt/sdl2_mixer/lib/libSDL2_mixer-2.0.0.dylib" \
    "@executable_path/../Frameworks/libSDL2_mixer-2.0.0.dylib" \
    "${APP_DIR}/Contents/MacOS/conr"

echo "==> Fixing dylib internal paths..."
install_name_tool -id \
    "@executable_path/../Frameworks/libSDL2-2.0.0.dylib" \
    "${APP_DIR}/Contents/Frameworks/libSDL2-2.0.0.dylib"

install_name_tool -id \
    "@executable_path/../Frameworks/libSDL2_ttf-2.0.0.dylib" \
    "${APP_DIR}/Contents/Frameworks/libSDL2_ttf-2.0.0.dylib"

install_name_tool -id \
    "@executable_path/../Frameworks/libSDL2_mixer-2.0.0.dylib" \
    "${APP_DIR}/Contents/Frameworks/libSDL2_mixer-2.0.0.dylib"

echo "==> Setting DMG volume icon..."
cp package/macos/AppIcon.icns dist/.VolumeIcon.icns
SetFile -a C dist

echo "==> Creating DMG..."
hdiutil create \
    -volname "${APP_NAME}" \
    -srcfolder dist \
    -ov \
    -format UDZO \
    "dist/${DMG_NAME}"

echo "==> Done: dist/${DMG_NAME}"