cmake_minimum_required(VERSION 2.5)
project(testSGIMemo)


# 设置调试信息 以及 启动C++11语言标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC")

# 定义参与编译的源代码文件 
aux_source_directory(. SRC_LIST)

# 编译生成动态库mymuduo
add_executable(testSGIMemo ${SRC_LIST})