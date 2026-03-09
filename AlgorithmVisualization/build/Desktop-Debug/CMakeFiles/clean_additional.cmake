# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "AlgorithmVisualizaion_autogen"
  "CMakeFiles/AlgorithmVisualizaion_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/AlgorithmVisualizaion_autogen.dir/ParseCache.txt"
  )
endif()
