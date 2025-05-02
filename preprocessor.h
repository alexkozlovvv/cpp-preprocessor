#pragma once

#include <filesystem>
#include <vector>

using namespace std;
using filesystem::path;

class Preprocessor {
public:
    Preprocessor(const path& path_to_in_file, const path& path_to_out_file, const vector<path>& include_directories);
    void Preprocess();
private:
    ifstream input_file_{};
    ofstream output_file_{};
    const path& path_to_in_file_;
    const vector<path>& include_directories_;
    
    void RecursivePreprocess(const path& path_to_in_file, ifstream& input_file);
};