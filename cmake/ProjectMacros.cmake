include(CMakeParseArguments)

# Add google tests macro
macro(ADD_GOOGLE_TESTS executable)
  foreach ( source ${ARGN} )
    string(REGEX MATCH .*cpp|.*cc source "${source}")
    if(source)
      file(READ "${source}" contents)
      string(REGEX MATCHALL "TEST_?F?\\(([A-Za-z_0-9 ,]+)\\)" found_tests ${contents})
      foreach(hit ${found_tests})
        string(REGEX REPLACE ".*\\(( )*([A-Za-z_0-9]+)( )*,( )*([A-Za-z_0-9]+)( )*\\).*" "\\2.\\5" test_name ${hit})
        add_test(${test_name} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${executable}" --gtest_filter=${test_name})
      endforeach(hit)
    endif()
  endforeach()
endmacro()

# Create source groups automatically based on file path
macro( CREATE_SRC_GROUPS SRC )
  foreach( F ${SRC} )
    string( REGEX MATCH "(^.*)([/\\].*$)" M ${F} )
    if(CMAKE_MATCH_1)
      string( REGEX REPLACE "[/\\]" "\\\\" DIR ${CMAKE_MATCH_1} )
      source_group( ${DIR} FILES ${F} )
    else()
      source_group( \\ FILES ${F} )
    endif()
  endforeach()
endmacro()

# Create test targets
macro( CREATE_TEST_TARGETS BASE_NAME SRC DEPENDENCIES )
  if( BUILD_TESTING )
    add_executable( ${BASE_NAME}_tests ${SRC} )

    CREATE_SRC_GROUPS( "${SRC}" )
    
    get_target_property(BASE_NAME_TYPE ${BASE_NAME} TYPE)
    if ("${BASE_NAME_TYPE}" STREQUAL "EXECUTABLE")
      # don't link base name
      set(ALL_DEPENDENCIES ${DEPENDENCIES} )
    else()
      # also link base name
      set(ALL_DEPENDENCIES ${BASE_NAME} ${DEPENDENCIES} )
    endif()
      
    target_link_libraries( ${BASE_NAME}_tests 
      ${ALL_DEPENDENCIES} 
      gtest 
      gtest_main
    )

    ADD_GOOGLE_TESTS( ${BASE_NAME}_tests ${SRC} )
  endif()
endmacro()

function( ADD_SIMULATION_TEST )
  set(options ANNUAL_SIMULATION DESIGN_DAY_ONLY)
  set(oneValueArgs IDF_FILE EPW_FILE)
  set(multiValueArgs "")
  cmake_parse_arguments(ADD_SIM_TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

  if( DESIGN_DAY_ONLY )
    set(ANNUAL_SIMULATION false)
  elseif( ADD_SIM_TEST_ANNUAL_SIMULATION OR TEST_ANNUAL_SIMULATION  )
    set(ANNUAL_SIMULATION true)
  else()
    set(ANNUAL_SIMULATION false)
  endif()

  get_filename_component(IDF_NAME "${ADD_SIM_TEST_IDF_FILE}" NAME_WE)
  if(BUILD_FORTRAN) #only do ExpandObjects in Fortran/Full builds
    add_test(NAME "integration.${IDF_NAME}" COMMAND ${CMAKE_COMMAND}
      -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
      -DBINARY_DIR=${CMAKE_BINARY_DIR}
      -DENERGYPLUS_EXE=$<TARGET_FILE:EnergyPlus>
      #-DEXPANDOBJECTS_EXE=$<TARGET_FILE:ExpandObjects>
      -DIDF_FILE=${ADD_SIM_TEST_IDF_FILE}
      -DEPW_FILE=${ADD_SIM_TEST_EPW_FILE}
      -DANNUAL_SIMULATION=${ANNUAL_SIMULATION}
      -DBUILD_FORTRAN=${BUILD_FORTRAN}
      -P ${CMAKE_SOURCE_DIR}/cmake/RunSimulation.cmake
    )
  else()
    add_test(NAME "integration.${IDF_NAME}" COMMAND ${CMAKE_COMMAND}
      -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
      -DBINARY_DIR=${CMAKE_BINARY_DIR}
      -DENERGYPLUS_EXE=$<TARGET_FILE:EnergyPlus>
      -DIDF_FILE=${ADD_SIM_TEST_IDF_FILE}
      -DEPW_FILE=${ADD_SIM_TEST_EPW_FILE}
      -DANNUAL_SIMULATION=${ANNUAL_SIMULATION}
      -DBUILD_FORTRAN=${BUILD_FORTRAN}
      -P ${CMAKE_SOURCE_DIR}/cmake/RunSimulation.cmake
    )  
  endif()
  SET_TESTS_PROPERTIES("integration.${IDF_NAME}" PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed")
  SET_TESTS_PROPERTIES("integration.${IDF_NAME}" PROPERTIES FAIL_REGULAR_EXPRESSION "ERROR;FAIL;Test Failed")
endfunction()

macro( ADD_CXX_DEFINITIONS NEWFLAGS )
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${NEWFLAGS}")
endmacro()

macro( ADD_CXX_DEBUG_DEFINITIONS NEWFLAGS )
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${NEWFLAGS}")
endmacro()

macro( ADD_CXX_RELEASE_DEFINITIONS NEWFLAGS )
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${NEWFLAGS}")
endmacro()

