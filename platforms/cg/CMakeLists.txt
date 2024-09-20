# Configure with [fxsdk build-fx] or [fxsdk build-cg], which provide the
# toolchain file and module path of the fxSDK

cmake_minimum_required(VERSION 3.15)
project(Raycaster)

include(GenerateG1A)
include(GenerateG3A)
include(Fxconv)
find_package(Gint 2.8 REQUIRED)

set(SOURCES
  src/render.c
  src/wall.c
  ../../src/main.c
  ../../src/fixed.c
  ../../src/raycaster.c
  # ...
)
set(ASSETS
  assets/wall.png
  # ...
)

set(INCDIRS
  ../platforms/cg/src
  src
  ../../src
)

fxconv_declare_assets(${ASSETS} WITH_METADATA)

add_executable(raycaster ${SOURCES} ${ASSETS})
target_include_directories(raycaster PRIVATE ${INCDIRS})
target_compile_options(raycaster PRIVATE -Wall -Wextra -Os -g)
target_link_libraries(raycaster Gint::Gint)

if("${FXSDK_PLATFORM_LONG}" STREQUAL fxCG50)
  generate_g3a(TARGET raycaster OUTPUT "Raycaster.g3a"
    NAME "Raycaster" ICONS assets/icon-uns.png assets/icon-sel.png)
endif()
