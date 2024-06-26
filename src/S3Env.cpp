#include "S3Env.h"

S3Env::S3Env(rocksdb::Env *base_env, const std::string &bucket_name)
    : rocksdb::EnvWrapper(base_env), bucket_name_(bucket_name)
{

    config_.endpointOverride = "http://127.0.0.1:9000"; // MinIO 服务器地址
    config_.scheme = Aws::Http::Scheme::HTTP;
    config_.verifySSL = false;
    config_.region = "cn-north-1";
    credentials_ = Aws::Auth::AWSCredentials("admin", "Ecbox2022!");

    std::cout << credentials_.GetAWSAccessKeyId() << std::endl;
    s3_client_ = std::make_shared<Aws::S3::S3Client>(credentials_, nullptr, config_);
}

S3Env::~S3Env()
{
    // Aws::ShutdownAPI(options_);
}

rocksdb::Status S3Env::NewWritableFile(const std::string &fname,
                                       std::unique_ptr<rocksdb::WritableFile> *result,
                                       const rocksdb::EnvOptions &options)
{
    auto s3_file = std::make_unique<S3WritableFile>(s3_client_, bucket_name_, fname);
    *result = std::move(s3_file);
    std::cout << "S3Env::NewWritableFile(): " << fname << std::endl;
    return rocksdb::Status::OK();
}

rocksdb::Status S3Env::NewRandomAccessFile(const std::string &fname,
                                           std::unique_ptr<rocksdb::RandomAccessFile> *result,
                                           const rocksdb::EnvOptions &options)
{
    auto s3_file = std::make_unique<S3RandomAccessFile>(s3_client_, bucket_name_, fname);
    *result = std::move(s3_file);
    std::cout << "S3Env::NewRandomAccessFile(): " << std::endl;
    return rocksdb::Status::OK();
}

S3Env::S3WritableFile::S3WritableFile(std::shared_ptr<Aws::S3::S3Client> s3_client, const std::string &bucket_name, const std::string &object_name)
    : s3_client_(s3_client), bucket_name_(bucket_name), object_name_(object_name) {}

S3Env::S3WritableFile::~S3WritableFile()
{
    UploadToS3();
}

rocksdb::Status S3Env::S3WritableFile::Append(const rocksdb::Slice &data)
{
    {

        Aws::S3::Model::PutObjectRequest request;
        request.SetBucket(bucket_name_);    // 替换成你的存储桶名称
        request.SetKey("/db/000000.dbtmp"); // 替换成你希望存储的对象的键（文件名）
        // 设置上传对象的内容（二进制数据）
        std::shared_ptr<Aws::IOStream> inputData = Aws::MakeShared<Aws::StringStream>("ObjectName");
        *inputData << data.data(); // 替换成你的二进制数据

        request.SetBody(inputData);

        // 执行上传操作
        auto outcome = s3_client_->PutObject(request);
        if (!outcome.IsSuccess())
        {
            auto error = outcome.GetError();
            std::cout << "Failed to upload object: " << error.GetExceptionName() << " - " << error.GetMessage() << std::endl;
        }
        else
        {
            std::cout << "Successfully uploaded object to S3" << std::endl;
        }
    }
    // Aws::S3::Model::PutObjectRequest request;
    // request.WithBucket(bucket_name_).WithKey(object_name_);

    // // 将数据添加到请求中
    // request.SetBody(Aws::MakeShared<Aws::StringStream>("", data.ToString()));

    // // 执行请求
    // auto outcome = s3_client_->PutObject(request);

    // // 检查结果
    // if (!outcome.IsSuccess())
    // {
    //     return rocksdb::Status::IOError(object_name_, outcome.GetError().GetMessage());
    // }

    return rocksdb::Status::OK();
}

rocksdb::Status S3Env::S3WritableFile::Close()
{
    return UploadToS3();
}

rocksdb::Status S3Env::S3WritableFile::Flush()
{
    return rocksdb::Status::OK();
}

rocksdb::Status S3Env::S3WritableFile::Sync()
{
    return Flush();
}

rocksdb::Status S3Env::S3WritableFile::UploadToS3()
{
    Aws::S3::Model::PutObjectRequest put_object_request;
    put_object_request.SetBucket(bucket_name_);
    put_object_request.SetKey(object_name_);
    const auto input_data = Aws::MakeShared<Aws::StringStream>("");
    *input_data << buffer_.str();
    put_object_request.SetBody(input_data);

    auto put_object_outcome = s3_client_->PutObject(put_object_request);
    if (put_object_outcome.IsSuccess())
    {
        return rocksdb::Status::OK();
    }
    else
    {
        return rocksdb::Status::IOError("Failed to upload to S3");
    }
}

S3Env::S3RandomAccessFile::S3RandomAccessFile(std::shared_ptr<Aws::S3::S3Client> s3_client, const std::string &bucket_name, const std::string &object_name)
    : s3_client_(s3_client), bucket_name_(bucket_name), object_name_(object_name)
{
    file_data_ = ReadFromS3();
}

std::string S3Env::S3RandomAccessFile::ReadFromS3() const
{
    Aws::S3::Model::GetObjectRequest get_object_request;
    get_object_request.SetBucket(bucket_name_);
    get_object_request.SetKey("/db/000000.dbtmp");

    auto get_object_outcome = s3_client_->GetObject(get_object_request);
    if (get_object_outcome.IsSuccess())
    {
        std::ostringstream ss;
        ss << get_object_outcome.GetResult().GetBody().rdbuf();
        return ss.str();
    }
    else
    {
        throw std::runtime_error("Failed to read from S3");
    }
}

rocksdb::Status S3Env::S3RandomAccessFile::Read(uint64_t offset, size_t n, rocksdb::Slice *result, char *scratch) const
{
    if (offset + n > file_data_.size())
    {
        return rocksdb::Status::IOError("Out of bounds read");
    }
    memcpy(scratch, file_data_.data() + offset, n);
    *result = rocksdb::Slice(scratch, n);
    return rocksdb::Status::OK();
}
