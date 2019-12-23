#include "platform.h"

#include <algorithm>
#include <fstream>

#ifdef _WIN32
#   include <Windows.h>
#endif
#ifdef linux
#   include <unistd.h>
#endif

/* used internally to retrieve the absolute path of the executable */
#define PATH_LENGTH_MAX 256
std::string get_exe_path()
{
    char path[PATH_LENGTH_MAX];
    std::string path_s("");
#   ifdef linux
        if (readlink( "/proc/self/exe", path, PATH_LENGTH_MAX ) == -1) {
            return std::string("");
        }
        path_s = std::string( path );
        path_s = path_s.substr(0, path_s.find_last_of('/')+1);
#   elif _WIN32
        if (GetModuleFileNameA(NULL, path, PATH_LENGTH_MAX) == 0) {
            return std::string("");
        }
        path_s = std::string(path);
        path_s = path_s.substr(0, path_s.find_last_of('\\')+1);
#   else
        return std::string("");
#   endif
    return path_s;
}

/* 
 * global variable holding the executable path for the current platform
 * so we dont need to calculate it more than once
*/
std::string g_exe_path = get_exe_path();

std::string to_absolute_path(std::string pRelativePath)
{
#   ifdef linux
        std::replace(pRelativePath.begin(), pRelativePath.end(), '\\', '/');
#   elif _WIN32
        std::replace(pRelativePath.begin(), pRelativePath.end(), '/', '\\');
#   endif
    return std::string(g_exe_path + pRelativePath);
}

std::string read_string_from_file(std::string pAbsolutePath)
{
    /* open file and check if it is valid */
    std::ifstream ifs(pAbsolutePath.c_str(), std::ifstream::in | std::ifstream::binary);

    if (!ifs) {
        ifs.close();
        return std::string("");
    }

    /*
     * init return_string to empty and copy letter by letter the contents
     * of the text file into the return string until the file ends
    */
    std::string return_string = "";
    while (ifs) {
        return_string += ifs.get();
    }
    /* pop back one value (fixes an issue where consistantly one additional letter was read) */
    return_string.pop_back();

    /* close the file and return the return_string */
    ifs.close();
    return return_string;
}

std::vector<char> read_data_from_binary_file(std::string abs_path)
{
    std::ifstream file(abs_path.c_str(), std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t file_size = (size_t)file.tellg();
    std::vector<char> buffer(file_size);
    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();
    return buffer;
}