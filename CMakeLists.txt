cmake_minimum_required(VERSION 3.25)
project(CaptureTheFlag VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
raylib
DOWNLOAD_EXTRACT_TIMESTAMP OFF
URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
)
FetchContent_GetProperties(raylib)

FetchContent_Declare(
    enet
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/lsalzman/enet/archive/refs/tags/v1.3.18.tar.gz
)
FetchContent_MakeAvailable(enet)

target_include_directories(enet PUBLIC
    ${enet_SOURCE_DIR}/include
)

if (NOT raylib_POPULATED) 
set(FETCHCONTENT_QUIET NO)
FetchContent_Populate(raylib)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
endif()

if (APPLE) 

elseif (UNIX AND NOT APPLE)
    
elseif (WIN32)
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
endif()

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp")

if (WIN32)
add_executable(${PROJECT_NAME}
    WIN32
    "${SRC}" 
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/res"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/res"
)

elseif(APPLE)
add_executable(${PROJECT_NAME}
    MACOSX_BUNDLE
    ${SRC})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/res
    $<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/res
)

else() # Linux
add_executable(${PROJECT_NAME}
    ${SRC})
  
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE raylib enet)

if (APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL")
elseif (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 winmm)

    target_compile_definitions(${PROJECT_NAME} PRIVATE "_CRT_SECURE_NO_WARNINGS")
    target_compile_options(${PROJECT_NAME} PRIVATE "/W4")
    target_link_options(${PROJECT_NAME} PRIVATE "/SUBSYSTEM:Windows;/ENTRY:mainCRTStartup")
endif()
