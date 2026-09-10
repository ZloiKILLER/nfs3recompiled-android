cmake_minimum_required(VERSION 3.15)
project(nfs_recompiled LANGUAGES C CXX ASM)

# CMAKE_SOURCE_DIR is the *top-level* project's source dir, not this file's
# own directory.  This file doubles as a standalone project (Windows/Linux
# desktop builds) and as a subproject add_subdirectory()'d from
# android/app/jni/CMakeLists.txt; in the latter case CMAKE_SOURCE_DIR would
# incorrectly point at android/app/jni.  CMAKE_CURRENT_SOURCE_DIR is correct
# in both cases at the point this line runs.
set(NFS_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# The project uses C++17 features ([[fallthrough]], etc.) but never pinned the
# standard, so MSVC silently fell back to C++14.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# options
option(WITH_PEDANTIC_FPU "Compile pedantic FPU implementation" OFF)
option(WITH_MMX "Enable MMX support" ON)
option(NFS_TRACE_MSG "Trace the win32 message pump to the log" OFF)
option(NFS2_ASSERT_TRAP "Make NFS2_ASSERT a hard trap (debugger break) instead of logging and continuing" OFF)

# add compile definitions based on options
if(WITH_PEDANTIC_FPU)
    add_compile_definitions(WITH_PEDANTIC_FPU)
endif()
if(WITH_MMX)
    add_compile_definitions(WITH_MMX)
endif()
if(NFS2_ASSERT_TRAP)
    add_compile_definitions(NFS2_ASSERT_TRAP)
endif()

# Detect Windows target: covers native Windows (WIN32/MINGW), an explicit
# -DCMAKE_SYSTEM_NAME=Windows, and cross-compilation via a MinGW toolchain
# supplied directly with -DCMAKE_C_COMPILER=…/x86_64-w64-mingw32-gcc (in
# which case WIN32/MINGW may not yet be set if no toolchain file is used).
if(WIN32 OR MINGW OR CMAKE_SYSTEM_NAME STREQUAL "Windows"
        OR CMAKE_C_COMPILER MATCHES "mingw"
        OR CMAKE_CXX_COMPILER MATCHES "mingw")
    set(TARGET_WINDOWS TRUE)
else()
    set(TARGET_WINDOWS FALSE)
endif()

# Warning policy is per target, not global:
#   nfs_core  -- hand-written runtime, warnings are errors.
#   nfs3hp -- ~85 MB of machine-generated disassembly that no human
#     edits and the generator re-emits from scratch; it trips thousands of
#     truncation/conversion warnings by construction, so -Werror there would
#     only mean nobody can build.  Warnings stay on, they just do not fail.
# -fno-strict-aliasing is global and mandatory: the generated code reaches guest
# memory through reinterpret_cast of a flat byte array.
if (MSVC)
    add_compile_options(/W4)
    # C4201 unnamed unions and C4458 shadowing are inherent to the x86::CPU
    # register layout in include/cpu.h; suppress rather than reshape the struct.
    #   C4127 constant conditional -- inherent to the NFS2_ASSERT macro.
    #   C4702 unreachable code -- gliderenderer.cpp texture upload uses a
    #     for-loop with an unconditional break where the mipmap chain was never
    #     finished; that function is rewritten for GLES, so drop this then.
    #   C4245 signed/unsigned in return -- the win32 layer returns -1 as the
    #     documented Win32 sentinel from functions with unsigned return types.
    set(NFS_STRICT_FLAGS /EHsc /WX /wd4201 /wd4458 /wd4127 /wd4702 /wd4245)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -fno-strict-aliasing)
    set(NFS_STRICT_FLAGS -Werror)
endif()

# When building for Windows we have a local copy of SDL3 under the
# `sdl3` subdirectory.  Point CMake at it so find_package() will
# locate the bundled config file automatically.
if(TARGET_WINDOWS)
    message(STATUS "Windows target detected – adding ${NFS_ROOT_DIR}/sdl3/cmake to CMAKE_PREFIX_PATH for SDL3")
    list(APPEND CMAKE_PREFIX_PATH "${NFS_ROOT_DIR}/sdl3/cmake")
    set(SDL3_DIR "${NFS_ROOT_DIR}/sdl3/cmake" CACHE PATH "SDL3 CMake directory")
endif()

# sse2neon is vendored; include it as SYSTEM so its warnings do not surface in
# the ~100 translation units that pull in mmx.h.
include_directories(SYSTEM ${NFS_ROOT_DIR}/third_party)

# The repository only ships prebuilt SDL3 for Windows; on Android
# android/app/jni/CMakeLists.txt add_subdirectory()s SDL3 from source (cloned
# into third_party/SDL) before this file, so SDL3::SDL3 already exists by the
# time we get here.  find_package() would try to redefine that target and
# fail with "target already exists" -- only call it when nothing did this yet.
if(NOT TARGET SDL3::SDL3)
    find_package(SDL3 REQUIRED)
endif()

# Android has no desktop OpenGL: link GLESv3 directly instead of going through
# find_package(OpenGL), which has no Android-CMake config to find anyway.
if(ANDROID)
    set(NFS_GL_LIBS GLESv3)
else()
    find_package(OpenGL REQUIRED)
    set(NFS_GL_LIBS OpenGL::GL)
endif()

# prefer modern imported targets if available
if(TARGET SDL3::SDL3)
    set(SDL3_LIB_TARGET SDL3::SDL3)
else()
    include_directories(${SDL3_INCLUDE_DIRS})
    set(SDL3_LIB_TARGET ${SDL3_LIBRARIES})
endif()

# SDL3main provides the WinMain→main entry-point shim required on Windows.
# The modern config exposes it as a separate target; the legacy FindSDL3.cmake
# already folds it into SDL3_LIBRARIES, so we only add it on top for the
# imported-target path.
if(TARGET SDL3::SDL3main)
    set(SDL3_MAIN_TARGET SDL3::SDL3main)
else()
    set(SDL3_MAIN_TARGET "")
endif()

file(GLOB NFS3HP_DISASM_SOURCES
     "${NFS_ROOT_DIR}/src/nfs3hp/disassembly/*.cpp")
file(GLOB NFS3HP_DISASM_HEADERS
     "${NFS_ROOT_DIR}/src/nfs3hp/disassembly/*.h")

# ---------------------------------------------------------------------------
# core static library
# ---------------------------------------------------------------------------

file(GLOB LIB_CPP_SOURCES CONFIGURE_DEPENDS
     "${NFS_ROOT_DIR}/src/lib/*.cpp"
     "${NFS_ROOT_DIR}/src/lib/sdl-backend/*.cpp"
     "${NFS_ROOT_DIR}/src/lib/winapi/*.cpp"
     "${NFS_ROOT_DIR}/src/lib/winapi/ddraw/*.cpp"
     "${NFS_ROOT_DIR}/src/lib/winapi/dinput/*.cpp"
     "${NFS_ROOT_DIR}/src/lib/winapi/dsound/*.cpp")

# x87.asm provides 80-bit extended-precision FPU helpers and is only needed
# (and only supported) on Linux with WITH_PEDANTIC_FPU=ON.
if(WITH_PEDANTIC_FPU AND TARGET_WINDOWS)
    message(FATAL_ERROR
        "WITH_PEDANTIC_FPU is not supported on Windows: the x87 NASM helper "
        "relies on Linux-specific 80-bit long double ABI conventions.")
endif()

if(WITH_PEDANTIC_FPU AND NOT TARGET_WINDOWS)
    set(X87_OBJ "${CMAKE_CURRENT_BINARY_DIR}/x87.o")
    add_custom_command(
        OUTPUT ${X87_OBJ}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND nasm -f elf64 -o ${X87_OBJ} ${NFS_ROOT_DIR}/src/lib/x87.asm
        DEPENDS ${NFS_ROOT_DIR}/src/lib/x87.asm
        COMMENT "Assembling x87.asm (elf64, pedantic FPU)"
        VERBATIM
    )
    list(APPEND LIB_CPP_SOURCES ${X87_OBJ})
endif()

add_library(nfs_core STATIC ${LIB_CPP_SOURCES})
target_include_directories(nfs_core PRIVATE
    ${NFS_ROOT_DIR}/include)
target_link_libraries(nfs_core PRIVATE ${SDL3_MAIN_TARGET} ${SDL3_LIB_TARGET} ${NFS_GL_LIBS})
target_compile_options(nfs_core PRIVATE ${NFS_STRICT_FLAGS})
if(NFS_TRACE_MSG)
    target_compile_definitions(nfs_core PRIVATE NFS_TRACE_MSG)
endif()

# helper function for adding the game executables
function(add_game_exe name)
    # script and directories
    set(game_src_dir "${NFS_ROOT_DIR}/src/${name}")

    # collect sources
    file(GLOB GAME_SOURCES "${game_src_dir}/*.cpp")

    # pick the hardcoded disasm set for this game
    if(name STREQUAL "nfs3hp")
        set(DISASM_SOURCES ${NFS3HP_DISASM_SOURCES})
        set(DISASM_HEADERS ${NFS3HP_DISASM_HEADERS})
    else()
        set(DISASM_SOURCES "")
        set(DISASM_HEADERS "")
    endif()

    # adding the headers to the executable source list is what makes CMake
    # aware that the custom command must run before compilation starts.
    if(ANDROID)
        # SDLActivity.java (getMainSharedObject/getLibraries) loads a fixed
        # "libmain.so" and dlsym()s SDL_main out of it; the CMake target name
        # matters only for build-tree bookkeeping, so it is renamed to "main"
        # below.
        add_library(${name} SHARED ${GAME_SOURCES} ${DISASM_SOURCES} ${DISASM_HEADERS})
        set_target_properties(${name} PROPERTIES OUTPUT_NAME "main")
        # Required for Play Store submission on a 16 KB-page kernel regardless
        # of what page size this build's host happens to use; see
        # src/lib/memmap.cpp for the matching runtime-side fix.
        target_link_options(${name} PRIVATE "-Wl,-z,max-page-size=16384")
    else()
        add_executable(${name} ${GAME_SOURCES} ${DISASM_SOURCES} ${DISASM_HEADERS})
    endif()
    set_source_files_properties(${DISASM_HEADERS} PROPERTIES HEADER_FILE_ONLY TRUE)
    target_include_directories(${name} PRIVATE
        ${game_src_dir}
        ${NFS_ROOT_DIR}/src/${name}/disassembly
        ${NFS_ROOT_DIR}/include)
    target_link_libraries(${name} PRIVATE ${SDL3_MAIN_TARGET} nfs_core ${SDL3_LIB_TARGET} ${NFS_GL_LIBS})
    if(WITH_PEDANTIC_FPU)
        target_compile_definitions(${name} PRIVATE WITH_PEDANTIC_FPU)
    endif()
    if(WITH_MMX)
        target_compile_definitions(${name} PRIVATE WITH_MMX)
    endif()
endfunction()

add_game_exe(nfs3hp)
option(NFS_BUILD_FF_TESTS "Build guest COM force-feedback regression checks" OFF)
if(NFS_BUILD_FF_TESTS AND NOT ANDROID)
    add_executable(force_feedback_checks tools/force_feedback_checks.cpp)
    target_include_directories(force_feedback_checks PRIVATE ${NFS_ROOT_DIR}/include)
    target_link_libraries(force_feedback_checks PRIVATE nfs_core ${SDL3_LIB_TARGET} ${NFS_GL_LIBS})
endif()

option(NFS_BUILD_MEMORY_TESTS "Build guest memory allocator checks" OFF)
if(NFS_BUILD_MEMORY_TESTS AND NOT ANDROID)
    add_executable(memmap_checks tools/memmap_checks.cpp)
    if(MSVC)
        target_compile_options(memmap_checks PRIVATE /EHsc)
    endif()
    target_include_directories(memmap_checks PRIVATE ${NFS_ROOT_DIR}/include)
    target_link_libraries(memmap_checks PRIVATE nfs_core ${SDL3_LIB_TARGET} ${NFS_GL_LIBS})
endif()
