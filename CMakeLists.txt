cmake_minimum_required(VERSION 2.8)

project(bbcp2)

set(EXEC_NAME bbcp2)

file(GLOB SRC src/*.cpp)

add_compile_options(-D_ALL_SOURCE -D_REENTRANT -DOO_STD -Wall -Werror -std=c++11) #-Wextra -Werror

if (APPLE)
    add_compile_options(-DMACOS)
    # right now we are using brew built libs, but adding a proper find module later on
    include_directories(/usr/local/opt/openssl/include)
elseif (UNIX) # linux, non apple unix
    add_compile_options(-Dunix -D_BSD -DLINUX)
elseif (MSVC)
endif ()

add_executable(${EXEC_NAME} ${SRC})

if (APPLE)
    target_link_libraries(${EXEC_NAME} pthread z crypto)
elseif (UNIX)
    target_link_libraries(${EXEC_NAME} nsl pthread rt z crypto)
elseif (MSVC)
endif ()

install(TARGETS ${EXEC_NAME}
        DESTINATION bin)
