#include "Shader.h"

Shader::Shader() {
    shaderID = 0;
}

void Shader::CreateFromString(const char* vertexCode, const char* fragmentCode) {
    CompileShader(vertexCode, fragmentCode);
}

void Shader::CreateFromFiles(const char* vertexLocation, const char* fragmentLocation) {
    std::string vertexString = ReadFile(vertexLocation);
    std::string fragmentString = ReadFile(fragmentLocation);

    const char* vertexCode = vertexString.c_str();//conversioin from string to char
    const char* fragmentCode = fragmentString.c_str();

    CompileShader(vertexCode, fragmentCode);
}

void Shader::CreateFromFiles(const char* vertexLocation, const char* geometryLocation, const char* fragmentLocation) {
    std::string vertexString = ReadFile(vertexLocation);
    std::string geometryString = ReadFile(geometryLocation);
    std::string fragmentString = ReadFile(fragmentLocation);

    const char* vertexCode = vertexString.c_str();//conversioin from string to char
    const char* geometryCode = geometryString.c_str();
    const char* fragmentCode = fragmentString.c_str();

    CompileShader(vertexCode, geometryCode, fragmentCode);
}

void Shader::CreateComputeShader(const char* computeLocation) {
    std::string computeString = ReadFile(computeLocation);
    const char* computeCode = computeString.c_str();

    shaderID = glCreateProgram();

    if (!shaderID) {
        printf("Error creating Compute Shader\n");
        return;
    }

    AddShader(shaderID, computeCode, GL_COMPUTE_SHADER);

    GLint result = 0;
    GLchar elog[1024] = { 0 };

    glLinkProgram(shaderID); //Linking
    glGetProgramiv(shaderID, GL_LINK_STATUS, &result);//to make sure the link is proper
    if (!result) {
        glGetProgramInfoLog(shaderID, sizeof(elog), NULL, elog);
        printf("Error Linking Program: %s \n", elog);
        return;
    }
}

std::string Shader::ReadFile(const char* FileLocation) {
    std::string content;
    std::ifstream fileStream(FileLocation, std::ios::in);

    if (!fileStream.is_open()) {
        printf("Failed to read %s! File doesn't exist.", FileLocation);
        return "";
    }

    std::string line = "";
    while (!fileStream.eof())
    {
        std::getline(fileStream, line);
        content.append(line + "\n");
    }

    fileStream.close();
    return content;
}

void Shader::UseShader() {
    glUseProgram(shaderID);
}

void Shader::ClearShader() {
    if (shaderID != 0) {
        glDeleteProgram(shaderID);
        shaderID = 0;
    }
}

void Shader::CompileShader(const char* vertexCode, const char* fragmentCode) {
    shaderID = glCreateProgram(); //Creating a shader

    if (!shaderID) {
        printf("Error creating shaderID");
        return;
    }

    AddShader(shaderID, vertexCode, GL_VERTEX_SHADER); //GL_VERTEX_SHADER is built in
    AddShader(shaderID, fragmentCode, GL_FRAGMENT_SHADER);

    CompileProgram();
}

void Shader::CompileShader(const char* vertexCode, const char* geometryCode, const char* fragmentCode) {
    shaderID = glCreateProgram(); //Creating a shader

    if (!shaderID) {
        printf("Error creating shaderID");
        return;
    }

    AddShader(shaderID, vertexCode, GL_VERTEX_SHADER); //GL_VERTEX_SHADER is built in
    AddShader(shaderID, geometryCode, GL_GEOMETRY_SHADER);
    AddShader(shaderID, fragmentCode, GL_FRAGMENT_SHADER);

    CompileProgram();
}

void Shader::Validate()
{
    GLint result = 0;
    GLchar eLog[1024] = { 0 };

    glValidateProgram(shaderID);
    glGetProgramiv(shaderID, GL_VALIDATE_STATUS, &result);
    if (!result)
    {
        glGetProgramInfoLog(shaderID, sizeof(eLog), NULL, eLog);
        printf("Error validating program: '%s'\n", eLog);
        return;
    }
}

void Shader::CompileProgram() {
    //For linking and debugging
    GLint result = 0;
    GLchar elog[1024] = { 0 };

    glLinkProgram(shaderID); //Linking
    glGetProgramiv(shaderID, GL_LINK_STATUS, &result);//to make sure the link is proper
    if (!result) {
        glGetProgramInfoLog(shaderID, sizeof(elog), NULL, elog);
        printf("Error Linking Program: %s \n", elog);
        return;
    }
}

void Shader::AddShader(GLuint theprogram, const char* shaderCode, GLenum shaderType) {
    GLuint theshader = glCreateShader(shaderType);

    const GLchar* theCode[1];//to point to the first address of the shader code

    theCode[0] = shaderCode;

    GLint codeLength[1];
    codeLength[0] = strlen(shaderCode);

    glShaderSource(theshader, 1, theCode, codeLength);//(shader we are modifying the code for, count, Code itself, code length)
    glCompileShader(theshader);

    GLint result = 0;
    GLchar elog[1024] = { 0 };

    glGetShaderiv(theshader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(theshader, sizeof(elog), NULL, elog);
        printf("Error compiling the %d shader: %s \n", shaderType, elog);
        return;
    }

    glAttachShader(theprogram, theshader); //We are attaching the compiled shader to shader program
}

Shader::~Shader() {
    ClearShader();
}