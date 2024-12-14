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

path operator""_p(const char* data, std::size_t sz) {
    return path(data, data + sz);
}

bool Preprocess(const path& in_file, const path& out_file, const vector<path>& include_directories){
    static regex include_reg1(R"/(\s*#\s*include\s*"([^"]*)"\s*)/");
    static regex include_reg2(R"/(\s*#\s*include\s*<([^>]*)>\s*)/");
    smatch m;

    ifstream in(in_file, ios::binary);
    if (!in) {
        return false;
    }

    path p = in_file.parent_path();
    std::string line;
    int line_number = 0;

    if(in.is_open()) {
        ofstream out(out_file, ios::binary | ios::app);
        while(getline(in, line)){
            ++line_number;
            int priznak_regexp = 0;
            path p_include;
            if(regex_match(line, m, include_reg1)){
                priznak_regexp = 1;
                p_include = p / string(m[1]);
            } else if (regex_match(line, m, include_reg2)){
                priznak_regexp = 2;
                p_include = p / string(m[1]);
            }

            if(priznak_regexp == 1 && filesystem::exists(p_include)){
                Preprocess(p_include, out_file, include_directories);
            } else if((priznak_regexp == 1 && !filesystem::exists(p_include))
                       || priznak_regexp == 2){
                bool nalichie = false;
                for(const auto& dir : include_directories){
                    if(filesystem::exists(dir)){
                        for(const auto& entry
                             : filesystem::recursive_directory_iterator(dir)){
                            path p1 = entry.path();
                            if(p1.filename() == p_include.filename()){
                                ifstream in_p1(p1, ios::binary);
                                if(in_p1){
                                    nalichie = true;
                                    Preprocess(p1, out_file, include_directories);
                                }
                            }
                        }
                    }

                }
                if (!nalichie) {
                    cout << "unknown include file "s
                         << p_include.filename().string()
                         << " at file "s
                         << in_file.string()
                         << " at line "
                         << line_number << endl;
                    return false;
                }
            } else{
                out << line << endl;
            }
        }
        out.close();
    }
    in.close();
    return true;
}

string GetFileContents(string file) {
    ifstream stream(file);
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
