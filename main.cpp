#include <iostream>
#include <fstream>
#include <filesystem>
#include <cctype>

#include "lib/TinyEXIF.h"

namespace fs = std::filesystem;

int sortDirectory(const fs::path& pathToSort);
void moveFile(const fs::path& old_path, const fs::path& new_path);

int main(int argc, char *argv[])
{
    // Russian language support
    setlocale(LC_ALL, "Russian");
    system("chcp 1251");

    fs::path pathToSort;
    if (argc <= 1) {
        // Use executable path if other is not specified
        pathToSort = fs::path(argv[0]).remove_filename();
    } else {
        std::string path_str = argv[1];
        // Get rid of possible extra symbol
        if (path_str.back() == '\"') {
            path_str.pop_back();
        }
        pathToSort = fs::path(path_str);
    }
    std::cout << "Sort directory " << pathToSort << " ?" << std::endl;
    std::cout << "[Y/n] + Enter" << std::endl;

    if (std::tolower(std::cin.get()) != 'y') {
        std::cout << "Sorting cancelled";
        return EXIT_FAILURE;
    }

    try {
        sortDirectory(pathToSort);
    }
    catch (std::exception& e) {
        std::cerr << std::endl << "ERROR: " << e.what();
    }

    std::cin.get();
    std::cin.get();
    return EXIT_SUCCESS;
}

int sortDirectory(const fs::path& pathToSort)
{
    // For every entry in sortable directory
    for(auto& dir_entry : fs::recursive_directory_iterator(pathToSort)) {
        if (fs::is_regular_file(dir_entry)) {
            const fs::path& cur_file_path = dir_entry.path();

            std::cout << cur_file_path.filename() << " --- ";

            //Open file and parse EXIF data
            std::ifstream file(cur_file_path, std::ios::binary);
            TinyEXIF::EXIFInfo fileEXIF(file);
            file.close();

            // If data info can't be found, move file to "Undefined" folder
            if (!fileEXIF.Fields) {
                std::cout << "No EXIF fields";
                fs::path new_path = pathToSort / "Undefined" / cur_file_path.filename();

                moveFile(cur_file_path, new_path);
            }
            else {
                std::cout << fileEXIF.DateTime;
            }
            std::cout << std::endl;
        }
    }
    return EXIT_SUCCESS;
}

// Move or rename file with creating required directories
void moveFile(const fs::path& old_path, const fs::path& new_path)
{
    if (!fs::exists(new_path.parent_path())) {
        if (!fs::create_directories(new_path.parent_path())) {
            throw std::runtime_error("Unable to create directory");
        }
    }
    fs::rename(old_path, new_path);
}