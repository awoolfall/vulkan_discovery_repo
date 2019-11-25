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
eastl::string get_exe_path()
{
    char path[PATH_LENGTH_MAX];
    eastl::string path_s("");
#   ifdef linux
        if (readlink( "/proc/self/exe", path, PATH_LENGTH_MAX ) == -1) {
            return eastl::string("");
        }
        path_s = eastl::string( path );
        path_s = path_s.substr(0, path_s.find_last_of('/')+1);
#   elif _WIN32
        if (GetModuleFileNameA(NULL, path, PATH_LENGTH_MAX) == 0) {
            return eastl::string("");
        }
        path_s = eastl::string(path);
        path_s = path_s.substr(0, path_s.find_last_of('\\')+1);
#   else
        return eastl::string("");
#   endif
    return path_s;
}

/* 
 * global variable holding the executable path for the current platform
 * so we dont need to calculate it more than once
*/
eastl::string g_exe_path = get_exe_path();

eastl::string to_absolute_path(eastl::string pRelativePath)
{
#   ifdef linux
        std::replace(pRelativePath.begin(), pRelativePath.end(), '\\', '/');
#   elif _WIN32
        std::replace(pRelativePath.begin(), pRelativePath.end(), '/', '\\');
#   endif
    return eastl::string(g_exe_path + pRelativePath);
}

eastl::string read_string_from_file(eastl::string pAbsolutePath)
{
    /* open file and check if it is valid */
    std::ifstream ifs(pAbsolutePath.c_str(), std::ifstream::in | std::ifstream::binary);

    if (!ifs) {
        ifs.close();
        return eastl::string("");
    }

    /*
     * init return_string to empty and copy letter by letter the contents
     * of the text file into the return string until the file ends
    */
    eastl::string return_string = "";
    while (ifs) {
        return_string += ifs.get();
    }
    /* pop back one value (fixes an issue where consistantly one additional letter was read) */
    return_string.pop_back();

    /* close the file and return the return_string */
    ifs.close();
    return return_string;
}