#include "shader.h"
#include "platform.h"

#include <regex>

GLuint attach_combined_glsl(GLuint shader_program, const char* abs_file_path)
{
    /* read text files located at pVertShaderPath and pFragShaderPath */
    std::string ShaderSrc = std::string(read_string_from_file(eastl::string(abs_file_path)).c_str());

    /*
     * extract each shader type from shader_src
     * regex looks for some variation of // @ VERTEX SHADER to indicate the beginning of a shader
     * closes that shader at a new // @ FRAGMENT SHADER or end of file
     * to indicate a shader type '//@' followed by the shader type:
     * V for vertex, F for fragment, G for geometry, T for tris?
    */
    std::regex Regex(R"(@(?:\s+)?([VFGT]).+([^@]+))");
    auto words_begin = std::sregex_iterator(ShaderSrc.begin(), ShaderSrc.end(), Regex);
    auto words_end = std::sregex_iterator();

    /* fill in following std::strings with respective source (where duplicate shader declarations replace old) */
    std::string vert_s = "", frag_s = "", geom_s = "", tris_s = "";
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        if (strcmp(match.str(1).c_str(), "V") == 0) {
            vert_s = match.str(2);
        } else if (strcmp(match.str(1).c_str(), "F") == 0) {
            frag_s = match.str(2);
        } else if (strcmp(match.str(1).c_str(), "G") == 0) {
            geom_s = match.str(2);
        } else if (strcmp(match.str(1).c_str(), "T") == 0) {
            tris_s = match.str(2);
        }
    }

    const char* data = vert_s.c_str();
    /* generate shader_modules from source using glh */
    GLuint vert_module = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_module, 1, &data, NULL);
    glCompileShader(vert_module);

    data = frag_s.c_str();
    GLuint frag_module = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_module, 1, &data, NULL);
    glCompileShader(frag_module);

    int success;
    glGetShaderiv(vert_module, GL_COMPILE_STATUS, &success);
    if (!(int)success) {
        printf("VERT (failed):\n%s\n\n", vert_s.c_str());
        abort();
    }
    glGetShaderiv(frag_module, GL_COMPILE_STATUS, &success);
    if (!(int)success) {
        printf("FRAG (failed):\n%s\n\n", vert_s.c_str());
        abort();
    }

    glAttachShader(shader_program, vert_module);
    glAttachShader(shader_program, frag_module);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if(!success) {
        printf("SHADER (failed to link)\n");
        abort();
    }

    glDetachShader(shader_program, vert_module);
    glDetachShader(shader_program, frag_module);
    glDeleteShader(vert_module);
    glDeleteShader(frag_module);
    return shader_program;
}