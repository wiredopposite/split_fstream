#ifndef _SPLIT_FSTREAM_H_
#define _SPLIT_FSTREAM_H_

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

namespace split {

class PathsWrapper {
public:
    explicit PathsWrapper(const std::vector<std::filesystem::path>& paths) : paths_(paths) {}
    operator std::vector<std::filesystem::path>() const { return paths_; }
    std::vector<std::string> string() const;

private:
    std::vector<std::filesystem::path> paths_;
};

class ofstream {
public:
    ofstream() {};
    ofstream(const std::filesystem::path &_Path, const std::streamsize &_Maxsize = UINT64_MAX);
    ofstream(ofstream&& other) noexcept;
    ofstream& operator=(ofstream&& other) noexcept;
    ~ofstream();

    bool operator!();

    void open(const std::filesystem::path &_Path, const std::streamsize &_Maxsize = UINT64_MAX);
    ofstream& seekp(std::streampos _Off, std::ios_base::seekdir _Way = std::ios_base::beg);
    ofstream& write(const char* _Str, std::streamsize _Count);
    std::streampos tellp();

    bool is_open() const;
    bool fail() const;
    bool bad() const;
    bool good() const;
    void close();
    void clear();

    PathsWrapper paths() const;
    
private:
    struct StreamInfo {
        std::ofstream stream;
        std::filesystem::path path;
        unsigned int index;
    };

    std::vector<StreamInfo> outfile;

    std::string file_stem;
    std::string file_ext;
    std::filesystem::path parent_path;

    unsigned int current_stream{0};
    std::streampos current_position{0};
    std::streampos max_filesize{UINT64_MAX};
    
    std::filesystem::path get_next_filepath();
    void open_new_stream();
    void rename_output_files();
    int num_digits(int number);
    std::string pad_digits(int number, int width);
    void clean_all();
};

class ifstream {
public:
    ifstream() {};
    ifstream(ifstream&& other) noexcept;
    ifstream& operator=(ifstream&& other) noexcept;
    ifstream(const std::vector<std::string>& file_paths);
    ifstream(const std::vector<std::filesystem::path>& file_paths);
    ifstream(const std::filesystem::path& file_path);
    ~ifstream();

    bool operator!();

    void open(const std::vector<std::string>& file_paths);
    void open(const std::vector<std::filesystem::path>& file_paths);
    void open(const std::filesystem::path& file_path);

    void push_back(const std::filesystem::path& file_path);
    void close();
    bool is_open() const;
    std::streampos tellg();
    void seekg(std::streampos pos, std::ios_base::seekdir dir = std::ios_base::cur);
    ifstream& read(char* buffer, std::streamsize num_bytes);
    std::streamsize gcount() const;

    bool eof() const;
    bool fail() const;
    bool bad() const;
    bool good() const;
    void clear();

private:
    void init_streams();

    struct StreamInfo {
        std::ifstream stream;
        std::streamsize size;
        std::filesystem::path path;
    };

    std::vector<StreamInfo> infile;
    std::streamsize total_size{0};
    unsigned int current_stream{0};
    std::streampos current_position{0};
    std::streamsize last_gcount{0};
    bool end_of_file{false};
};

}; // namespace split

#endif // _SPLIT_FSTREAM_H_