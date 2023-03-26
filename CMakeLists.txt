# CMake最低版本要求
cmake_minimum_required (VERSION 2.8)
# 项目信息
project (pl0)
# 指定当前目录下的源文件，保存到<ALL_SRCS>变量中
# aux_source_directory(. ALL_SRCS)
file(GLOB_RECURSE SRC_FILES ./*.cc)
# 制定生成目标
add_executable(pl0 ${SRC_FILES})
