#!/bin/sh

SHADER_INPUT_FILE_NAME="$1"
OUTPUT_PATH_ROOT="$2"
SHADER_OUTPUT_FILE_BASE_NAME="$3"
METAL_VERSION=1.2
GENERIC_OPTIONS="-D MAP_DIFFUSE_ENABLED=1" 
#"-I ../../../../finjin-engine/cpp/library/src/finjin/engine/internal/gpu/metal"
COMPILER_PLATFORMS_PATH_ROOT="/Applications/Xcode.app/Contents/Developer/Platforms"


#macOS
COMPILER_PATH="xcrun -sdk macosx metal"
AR_COMPILER_PATH="$COMPILER_PATH-ar"
METALLIB_COMPILER_PATH="$COMPILER_PATH"lib
COMPILER_TARGET_PLATFORM_ID=osx
OUTPUT_TARGET_PLATFORM_ID=macos

OUTPUT_PATH="$OUTPUT_PATH_ROOT/shaders-$OUTPUT_TARGET_PLATFORM_ID-metal"
mkdir -p "$OUTPUT_PATH"
$COMPILER_PATH -std=$COMPILER_TARGET_PLATFORM_ID-metal$METAL_VERSION $GENERIC_OPTIONS -o "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.air" "$SHADER_INPUT_FILE_NAME"
$AR_COMPILER_PATH rcs "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.metalar" "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.air"
rm "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.metallib"
$METALLIB_COMPILER_PATH -o "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.metallib" "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.metalar"
rm "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.air"
rm "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.metalar"


#iOS
COMPILER_PATH="xcrun -sdk iphoneos metal"
AR_COMPILER_PATH="$COMPILER_PATH-ar"
METALLIB_COMPILER_PATH="$COMPILER_PATH"lib
COMPILER_TARGET_PLATFORM_ID=ios
OUTPUT_TARGET_PLATFORM_ID=ios

OUTPUT_PATH="$OUTPUT_PATH_ROOT/shaders-$OUTPUT_TARGET_PLATFORM_ID-metal"
mkdir -p "$OUTPUT_PATH"
$COMPILER_PATH -std=$COMPILER_TARGET_PLATFORM_ID-metal$METAL_VERSION $GENERIC_OPTIONS -o "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.air" "$SHADER_INPUT_FILE_NAME"
$AR_COMPILER_PATH rcs "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.metalar" "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.air"
rm "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.metallib"
$METALLIB_COMPILER_PATH -o "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.metallib" "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.metalar"
rm "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.air"
rm "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.metalar"


#tvOS
COMPILER_PATH="xcrun -sdk appletvos metal"
AR_COMPILER_PATH="$COMPILER_PATH-ar"
METALLIB_COMPILER_PATH="$COMPILER_PATH"lib
COMPILER_TARGET_PLATFORM_ID=ios
OUTPUT_TARGET_PLATFORM_ID=tvos

OUTPUT_PATH="$OUTPUT_PATH_ROOT/shaders-$OUTPUT_TARGET_PLATFORM_ID-metal"
mkdir -p "$OUTPUT_PATH"
$COMPILER_PATH -std=$COMPILER_TARGET_PLATFORM_ID-metal$METAL_VERSION $GENERIC_OPTIONS -o "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.air" "$SHADER_INPUT_FILE_NAME"
$AR_COMPILER_PATH rcs "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.metalar" "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.air"
rm "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.metallib"
$METALLIB_COMPILER_PATH -o "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.metallib" "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.metalar"
rm "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.air"
rm "$OUTPUT_PATH/$SHADER_OUTPUT_FILE_BASE_NAME.metalar"
