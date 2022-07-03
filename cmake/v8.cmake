# v8 CMake Import
add_library(v8 STATIC IMPORTED)
set_target_properties(v8
    PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/libs/v8.lib")