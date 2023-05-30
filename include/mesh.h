#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <shader.h>

#include <string>
#include <vector>
#include <limits>

using std::string, std::vector;

#define MAX_BONE_INFLUENCE 4

struct Vertex {
	// position
	glm::vec3 Position;
	// normal
	glm::vec3 Normal;
	// texCoords
	glm::vec2 TexCoords;
	// tangent
	glm::vec3 Tangent;
	// bitangent
	glm::vec3 Bitangent;
	// bone indexes which will influence this vertex
	int m_BoneIDs[MAX_BONE_INFLUENCE];
	// weights from each bone
	float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
	unsigned int id;
	string type;
	string path;
};

class Mesh {
public:
	// mesh Data
	vector<Vertex>       vertices;
	vector<unsigned int> indices;
	vector<Texture>      textures;

	unsigned int VAO;

	// constructor
	Mesh(const vector<Vertex>& vertices, const vector<unsigned int>& indices, const vector<Texture>& textures)
		:vertices(vertices), indices(indices), textures(textures)
	{
		// now that we have all the required data, set the vertex buffers and its attribute pointers.
		setupMesh();
	}

	// render the mesh
	void Draw(Shader& shader) const
	{
		// bind appropriate textures
		unsigned int diffuseNr = 0;
		unsigned int specularNr = 0;
		// unsigned int normalNr = 0;
		unsigned int heightNr = 0;
		unsigned int reflectionNr = 0;

		unsigned int i;
		for (i = 0; i < textures.size(); i++)
		{
			// active proper texture unit before binding
			// binding multiple textures for a single drawing call
			glActiveTexture(GL_TEXTURE0 + i);
			// retrieve texture number (the N in diffuse_textureN)
			string number;
			string type = textures[i].type;

			if (type == "texture_diffuse")
				number = std::to_string(diffuseNr++);
			else if (type == "texture_specular")
				number = std::to_string(specularNr++); // transfer unsigned int to string
			// else if (type == "texture_normal")
			//	number = std::to_string(normalNr++); // transfer unsigned int to string
			else if (type == "texture_height")
				number = std::to_string(heightNr++); // transfer unsigned int to string
			else if (type == "texture_reflection")	// We'll now also need to add the code to set and bind to reflection textures
				number = std::to_string(reflectionNr++);

			// now set the sampler to the correct texture unit
			glUniform1i(glGetUniformLocation(shader.ID, ("material." + type + "[" + number + "]").c_str()), i);
			// and finally bind the texture
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}
		glUniform1i(glGetUniformLocation(shader.ID, "material.texture_diffuse_num"), diffuseNr);
		glUniform1i(glGetUniformLocation(shader.ID, "material.texture_specular_num"), specularNr);
		glUniform1i(glGetUniformLocation(shader.ID, "material.texture_reflection_num"), reflectionNr);
		glUniform1i(glGetUniformLocation(shader.ID, "material.texture_height_num"), heightNr);

		// draw mesh
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// always good practice to set everything back to defaults once configured.
		glActiveTexture(GL_TEXTURE0);
	}

	void DrawInstanced(Shader& shader, const unsigned int count) const
	{
		// bind appropriate textures
		unsigned int diffuseNr = 0;
		unsigned int specularNr = 0;
		unsigned int heightNr = 0;
		unsigned int reflectionNr = 0;

		unsigned int i;
		for (i = 0; i < textures.size(); i++)
		{
			// active proper texture unit before binding
			// binding multiple textures for a single drawing call
			glActiveTexture(GL_TEXTURE0 + i);
			// retrieve texture number (the N in diffuse_textureN)
			string number;
			string type = textures[i].type;

			if (type == "texture_diffuse")
				number = std::to_string(diffuseNr++);
			else if (type == "texture_specular")
				number = std::to_string(specularNr++); // transfer unsigned int to string
			else if (type == "texture_height")
				number = std::to_string(heightNr++); // transfer unsigned int to string
			else if (type == "texture_reflection")	// We'll now also need to add the code to set and bind to reflection textures
				number = std::to_string(reflectionNr++);

			// now set the sampler to the correct texture unit
			glUniform1i(glGetUniformLocation(shader.ID, ("material." + type + "[" + number + "]").c_str()), i);
			// and finally bind the texture
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}
		glUniform1i(glGetUniformLocation(shader.ID, "material.texture_diffuse_num"), diffuseNr);
		glUniform1i(glGetUniformLocation(shader.ID, "material.texture_specular_num"), specularNr);
		glUniform1i(glGetUniformLocation(shader.ID, "material.texture_reflection_num"), reflectionNr);
		glUniform1i(glGetUniformLocation(shader.ID, "material.texture_height_num"), heightNr);

		// draw mesh
		glBindVertexArray(VAO);
		glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0, count);
		glBindVertexArray(0);

		// always good practice to set everything back to defaults once configured.
		glActiveTexture(GL_TEXTURE0);
	}
private:
	// render data 
	unsigned int VBO, EBO;

	// initializes all the buffer objects/arrays
	void setupMesh()
	{
		// create buffers/arrays
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		// bind VAO
		// Any subsequent VBO, EBO, glVertexAttribPointer and glEnableVertexAttribArray
		// calls will be stored inside the VAO currently bound.
		glBindVertexArray(VAO);

		// load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// A great thing about structs is that their memory layout is sequential for all its items.
		// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
		// again translates to 3/2 floats which translates to a byte array.
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		// set the vertex attribute pointers
		// vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

		// vertex normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

		// vertex texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

		// vertex tangent
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

		// vertex bitangent
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

		// ids
		glEnableVertexAttribArray(5);
		glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

		// weights
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));

		// unbind VAO
		glEnableVertexAttribArray(0);
	}
};
#endif // !MESH_H
