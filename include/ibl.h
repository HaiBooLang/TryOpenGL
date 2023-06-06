#pragma once
//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <model.h>
#include <shader.h>

#include <vector>
#include <string>
#include <iostream>

class IBL
{
public:
	IBL(const char* hdr_environment_map_path, const char* skybox_vertex_shader_path, const char* skybox_fragment_shader_path,
		const char* equirectangular_to_cubemap_vertex_shader_path, const char* equirectangular_to_cubemap_fragment_shader_path);

	void render_skybox(const glm::mat4& view);

	void set_skybox_shader_projection(const glm::mat4& projection);

private:
	void render_cube();

	void load_hdr_environment_map(const char* hdr_environment_map_path);

	void setup_cubemap();

	void equirectangular_to_cubemap();

	void setup_capture_framebuffer();

	void setup_cube();

private:
	unsigned int hdr_texture;
	unsigned int environment_cubemap;

	unsigned int capture_FBO;
	unsigned int capture_RBO;

	unsigned int cube_VAO;
	unsigned int cube_VBO;

	Shader skybox_shader;

	Shader equirectangular_to_cubemap_shader;

	glm::mat4 capture_projection{ glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f) };

	glm::mat4 capture_views[6]
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};
};