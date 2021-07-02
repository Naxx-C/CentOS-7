/*
 * @Author       : mark
 * @Date         : 2020-06-26
 * @copyleft Apache 2.0
 */ 
#include "buffer.h"

Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize), readPos_(0), writePos_(0) {}

// 缓冲区可以读取数据的长度
size_t Buffer::ReadableBytes() const {
    return writePos_ - readPos_;
}

// 缓冲区可写入数据的大小
size_t Buffer::WritableBytes() const {
    return buffer_.size() - writePos_;
}

// 缓冲区已读数据的大小（位置）
size_t Buffer::PrependableBytes() const {
    return readPos_;
}

// 缓冲区中已经读到数据的位置（读指针的位置）
const char* Buffer::Peek() const {
    return BeginPtr_() + readPos_;
}

// 将缓冲区读指针的位置移动len位
void Buffer::Retrieve(size_t len) {
    assert(len <= ReadableBytes());
    readPos_ += len;
}

// 将缓冲区读指针的位置移到end的位置
void Buffer::RetrieveUntil(const char* end) {
    assert(Peek() <= end );
    Retrieve(end - Peek());
}

// 清空缓冲区，读写指针置0
void Buffer::RetrieveAll() {
    bzero(&buffer_[0], buffer_.size());
    readPos_ = 0;
    writePos_ = 0;
}

std::string Buffer::RetrieveAllToStr() {
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
}

// 数据已经写入的位置（写指针的当前位置）
const char* Buffer::BeginWriteConst() const {
    return BeginPtr_() + writePos_;
}

// 数据已经写入的位置（写指针的当前位置）
char* Buffer::BeginWrite() {
    return BeginPtr_() + writePos_;
}

// 将写数据的位置+len
void Buffer::HasWritten(size_t len) {
    writePos_ += len;
} 

void Buffer::Append(const std::string& str) {
    Append(str.data(), str.length());
}

void Buffer::Append(const void* data, size_t len) {
    assert(data);
    Append(static_cast<const char*>(data), len);
}

void Buffer::Append(const char* str, size_t len) {
    assert(str);
    EnsureWriteable(len); // 扩容操作 
    std::copy(str, str + len, BeginWrite());
    HasWritten(len);
}

void Buffer::Append(const Buffer& buff) {
    Append(buff.Peek(), buff.ReadableBytes());
}

void Buffer::EnsureWriteable(size_t len) {
    // 如果可以写的字节空间小于需要写入的字节数，开辟新的空间
    if(WritableBytes() < len) {
        MakeSpace_(len);
    }
    assert(WritableBytes() >= len);
}

ssize_t Buffer::ReadFd(int fd, int* saveErrno) {
    char buff[65535];   // 临时的数组，保证把所有的数据都读出来
    struct iovec iov[2];
    const size_t writable = WritableBytes();
    /* 分散读， 保证数据全部读完 */
    iov[0].iov_base = BeginPtr_() + writePos_;
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const ssize_t len = readv(fd, iov, 2);
    if(len < 0) {
        *saveErrno = errno;
    }
    else if(static_cast<size_t>(len) <= writable) {
        writePos_ += len;
    }
    else {
        writePos_ = buffer_.size();
        Append(buff, len - writable); // 扩冲读缓冲区
    }
    return len;
}

ssize_t Buffer::WriteFd(int fd, int* saveErrno) {
    size_t readSize = ReadableBytes();
    ssize_t len = write(fd, Peek(), readSize);
    if(len < 0) {
        *saveErrno = errno;
        return len;
    } 
    readPos_ += len;
    return len;
}

// 储存容器开始的位置
char* Buffer::BeginPtr_() {
    return &*buffer_.begin();
}
// 储存容器开始的位置
const char* Buffer::BeginPtr_() const {
    return &*buffer_.begin();
}

// 开辟新的容器空间
void Buffer::MakeSpace_(size_t len) {
    if(WritableBytes() + PrependableBytes() < len) {
        // 如果可用的写字节空间+ 前面已读的字节空间 < 需要写入的字节数
        // 更改储存数据容器的大小
        buffer_.resize(writePos_ + len + 1);
    } 
    else {
        // 如果可用的写字节空间+ 前面已读的字节空间 > 需要写入的字节数
        // 将容器中的数据整体前移，覆盖已经读完的数据，在容器末尾留出空间，写入待写入的数据
        size_t readable = ReadableBytes();
        std::copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());
        readPos_ = 0;
        writePos_ = readPos_ + readable;
        assert(readable == ReadableBytes());
    }
}