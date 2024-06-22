#include "split_fstream.h"

split::ifstream::ifstream(ifstream&& other) noexcept
    : infile(std::move(other.infile)),
      current_stream(other.current_stream),
      current_position(other.current_position),
      end_of_file(other.end_of_file),
      total_size(other.total_size),
      last_gcount(other.last_gcount) {}

split::ifstream& split::ifstream::operator=(ifstream&& other) noexcept {
    if (this != &other) {
        infile = std::move(other.infile);
        current_stream = other.current_stream;
        current_position = other.current_position;
        end_of_file = other.end_of_file;
        total_size = other.total_size;
        last_gcount = other.last_gcount;
    }
    return *this;
}

split::ifstream::ifstream(const std::vector<std::filesystem::path>& file_paths) {
    infile.resize(file_paths.size());
    for (size_t i = 0; i < file_paths.size(); i++) {
        infile[i].path = file_paths[i];
    }
    init_streams();
}

split::ifstream::ifstream(const std::vector<std::string>& file_paths) {
    infile.resize(file_paths.size());
    for (size_t i = 0; i < file_paths.size(); i++) {
        infile[i].path = std::filesystem::absolute(file_paths[i]);
    }
    init_streams();
}

split::ifstream::ifstream(const std::filesystem::path& file_path) {
    infile.resize(1);
    infile[0].path = file_path;
    init_streams();
}

split::ifstream::~ifstream() {
    close();
}

void split::ifstream::open(const std::vector<std::filesystem::path>& file_paths) {
    if (is_open()) {
        return;
    }
    close();
    infile.resize(file_paths.size());
    for (size_t i = 0; i < file_paths.size(); i++) {
        infile[i].path = file_paths[i];
    }
    init_streams();
}

void split::ifstream::open(const std::vector<std::string>& file_paths) {
    if (is_open()) {
        return;
    }
    close();
    infile.resize(file_paths.size());
    for (size_t i = 0; i < file_paths.size(); i++) {
        infile[i].path = std::filesystem::absolute(file_paths[i]);
    }
    init_streams();
}

void split::ifstream::open(const std::filesystem::path& file_path) {
    if (is_open()) {
        return;
    }
    close();
    infile.resize(1);
    infile[0].path = file_path;
    init_streams();
}

void split::ifstream::push_back(const std::filesystem::path& file_path) {
    StreamInfo new_stream_info = { std::ifstream(file_path, std::ios::binary), std::filesystem::file_size(file_path), std::filesystem::absolute(file_path) };
    infile.push_back(std::move(new_stream_info));
    total_size += infile.back().size;
    end_of_file = false;
}

void split::ifstream::init_streams() {
    for (auto& file : infile) {
        file.stream.open(file.path, std::ios::binary);
        if (!file.stream.is_open()) {
            throw std::runtime_error("Failed to open file: " + file.path.string());
        }

        file.size = std::filesystem::file_size(file.path);
        total_size += file.size;
    }
}

void split::ifstream::seekg(std::streampos pos, std::ios_base::seekdir dir) {
    switch (dir) {
        case std::ios_base::beg:
            current_position = pos;
            break;
        case std::ios_base::cur:
            current_position += pos;
            break;
        case std::ios_base::end:
            current_position = total_size - pos;
            break;
        default:
            throw std::invalid_argument("Invalid seek direction");
    }

    std::streamsize bytes_left = current_position;

    if (bytes_left > total_size) {
        current_stream = static_cast<unsigned int>(infile.size() - 1);
        infile[current_stream].stream.seekg(0, std::ios_base::end);
        end_of_file = infile[current_stream].stream.eof();
        return;
    }

    for (current_stream = 0; current_stream < infile.size(); ++current_stream) {
        if (bytes_left <= infile[current_stream].size) {
            infile[current_stream].stream.seekg(bytes_left, std::ios::beg);
            end_of_file = false;
            return;
        }
        bytes_left -= infile[current_stream].size;
    }
}

split::ifstream& split::ifstream::read(char* buffer, std::streamsize num_bytes) {
    std::streamsize bytes_to_read = num_bytes;
    last_gcount = 0;

    while (bytes_to_read > 0) {
        std::streamsize bytes_left_in_stream = infile[current_stream].size - infile[current_stream].stream.tellg();
        
        if (bytes_left_in_stream >= bytes_to_read) {
            // The current stream has enough bytes to fulfill the read request
            infile[current_stream].stream.read(buffer, bytes_to_read);
            last_gcount += infile[current_stream].stream.gcount();
            current_position += last_gcount;
            return *this;
        } else {
            // Current stream doesn't have enough bytes
            infile[current_stream].stream.read(buffer, bytes_left_in_stream);
            last_gcount += infile[current_stream].stream.gcount();
            current_position += bytes_left_in_stream;
            buffer += bytes_left_in_stream;
            bytes_to_read -= bytes_left_in_stream;
            current_stream++;

            if (current_stream >= infile.size()) {
                end_of_file = true;
                return *this;
            }

            infile[current_stream].stream.seekg(0, std::ios::beg);
        }
    }
    return *this;
}

std::streampos split::ifstream::tellg() {
    return current_position;
}

std::streamsize split::ifstream::gcount() const {
    return last_gcount;
}

bool split::ifstream::eof() const {
    return end_of_file;
}

bool split::ifstream::operator!() {
    return !good();
}

bool split::ifstream::is_open() const {
    return std::all_of(infile.begin(), infile.end(), [](const StreamInfo& si) { return si.stream.is_open(); });
}

bool split::ifstream::fail() const {
    return std::any_of(infile.begin(), infile.end(), [](const StreamInfo& si) { return si.stream.fail(); });
}

bool split::ifstream::bad() const {
    return std::any_of(infile.begin(), infile.end(), [](const StreamInfo& si) { return si.stream.bad(); });
}

bool split::ifstream::good() const {
    return std::all_of(infile.begin(), infile.end(), [](const StreamInfo& si) { return si.stream.good(); });
}

void split::ifstream::clear() {
    for (auto& file : infile) {
        file.stream.clear();
    }
}

void split::ifstream::close() {
    for (auto& file : infile) {
        if (file.stream.is_open()) {
            file.stream.close();
        }
    }
    infile.clear();
    current_stream = 0;
    current_position = 0;
    last_gcount = 0;
    end_of_file = false;
}