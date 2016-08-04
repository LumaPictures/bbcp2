cmake_minimum_required(VERSION 3.5)

project(bbcp2)

set(EXEC_NAME bbcp2)

file(GLOB SRC src/*.cpp)

add_compile_options(-Dunix -D_BSD -D_ALL_SOURCE -D_REENTRANT -DOO_STD -DLINUX -Wall -Werror)

add_executable(${EXEC_NAME} ${SRC})
target_link_libraries(${EXEC_NAME} nsl pthread rt z crypto)

install(TARGETS ${EXEC_NAME}
        DESTINATION bin)
