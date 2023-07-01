cmake_minimum_required(VERSION 3.3)
cmake_policy(SET CMP0054 NEW)
include("CMakeHelpers.cmake")


# ===== Build options ===== #
set(PROJECT_BUILD_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
option(BUILD_SHARED_LIBS "Build shared libs" OFF)


# ===== Project ===== #
project("mania" LANGUAGES C)
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-g -O0)
endif()


# ===== Third Party libraries ===== #
add_submodule("raylib" "src" "raylib")
add_submodule("raygui" "src" "raygui")
add_submodule("humanize" "src" "humanize")
add_submodule("klib" "src" "klib")
add_submodule("zip" "src" "zip")
add_submodule("inih" "src" "inih")


# ===== Source files ===== #
list_sources(SOURCES "src")
set(SOURCE_MAIN "src/main.c")


# ===== Include directories ===== #
list(APPEND INCLUDES "src")
include_directories(${INCLUDES})


# ===== Link libraries ===== #
list(APPEND LINK_LIBRARIES "")
link_libraries(${LINK_LIBRARIES})


# ===== Targets ===== #
add_executable(${PROJECT_NAME} ${SOURCE_MAIN} ${SOURCES})
add_library("lib-${PROJECT_NAME}" STATIC ${SOURCES})
set_target_properties(
	${PROJECT_NAME}
	PROPERTIES
	OUTPUT_NAME "${PROJECT_NAME}"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BUILD_DIRECTORY}"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BUILD_DIRECTORY}"
)
