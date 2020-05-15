# This is the CMakeLists file for the Library folder source code.
cmake_minimum_required(VERSION 2.8.9)
if(COMMAND CMAKE_POLICY)
  cmake_policy(SET CMP0003 NEW)
endif()

project(solutio)

# FFTW
#find_library(FFTW_LIBRARY NAMES fftw3 fftw fftw3_omp)
find_library(FFTW_LIBRARY NAMES fftw3)
set(FFTW_LIBRARIES "${FFTW_LIBRARY}")

# Grassroots DICOM (GDCM)
find_package(GDCM)
set(GDCM_LIB gdcmMSFF)

# Set compiler flags and linked libraries
SET(GCC_COVERAGE_COMPILE_FLAGS "-fopenmp -std=c++17")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

# Set include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(LIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

# Add sources and headers
set(SOURCE
  ${SOURCE}
  # Geometry
  ${CMAKE_CURRENT_SOURCE_DIR}/Geometry/Cylinder.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Geometry/GeometricObjectModel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Geometry/Ray3.cpp
  # Imaging
  ${CMAKE_CURRENT_SOURCE_DIR}/Imaging/ObjectModelXray.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Imaging/RayCT.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Imaging/Tasmip.cpp
  # Physics
  ${CMAKE_CURRENT_SOURCE_DIR}/Physics/NistEstar.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Physics/NistPad.cpp
  # Utilities
  ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/FileIO.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/Statistics.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/fftw++-2.05/fftw++.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/Dicom/DicomDatabase.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/Dicom/DicomIODs.cpp
  # Therapy
  ${CMAKE_CURRENT_SOURCE_DIR}/Therapy/CBDose.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Therapy/BrachyDose43.cpp
)

set(HEADERS
  ${HEADERS}
  # Geometry
  ${CMAKE_CURRENT_SOURCE_DIR}/Geometry/Cylinder.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Geometry/GeometricObject.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Geometry/GeometricObjectModel.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Geometry/Ray3.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Geometry/Vec3.hpp
  # Imaging
  ${CMAKE_CURRENT_SOURCE_DIR}/Imaging/ObjectModelXray.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Imaging/RayCT.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Imaging/Tasmip.hpp
  # Physics
  ${CMAKE_CURRENT_SOURCE_DIR}/Physics/NistEstar.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Physics/NistPad.hpp
  # Utilities
  ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/DataInterpolation.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/FileIO.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/GenericImage.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/Statistics.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/fftw++-2.05/fftw++.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/Dicom/DicomAttributes.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/Dicom/DicomDatabase.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/Dicom/DicomIODs.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/Dicom/DicomModules.hpp
  # Therapy
  ${CMAKE_CURRENT_SOURCE_DIR}/Therapy/CBDose.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Therapy/BrachyDose43.hpp
)

add_library(solutio STATIC ${SOURCE} ${HEADERS} ${FFTW_LIBRARIES})
target_link_libraries(solutio gdcmMSFF)

# Install library
install(TARGETS solutio)
# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Geometry DESTINATION "include/SolutioCpp" FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Imaging DESTINATION "include/SolutioCpp" FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Physics DESTINATION "include/SolutioCpp" FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Utilities DESTINATION "include/SolutioCpp"
  FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Therapy DESTINATION "include/SolutioCpp" FILES_MATCHING PATTERN "*.hpp")
