#!/bin/bash

if [[ -z "${UCVM_INSTALL_PATH}" ]]; then
  if [[ -f "${UCVM_INSTALL_PATH}/model/linthurber/lib" ]]; then
    env DYLD_LIBRARY_PATH=${UCVM_INSTALL_PATH}/model/linthurber/lib ./test_linthurber
    exit
  fi
fi
env DYLD_LIBRARY_PATH=../src ./test_linthurber
