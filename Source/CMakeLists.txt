if (VISUS_IDX2)
	add_compile_definitions(VISUS_IDX2=1)
ENDIF()

cmake_minimum_required(VERSION 3.8)
project(idx2 CXX)
add_subdirectory(Core)
set(IDX2_LIB_SOURCE_FILES idx2.h idx2.cpp)

add_library(idx2 STATIC ${IDX2_LIB_SOURCE_FILES})

set_target_properties(idx2 PROPERTIES 
  PUBLIC_HEADER "idx2.h"
  POSITION_INDEPENDENT_CODE ON)

target_link_libraries(idx2 PUBLIC idx2Core)

add_subdirectory(Applications)

if (NOT VISUS_IDX2)

	option(BUILD_IDX2PY "Build the Python interface for idx2" OFF)
	if(BUILD_IDX2PY)
	  add_subdirectory(Python)
	endif()

	install(TARGETS idx2
	  EXPORT idx2Targets
	  LIBRARY DESTINATION lib
	  ARCHIVE DESTINATION lib
	  PUBLIC_HEADER DESTINATION include)

	install(EXPORT idx2Targets
	  FILE idx2Targets.cmake
	  DESTINATION lib/cmake/idx2)
  
	install(FILES idx2Config.cmake DESTINATION lib/cmake/idx2)

endif()
