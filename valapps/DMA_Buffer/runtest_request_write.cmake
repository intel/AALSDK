execute_process(COMMAND lsmod
  COMMAND grep cci
  RESULT_VARIABLE HAD_ERROR
  OUTPUT_VARIABLE OUT)

if(${OUT} STREQUAL "")
  message(FATAL_ERROR "CCI driver is not loaded")
endif()

# we are piping stdout into log file so only stderr should appear
execute_process(COMMAND ${CMAKE_BINARY_DIR}/../bin/DMA_Buffer -s ${I}
  OUTPUT_FILE of=DMA_Buffer_${I}.log
  ERROR_VARIABLE OUTERROR)

if(${OUTERROR} MATCHES ".*Error.*")
  message(FATAL_ERROR "Error raised during execution")
endif()

# Check that log file does not contain error
execute_process(COMMAND cat DMA_Buffer_${I}.log
  COMMAND grep Error
  OUTPUT_VARIABLE HAD_ERROR)

if(HAD_ERROR)
  message(FATAL_ERROR "Error raised during runtime")
endif()
