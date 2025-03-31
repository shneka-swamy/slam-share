cmake_minimum_required(VERSION 3.12.1)
project(DBoW2)

find_package(OpenCV 4 QUIET)
if(NOT OpenCV_FOUND)
   find_package(OpenCV 3.0 QUIET)
   if(NOT OpenCV_FOUND)
      message(FATAL_ERROR "OpenCV > 3.0 not found.")
   endif()
endif()

add_library(DBoW2
  DBoW2/BowVector.cpp
  DBoW2/FORB.cpp
  DBoW2/FeatureVector.cpp
  DBoW2/ScoringObject.cpp
  DUtils/Random.cpp
  DUtils/Timestamp.cpp)
target_link_libraries(DBoW2 ${OpenCV_LIBS})
set_target_properties(DBoW2 PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
  LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
  ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
)
target_compile_options(DBoW2 PUBLIC
  -Wno-deprecated-declarations
  )


