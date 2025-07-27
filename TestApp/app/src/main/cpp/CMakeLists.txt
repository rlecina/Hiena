# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

cmake_minimum_required(VERSION 3.22.1)

project("testapp")

# Creates your game shared library. The name must be the same as the
# one used for loading in your Kotlin/Java or AndroidManifest.txt files.
add_library(testapp SHARED
        main.cpp
        AndroidOut.cpp
        MainActivity.cpp
        TestClass.cpp)

# Searches for a package provided by the game activity dependency
find_package(game-activity REQUIRED CONFIG)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../.. Hiena)

set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -u Java_com_google_androidgamesdk_GameActivity_initializeNativeCode")

# Configure libraries CMake uses to link your target library.
target_link_libraries(testapp
        # The game activity
        game-activity::game-activity_static

        # EGL and other dependent libraries required for drawing
        # and interacting with Android system
        android
        log
        Hiena)