# This is the CMakeLists file for the example programs.
cmake_minimum_required(VERSION 3.0.0)
if(COMMAND CMAKE_POLICY)
  cmake_policy(SET CMP0003 NEW)
endif()

add_subdirectory(AbsoluteDoseCalibrationDemo)
add_subdirectory(DoseCalcDemo)
add_subdirectory(PhysicsDemo)
add_subdirectory(RayCtDemo)
