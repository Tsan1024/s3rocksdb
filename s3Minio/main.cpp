#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <iostream>
#include <fstream>
#include <string>
#include <aws/core/utils/memory/stl/AWSString.h>

int main()
{
    std::string bucketName = "awstest";
    std::string objectKey = "main.cpp";
    std::string filePath = "/home/tsandl/cplusplus/aws_s3_example/test.cpp";
    // 初始化AWS SDK
    Aws::SDKOptions options;
    Aws::InitAPI(options);

    // 配置凭证
    Aws::Auth::AWSCredentials credentials(
        "admin",     // 替换为你的Minio访问密钥
        "Ecbox2022!" // 替换为你的Minio秘密访问密钥
    );

    // 创建S3客户端配置
    Aws::S3::S3ClientConfiguration clientConfig;
    clientConfig.region = Aws::Region::US_EAST_1;  // 设置默认区域，这里使用us-east-1作为示例
    clientConfig.scheme = Aws::Http::Scheme::HTTP; // 设置协议，这里使用HTTP作为示例
    clientConfig.verifySSL = false;
    clientConfig.endpointOverride = "127.0.0.1:9000"; // 设置Minio端点
    // std::shared_ptr<Aws::Auth::S> endpointProvider = std::make_shared<S3EndpointProvider>(clientConfig);

    // 创建S3客户端
    Aws::S3::S3Client s3_client(credentials, nullptr, clientConfig);

    // 上传文件到Minio
    {
        // 设置上传请求
        Aws::S3::Model::PutObjectRequest request;
        request.WithBucket(bucketName)                                                                        // 替换为你的Minio存储桶名称
            .WithKey(objectKey)                                                                               // 替换为你的对象键
            .SetBody(Aws::MakeShared<Aws::FStream>("", filePath, std::ios_base::in | std::ios_base::binary)); // 替换为你的本地文件路径

        // 执行上传操作
        auto outcome = s3_client.PutObject(request);

        if (outcome.IsSuccess())
        {
            std::cout << "File uploaded successfully." << std::endl;
        }
        else
        {
            std::cout << "Error uploading file: " << outcome.GetError().GetMessage() << std::endl;
        }
    }

    // 从Minio下载文件
    {
        // 设置下载请求
        Aws::S3::Model::GetObjectRequest request;
        request.WithBucket(bucketName) // 替换为你的Minio存储桶名称
            .WithKey(objectKey);       // 替换为你的对象键

        // 执行下载操作
        auto outcome = s3_client.GetObject(request);

        if (outcome.IsSuccess())
        {
            // 将下载的文件内容写入本地文件
            std::ofstream output_file("/home/tsandl/cplusplus/aws_s3_example/test.cpp", std::ios::binary);
            output_file << outcome.GetResult().GetBody().rdbuf();
            std::cout << "File downloaded successfully." << std::endl;
        }
        else
        {
            std::cout << "Error downloading file: " << outcome.GetError().GetMessage() << std::endl;
        }
    }

    {
        Aws::S3::Model::PutObjectRequest request;
        request.SetBucket("awstest");      // 替换成你的存储桶名称
        request.SetKey("your-object-key"); // 替换成你希望存储的对象的键（文件名）
        // 设置上传对象的内容（二进制数据）
        std::shared_ptr<Aws::IOStream> inputData = Aws::MakeShared<Aws::StringStream>("ObjectName");
        *inputData << "Your binary data here"; // 替换成你的二进制数据

        request.SetBody(inputData);

        // 执行上传操作
        auto outcome = s3_client.PutObject(request);
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

    // 关闭AWS SDK
    Aws::ShutdownAPI(options);

    return 0;
}
