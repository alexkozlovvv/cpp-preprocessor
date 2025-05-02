#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <vector>
#include <string>
#include "preprocessor.h"

namespace {

    regex local_include(R"/(\s*#\s*include\s*"([^"]*)"\s*)/");
    regex default_include(R"/(\s*#\s*include\s*<([^>]*)>\s*)/");

    bool OpenHeader(const path& path_to_header, ifstream& header ) {

        header.open(path_to_header.string());
        if(!header.is_open()) {
            return false;
        }
        return true;
    }

    bool SearchDefaultHeader(const vector<path>& include_directories, const path& include_path, path& path_to_header,
                                ifstream& header_file) {

        error_code   err;

        for(const path& directory : include_directories) {
            path_to_header = directory / include_path;
            auto status = filesystem::status(path_to_header, err);

            if(!err && (status.type() == filesystem::file_type::regular)) {
                return OpenHeader(path_to_header, header_file);
            }
        }
        return false;
    }

    bool SearchLocalHeader(const path& path_to_in_file, const path& include_path, path& path_to_header,
                            ifstream& header_file) {

        error_code   err;

        path_to_header = path_to_in_file.parent_path() / include_path;
                
        auto status = filesystem::status(path_to_header, err);

        if(!err && (status.type() == filesystem::file_type::regular)) {
            return OpenHeader(path_to_header, header_file);
        }
        return false;
    }
}

Preprocessor::Preprocessor(const path& path_to_in_file, const path& path_to_out_file, const vector<path>& include_directories) :
    path_to_in_file_(path_to_in_file), include_directories_(include_directories)
{
    
    input_file_.open(path_to_in_file.string());

    if(!input_file_.is_open()) {
        throw runtime_error("error to open input file " + path_to_in_file.string());
    }

    output_file_.open(path_to_out_file.string());

    if(!output_file_.is_open()) {
        throw runtime_error("error to open output file " + path_to_in_file.string());
    }

}

void Preprocessor::Preprocess() {
    RecursivePreprocess(path_to_in_file_, input_file_);
}

void Preprocessor::RecursivePreprocess(const path& path_to_in_file, ifstream& input_file) {
    
    error_code   err;
    string temp_line;
    smatch result_of_match;
    path include_path;
    path path_to_header;
    ifstream header;
    int line_count = 0;

    while(getline(input_file, temp_line)) {

        ++line_count;
    
        if (regex_match( temp_line, result_of_match, local_include)) {

            include_path = string(result_of_match[1]);
            if(!SearchLocalHeader(path_to_in_file, include_path, path_to_header, header)) {
                if(!SearchDefaultHeader(include_directories_, include_path, path_to_header, header)){
                    throw(runtime_error("unknown include file "s + include_path.string() + 
                    " at file "s + path_to_in_file.string() + " at line "s + to_string(line_count)));
                }
            }
            RecursivePreprocess(path_to_header, header);
            continue;

        } else if (regex_match( temp_line, result_of_match, default_include)) {

            include_path = string(result_of_match[1]);
            if(!SearchDefaultHeader(include_directories_, include_path, path_to_header, header)){
                throw(runtime_error("unknown include file " + include_path.string() + 
                    " at file "s + path_to_in_file.string() + " at line "s + to_string(line_count)));
            }
            RecursivePreprocess(path_to_header, header);
            continue;
        }
        output_file_ << temp_line << endl;
    }
    input_file.close();
}