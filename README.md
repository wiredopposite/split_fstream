# split_fstream
An ifstream/ofstream wrapper meant for reading and writing split binary files as you would a normal file stream.

## split::ofstream
Pass a filepath and a max file size to split the files at. The output filename will have its extension prepended with the number of the file, indexed at 1: filename.number.extension

Once fout.close() is called, it will determine if the subextension needs to be padded. Say you've created 100 files, filename.1.ext will be renamed to filename.001.ext
After this you can call fout.fs_paths() or fout.str_paths() which will return a vector with all the output paths created.

If you'd like to use the class in way that won't split your files, you can pass some incredibly high number as the max size, UINT64_MAX for instance.

## split::ifstream
Pass a vector of filepaths to open, this can be as many as you'd like, though all streams remain open until fin.close() or the class's destructor is called so keep that in mind.