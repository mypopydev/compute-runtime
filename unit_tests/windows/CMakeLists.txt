#
# Copyright (C) 2018 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(WIN32)
project(igdrcl_windows_dll_tests)

add_executable(igdrcl_windows_dll_tests
  ${IGDRCL_SOURCE_DIR}/runtime/os_interface/windows/wddm/wddm_create.cpp
  ${IGDRCL_SOURCE_DIR}/unit_tests/ult_configuration.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/wddm_create_tests.cpp
  $<TARGET_OBJECTS:igdrcl_libult>
  $<TARGET_OBJECTS:igdrcl_libult_cs>
  $<TARGET_OBJECTS:igdrcl_libult_env>
)

target_link_libraries(igdrcl_windows_dll_tests ${NEO_MOCKABLE_LIB_NAME} igdrcl_mocks gmock-gtest ${IGDRCL_EXTRA_LIBS})

target_include_directories(igdrcl_windows_dll_tests PRIVATE
  ${IGDRCL_SOURCE_DIR}/unit_tests/gen_common
)

create_project_source_tree(igdrcl_windows_dll_tests ${IGDRCL_SOURCE_DIR}/runtime ${IGDRCL_SOURCE_DIR}/unit_tests)

add_custom_target(run_windows_dll_tests ALL DEPENDS unit_tests igdrcl_windows_dll_tests)
add_custom_command(
  TARGET run_windows_dll_tests
  POST_BUILD
  COMMAND WORKING_DIRECTORY ${TargetDir}
  COMMAND echo Target Directory is: ${TargetDir}
  COMMAND echo Running Windows dll tests
  COMMAND igdrcl_windows_dll_tests
)
add_dependencies(run_unit_tests run_windows_dll_tests)

set_target_properties(igdrcl_windows_dll_tests PROPERTIES FOLDER ${TEST_PROJECTS_FOLDER})
set_target_properties(run_windows_dll_tests PROPERTIES FOLDER ${TEST_PROJECTS_FOLDER})
endif()