#pragma once
#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include "glm\glm.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

//将着色器单独抽象成一个类
class Shader
{
public:
	unsigned int ID;
	Shader()
	{
		const char *vertexShaderSource = "#version 330 core\n"
			"layout (location = 0) in vec3 aPos;\n"
			"layout (location = 1) in vec2 aTexCoord;\n"
			"out vec2 TexCoord;\n"

			"uniform mat4 model;\n"
			"uniform mat4 view;\n"
			"uniform mat4 projection;\n"
			"void main()\n"
			"{\n"
			"   gl_Position = projection * view * model * vec4(aPos, 1.0f);\n"
			"   TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
			"}\0";

		const char *fragmentShaderSource = "#version 330 core\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoord;\n"
			"uniform sampler2D texture1;\n"
			"uniform sampler2D texture2;\n"

			"void main()\n"
			"{\n"
			"   FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);\n"
			"}\n\0";

		//根据源码编译两个着色器
		unsigned int myVertexShader;
		myVertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(myVertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(myVertexShader);
		checkCompileErrors(myVertexShader, "VERTEX");

		unsigned int myFragmentShader;
		myFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(myFragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(myFragmentShader);
		checkCompileErrors(myFragmentShader, "VERTEX");

		//创建程序对象，将着色器附加上去，然后将程序链接,激活，就可以把着色器对象删掉了
		ID = glCreateProgram();
		glAttachShader(ID, myVertexShader);
		glAttachShader(ID, myFragmentShader);
		glLinkProgram(ID);

		glDeleteShader(myVertexShader);
		glDeleteShader(myFragmentShader);
	}
	// activate the shader
	// ------------------------------------------------------------------------
	void use()
	{
		glUseProgram(ID);
	}
	// utility uniform functions
	// ------------------------------------------------------------------------
	void setBool(const std::string &name, bool value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}
	// ------------------------------------------------------------------------
	void setInt(const std::string &name, int value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	// ------------------------------------------------------------------------
	void setFloat(const std::string &name, float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}
	// ------------------------------------------------------------------------
	void setVec2(const std::string &name, const glm::vec2 &value) const
	{
		glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec2(const std::string &name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
	}
	// ------------------------------------------------------------------------
	void setVec3(const std::string &name, const glm::vec3 &value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec3(const std::string &name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}
	// ------------------------------------------------------------------------
	void setVec4(const std::string &name, const glm::vec4 &value) const
	{
		glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec4(const std::string &name, float x, float y, float z, float w)
	{
		glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
	}
	// ------------------------------------------------------------------------
	void setMat2(const std::string &name, const glm::mat2 &mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat3(const std::string &name, const glm::mat3 &mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat4(const std::string &name, const glm::mat4 &mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

private:
	// utility function for checking shader compilation/linking errors.
	// ------------------------------------------------------------------------
	void checkCompileErrors(GLuint shader, std::string type)
	{
		GLint success;
		GLchar infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}
};
#endif