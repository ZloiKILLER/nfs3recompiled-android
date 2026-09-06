cmake_minimum_required(VERSION 3.15)
project(nfs3hp_android)

# The repository only ships prebuilt SDL3 binaries for Windows (sdl3/), so on
# Android SDL3 is built from source.  This must configure before the root
# project below: the root CMakeLists.txt only calls find_package(SDL3) when
# the SDL3::SDL3 target does not already exist, and add_subdirectory() here is
# what makes it exist first.
add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/SDL"
    "${CMAKE_CURRENT_BINARY_DIR}/SDL")

# The project's own CMakeLists.txt (shared with the Windows/desktop build) is
# three levels up from android/app/jni/.  It builds nfs_core plus a SHARED
# "main" library when it detects ANDROID, which is exactly the libmain.so
# SDLActivity.java expects.
add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../../.."
    "${CMAKE_CURRENT_BINARY_DIR}/nfs_recompiled")
