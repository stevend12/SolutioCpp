# This is the CMakeLists file for the PhotonDemo program.
cmake_minimum_required(VERSION 2.8.9)
if(COMMAND CMAKE_POLICY)
  cmake_policy(SET CMP0003 NEW)
endif()

project(solutio)

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
  ${CMAKE_CURRENT_SOURCE_DIR}/Physics/NistPad.cpp
  # Utilities
  ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/FileIO.cpp
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
  ${CMAKE_CURRENT_SOURCE_DIR}/Physics/NistPad.hpp
  # Utilities
  ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/DataInterpolation.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/FileIO.hpp
  # Therapy
  ${CMAKE_CURRENT_SOURCE_DIR}/Therapy/CBDose.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Therapy/BrachyDose43.hpp
)

add_library(solutio STATIC ${SOURCE} ${HEADERS})
