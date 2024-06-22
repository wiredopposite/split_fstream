#include <cmath>
#include <iomanip>
#include <sstream>

#include "split_fstream.h"

split::ofstream::ofstream(const std::filesystem::path &_Path, const std::streamsize &_Maxsize)
    :   parent_path(_Path.parent_path()),
        file_stem(_Path.stem().string()), 
        file_ext(_Path.extension().string()),
        max_filesize(_Maxsize), 
        current_stream(0), 
        current_position(0) {

    open_new_stream();
}

split::ofstream::ofstream(ofstream&& other) noexcept
    : outfile(std::move(other.outfile)),
      parent_path(std::move(other.parent_path)),
      file_stem(std::move(other.file_stem)),
      file_ext(std::move(other.file_ext)),
      max_filesize(other.max_filesize),
      current_stream(other.current_stream),
      current_position(other.current_position) {
}

split::ofstream& split::ofstream::operator=(ofstream&& other) noexcept {
    if (this != &other) {
        close();
        outfile = std::move(other.outfile);
        parent_path = std::move(other.parent_path);
        file_stem = std::move(other.file_stem);
        file_ext = std::move(other.file_ext);
        max_filesize = other.max_filesize;
        current_stream = other.current_stream;
        current_position = other.current_position;
    }
    return *this;
}

split::ofstream::~ofstream() {
    close();
    clean_all();
}

void split::ofstream::open(const std::filesystem::path &_Path, const std::streamsize &_Maxsize) {
    if (is_open()) {
        return;
    }
    parent_path = _Path.parent_path();
    file_stem = _Path.stem().string(); 
    file_ext = _Path.extension().string();
    max_filesize = _Maxsize;
    current_stream = 0;
    current_position = 0;
    open_new_stream();
}

split::ofstream& split::ofstream::seekp(std::streampos _Off, std::ios_base::seekdir _Way) {
    switch (_Way) {
        case std::ios_base::beg:
            current_position = _Off;
            break;
        case std::ios_base::cur:
            current_position += _Off;
            break;
        case std::ios_base::end: {
            outfile.back().stream.seekp(0, std::ios::end);
            std::streamsize last_pos = (outfile.size() - 1) * max_filesize + outfile.back().stream.tellp();
            current_position = (_Off > 0) ? last_pos : (last_pos + _Off);
            break;
        }
        default:
            throw std::invalid_argument("Invalid seek direction");
    }

    std::streamsize bytes_left = current_position;

    for (current_stream = 0; current_stream < outfile.size(); ++current_stream) {
        if (bytes_left <= max_filesize) {
            outfile[current_stream].stream.seekp(bytes_left, std::ios::beg);
            break;
        } else {
            bytes_left -= max_filesize;
        }
    }
    return *this;
}

split::ofstream& split::ofstream::write(const char* _Str, std::streamsize _Count) {
    while (_Count > 0) {
        std::streamsize bytes_left = max_filesize - outfile[current_stream].stream.tellp();

        if (bytes_left <= 0) {
            current_stream++;
            if (current_stream >= outfile.size()) {
                open_new_stream();
            }
            continue;
        }

        std::streamsize to_write = std::min(_Count, bytes_left);
        outfile[current_stream].stream.write(_Str, to_write);
        _Str += to_write;
        _Count -= to_write;
        current_position += to_write;
    }
    return *this;
}

std::streampos split::ofstream::tellp() {
    return current_position;
}

bool split::ofstream::operator!() {
    return !good();
}

bool split::ofstream::is_open() const {
    return std::any_of(outfile.begin(), outfile.end(), [](const StreamInfo& si) { return si.stream.is_open(); });
}

bool split::ofstream::fail() const {
    return std::any_of(outfile.begin(), outfile.end(), [](const StreamInfo& si) { return si.stream.fail(); });
}

bool split::ofstream::bad() const {
    return std::any_of(outfile.begin(), outfile.end(), [](const StreamInfo& si) { return si.stream.bad(); });
}

bool split::ofstream::good() const {
    return std::all_of(outfile.begin(), outfile.end(), [](const StreamInfo& si) { return si.stream.good(); });
}

void split::ofstream::clear() {
    for (auto& file : outfile) {
        file.stream.clear();
    }
}

void split::ofstream::close() {
    for (auto& file : outfile) {
        file.stream.close();
    }
    rename_output_files();
}

void split::ofstream::clean_all() {
    for (auto& file : outfile) {
        if (file.stream.is_open()) {
            file.stream.close();
        }
    }
    outfile.clear();
    current_stream = 0;
    current_position = 0;
    max_filesize = UINT64_MAX;
    parent_path.clear();
    file_stem.clear();
    file_ext.clear();
}

std::filesystem::path split::ofstream::get_next_filepath() {
    return parent_path / (file_stem + "." + std::to_string(current_stream + 1) + file_ext);
}

void split::ofstream::open_new_stream() {
    std::filesystem::path filepath = get_next_filepath();
    outfile.push_back({ std::ofstream(filepath, std::ios::binary), filepath, current_stream });
}

void split::ofstream::rename_output_files() {
    if (outfile.size() < 10) {
        return;
    }
    int digits = num_digits(static_cast<int>(outfile.size()));

    for (auto& file : outfile) {
        bool err = false;
        std::filesystem::path new_path = parent_path / (file_stem + "." + pad_digits(file.index + 1, digits) + file_ext);
        try {
            std::filesystem::rename(file.path, new_path);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            err = true;
        }
        if (!err) {
            file.path = new_path;
        }
    }
}

int split::ofstream::num_digits(int number) {
    if (number == 0) {
        return 1;
    }
    return static_cast<int>(std::log10(std::abs(number))) + 1;
}

std::string split::ofstream::pad_digits(int number, int width) {
    std::ostringstream oss;
    oss << std::setw(width) << std::setfill('0') << number;
    return oss.str();
}



split::PathsWrapper split::ofstream::paths() const {
    std::vector<std::filesystem::path> paths;
    for (const auto& file : outfile) {
        paths.push_back(file.path);
    }
    return PathsWrapper(paths);
}

std::vector<std::string> split::PathsWrapper::string() const {
    std::vector<std::string> str_paths;
    str_paths.reserve(paths_.size());
    for (const auto& path : paths_) {
        str_paths.push_back(path.string());
    }
    return str_paths;
}