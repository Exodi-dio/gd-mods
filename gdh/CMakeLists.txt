cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
    # set(CMAKE_OSX_ARCHITECTURES "x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(GDH VERSION 1.0.0)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE IMGUI_DEFINE_MATH_OPERATORS)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

target_include_directories(${PROJECT_NAME} PRIVATE libs libs/imgui)

set(HAS_IMGUI ON)
add_subdirectory(libs/imgui)
CPMAddPackage("gh:prevter/gd-imgui-cocos#6383c75")

if (WIN32)
    add_subdirectory(libs/discord-rpc)
    target_link_libraries(${PROJECT_NAME} discord-rpc)
endif()

target_link_libraries(${PROJECT_NAME} imgui-cocos imgui)

if (ANDROID AND ANDROID_ABI STREQUAL "arm64-v8a") 
    target_link_options(${PROJECT_NAME} PRIVATE "-Wl,-Bsymbolic")
    target_include_directories(${PROJECT_NAME} PRIVATE libs/ffmpeg/arm64-v8a)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libs/ffmpeg/arm64-v8a/libavcodec/libavcodec.a)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libs/ffmpeg/arm64-v8a/libavdevice/libavdevice.a)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libs/ffmpeg/arm64-v8a/libavfilter/libavfilter.a)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libs/ffmpeg/arm64-v8a/libavformat/libavformat.a)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libs/ffmpeg/arm64-v8a/libavutil/libavutil.a)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libs/ffmpeg/arm64-v8a/libswresample/libswresample.a)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libs/ffmpeg/arm64-v8a/libswscale/libswscale.a)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libs/ffmpeg/arm64-v8a/x264/libx264.a)
endif()

if (ANDROID AND ANDROID_ABI STREQUAL "armeabi-v7a") 
    target_link_options(${PROJECT_NAME} PRIVATE "-Wl,-Bsymbolic")
    target_include_directories(${PROJECT_NAME} PRIVATE libs/ffmpeg/armeabi-v7a)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libs/ffmpeg/armeabi-v7a/libavcodec/libavcodec.a)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libs/ffmpeg/armeabi-v7a/libavdevice/libavdevice.a)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libs/ffmpeg/armeabi-v7a/libavfilter/libavfilter.a)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libs/ffmpeg/armeabi-v7a/libavformat/libavformat.a)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libs/ffmpeg/armeabi-v7a/libavutil/libavutil.a)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libs/ffmpeg/armeabi-v7a/libswresample/libswresample.a)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libs/ffmpeg/armeabi-v7a/libswscale/libswscale.a)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libs/ffmpeg/armeabi-v7a/x264/libx264.a)
endif()

if (APPLE)
    target_include_directories(${PROJECT_NAME} PRIVATE libs/ffmpeg/mac/include)

    target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libs/ffmpeg/mac/libffmpeg_all.a)

    target_link_libraries(${PROJECT_NAME}
        "-framework CoreVideo"
        "-framework CoreMedia"
        "-framework CoreFoundation"
        "-framework AVFoundation"
        "-framework AudioToolbox"
        "-framework Security"
        z
        iconv
        bz2
    )
endif()

setup_geode_mod(${PROJECT_NAME})

if(DEFINED ENV{CI})
    set_target_properties(
        ${PROJECT_NAME} PROPERTIES
        UNITY_BUILD ON
        UNITY_BUILD_BATCH_SIZE 45
        UNITY_BUILD_UNIQUE_ID "GEODE_UNITY_NS_ID"
    )

    set_target_properties(GeodeBindings PROPERTIES
        UNITY_BUILD_BATCH_SIZE 300
    )
endif()