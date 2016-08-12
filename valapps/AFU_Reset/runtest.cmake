execute_process(COMMAND lsmod
  COMMAND grep cci
  RESULT_VARIABLE HAD_ERROR
  OUTPUT_VARIABLE OUT)

if(${OUT} STREQUAL "")
  message(FATAL_ERROR "CCI driver is not loaded")
endif()

execute_process(COMMAND dmesg
  COMMAND tail -n 1000
  COMMAND dd of=initial.txt
  RESULT_VARIABLE HAD_ERROR)

if(HAD_ERROR)
  message(FATAL_ERROR "Unable to query dmesg")
endif()

execute_process(COMMAND ${CMAKE_BINARY_DIR}/../bin/AFU_Reset
  OUTPUT_FILE AFU_Reset.log
  ERROR_VARIABLE OUTERROR)

if(${OUTERROR} MATCHES ".*Error.*")
  message(FATAL_ERROR "Error raised during execution")
endif()

execute_process(COMMAND dmesg
  COMMAND tail -n 1000
  COMMAND dd of=final.txt
  RESULT_VARIABLE HAD_ERROR)

if(HAD_ERROR)
  message(FATAL_ERROR "Unable to query dmesg")
endif()

execute_process(COMMAND diff -a --suppress-common-lines final.txt initial.txt
  COMMAND grep "<"
#  COMMAND awk '{for (i=2; i<NF; i++) printf $i " "; print $NF}'
  COMMAND dd of=differences.txt
  RESULT_VARIABLE HAD_ERROR)

if(HAD_ERROR)
  message(FATAL_ERROR "Unable to perform diff between dmesg log files")
endif()

# Check correct sequence, as of now
# get_port_feature(), port_afu_quiesce_and_halt(), port_afu_enable()
# also check that it matches requesting pid
if(NOT EXISTS "${SOURCEDIR}/expected.txt")
  message(FATAL_ERROR "Reference file ${SOURCEDIR}/expected.txt is not found")
endif()
execute_process(COMMAND grep -Fxq differences.txt ${SOURCEDIR}/expected.txt
  RESULT_VARIABLE HAD_ERROR)

if(RESULT_VARIABLE)
  message(FATAL_ERROR "Incorrect sequence")
endif()
