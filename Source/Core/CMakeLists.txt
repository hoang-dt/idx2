cmake_minimum_required(VERSION 3.8)
project(idx2Core CXX)

set(IDX2_CORE_HEADER_FILES
  Algorithm.h
  Args.h
  Array.h
  Assert.h
  BitOps.h
  BitStream.h
  CircularQueue.h
  Codecs.h
  Common.h
  Core.h
  DataSet.h
  DataTypes.h
  DebugBreak.h
  Enum.h
  Error.h
  ErrorCodes.h
  Expected.h
  FileSystem.h
  FileUtilities.h
  Format.h
  Function.h
  HashSet.h
  HashTable.h
  idx2Common.h
  idx2Decode.h
  idx2Encode.h
  idx2Lookup.h
  InputOutput.h
  LinkedList.h
  Logger.h
  Macros.h
  Math.h
  Memory.h
  MemoryMap.h
  Mutex.h
  Random.h
  ScopeGuard.h
  StackTrace.h
  Statistics.h
  String.h
  Test.h
  Timer.h
  Utilities.h
  VarInt.h
  Volume.h
  Wavelet.h
  Zfp.h
)

set(IDX2_CORE_SOURCE_FILES
  Args.cpp  
  Assert.cpp  
  DataSet.cpp
  FileSystem.cpp  
  Format.cpp
  idx2Common.cpp  
  idx2Decode.cpp    
  idx2Encode.cpp  
  idx2Lookup.cpp
  InputOutput.cpp    
  Logger.cpp  
  Memory.cpp  
  MemoryMap.cpp
  StackTrace.cpp
  String.cpp  
  Utilities.cpp  
  VarInt.cpp  
  Volume.cpp  
  Wavelet.cpp  
  Zfp.cpp  
)

add_library(idx2Core ${IDX2_CORE_HEADER_FILES} ${IDX2_CORE_SOURCE_FILES})
set_target_properties(idx2Core PROPERTIES 
  POSITION_INDEPENDENT_CODE ON
  PUBLIC_HEADER "${IDX2_CORE_HEADER_FILES}")
target_compile_features(idx2Core PUBLIC cxx_std_17)
target_compile_definitions(idx2Core PUBLIC -Didx2_Avx2)

if (MSVC)
  target_compile_definitions(idx2Core PUBLIC -D_CRT_SECURE_NO_WARNINGS)
  target_compile_options(idx2Core PUBLIC /Zc:preprocessor /Zc:__cplusplus /wd5105)
  target_link_options(idx2Core PUBLIC dbghelp.lib)
elseif (UNIX)
  target_compile_options(idx2Core PUBLIC -Wno-format-zero-length)
endif()

install(TARGETS idx2Core
	EXPORT idx2CoreConfig
	LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include/Core)

install(EXPORT idx2CoreConfig DESTINATION lib/cmake/idx2)
