#!/bin/bash
export SRUN_CTEST_TIME_LIMIT_MINUTES=300 # 5 hour time limit
$WORKSPACE/Trilinos/cmake/ctest/drivers/atdm/mutrino/local-driver.sh
