#include "lab4/resource/resource.hpp"
#include <catch2/catch_all.hpp>
#include <filesystem>
#include <fstream>

using namespace lab4::resource;

void create_test_file(const std::string& name, const std::string& content)
{
    std::ofstream f(name);
    f << content;
    f.close();
}

void remove_test_file(const std::string& name)
{
    std::error_code ec;
    std::filesystem::remove(name, ec);
}

TEST_CASE("ResourceError can be thrown and caught", "[error]")
{
    REQUIRE_THROWS_AS(throw ResourceError("test"), ResourceError);
    try
    {
        throw ResourceError("some error");
    }
    catch (const ResourceError& e)
    {
        REQUIRE(std::string(e.what()) == "some error");
    }
}

TEST_CASE("FileHandle opens and closes file", "[file]")
{
    const std::string filename = "test_open_close.txt";
    create_test_file(filename, "hello");
    {
        FileHandle fh(filename, std::ios::in);
        REQUIRE(fh.is_open() == true);
    }
    remove_test_file(filename);
}

TEST_CASE("FileHandle throws if cannot open", "[file]")
{
    REQUIRE_THROWS_AS(FileHandle("nonexistent_file_xyz.txt", std::ios::in), ResourceError);
}

TEST_CASE("FileHandle move semantics", "[file]")
{
    const std::string filename = "test_move.txt";
    create_test_file(filename, "move test");
    FileHandle fh1(filename, std::ios::in);
    REQUIRE(fh1.is_open() == true);
    FileHandle fh2 = std::move(fh1);
    REQUIRE(fh1.is_open() == false);
    REQUIRE(fh2.is_open() == true);
    remove_test_file(filename);
}

TEST_CASE("FileHandle read_line works", "[file]")
{
    const std::string filename = "test_read.txt";
    create_test_file(filename, "line1\nline2\nline3");
    FileHandle fh(filename, std::ios::in);
    REQUIRE(fh.read_line() == "line1");
    REQUIRE(fh.read_line() == "line2");
    REQUIRE(fh.read_line() == "line3");
    remove_test_file(filename);
}

TEST_CASE("FileHandle write_line works", "[file]")
{
    const std::string filename = "test_write.txt";
    {
        FileHandle fh(filename, std::ios::out);
        fh.write_line("alpha");
        fh.write_line("beta");
    }
    std::ifstream in(filename);
    std::string line;
    std::getline(in, line);
    REQUIRE(line == "alpha");
    std::getline(in, line);
    REQUIRE(line == "beta");
    remove_test_file(filename);
}

TEST_CASE("ResourceManager returns same shared_ptr for same file", "[manager]")
{
    const std::string filename = "test_cache.txt";
    create_test_file(filename, "cache test");
    ResourceManager rm;
    auto p1 = rm.get_file(filename);
    auto p2 = rm.get_file(filename);
    REQUIRE(p1 == p2);
    REQUIRE(p1.use_count() >= 2);
    remove_test_file(filename);
}

TEST_CASE("ResourceManager releases file when all shared_ptr go out of scope", "[manager]")
{
    const std::string filename = "test_release.txt";
    create_test_file(filename, "release test");
    ResourceManager rm;
    std::weak_ptr<FileHandle> weak;
    {
        auto p = rm.get_file(filename);
        weak = p;
        REQUIRE(weak.expired() == false);
    }
    REQUIRE(weak.expired() == true);
    auto p2 = rm.get_file(filename);
    REQUIRE(p2 != nullptr);
    remove_test_file(filename);
}

TEST_CASE("ResourceManager release_file removes from cache", "[manager]")
{
    const std::string filename = "test_remove.txt";
    create_test_file(filename, "remove test");
    ResourceManager rm;
    auto p1 = rm.get_file(filename);
    rm.release_file(filename);
    auto p2 = rm.get_file(filename);
    REQUIRE(p1 != p2);
    remove_test_file(filename);
}