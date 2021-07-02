/*
 * @Author       : mark
 * @Date         : 2020-06-26
 * @copyleft Apache 2.0
 */ 

#ifndef BUFFER_H
#define BUFFER_H
#include <cstring>   //perror
#include <iostream>
#include <unistd.h>  // write
#include <sys/uio.h> //readv
#include <vector> //readv
#include <atomic>
#include <assert.h>
class Buffer {
public:
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    size_t WritableBytes() const;       
    size_t ReadableBytes() const ;
    size_t PrependableBytes() const;

    const char* Peek() const;
    void EnsureWriteable(size_t len);
    void HasWritten(size_t len);

    void Retrieve(size_t len);
    void RetrieveUntil(const char* end);

    void RetrieveAll() ;
    std::string RetrieveAllToStr();

    const char* BeginWriteConst() const;
    char* BeginWrite();

    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buff);

    ssize_t ReadFd(int fd, int* Errno);
    ssize_t WriteFd(int fd, int* Errno);

private:
    char* BeginPtr_();              // 储存容器开始的位置
    const char* BeginPtr_() const;  // 储存容器开始的位置
    void MakeSpace_(size_t len);    // 开辟新的空间

    std::vector<char> buffer_; // 储存数据的容器，有读缓冲区和写缓冲区，每个缓冲区都有读、写两个指针
    std::atomic<std::size_t> readPos_; // 缓冲区读指针的位置（读指针）
    std::atomic<std::size_t> writePos_; // 缓冲区写指针的位置（写指针）
};

#endif //BUFFER_H