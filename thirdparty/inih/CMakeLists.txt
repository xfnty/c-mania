
project(inih LANGUAGES C)

include_directories("src")

add_library(${PROJECT_NAME} STATIC "src/ini.c")
