#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h>

#include <shader.h>

#include <vector>
#include <string>
#include <iostream>

class Skybox
{
public:
	Skybox(const std::vector<std::string>& faces, const char* vertexPath, const char* fragmentPath)
		:faces(faces), shader(vertexPath, fragmentPath)
	{
		setupSkybox();
	}

	unsigned int cubemapTexture() const
	{
		return textureID;
	}

	void draw(const glm::mat4& projection, const glm::mat4& view) const
	{
		// draw skybox as last
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		shader.use();
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		
		// skybox cube
		glBindVertexArray(VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default;
	}
private:
	void setupSkybox() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

		loadCubemap(faces);

#ifdef _DEBUG
		std::cout << "SUCCESSFULLY::SKYBOX::SUCCESSFULLY_SET_UP_SKYBOX\n";
#endif
	}

	void loadCubemap(const std::vector<std::string>& faces)
	{
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

		int width, height, nrChannels;
		for (unsigned int i = 0; i < faces.size(); i++)
		{
			unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
			if (data)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				stbi_image_free(data);
			}
			else
			{
				std::cerr << "ERROR::SKYBOX::LOAD_TEXTURE_FAILED\n"
					<< "    Cubemap texture failed to load at path : " << faces[i] << "\n";
				stbi_image_free(data);
			}
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
private:
	Shader shader;
	std::vector<std::string> faces;
	unsigned int textureID;
	unsigned int VAO, VBO;
	const float vertices[108] = {
		// positions          
		-1.0f,  1.0f, -1.0f,	-1.0f, -1.0f, -1.0f,	1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,	1.0f,  1.0f, -1.0f,		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,	-1.0f, -1.0f, -1.0f,	-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,	-1.0f,  1.0f,  1.0f,	-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,	1.0f, -1.0f,  1.0f,		1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,	1.0f,  1.0f, -1.0f,		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,	-1.0f,  1.0f,  1.0f,	1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,	1.0f, -1.0f,  1.0f,		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,	1.0f,  1.0f, -1.0f,		1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,	-1.0f,  1.0f,  1.0f,	-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,	-1.0f, -1.0f,  1.0f,	1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,	-1.0f, -1.0f,  1.0f,	1.0f, -1.0f,  1.0f
	};
};
#endif