#ifndef S3_ENV_H
#define S3_ENV_H

#include <rocksdb/env.h>
#include <rocksdb/status.h>
#include <rocksdb/options.h>
#include <rocksdb/env.h>
#include <rocksdb/slice.h>
#include <rocksdb/table.h>
#include <rocksdb/write_batch.h>
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/core/auth/AWSCredentials.h>
#include <fstream>
#include <sstream>
#include <memory>

class S3Env : public rocksdb::EnvWrapper
{
public:
    S3Env(rocksdb::Env *base_env, const std::string &bucket_name);
    ~S3Env() override;

    rocksdb::Status NewWritableFile(const std::string &fname,
                                    std::unique_ptr<rocksdb::WritableFile> *result,
                                    const rocksdb::EnvOptions &options) override;

    rocksdb::Status NewRandomAccessFile(const std::string &fname,
                                        std::unique_ptr<rocksdb::RandomAccessFile> *result,
                                        const rocksdb::EnvOptions &options) override;

private:
    // Aws::SDKOptions options_;
    Aws::S3::S3ClientConfiguration config_;
    Aws::Auth::AWSCredentials credentials_;
    std::shared_ptr<Aws::S3::S3Client> s3_client_;
    std::string bucket_name_;

    class S3WritableFile : public rocksdb::WritableFile
    {
    public:
        S3WritableFile(std::shared_ptr<Aws::S3::S3Client> s3_client, const std::string &bucket_name, const std::string &object_name);
        ~S3WritableFile() override;

        rocksdb::Status Append(const rocksdb::Slice &data) override;
        rocksdb::Status Close() override;
        rocksdb::Status Flush() override;
        rocksdb::Status Sync() override;

    private:
        rocksdb::Status UploadToS3();
        std::shared_ptr<Aws::S3::S3Client> s3_client_;
        std::string bucket_name_;
        std::string object_name_;
        std::ostringstream buffer_;
    };

    class S3RandomAccessFile : public rocksdb::RandomAccessFile
    {
    public:
        S3RandomAccessFile(std::shared_ptr<Aws::S3::S3Client> s3_client, const std::string &bucket_name, const std::string &object_name);
        ~S3RandomAccessFile() override = default;

        rocksdb::Status Read(uint64_t offset, size_t n, rocksdb::Slice *result, char *scratch) const override;

    private:
        std::string ReadFromS3() const;
        std::shared_ptr<Aws::S3::S3Client> s3_client_;
        std::string bucket_name_;
        std::string object_name_;
        mutable std::string file_data_;
    };
};

#endif // S3_ENV_H
