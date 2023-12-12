#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using filesystem::path;

static regex local_include(R"/(\s*#\s*include\s*"([^"]*)"\s*)/");
static regex default_include(R"/(\s*#\s*include\s*<([^>]*)>\s*)/");

path operator""_p(const char* data, std::size_t sz) {
    return path(data, data + sz);
}

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

bool RecursivePreprocess(const path& path_to_in_file, ifstream& input_file, ofstream& output_file, const vector<path>& include_directories) {
    
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
                if(!SearchDefaultHeader(include_directories, include_path, path_to_header, header)){
                    cout << "unknown include file "s << include_path.string() << 
                            " at file "s << path_to_in_file.string() << " at line "s << line_count << endl;
                    return false;
                }
            }
            if(!RecursivePreprocess(path_to_header, header, output_file, include_directories)) {
                return false;
            };

            continue;

        } else if (regex_match( temp_line, result_of_match, default_include)) {

            include_path = string(result_of_match[1]);
            if(!SearchDefaultHeader(include_directories, include_path, path_to_header, header)){
                cout << "unknown include file "s << include_path.string() << 
                        " at file "s << path_to_in_file.string() << " at line "s << line_count << endl;
                return false;
            }
            if(!RecursivePreprocess(path_to_header, header, output_file, include_directories)) {
                return false;
            }
            continue;
        }
        
        output_file << temp_line << endl;
    }
    input_file.close();
    return true;
}

bool Preprocess(const path& path_to_in_file, const path& path_to_out_file, const vector<path>& include_directories) {
    
    ifstream input_file;
    input_file.open(path_to_in_file.string());

    if(!input_file.is_open()) {
        return false;
    }

    ofstream output_file;
    output_file.open(path_to_out_file.string());

    if(!output_file.is_open()) {
        return false;
    }

    return RecursivePreprocess(path_to_in_file, input_file, output_file, include_directories);

}

string GetFileContents(string file) {
    ifstream stream(file);

    // конструируем string по двум итераторам
    return {(istreambuf_iterator<char>(stream)), istreambuf_iterator<char>()};
}

void Test() {
    error_code err;
    filesystem::remove_all("sources"_p, err);
    filesystem::create_directories("sources"_p / "include2"_p / "lib"_p, err);
    filesystem::create_directories("sources"_p / "include1"_p, err);
    filesystem::create_directories("sources"_p / "dir1"_p / "subdir"_p, err);

    {
        ofstream file("sources/a.cpp");
        file << "// this comment before include\n"
                "#include \"dir1/b.h\"\n"
                "// text between b.h and c.h\n"
                "#include \"dir1/d.h\"\n"
                "\n"
                "int SayHello() {\n"
                "    cout << \"hello, world!\" << endl;\n"
                "#   include<dummy.txt>\n"
                "}\n"s;
    }
    {
        ofstream file("sources/dir1/b.h");
        file << "// text from b.h before include\n"
                "#include \"subdir/c.h\"\n"
                "// text from b.h after include"s;
    }
    {
        ofstream file("sources/dir1/subdir/c.h");
        file << "// text from c.h before include\n"
                "#include <std1.h>\n"
                "// text from c.h after include\n"s;
    }
    {
        ofstream file("sources/dir1/d.h");
        file << "// text from d.h before include\n"
                "#include \"lib/std2.h\"\n"
                "// text from d.h after include\n"s;
    }
    {
        ofstream file("sources/include1/std1.h");
        file << "// std1\n"s;
    }
    {
        ofstream file("sources/include2/lib/std2.h");
        file << "// std2\n"s;
    }

    assert((!Preprocess("sources"_p / "a.cpp"_p, "sources"_p / "a.in"_p,
                                  {"sources"_p / "include1"_p,"sources"_p / "include2"_p})));

    ostringstream test_out;
    test_out << "// this comment before include\n"
                "// text from b.h before include\n"
                "// text from c.h before include\n"
                "// std1\n"
                "// text from c.h after include\n"
                "// text from b.h after include\n"
                "// text between b.h and c.h\n"
                "// text from d.h before include\n"
                "// std2\n"
                "// text from d.h after include\n"
                "\n"
                "int SayHello() {\n"
                "    cout << \"hello, world!\" << endl;\n"s;

    assert(GetFileContents("sources/a.in"s) == test_out.str());
}

int main() {
    Test();
}
