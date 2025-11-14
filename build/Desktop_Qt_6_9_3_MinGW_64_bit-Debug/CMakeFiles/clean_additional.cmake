# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\InterfaceAvaliacoes_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\InterfaceAvaliacoes_autogen.dir\\ParseCache.txt"
  "InterfaceAvaliacoes_autogen"
  )
endif()
