
cmake_minimum_required(VERSION 3.16)

project(COMTEK4)

file(GLOB_RECURSE SRC_FILES
    "src/*.cpp"
    "../common/src/*.cpp"
)

add_executable(COMTEK4 ${SRC_FILES})

target_include_directories(COMTEK4 PUBLIC
    "./include"
    "../common/include"
)

target_compile_features(COMTEK4 PUBLIC cxx_std_23)
