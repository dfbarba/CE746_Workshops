#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "libjpeg-turbo::jpeg" for configuration "Release"
set_property(TARGET libjpeg-turbo::jpeg APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(libjpeg-turbo::jpeg PROPERTIES
  IMPORTED_LOCATION_RELEASE "/Users/dfbarbag/Documents/Projects/00_CE746_Workshops/3_OpenQuake/.env/lib/libjpeg.8.3.2.dylib"
  IMPORTED_SONAME_RELEASE "@rpath/libjpeg.8.dylib"
  )

list(APPEND _cmake_import_check_targets libjpeg-turbo::jpeg )
list(APPEND _cmake_import_check_files_for_libjpeg-turbo::jpeg "/Users/dfbarbag/Documents/Projects/00_CE746_Workshops/3_OpenQuake/.env/lib/libjpeg.8.3.2.dylib" )

# Import target "libjpeg-turbo::turbojpeg" for configuration "Release"
set_property(TARGET libjpeg-turbo::turbojpeg APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(libjpeg-turbo::turbojpeg PROPERTIES
  IMPORTED_LOCATION_RELEASE "/Users/dfbarbag/Documents/Projects/00_CE746_Workshops/3_OpenQuake/.env/lib/libturbojpeg.0.5.0.dylib"
  IMPORTED_SONAME_RELEASE "@rpath/libturbojpeg.0.dylib"
  )

list(APPEND _cmake_import_check_targets libjpeg-turbo::turbojpeg )
list(APPEND _cmake_import_check_files_for_libjpeg-turbo::turbojpeg "/Users/dfbarbag/Documents/Projects/00_CE746_Workshops/3_OpenQuake/.env/lib/libturbojpeg.0.5.0.dylib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
