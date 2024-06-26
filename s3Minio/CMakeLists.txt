cmake_minimum_required(VERSION 3.10)
project(AwsS3Example)

set(CMAKE_CXX_STANDARD 17)

# 查找 AWS SDK for C++ 组件
find_package(AWSSDK REQUIRED COMPONENTS s3)

# 添加源文件
add_executable(AwsS3Example main.cpp)

# 链接 AWS SDK for C++ 库
target_link_libraries(AwsS3Example ${AWSSDK_LINK_LIBRARIES})

# 包含 AWS SDK for C++ 头文件
target_include_directories(AwsS3Example PRIVATE ${AWSSDK_INCLUDE_DIR})
