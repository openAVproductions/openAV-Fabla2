
cmake_minimum_required (VERSION 2.6)

project (FABLA2)

set(FABLA2_VERSION_MAJOR "2")
set(FABLA2_VERSION_MINOR "0")
set(FABLA2_VERSION_PATCH "0")

set(FABLA2_VERSION "${FABLA2_VERSION_MAJOR}.${FABLA2_VERSION_MINOR}.${FABLA2_VERSION_PATCH}")

option(FABLA2_RELEASE_BUILD   "Build for releasing"         OFF )
option(FABLA2_DEBUG_PRINTS    "Build with debugging prints" ON  )
option(FABLA2_TESTS           "Build component tests"       OFF )


################################################################################
### Code compilation starts here
################################################################################

ADD_DEFINITIONS( "-DFABLA2_VERSION_STRING=\"v ${FABLA2_VERSION}\"" )

include_directories ("${PROJECT_SOURCE_DIR}/src/ui/avtk/avtk/")

find_package(PkgConfig)

ADD_DEFINITIONS( "-DAVTK_SNDFILE" )
pkg_check_modules(SNDFILE sndfile REQUIRED)
include_directories( ${SNDFILE_INCLUDE_DIRS})
link_directories   ( ${SNDFILE_LIBRARY_DIRS})

ADD_DEFINITIONS( "-DPUGL_HAVE_CAIRO" )
pkg_check_modules(CAIRO cairo REQUIRED)
include_directories( ${CAIRO_INCLUDE_DIRS})
link_directories   ( ${CAIRO_LIBRARY_DIRS})

pkg_check_modules(X11 x11 REQUIRED)
include_directories( ${X11_INCLUDE_DIRS} )
link_directories   ( ${X11_LIBRARY_DIRS} )

SET(CMAKE_CXX_FLAGS "-g -trigraphs -Wl,-z,nodelete  -Wl,--no-undefined -fPIC -shared ")





######################
### Build Tests OR lv2
######################
IF( FABLA2_TESTS )
  
  ADD_DEFINITIONS( "-DFABLA2_COMPONENT_TEST" )
  SET(CMAKE_CXX_FLAGS "-g -trigraphs -Wl,-z,nodelete  -Wl,--no-undefined")
  FILE(GLOB srcDspTests src/dsp/tests/*.cxx src/dsp/*.cxx )
  ADD_EXECUTABLE( fabla2test ${srcDspTests} )
  target_link_libraries( fabla2test ${SNDFILE_LIBRARIES})
  configure_file( "src/dsp/tests/test.wav" "test.wav" COPYONLY)

ELSE()

  FILE(GLOB srcDSP src/dsp.cxx src/lv2_work.cxx src/dsp/*.cxx )
  FILE(GLOB srcUI  src/ui.cxx  src/ui/*.cxx src/ui/avtk/avtk/*.cxx src/ui/avtk/avtk/pugl/pugl_x11.c )

  ADD_LIBRARY( fabla2   SHARED ${srcDSP} )
  ADD_LIBRARY( fabla2ui SHARED ${srcUI}  )
  set_target_properties(fabla2   PROPERTIES PREFIX "")
  set_target_properties(fabla2ui PROPERTIES PREFIX "")


  target_link_libraries( fabla2 ${SNDFILE_LIBRARIES})

  target_link_libraries( fabla2ui ${CAIRO_LIBRARIES}  )
  target_link_libraries( fabla2ui ${X11_LIBRARIES}    )
  target_link_libraries( fabla2ui ${SNDFILE_LIBRARIES})


  # install CMake compiled files
  install (TARGETS fabla2   DESTINATION lib/lv2/fabla2.lv2/)
  install (TARGETS fabla2ui DESTINATION lib/lv2/fabla2.lv2/)

  # install LV2 .ttl files
  FILE(GLOB ttls "src/*.ttl")
  install(FILES ${ttls}   DESTINATION lib/lv2/fabla2.lv2/)

ENDIF()
