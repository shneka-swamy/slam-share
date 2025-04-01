#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "GLEW::glew_s" for configuration "Debug"
set_property(TARGET GLEW::glew_s APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(GLEW::glew_s PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "/usr/lib/x86_64-linux-gnu/libGL.so;/usr/lib/x86_64-linux-gnu/libGLU.so;/usr/lib/x86_64-linux-gnu/libSM.so;/usr/lib/x86_64-linux-gnu/libICE.so;/usr/lib/x86_64-linux-gnu/libX11.so;/usr/lib/x86_64-linux-gnu/libXext.so"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/lib/libGLEWd.a"
  )

list(APPEND _cmake_import_check_targets GLEW::glew_s )
list(APPEND _cmake_import_check_files_for_GLEW::glew_s "${_IMPORT_PREFIX}/debug/lib/libGLEWd.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
