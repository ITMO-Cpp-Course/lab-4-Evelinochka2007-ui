#include "lab4/resource/resource.hpp"
#include <iostream>

namespace lab4::resource
{

FileHandle::FileHandle(const std::string& filename, std::ios::openmode mode) : filename_(filename)
{
    stream_.open(filename, mode);
    if (!stream_.is_open())
    {
        throw ResourceError("Failed to open file: " + filename);
    }
}

FileHandle::~FileHandle()
{
    close();
}

FileHandle::FileHandle(FileHandle&& other) noexcept
    : stream_(std::move(other.stream_)), filename_(std::move(other.filename_))
{
}

FileHandle& FileHandle::operator=(FileHandle&& other) noexcept
{
    if (this != &other)
    {
        close();
        stream_ = std::move(other.stream_);
        filename_ = std::move(other.filename_);
    }
    return *this;
}

bool FileHandle::is_open() const
{
    return stream_.is_open();
}

void FileHandle::close()
{
    if (stream_.is_open())
    {
        stream_.close();
    }
}

std::fstream& FileHandle::get_stream()
{
    if (!stream_.is_open())
    {
        throw ResourceError("FileHandle: file is not open: " + filename_);
    }
    return stream_;
}

std::string FileHandle::read_line()
{
    if (!stream_.is_open())
    {
        throw ResourceError("Cannot read from closed file: " + filename_);
    }
    std::string line;
    std::getline(stream_, line);
    if (stream_.fail() && !stream_.eof())
    {
        throw ResourceError("Read error from file: " + filename_);
    }
    return line;
}

void FileHandle::write_line(const std::string& line)
{
    if (!stream_.is_open())
    {
        throw ResourceError("Cannot write to closed file: " + filename_);
    }
    stream_ << line << '\n';
    if (stream_.fail())
    {
        throw ResourceError("Write error to file: " + filename_);
    }
}

std::shared_ptr<FileHandle> ResourceManager::get_file(const std::string& filename, std::ios::openmode mode)
{
    auto it = cache_.find(filename);
    if (it != cache_.end())
    {
        auto ptr = it->second.lock();
        if (ptr)
        {
            return ptr;
        }
        cache_.erase(it);
    }

    auto ptr = std::make_shared<FileHandle>(filename, mode);
    cache_[filename] = ptr;
    return ptr;
}

void ResourceManager::release_file(const std::string& filename)
{
    auto it = cache_.find(filename);
    if (it != cache_.end())
    {
        cache_.erase(it);
    }
}

void ResourceManager::clear()
{
    cache_.clear();
}

} // namespace lab4::resource