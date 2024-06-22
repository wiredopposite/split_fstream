# split_fstream
This is still being tested, use at your own peril!

An `std::ifstream`/`std::ofstream` wrapper meant for reading and writing split binary files as you would a normal file stream. This was created to more easily work with files on filesystems where size limits are an issue, but there are bound to be a few more cases where it could be useful. 

## split::ofstream
Construct with a filepath and a max size to split the files at. 
```cpp
split::ofstream fout("file.bin", UINT32_MAX);
```
Or omit the max size parameter if you don't want to split the file (sets max size to UINT64_MAX)
```cpp
split::ofstream fout("file.bin");
```
You can also declare the stream and construct it later:
```cpp
split::ofstream fout;
if (something) {
    fout = split::ofstream(path, max_size);
} else {
    fout = split::ofstream(other_path, max_size);
}
```
The output filename will have its extension prepended with the number of the file, indexed at 1: `filename.number.extension`

Once `fout.close()` is called, it will determine if the subextension needs to be zero padded. Say you've created 100 files, `filename.1.ext` will be renamed to `filename.001.ext`

After this you can call `fout.paths()` or `fout.paths().string()` which will return a vector of all the output paths created:
```cpp
std::vector<std::filesystem::path> fout_paths = fout.paths();
std::vector<std::string> fout_str_paths = fout.paths().string();
```

## split::ifstream
Construct with a vector of filepaths, or a single one, it's as many as you'd like, though all streams will remain open until `fin.close()` or the class's destructor is called so keep that in mind.
```cpp
std::vector<std::filesystem::path> in_paths = {
    "file.1.bin",
    "file.2.bin",
    "file.3.bin",
    "file.4.bin"
};

split::ifstream fin(in_paths);
```
You can also use the `fin.push_pack()` operator to append files to the end of the stream later if needed:
```cpp
split::ifstream fin("file.1.bin");

if (found_file2) {
    fin.push_back("file.2.bin");
}
```
Like `split::ofstream`, `split::ifstream` can be constructed after being declared. 