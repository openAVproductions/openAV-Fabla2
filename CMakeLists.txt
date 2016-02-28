
cmake_minimum_required (VERSION 2.6)

project (FABLA2)

set(FABLA2_VERSION_MAJOR "2")
set(FABLA2_VERSION_MINOR "0")
set(FABLA2_VERSION_PATCH "0")

set(FABLA2_VERSION "${FABLA2_VERSION_MAJOR}.${FABLA2_VERSION_MINOR}.${FABLA2_VERSION_PATCH}")

option(FABLA2_RELEASE_BUILD   "Build for releasing"         ON )
option(FABLA2_DEBUG_PRINTS    "Build with debugging prints" ON )
option(FABLA2_TESTS           "Build component tests"       OFF)
option(BUILD_GUI              "Build GUI"                   ON )


################################################################################
### Code compilation starts here
################################################################################

ADD_DEFINITIONS( "-DFABLA2_VERSION_STRING=\"v ${FABLA2_VERSION}\"" )

IF( FABLA2_DEBUG_PRINTS )
  # For insane debugging of the AVTK / Fabla2 UI
  #ADD_DEFINITIONS( "-DAVTK_DEBUG" )
  ADD_DEFINITIONS( "-DFABLA2_DEBUG" )
ENDIF()

include_directories ("${PROJECT_SOURCE_DIR}/src/ui/avtk/avtk/")

find_package(PkgConfig)


pkg_check_modules(LV2 lv2>=1.8 REQUIRED)
include_directories( ${LV2_INCLUDE_DIRS}  )

ADD_DEFINITIONS( "-DAVTK_SNDFILE" )
pkg_check_modules(SNDFILE sndfile REQUIRED)
include_directories( ${SNDFILE_INCLUDE_DIRS})
link_directories   ( ${SNDFILE_LIBRARY_DIRS})

pkg_check_modules(SAMPLERATE samplerate REQUIRED)
include_directories( ${SAMPLERATE_INCLUDE_DIRS}  )
link_directories   ( ${SAMPLERATE_LIBRARY_DIRS}  )


IF(BUILD_GUI)
	ADD_DEFINITIONS( "-DPUGL_HAVE_CAIRO" )
	pkg_check_modules(CAIRO cairo REQUIRED)
	include_directories( ${CAIRO_INCLUDE_DIRS})
	link_directories   ( ${CAIRO_LIBRARY_DIRS})

	pkg_check_modules(X11 x11 REQUIRED)
	ADD_DEFINITIONS( "-DHAVE_X11" )
	include_directories( ${X11_INCLUDE_DIRS} )
	link_directories   ( ${X11_LIBRARY_DIRS} )
ENDIF(BUILD_GUI)

SET(CMAKE_C_FLAGS "-fPIC" ${CFLAGS}  )
SET(CMAKE_CXX_FLAGS "-Wall -Wno-sign-compare -g -trigraphs -Wno-unused-function -Wno-unused-variable -Wno-reorder -Wuninitialized -Wl,-z,nodelete  -Wl,--no-undefined -fPIC -shared ")

# for profiling runtime
#ADD_DEFINITIONS( "-DOPENAV_PROFILE" )
#ADD_DEFINITIONS( "-DPROFINY_CALL_GRAPH_PROFILER" )
#ADD_DEFINITIONS( "-DPROFINY_FLAT_PROFILER" )

######################
### Build Tests OR lv2
######################
IF( FABLA2_TESTS )
  
  ADD_DEFINITIONS( "-DFABLA2_COMPONENT_TEST" )
  
  SET(CMAKE_CXX_FLAGS "-g -trigraphs -Wl,-z,nodelete  -Wl,--no-undefined") # -fsanitize=address
  FILE(GLOB srcDspTests src/dsp/tests/*.cxx src/dsp/*.cxx )
  ADD_EXECUTABLE( fabla2test ${srcDspTests} )
  target_link_libraries( fabla2test ${SNDFILE_LIBRARIES})
  target_link_libraries( fabla2test ${SAMPLERATE_LIBRARIES} )
  configure_file( "src/dsp/tests/test.wav" "test.wav" COPYONLY)

ELSE()

  SET(CMAKE_C_FLAGS "-fPIC" ${CFLAGS}  )
  
  FILE(GLOB srcDSP src/dsp.cxx src/lv2_work.cxx src/lv2_state.cxx src/dsp/*.cxx src/dsp/*.c )
  FILE(GLOB srcUI  src/ui.cxx src/ui/sofd/libsofd.c src/ui/*.cxx src/ui/avtk/avtk/*.cxx src/ui/avtk/avtk/pugl/pugl_x11.c )
  
  add_library( fabla2   SHARED ${srcDSP} )

IF(BUILD_GUI)
	add_library( fabla2ui SHARED ${srcUI}  )
ENDIF(BUILD_GUI)
  
  set_target_properties(fabla2   PROPERTIES PREFIX "")
IF(BUILD_GUI)
  set_target_properties(fabla2ui PROPERTIES PREFIX "")
ENDIF(BUILD_GUI)
  
  target_link_libraries( fabla2 ${SNDFILE_LIBRARIES}    )
  target_link_libraries( fabla2 ${SAMPLERATE_LIBRARIES} )
  
IF(BUILD_GUI)
  target_link_libraries( fabla2ui ${CAIRO_LIBRARIES}  )
  target_link_libraries( fabla2ui ${X11_LIBRARIES}    )
  target_link_libraries( fabla2ui ${SNDFILE_LIBRARIES})
ENDIF(BUILD_GUI)
  
  # install CMake compiled files
  install (TARGETS fabla2   DESTINATION lib/lv2/fabla2.lv2/)
IF(BUILD_GUI)
  install (TARGETS fabla2ui DESTINATION lib/lv2/fabla2.lv2/)
ENDIF(BUILD_GUI)

  # install LV2 .ttl files
  FILE(GLOB ttls "src/*.ttl")
  install(FILES ${ttls}   DESTINATION lib/lv2/fabla2.lv2/)
  
  # Need to be compiled in
  #configure_file( "src/ui/themes/default.avtk"  "default.avtk" COPYONLY)
  #configure_file( "src/ui/themes/green.avtk"    "green.avtk"   COPYONLY)
  #configure_file( "src/ui/themes/orange.avtk"   "orange.avtk"  COPYONLY)
  #configure_file( "src/ui/themes/red.avtk"      "red.avtk"     COPYONLY)
  #configure_file( "src/ui/themes/yellow.avtk"   "yellow.avtk"  COPYONLY)
ENDIF()
