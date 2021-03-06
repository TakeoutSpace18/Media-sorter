#pragma once
#include <iostream>
#include <filesystem>
#include <cctype>
#include <optional>
#include <string>
#include <Windows.h>

namespace fs = std::filesystem;

struct FileInfo
{
    std::string year;
    std::string month;
    std::string preferableFilename;
};

int sortDirectory(const fs::path& pathToSort);
void moveFile(const fs::path& old_path, const fs::path& new_path);
std::optional<FileInfo> getFileInfo(const fs::path& file_path);

const std::string g_daysOfWeek[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const std::string g_months[12] = {
        "January",
        "February",
        "March",
        "April",
        "May",
        "June",
        "Jule",
        "August",
        "September",
        "October",
        "November",
        "December"
};

int main(int argc, char *argv[])
{
    // Russian language support
    setlocale(LC_ALL, "Russian");

    fs::path pathToSort;
    if (argc <= 1) {
        // Use executable path if other is not specified
        pathToSort = fs::path(argv[0]).remove_filename();
        std::cout << "Usage: Media_sorter.exe \"path to sort\"" << std::endl;
        std::cout << "Executable path is set by default." << std::endl << std::endl;
    } else {
        std::string path_str = argv[1];
        // Get rid of possible extra symbol
        if (path_str.back() == '\"') {
            path_str.pop_back();
        }
        pathToSort = fs::path(path_str);
    }
    std::cout << R"(This program will rename all files in specified directory by "YYYY-MONTH-DD_DayOfWeek" template)" << std::endl;
    std::cout << R"(and sort them by last write date, moving to "year" and "month" folders.)" << std::endl;
    std::cout << R"(All remaining empty directories will be deleted.)" << std::endl << std::endl;
    std::cout << "Do you want to sort directory " << pathToSort << " ?" << std::endl;
    std::cout << "[Y/n] + Enter" << std::endl;

    if (std::tolower(std::cin.get()) != 'y') {
        std::cout << std::endl << "Sorting cancelled" << std::endl;;
    } else {
        try {
            sortDirectory(pathToSort);
        }
        catch (std::exception& e) {
            std::cerr << std::endl << "ERROR: " << e.what();
        }
    }

    std::cin.clear();
    std::cin.ignore(32767, '\n');
    std::cin.get();
    return EXIT_SUCCESS;
}

int sortDirectory(const fs::path& pathToSort)
{
    if (fs::is_empty(pathToSort)) {
        std::cerr << "Directory is empty";
        return EXIT_FAILURE;
    }

    // For every entry in sortable directory
    size_t filesCount = 0;
    for(auto& dir_entry : fs::recursive_directory_iterator(pathToSort)) {
        if (fs::is_regular_file(dir_entry)) {
            const fs::path& curFilePath = dir_entry.path();
            std::optional<FileInfo> fileInfo = getFileInfo(curFilePath);

            if (!fileInfo) {
                // If file info can't be found, move file to "Undefined" folder
                std::cerr << "Can't get attributes for file \"" << curFilePath.filename() << std::endl;
                fs::path newPath = pathToSort / "Undefined" / curFilePath.filename();

                moveFile(curFilePath, newPath);
            }
            else {
                // Move and rename file to ./Year/Season/preferableFilename.extension
                fs::path newPath = pathToSort / fileInfo->year / fileInfo->month / fileInfo->preferableFilename;

                moveFile(curFilePath, newPath);
                filesCount++;
            }
        }
    }

    // Remove empty directories
    bool rmFlag = true;
    size_t removedDirCount = 0;
    while (rmFlag) {
        rmFlag = false;
        fs::recursive_directory_iterator it(pathToSort);
        fs::recursive_directory_iterator itEnd;
        while (it != itEnd) {
            if (fs::is_directory(*it) && fs::is_empty(*it)) {
                fs::path pathToRemove = it->path();
                ++it;
                fs::remove(pathToRemove);
                removedDirCount++;
                rmFlag = true;
            } else {
                ++it;
            }
        }
    }

    std::cout << "Successfully sorted " << filesCount << " file(s)" << std::endl;
    if (removedDirCount != 0) {
        std::cout << "Removed " << removedDirCount << " directory(es)" << std::endl;
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
    // Check if file with the same name already exists
    // and assign counter
    if (fs::exists(new_path)) {
        size_t counter = 1;
        fs::path newPathWithCounter;

        do {
            newPathWithCounter = new_path.parent_path() / (new_path.stem().string() + '_' + std::to_string(counter) + new_path.extension().string());
            counter++;
        } while (fs::exists(newPathWithCounter));
        fs::rename(old_path, newPathWithCounter);
    }
    else {
        fs::rename(old_path, new_path);
    }
}

std::optional<FileInfo> getFileInfo(const fs::path& file_path)
{
    // Use windows API to get file information
    WIN32_FIND_DATAW findData;
    void* hSearch = FindFirstFileW(file_path.c_str(), &findData);
    if (hSearch == INVALID_HANDLE_VALUE) return {};

    // Convert FILETIME to SYSTEMTIME
    SYSTEMTIME fileLastWriteTime;
    FileTimeToSystemTime(&findData.ftLastWriteTime, &fileLastWriteTime);

    // Extract required values
    FileInfo fileInfo;
    fileInfo.year = std::to_string(fileLastWriteTime.wYear);
    fileInfo.month = g_months[fileLastWriteTime.wMonth - 1];

    // Create future filename: "YYYY-MONTH-DD_DayOfWeek"
    fileInfo.preferableFilename = fileInfo.year + '-'
            + fileInfo.month + '-'
            + std::to_string(fileLastWriteTime.wDay) + '_'
            + g_daysOfWeek[fileLastWriteTime.wDayOfWeek]
            + file_path.extension().string();

    return std::make_optional(std::move(fileInfo));
}