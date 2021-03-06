#!/usr/bin/env bash

set -e
set -o pipefail

# Track individual architectures
scripts/log_binary_size.sh "platform/android/MapboxGLAndroidSDK/build/intermediates/intermediate-jars/release/jni/armeabi-v7a/libmapbox-gl.so" "Platform=Android,Arch=arm-v7"
scripts/log_binary_size.sh "platform/android/MapboxGLAndroidSDK/build/intermediates/intermediate-jars/release/jni/arm64-v8a/libmapbox-gl.so"   "Platform=Android,Arch=arm-v8"
scripts/log_binary_size.sh "platform/android/MapboxGLAndroidSDK/build/intermediates/intermediate-jars/release/jni/x86/libmapbox-gl.so"         "Platform=Android,Arch=x86"
scripts/log_binary_size.sh "platform/android/MapboxGLAndroidSDK/build/intermediates/intermediate-jars/release/jni/x86_64/libmapbox-gl.so"      "Platform=Android,Arch=x86_64"

# Track overall library size
scripts/log_binary_size.sh "platform/android/MapboxGLAndroidSDK/build/outputs/aar/MapboxGLAndroidSDK-release.aar"                    "Platform=Android,Arch=Archive"
