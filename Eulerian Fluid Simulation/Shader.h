#pragma once

#include<stdio.h>
#include<string>
#include<iostream>
#include<fstream>

#include<glad/glad.h>

#include<glm/glm.hpp>
#include<glm/gtc/type_ptr.hpp>

class Shader
{
public:
	Shader();

	void CreateFromString(const char* vertexCode, const char* fragmentCode);
	void CreateFromFiles(const char* vertexLocation, const char* fragmentLocation);
	void CreateFromFiles(const char* vertexLocation, const char* geometryLocation, const char* fragmentLocation);

	void CreateComputeShader(const char* computeLocation);

	void Validate();

	std::string ReadFile(const char* FileLocation);

	GLuint GetShaderID() { return shaderID; }

	void UseShader();
	void ClearShader();

	~Shader();
private:

	GLuint shaderID;

	void CompileShader(const char* vertexCode, const char* fragmentCode);
	void CompileShader(const char* vertexCode, const char* geometryCode, const char* fragmentCode);
	void AddShader(GLuint theprogram, const char* shaderCode, GLenum shaderType);

	void CompileProgram();
};

