project(CFE_SAMPLE_APP C)

set(APP_SRC_FILES
  fsw/src/sample_app.c
  fsw/src/sample_app_cmds.c
  fsw/src/sample_app_utils.c
)

if (CFE_EDS_ENABLED_BUILD)
  list(APPEND APP_SRC_FILES
    fsw/src/sample_app_eds_dispatch.c
  )
else()
  list(APPEND APP_SRC_FILES
    fsw/src/sample_app_dispatch.c
  )
endif()

# Create the app module
add_cfe_app(sample_app ${APP_SRC_FILES})

# Add include directories
target_include_directories(sample_app PUBLIC fsw/inc)

# Include the public API from sample_lib to demonstrate how
# to call library-provided functions
add_cfe_app_dependency(sample_app sample_lib)

# Add table
add_cfe_tables(sample_app fsw/tables/sample_app_tbl.c)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-pedantic -Wno-nested-externs")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-pedantic -Wno-nested-externs")

# Include the cFE core modules directory for time module
# Assuming ${CFE_CORE_SRC} is set to the cFE core source directory
include_directories(${CFE_SOURCE_DIR}/modules/time/fsw/src/)
include_directories(${CFE_SOURCE_DIR}/modules/core_private/fsw/inc/)

# Link against cFE libraries (if necessary)
# target_link_libraries(sample_app cfe_core)

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
if (ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif (ENABLE_UNIT_TESTS)
