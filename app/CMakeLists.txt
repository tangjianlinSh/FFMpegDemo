cmake_minimum_required(VERSION 3.4.1)
file(GLOB my_source src/main/cpp/*.cpp src/main/cpp/*.c)
add_library(
        native-lib
        SHARED
        ${my_source}
)
find_library(
        log-lib
        log
)
find_library(android-lib android)

set(JNI_LIBS_DIR ${CMAKE_SOURCE_DIR}/src/main/jniLibs)

add_library(
        avutil-55
        SHARED
        IMPORTED
)
set_target_properties(
        avutil-55
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavutil-55.so
)
add_library(
        swresample-2
        SHARED
        IMPORTED
)
set_target_properties(
        swresample-2
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libswresample-2.so
)
add_library(
        avcodec-57
        SHARED
        IMPORTED
)
set_target_properties(
        avcodec-57
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavcodec-57.so
)
add_library(
        avfilter-6
        SHARED
        IMPORTED
)
set_target_properties(
        avfilter-6
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavfilter-6.so
)
add_library(
        swscale-4
        SHARED
        IMPORTED
)
set_target_properties(
        swscale-4
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libswscale-4.so
)
add_library(
        avdevice-57
        SHARED
        IMPORTED
)
set_target_properties(
        avdevice-57
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavdevice-57.so
)
add_library(
        avformat-57
        SHARED
        IMPORTED
)
set_target_properties(
        avformat-57
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavformat-57.so
)
add_library(
        postproc-54
        SHARED
        IMPORTED
)
set_target_properties(
        postproc-54
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libpostproc-54.so
)
include_directories(${JNI_LIBS_DIR}/include)
# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        native-lib
        avutil-55 #工具库(大部分需要)
        swresample-2 #音频采样数据格式转换
        avcodec-57 #编解码(重要)
        avfilter-6 #滤镜特效处理
        swscale-4 #视频像素数据格式转换
        avdevice-57 #各种设备的输入输出
        avformat-57 #封装格式处理
        postproc-54 #后加工
        OpenSLES
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        ${android-lib})