cmake_minimum_required(VERSION 3.10)
project(RocksDB_S3)

set(CMAKE_CXX_STANDARD 14)

# 设置 RocksDB 和 AWS SDK 的路径
set(ROCKSDB_INCLUDE_DIR "/usr/local/include/rocksdb")
set(ROCKSDB_LIB_DIR "/usr/local/lib")

find_package(AWSSDK REQUIRED COMPONENTS s3)

# 包含头文件目录
include_directories(${ROCKSDB_INCLUDE_DIR})
include_directories(${AWSSDK_INCLUDE_DIRS})

# 查找库文件
find_library(ROCKSDB_LIBRARY rocksdb HINTS ${ROCKSDB_LIB_DIR})

if (NOT ROCKSDB_LIBRARY)
    message(FATAL_ERROR "RocksDB library not found")
endif()

# 添加源文件
add_executable(RocksDB_S3 main.cpp S3Env.cpp)

# add_executable(RocksDB_S3_test main.cpp)

# 启用 RTTI
target_compile_options(RocksDB_S3 PRIVATE -fno-rtti)

# 链接库
target_link_libraries(RocksDB_S3 PRIVATE ${ROCKSDB_LIBRARY} aws-cpp-sdk-s3 aws-cpp-sdk-core)

# 设置库路径
link_directories(${ROCKSDB_LIB_DIR})
