#include <filesystem>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>

#include "split_fstream.h"

bool compare_files_checksum(const std::string& file_path1, const std::string& file_path2);

void print_progress(uint64_t bytes_left, uint64_t total_size) {
    double progress = 100.0 * (static_cast<double>(total_size - bytes_left) / total_size);
    std::cerr << std::fixed << std::setprecision(2);
    std::cerr << "Progress: " << progress << "%" << "\r";
    if (bytes_left == 0) {
        std::cerr << std::endl;
    }
}

void generate_random_file(const std::filesystem::path& file_path, uint64_t size) {
    std::cerr << "Generating random file: " << file_path << std::endl;

    std::ofstream fout(file_path, std::ios::binary);
    if (!fout.is_open()) {
        throw std::runtime_error("Could not open file: " + file_path.string());
    }

    std::vector<char> buffer(2048);
    uint64_t bytes_left = size;

    while (bytes_left > 0) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        for (uint64_t i = 0; i < buffer.size(); ++i) {
            buffer[i] = static_cast<char>(dis(gen));
        }

        uint64_t bytes_to_write = std::min(bytes_left, static_cast<uint64_t>(buffer.size()));
        fout.write(buffer.data(), bytes_to_write);
        if (fout.fail()) {
            throw std::runtime_error("Failed to write to file: " + file_path.string());
        }

        bytes_left -= bytes_to_write;

        print_progress(bytes_left, size);
    }
}

int main() {
    std::filesystem::path random_file = "random.bin";
    generate_random_file(random_file, 1024 * 1024 * 100); // 100 MB

    uint64_t bytes_left = std::filesystem::file_size(random_file);
    uint64_t fin_size = bytes_left;
    std::ifstream fin(random_file, std::ios::binary);
    if (!fin.is_open()) {
        throw std::runtime_error("Could not open file: " + random_file.string());
    }
    
    split::ofstream split_fout("random_split.bin", 1024 * 1024 * 10); // 10 MB
    if (!split_fout.is_open()) {
        throw std::runtime_error("Could not open split file.");
    }

    std::vector<char> buffer(2048);

    uint64_t split_fout_pos = 0;
    std::cerr << "Writing split files..." << std::endl;

    while (bytes_left > 0) {
        fin.read(buffer.data(), std::min(bytes_left, static_cast<uint64_t>(buffer.size())));
        if (fin.fail()) {
            throw std::runtime_error("Failed to read from file: " + random_file.string());
        }

        bytes_left -= fin.gcount();

        split_fout.seekp(split_fout_pos, std::ios::beg);
        split_fout.write(buffer.data(), fin.gcount());
        if (split_fout.fail()) {
            throw std::runtime_error("Failed to write to split file.");
        }
        split_fout_pos = split_fout.tellp();
        // test if seekp and tellp are updating the position correctly
        split_fout.seekp(0, std::ios::end);
        split_fout.seekp(0, std::ios::beg);
        if (split_fout.fail()) {
            throw std::runtime_error("Failed to seek to position in split file.");
        }
        
        print_progress(bytes_left, fin_size);
    }

    fin.close();
    split_fout.close();

    std::vector<std::filesystem::path> split_files = split_fout.paths();

    split::ifstream split_fin(split_files);
    if (!split_fin.is_open()) {
        throw std::runtime_error("Could not open split file.");
    }
    
    std::filesystem::path random_file_recon = "random_reconstructed.bin";
    std::ofstream fout(random_file_recon, std::ios::binary);
    if (!fout.is_open()) {
        throw std::runtime_error("Could not open file: " + random_file_recon.string());
    }

    bytes_left = split_fin.size();
    std::cerr << "Original file size: " << fin_size << " Recombined file size: " << bytes_left << std::endl;
    if (fin_size != bytes_left) {
        throw std::runtime_error("File sizes do not match.");
    }

    uint64_t split_fin_pos = 0;
    std::cerr << "Reconstructing file..." << std::endl;

    while (bytes_left > 0) {
        split_fin.seekg(split_fin_pos, std::ios::beg);
        split_fin.read(buffer.data(), std::min(bytes_left, static_cast<uint64_t>(buffer.size())));
        if (split_fin.fail()) {
            throw std::runtime_error("Failed to read from split file.");
        }

        bytes_left -= split_fin.gcount();

        split_fin_pos = split_fin.tellg();
        split_fin.seekg(0, std::ios::end);
        split_fin.seekg(0, std::ios::beg);
        if (split_fin.fail()) {
            throw std::runtime_error("Failed to seek to position in split file.");
        }

        fout.write(buffer.data(), split_fin.gcount());
        if (fout.fail()) {
            throw std::runtime_error("Failed to write to file: " + random_file_recon.string());
        }

        print_progress(bytes_left, fin_size);
    }

    split_fin.close();
    fout.close();

    std::cerr << "Files written, calculating checksums..." << std::endl;

    if (compare_files_checksum(random_file.string(), random_file_recon.string())) {
        std::cout << "Files have the same checksum." << std::endl;
    } else {
        throw std::runtime_error("Files have different checksums.");
    }

    std::cerr << "Cleaning up..." << std::endl;
    split_files.push_back(random_file_recon);
    split_files.push_back(random_file);
    for (auto& file : split_files) {
        try {
            if (std::filesystem::exists(file)) {
                std::filesystem::remove(file);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    std::cerr << "Done." << std::endl;
}