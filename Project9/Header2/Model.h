#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <set>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <iostream>


#include "Map.h"

#include "../utils/math/vector.hpp"
#include "../utils/math/matrix.hpp"
#include "../utils/event/action.h"

// vcpkg
#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
// vcpkg

// 3rd party headers
#include "stb_image.h"
// 3rd party headers

struct vertex
{
	vec3 position;
	vec3 normal;
	vec2 uv;
};
struct texture
{
	unsigned int handle = 0;
	std::string type;
};
struct mesh
{
private:
	inline static unsigned int empty_texture = 0;
public:
	// texture textures[4];
	texture Diffuse;
	texture Specular;
public:
	unsigned int vao;
	unsigned int vbo;
	unsigned int ebo;
	unsigned int siz;


	mesh(std::vector<vertex> v, std::vector<unsigned> i)
	{
		if (empty_texture == 0)
		{
			glGenTextures(1, &empty_texture);
			glBindTexture(GL_TEXTURE_2D, empty_texture);

			unsigned char bits[4 * 4 * 4]
			{
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,
			};

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4, 4, 0, GL_RGB, GL_UNSIGNED_BYTE, bits);
		}
		Diffuse.handle = empty_texture;
		Specular.handle = empty_texture;


		siz = i.size();

		// create buffers
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		// bind state memory
		glBindVertexArray(vao);

		// vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * v.size(), v.data(), GL_STATIC_DRAW);

		// index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * i.size(), i.data(), GL_STATIC_DRAW);

		// vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, position));
		// vertex normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, normal));
		// vertex texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, uv));
	}
};

class model
{
public:
	std::string directory;
	std::vector<mesh> meshes;



	void load(const std::string& path)
	{
		meshes.reserve(100);
		printf("loading '%s'\n", path.c_str());

		using std::cout;
		using std::endl;

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcessPreset_TargetRealtime_Fast);
		

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
			return;
		}
		directory = path.substr(0, path.find_last_of('\\'));

		processNode(scene->mRootNode, scene);
		
		printf("loaded '%s'\n", path.c_str());
	}

	void processNode(aiNode* node, const aiScene* scene)
	{
		printf("processing node: '%s'\n\tchildren:%u\n",node->mName.C_Str(),node->mNumChildren);

		// process all the node's meshes (if any)
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		// then do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}


	mesh processMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<vertex> vertices;
		std::vector<unsigned int> indices;
		
		texture TexDiffuse{};
		texture TexSpecular{};

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			vertex vertex;
			// process vertex positions, normals and texture coordinates
			vec3 vector;
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.position = vector;
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.normal = vector;

			if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
			{
				vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.uv = vec;
			}
			else 
			{
				vertex.uv = vec2(0.0f, 0.0f);
			}

			vertices.push_back(vertex);
		}
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		// process material
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			std::vector<texture> diffuseMaps = loadMaterialTextures(material,
				aiTextureType_DIFFUSE, "diffuse");
			
			// assume only one diffuse map
			if (!diffuseMaps.empty()) TexDiffuse = diffuseMaps.front();

			// assume only one specular map
			std::vector<texture> specularMaps = loadMaterialTextures(material,
				aiTextureType_SPECULAR, "specular");
			
			if (!specularMaps.empty()) TexSpecular = specularMaps.front();
		}

		auto ret = ::mesh(vertices, indices);
		ret.Diffuse = TexDiffuse;
		ret.Specular = TexSpecular;
		return ret;
	}
	std::vector<texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
	{
		std::vector<texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			texture texture;
			texture.handle = TextureFromFile(str.C_Str(), directory, false);
			texture.type = typeName;
			// texture.path = str;
			textures.push_back(texture);
		}
		return textures;
	}



	unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma)
	{
		std::string filename = std::string(path);
		filename = directory + '/' + filename;

		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << path << std::endl;
			stbi_image_free(data);
		}

		return textureID;
	}
};

class geometry_builder
{
	std::vector<vec3> vertices;
	struct triangle
	{
		size_t vertex_a;
		size_t vertex_b;
		size_t vertex_c;

		vec3 normal;
	};
	std::vector<triangle> faces;

public:

	geometry_builder& define_point(vec3 position)
	{
		vertices.push_back(position);
		return *this;
	}
	geometry_builder& define_face(size_t top_left, size_t top_right, size_t bottom_left,size_t bottom_right)
	{
		triangle t;
		t.vertex_a = top_left;
		t.vertex_b = top_right;
		t.vertex_c = bottom_left;
		faces.push_back(t);
		t.vertex_a = bottom_left;
		t.vertex_b = top_right;
		t.vertex_c = bottom_right;
		faces.push_back(t);
		return *this;
	}

	void build(std::vector<vertex>& out_vertices, std::vector<unsigned>& out_indices)
	{
		for (auto& face : faces)
		{
			vec3 vertex_a = vertices[face.vertex_a];
			vec3 vertex_b = vertices[face.vertex_b];
			vec3 vertex_c = vertices[face.vertex_c];

			// calculate normal for each vertex
			vec3 normal = calc_triangle_normal(vertex_a, vertex_b, vertex_c);

			// todo: calculate uv for each vertex

			// if the vertex exists, just add the index into indices
			// otherwise create the vertex & add its index into indices
			vertex vert;

			vert.uv = 0;
			vert.normal = normal;
			vert.position = vertex_a;
			find_or_create_if_noexist(vert, out_vertices, out_indices);
			
			vert.uv = 0;
			vert.normal = normal;
			vert.position = vertex_b;
			find_or_create_if_noexist(vert, out_vertices, out_indices);
			
			vert.uv = 0;
			vert.normal = normal;
			vert.position = vertex_c;
			find_or_create_if_noexist(vert, out_vertices, out_indices);
		}
	}

	void find_or_create_if_noexist(vertex v, std::vector<vertex>& out_vertices, std::vector<unsigned>& out_indices)
	{
		for (int i = 0; i < out_vertices.size(); ++i)
		{
			if (out_vertices[i].position == v.position && out_vertices[i].normal == v.normal)
			{
				out_indices.push_back(i);
				return;
			}
		}

		out_indices.push_back(out_vertices.size());
		out_vertices.push_back(v);
	}

	vec3 calc_triangle_normal(vec3 a, vec3 b, vec3 c)
	{
		// return vec3(0, 0, 1);

		auto edge1 = a - b;
		auto edge2 = a - c;
		// 
		return cross(edge1, edge2).normalized();
	}
};