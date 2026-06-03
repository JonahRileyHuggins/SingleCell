find_package(Git)
if(Git_FOUND)
  execute_process(
    COMMAND
      sh -c
      "'${GIT_EXECUTABLE}' describe --abbrev=4 --dirty=-dirty --always --tags  | cut -c 2- | tr -d '\n' | sed s/-/./"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE PROJECT_VERSION_GIT)
endif()

# get project root directory
get_filename_component(CMAKE_PARENT_LIST_DIR ${CMAKE_PARENT_LIST_FILE}
                       DIRECTORY)
get_filename_component(CMAKE_PARENT_LIST_DIR ${CMAKE_PARENT_LIST_DIR} DIRECTORY)

message(STATUS "CMAKE_PARENT_LIST_FILE=${CMAKE_PARENT_LIST_FILE}")
message(STATUS "CMAKE_PARENT_LIST_DIR=${CMAKE_PARENT_LIST_DIR}")
message(STATUS "CMAKE_CURRENT_LIST_DIR=${CMAKE_CURRENT_LIST_DIR}")
file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/../version.txt" PROJECT_VERSION)
message(DEBUG "Version number: ${PROJECT_VERSION}")
