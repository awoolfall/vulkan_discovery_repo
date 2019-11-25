#include <glh.h>

#include "../vendor/glad/glad.h"

#include <memory>
#include <unordered_map>

namespace opengl_renderer
{

struct data {
    unsigned int mActiveShaderProgram = UINT_MAX;
    unsigned int mActiveVertexArray   = UINT_MAX;
};

/* GLOBAL */
std::unique_ptr<data> gGLHRenderer;
/* GLOBAL */

void init() {
    gGLHRenderer = std::make_unique<::opengl_renderer::data> ();
}

void useShaderProgram(unsigned int pShaderProgramID) {
    if (gGLHRenderer->mActiveShaderProgram != pShaderProgramID) {
        glUseProgram(pShaderProgramID);
        gGLHRenderer->mActiveShaderProgram = pShaderProgramID;
    }
}

void useVertexArray(unsigned int pVertexArrayID) {
    if (gGLHRenderer->mActiveVertexArray != pVertexArrayID) {
        glBindVertexArray(pVertexArrayID);
        gGLHRenderer->mActiveVertexArray = pVertexArrayID;
    }
}

}

namespace glh
{

std::unordered_map<RenderBuffer, GLenum> RenderBufferMap = {
    {GLH_COLOR_BUFFER,  GL_COLOR_BUFFER_BIT},
    {GLH_DEPTH_BUFFER,  GL_DEPTH_BUFFER_BIT}
};

std::unordered_map<CullMode, GLenum> CullModeMap = {
    {GLH_CULL_NONE,     0x00},
    {GLH_CULL_BACK,     GL_BACK},
    {GLH_CULL_FRONT,    GL_FRONT},
    {GLH_CULL_BOTH,     GL_FRONT_AND_BACK}
};

std::unordered_map<TextureFormat, GLenum> TextureFormatMap = {
    {GLH_FORMAT_R,      GL_RED},
    {GLH_FORMAT_RG,     GL_RG},
    {GLH_FORMAT_RGB,    GL_RGB},
    {GLH_FORMAT_RGBA,   GL_RGBA},
    {GLH_FORMAT_BGR,    GL_BGR},
    {GLH_FORMAT_BGRA,   GL_BGRA},
    {GLH_FORMAT_DEPTH,  GL_DEPTH}
};

std::unordered_map<TextureWrapping, GLenum> TextureWrappingMap = {
    {GLH_WRAP_CLAMP,            GL_CLAMP_TO_EDGE},
    {GLH_WRAP_MIRRORED_REPEAT,  GL_MIRRORED_REPEAT},
    {GLH_WRAP_REPEAT,           GL_REPEAT}
};

std::unordered_map<TextureFiltering, GLenum> TextureFilteringMap = {
    {GLH_FILTER_LINEAR,     GL_LINEAR},
    {GLH_FILTER_NEAREST,    GL_NEAREST}
};

bool renderer::init(void* pProcAddress) 
{
    opengl_renderer::init();
    gladLoadGLLoader((GLADloadproc)pProcAddress);
    /* set some opengl parameters */
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    /* the following allows opengl textures to not have to be 4 byte aligned (helps with font loading) */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    return true;
}

void renderer::setClearColor(float pRed, float pBlue, float pGreen, float pAlpha)
{
    glClearColor(pRed, pGreen, pBlue, pAlpha);
}

void renderer::clearBuffers(RenderBuffer* pBufferArray, unsigned int pBufferLength)
{
    GLenum e = 0x00;
    for (unsigned int i = 0; i < pBufferLength; i++) {
        e = e | RenderBufferMap[pBufferArray[i]];
    }
    glClear(e);
}

void renderer::clearBuffer(RenderBuffer pBuffer)
{
    glClear(RenderBufferMap[pBuffer]);
}

void renderer::enableDepthTesting(bool pEnabled)
{
    if (pEnabled) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

void renderer::setCullMode(CullMode pFace)
{
    if (pFace == GLH_CULL_NONE) {
        glDisable(GL_CULL_FACE);
    } else {
        glCullFace(CullModeMap[pFace]);
        glEnable(GL_CULL_FACE);
    }
}

void renderer::setViewport(int pLeftX, int pBottomY, int pWidth, int pHeight)
{
    glViewport(pLeftX, pBottomY, pWidth, pHeight);
}


/* 
** NOTE: GL_TEXTURE0 is used to temporarily bind and modify a texture
** actual rendering textures must begin at GL_TEXTURE1 in order to avoid
** unwanted conflicts
*/

void texture_module::init(const char* pData, unsigned int pWidth, unsigned int pHeight, TextureFormat pFormat)
{
    glGenTextures(1, &Data.mTextureID);
    glBindTexture(GL_TEXTURE_2D, Data.mTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, TextureFormatMap[pFormat], pWidth, pHeight, 0, TextureFormatMap[pFormat], GL_UNSIGNED_BYTE, pData);
}

void texture_module::del()
{
    glDeleteTextures(1, &Data.mTextureID);
}

void texture_module::generateMipMaps()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Data.mTextureID);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void texture_module::setTextureWrapping(TextureWrapping pWrapping)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Data.mTextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TextureWrappingMap[pWrapping]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TextureWrappingMap[pWrapping]);
}

void texture_module::setTextureFiltering(TextureFiltering pFiltering)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Data.mTextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TextureFilteringMap[pFiltering]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TextureFilteringMap[pFiltering]);
}


void shader_module::init(const char* pShaderData, ShaderType pType)
{
    GLenum ShaderType;
    switch (pType) {
        case GLH_SHADER_VERTEX:     ShaderType = GL_VERTEX_SHADER; break;
        case GLH_SHADER_FRAGMENT:   ShaderType = GL_FRAGMENT_SHADER; break;
        default:                    ShaderType = GL_VERTEX_SHADER; break;
    }
    Data.mShaderID = glCreateShader(ShaderType);
    glShaderSource(Data.mShaderID, 1, &pShaderData, NULL);
    glCompileShader(Data.mShaderID);
}

void shader_module::del()
{
    glDeleteShader(Data.mShaderID);
}

bool shader_module::isValid() const
{
    int success;
    glGetShaderiv(Data.mShaderID, GL_COMPILE_STATUS, &success);
    return (bool)success;
}


void shader_program::init(std::vector<shader_module*> pShaderModules)
{
    Data.mProgramID = glCreateProgram();
    for (unsigned int i = 0; i < pShaderModules.size(); i++) {
        glAttachShader(Data.mProgramID, pShaderModules[i]->Data.mShaderID);
    }
    glLinkProgram(Data.mProgramID);

    int success;
    glGetProgramiv(Data.mProgramID, GL_LINK_STATUS, &success);
    if(!success) {
        opengl_renderer::useShaderProgram(UINT_MAX);
    }
    opengl_renderer::useShaderProgram(Data.mProgramID);
}

void shader_program::del()
{
    glDeleteProgram(Data.mProgramID);
}

void shader_program::useShaderProgram()
{
    opengl_renderer::useShaderProgram(Data.mProgramID);
}

UniformType _glhOpenglUniformTypeToGLH(GLenum pUniformType) {
    switch (pUniformType) {
        case GL_UNSIGNED_INT:       return GLH_UNIFORM_UINT;
        case GL_INT:                return GLH_UNIFORM_INT;
        case GL_BOOL:               return GLH_UNIFORM_BOOL;
        case GL_FLOAT:              return GLH_UNIFORM_FLOAT;
        case GL_FLOAT_VEC2:         return GLH_UNIFORM_VEC2;
        case GL_FLOAT_VEC3:         return GLH_UNIFORM_VEC3;
        case GL_FLOAT_VEC4:         return GLH_UNIFORM_VEC4;
        case GL_FLOAT_MAT2:         return GLH_UNIFORM_MAT2;
        case GL_FLOAT_MAT3:         return GLH_UNIFORM_MAT3;
        case GL_FLOAT_MAT4:         return GLH_UNIFORM_MAT4;
        case GL_SAMPLER_2D:         return GLH_UNIFORM_SAMPLER2D;
        default:                    return GLH_UNIFORM_ERROR;
    }
}

int shader_program::getUniformCount()
{
    int count;
    glGetProgramiv(Data.mProgramID, GL_ACTIVE_UNIFORMS, &count);
    return count;
}

std::vector<uniform_entry> shader_program::listUniforms()
{
    std::vector<uniform_entry> ret_vec;
    int uniformCount = getUniformCount();
    for (int i = 0; i < uniformCount; i++) {
        GLenum type;
        uniform_entry e;
        glGetActiveUniform(Data.mProgramID, (GLuint)i, 64, NULL, &e.size, &type, e.name);
        e.type = _glhOpenglUniformTypeToGLH(type);
        ret_vec.push_back(e);
    }
    return ret_vec;
}

int shader_program::getUniformID(const char* pUniformName)
{
    return glGetUniformLocation(Data.mProgramID, pUniformName);
}

void shader_program::setUniformBool(int pUniformID, bool pValue)
{
    opengl_renderer::useShaderProgram(Data.mProgramID);
    glUniform1i(pUniformID, (GLint)pValue);
}

void shader_program::setUniformInt(int pUniformID, int pValue)
{
    opengl_renderer::useShaderProgram(Data.mProgramID);
    glUniform1i(pUniformID, pValue);
}

void shader_program::setUniformFloat(int pUniformID, float pValue)
{
    opengl_renderer::useShaderProgram(Data.mProgramID);
    glUniform1f(pUniformID, pValue);
}

void shader_program::setUniformVec2v(int pUniformID, const float* pValue)
{
    opengl_renderer::useShaderProgram(Data.mProgramID);
    glUniform2fv(pUniformID, 1, pValue);
}

void shader_program::setUniformVec2(int pUniformID, float pValueX, float pValueY)
{
    opengl_renderer::useShaderProgram(Data.mProgramID);
    glUniform2f(pUniformID, pValueX, pValueY);
}

void shader_program::setUniformVec3v(int pUniformID, const float* pValue)
{
    opengl_renderer::useShaderProgram(Data.mProgramID);
    glUniform3fv(pUniformID, 1, pValue);
}

void shader_program::setUniformVec3(int pUniformID, float pValueX, float pValueY, float pValueZ)
{
    opengl_renderer::useShaderProgram(Data.mProgramID);
    glUniform3f(pUniformID, pValueX, pValueY, pValueZ);
}

void shader_program::setUniformVec4v(int pUniformID, const float* pValue)
{
    opengl_renderer::useShaderProgram(Data.mProgramID);
    glUniform4fv(pUniformID, 1, pValue);
}

void shader_program::setUniformVec4(int pUniformID, float pValueX, float pValueY, float pValueZ, float pValueW)
{
    opengl_renderer::useShaderProgram(Data.mProgramID);
    glUniform4f(pUniformID, pValueX, pValueY, pValueZ, pValueW);
}

void shader_program::setUniformMat2(int pUniformID, const float* pValue)
{
    opengl_renderer::useShaderProgram(Data.mProgramID);
    glUniformMatrix2fv(pUniformID, 1, GL_FALSE, pValue);
}

void shader_program::setUniformMat3(int pUniformID, const float* pValue)
{
    opengl_renderer::useShaderProgram(Data.mProgramID);
    glUniformMatrix3fv(pUniformID, 1, GL_FALSE, pValue);
}

void shader_program::setUniformMat4(int pUniformID, const float* pValue)
{
    opengl_renderer::useShaderProgram(Data.mProgramID);
    glUniformMatrix4fv(pUniformID, 1, GL_FALSE, pValue);
}

void shader_program::setSamplerPosition(int pUniformID, unsigned int pValue)
{
    if (pValue == 0) {
        printf("GLH WARNING: setting sampler position to 0 may cause undefined behaviour\n");
    }
    setUniformInt(pUniformID, pValue);
}

void shader_program::setUniformSampler2D(int pUniformID, const texture_module& pValue)
{
    GLint texture_position;
    opengl_renderer::useShaderProgram(Data.mProgramID);
    glGetUniformiv(Data.mProgramID, pUniformID, &texture_position);
    glActiveTexture(GL_TEXTURE0 + texture_position);
    glBindTexture(GL_TEXTURE_2D, pValue.Data.mTextureID);
}

void shader_program::setUniformSamplerCube(int pUniformID, const texture_module& pValue)
{
    GLint texture_position;
    opengl_renderer::useShaderProgram(Data.mProgramID);
    glGetUniformiv(Data.mProgramID, pUniformID, &texture_position);
    glActiveTexture(GL_TEXTURE0 + texture_position);
    glBindTexture(GL_TEXTURE_CUBE_MAP, pValue.Data.mTextureID);
}


void vertex_buffer::init(const float* pData, unsigned long pDataLength, std::vector<unsigned int> pAttributes)
{
    opengl_renderer::useVertexArray(0);

    Data.mAttributes = pAttributes;
    Data.mDataLength = pDataLength;

    Data.mStride = 0;
    if (pAttributes.size() > 1) {
        for (unsigned int i = 0; i < pAttributes.size(); i++) {
            Data.mStride += pAttributes[i];
        }
    }

    glGenBuffers(1, &Data.mVertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, Data.mVertexBufferID);
    if (pData == nullptr) {
        glBufferData(GL_ARRAY_BUFFER, pDataLength * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    } else {
        glBufferData(GL_ARRAY_BUFFER, pDataLength * sizeof(float), pData, GL_STATIC_DRAW);
    }
}

void vertex_buffer::del()
{
    glDeleteBuffers(1, &Data.mVertexBufferID);
}

void vertex_buffer::setVertexBufferData(const float* pData, unsigned long pDataLength)
{
    opengl_renderer::useVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, Data.mVertexBufferID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, pDataLength * sizeof(float), pData);
}


void index_buffer::init(const unsigned int* pData, unsigned long pDataLength)
{
    opengl_renderer::useVertexArray(0);

    this->mNumIndicies = pDataLength;
    glGenBuffers(1, &Data.mIndexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Data.mIndexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, pDataLength * sizeof(unsigned int), pData, GL_STATIC_DRAW);
}

void index_buffer::del()
{
    glDeleteBuffers(1, &Data.mIndexBufferID);
}


void vertex_array::init()
{
    Data.mAttribLocation = 0;
    glGenVertexArrays(1, &Data.mVertexArrayID);

    opengl_renderer::useVertexArray(0);
}

void vertex_array::del()
{
    if (AutoDeleteBuffers) {
        for (size_t i = 0; i < mVertexBuffers.size(); i++) {
            mVertexBuffers[i].del();
        }
        if (isIndexed()) mIndexBuffer.del();
    }
    glDeleteVertexArrays(1, &Data.mVertexArrayID);
}

void vertex_array::flush()
{
    opengl_renderer::useVertexArray(this->Data.mVertexArrayID);

    /* reset attrib locations */
    while (this->Data.mAttribLocation > 0) {
        this->Data.mAttribLocation -= 1;
        glDisableVertexAttribArray(this->Data.mAttribLocation);
    }

    /* set and enable attributes for all vertex buffers */
    for (size_t i = 0; i < this->mVertexBuffers.size(); i++) {
        glBindBuffer(GL_ARRAY_BUFFER, this->mVertexBuffers[i].Data.mVertexBufferID);
        unsigned int offset = 0;
        for (unsigned int j = 0; j < this->mVertexBuffers[i].Data.mAttributes.size(); j++) {
            glEnableVertexAttribArray(Data.mAttribLocation);
            glVertexAttribPointer(Data.mAttribLocation, this->mVertexBuffers[i].Data.mAttributes[j], GL_FLOAT, GL_FALSE, this->mVertexBuffers[i].Data.mStride * sizeof(float), (void*)(offset*sizeof(float)));
            Data.mAttribLocation += 1;
            offset += this->mVertexBuffers[i].Data.mAttributes[j];
        }
    }

    /* enable or disable the index buffer */
    if (this->mIndexBuffer.getNumIndicies() > 0) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer.Data.mIndexBufferID);
    } else {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    /* cleanup */
    opengl_renderer::useVertexArray(0);
}

// @TODO: @FIX array drawing (first and count arguments)

void vertex_array::draw()
{
    opengl_renderer::useVertexArray(Data.mVertexArrayID);
    if (this->isIndexed()) {
        glDrawElements(GL_TRIANGLES, mIndexBuffer.getNumIndicies(), GL_UNSIGNED_INT, NULL);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, mVertexBuffers[0].Data.mDataLength);
    }
}

void vertex_array::drawInstanced(unsigned int pInstanceCount)
{
    opengl_renderer::useVertexArray(Data.mVertexArrayID);
    if (this->isIndexed()) {
        glDrawElementsInstanced(GL_TRIANGLES, mIndexBuffer.getNumIndicies(), GL_UNSIGNED_INT, NULL, pInstanceCount);
    } else {
        glDrawArraysInstanced(GL_TRIANGLES, 0, mVertexBuffers[0].Data.mDataLength, pInstanceCount);
    }
}

}