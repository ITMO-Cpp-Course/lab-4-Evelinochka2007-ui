#pragma once

#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace lab4::resource
{

class ResourceError : public std::runtime_error
{
  public:
    explicit ResourceError(const std::string& msg) : std::runtime_error(msg) {}
};

class FileHandle
{
  public:
    FileHandle(const std::string& filename, std::ios::openmode mode = std::ios::in);
    ~FileHandle();

    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    FileHandle(FileHandle&& other) noexcept;
    FileHandle& operator=(FileHandle&& other) noexcept;

    bool is_open() const;
    void close();
    std::fstream& get_stream();
    std::string read_line();
    void write_line(const std::string& line);

  private:
    std::fstream stream_;
    std::string filename_;
};

class ResourceManager
{
  public:
    std::shared_ptr<FileHandle> get_file(const std::string& filename, std::ios::openmode mode = std::ios::in);
    void release_file(const std::string& filename);
    void clear();

  private:
    std::unordered_map<std::string, std::weak_ptr<FileHandle>> cache_;
};

} // namespace lab4::resource