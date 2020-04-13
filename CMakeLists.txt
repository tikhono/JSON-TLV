cmake_minimum_required (VERSION 2.8.11)
project (TEST_TASK)

set(JSON_BuildTests OFF CACHE INTERNAL "")

add_subdirectory(json)

add_executable (exec test_task.cpp tlv.cpp tlv_box.cpp)
