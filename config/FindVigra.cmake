# This module finds an installed Vigra package.

FIND_PATH(VIGRA_INCLUDE_DIR vigra/matrix.hxx)
FIND_LIBRARY(VIGRA_IMPEX_LIBRARY NAMES vigraimpex)

IF (VIGRA_INCLUDE_DIR)
    SET(VIGRA_FOUND TRUE)
ENDIF()

IF(VIGRA_IMPEX_LIBRARY)
    SET(VIGRA_IMPEX_LIBRARY_FOUND TRUE)
ENDIF()

IF(VIGRA_FOUND)
    IF (NOT Vigra_FIND_QUIETLY)
      MESSAGE(STATUS "Found Vigra:")
      MESSAGE(STATUS "  > includes:            ${VIGRA_INCLUDE_DIR}")
      MESSAGE(STATUS "  > impex library:       ${VIGRA_IMPEX_LIBRARY}")
    ENDIF()
ELSE (VIGRA_FOUND)
    IF(Vigra_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find Vigra")
    ENDIF()
ENDIF()
