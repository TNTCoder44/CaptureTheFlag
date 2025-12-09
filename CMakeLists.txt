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
if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
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

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*")

if (WIN32)
add_executable(${PROJECT_NAME}
    WIN32
    "${SRC}" 
)

elseif(APPLE)
add_executable(${PROJECT_NAME}
    MACOSX_BUNDLE
    ${SRC})

else() # Linux
add_executable(${PROJECT_NAME}
    ${SRC})
  
endif()

target_link_libraries(${PROJECT_NAME} raylib)

if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
elseif (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE "_CRT_SECURE_NO_WARNINGS")
    target_compile_options(${PROJECT_NAME} PRIVATE "/W4")
    target_link_options(${PROJECT_NAME} PRIVATE "/SUBSYSTEM:Windows;/ENTRY:mainCRTStartup")
endif()
