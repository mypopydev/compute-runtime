#
# Copyright (C) 2017-2018 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

set(IGDRCL_SRCS_tests_gen_common
  ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
  ${CMAKE_CURRENT_SOURCE_DIR}/cmd_parse_base.inl
  ${CMAKE_CURRENT_SOURCE_DIR}/cmd_parse_base_mi_arb.inl
  ${CMAKE_CURRENT_SOURCE_DIR}/cmd_parse_gpgpu_walker.inl
  ${CMAKE_CURRENT_SOURCE_DIR}/cmd_parse_sip.inl
  ${CMAKE_CURRENT_SOURCE_DIR}/gen_cmd_parse.h
  ${CMAKE_CURRENT_SOURCE_DIR}/gen_commands_common_validation.h
  ${CMAKE_CURRENT_SOURCE_DIR}/matchers.h
  ${CMAKE_CURRENT_SOURCE_DIR}/test.h
)
target_sources(igdrcl_tests PRIVATE ${IGDRCL_SRCS_tests_gen_common})

add_subdirectories()
