#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "core::core_mineserver" for configuration "Debug"
set_property(TARGET core::core_mineserver APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(core::core_mineserver PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/libcore_mineserver.dll.a"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/libcore_mineserver.dll"
  )

list(APPEND _cmake_import_check_targets core::core_mineserver )
list(APPEND _cmake_import_check_files_for_core::core_mineserver "${_IMPORT_PREFIX}/lib/libcore_mineserver.dll.a" "${_IMPORT_PREFIX}/bin/libcore_mineserver.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
