#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <iostream>
#include "S3Env.h" // 包含自定义的 S3Env 头文件

int main()
{
    // 初始化 AWS SDK
    Aws::SDKOptions options_;
    Aws::InitAPI(options_);

    try
    {
        rocksdb::Options rocksdbOptions;
        rocksdbOptions.create_if_missing = true;
        // 使用自定义的 S3Env
        rocksdbOptions.env = new S3Env(rocksdb::Env::Default(), "awstest");

        rocksdb::DB *db;
        rocksdb::Status status = rocksdb::DB::Open(rocksdbOptions, "./db", &db);
        if (!status.ok())
        {
            std::cerr << "Unable to open RocksDB: " << status.ToString() << std::endl;
            return 1;
        }

        // 使用 RocksDB 示例代码
        std::string value;
        status = db->Put(rocksdb::WriteOptions(), "key", "value");
        if (!status.ok())
        {
            std::cerr << "Put failed: " << status.ToString() << std::endl;
        }

        status = db->Get(rocksdb::ReadOptions(), "key", &value);
        if (!status.ok())
        {
            std::cerr << "Get failed: " << status.ToString() << std::endl;
        }
        else
        {
            std::cout << "Get value: " << value << std::endl;
        }

        delete db;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return 1;
    }

    // 关闭 AWS SDK
    Aws::ShutdownAPI(options_);

    return 0;
}
