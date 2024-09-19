#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <set>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <iostream>
#include <unordered_map>


#include "../utils/math/vector.hpp"
#include "../utils/math/matrix.hpp"
#include "../utils/event/action.h"

#include "./components.h"

// vcpkg
#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
// vcpkg

// 3rd party headers
#include "stb_image.h"
// 3rd party headers


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class transform;
class texture;
class shader;
class scene_graph;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using std::string;
std::string read_file(const std::string& filename);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class shader_parameter_type
{
	integer,
	floating,
	vec2, vec3, vec4,
	mat2, mat3, mat4,

	cubemap,
	texture1D, texture2D, texture3D,
};

struct shader_uniform
{
	std::string name;
	shader_parameter_type type;
	unsigned int index = 0;
};

struct shader_binding
{
	std::string name; // camera
	unsigned int slot = 0; // 0
};

struct material
{
	std::string _name;

	shader* shader = nullptr;

	texture* diffuse = nullptr;
	texture* specular = nullptr;



	bool set_uniforms();
};

class shader
{
	std::string _name;
	int _targets;

	unsigned int _handle;
	std::vector<shader_uniform> uniforms;
	std::vector<shader_binding> bindings;

	std::string _path_vert;
	std::string _path_frag;
	std::string _path_geom;

	unsigned int build_module(unsigned int Type = GL_VERTEX_SHADER, const std::string& insource = "")
	{
		const char* source = insource.c_str();

		unsigned int id = glCreateShader(Type);
		glShaderSource(id, 1, &source, NULL);
		glCompileShader(id);

		// check for compilation errors
		int success;
		char infoLog[512];
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(id, 512, NULL, infoLog);
			printf("failed to compile shader: '%s'\n---\n", infoLog);
			return 0;
		}

		return id;
	}
	unsigned int build_shader(unsigned int& Vert, unsigned int& Frag, unsigned int& Geom)
	{
		// link shaders
		unsigned int shaderProgram = glCreateProgram();
		if (Vert) glAttachShader(shaderProgram, Vert);
		if (Frag) glAttachShader(shaderProgram, Frag);
		if (Geom) glAttachShader(shaderProgram, Geom);
		glLinkProgram(shaderProgram);

		// check for linking errors
		int success;
		char infoLog[512];
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			printf("failed to link shader: '%s'\n---\n");
		}

		if (Vert) glDeleteShader(Vert);
		if (Frag) glDeleteShader(Frag);
		if (Geom) glDeleteShader(Geom);

		return shaderProgram;
	}


	void read_shader_interface()
	{
		int count;

		int size; // size of the variable
		unsigned int type; // type of the variable (float, vec3 or mat4, etc)

		enum { bufSize = 512 };

		// attributes
		{
			glGetProgramiv(_handle, GL_ACTIVE_ATTRIBUTES, &count);
			printf("\tActive Attributes: %d\n", count);

			for (int i = 0; i < count; i++)
			{
				char name[bufSize]{};
				glGetActiveAttrib(_handle, (unsigned int)i, bufSize, NULL, &size, &type, name);

				printf("\t\t#%d Type: %u Name: %s\n", i, type, name);
			}
		}

		// uniforms
		{
			glGetProgramiv(_handle, GL_ACTIVE_UNIFORMS, &count);
			printf("\tActive Uniforms: %d\n", count);

			for (int i = 0; i < count; i++)
			{
				char name[bufSize]{};

				glGetActiveUniform(_handle, (unsigned int)i, bufSize, NULL, &size, &type, name);

				printf("\t\tUniform #%d Type: %u Name: %s\n", i, type, name);

				shader_uniform add;
				add.name = name;
				add.index = i;

				// primitives
				if (type == GL_INT) add.type = shader_parameter_type::integer;
				else if (type == GL_FLOAT) add.type = shader_parameter_type::floating;
				// vectors
				else if (type == GL_FLOAT_VEC2) add.type = shader_parameter_type::vec2;
				else if (type == GL_FLOAT_VEC3) add.type = shader_parameter_type::vec3;
				else if (type == GL_FLOAT_VEC4) add.type = shader_parameter_type::vec4;
				// matrices
				else if (type == GL_FLOAT_MAT2) add.type = shader_parameter_type::mat2;
				else if (type == GL_FLOAT_MAT3) add.type = shader_parameter_type::mat3;
				else if (type == GL_FLOAT_MAT4) add.type = shader_parameter_type::mat4;
				// samplers
				else if (type == GL_SAMPLER_1D) add.type = shader_parameter_type::texture1D;
				else if (type == GL_SAMPLER_2D) add.type = shader_parameter_type::texture2D;
				else if (type == GL_SAMPLER_3D) add.type = shader_parameter_type::texture3D;
				// other
				else __debugbreak();

				uniforms.push_back(add);
			}
		}

		// uniform blocks
		{
			glGetProgramiv(_handle, GL_ACTIVE_UNIFORM_BLOCKS, &count);
			printf("\tActive Uniform Blocks: %d\n", count);

			for (int i = 0; i < count; i++)
			{
				char name[bufSize]{};

				glGetActiveUniformBlockName(_handle, (unsigned int)i, bufSize, NULL, name);

				printf("\t\tUniform block %d: '%s'\n", i, name);

				shader_binding add;
				add.name = name;
				add.slot = i;
				bindings.push_back(add);
			}
		}
	}

public:
	string name() { return _name; }
	int targets() { return _targets; }
	unsigned int handle() { return _handle; }

	shader(const std::string& name) : _name(name)
	{
		_path_vert = "Resources//Shaders//" + name + ".vert";
		_path_frag = "Resources//Shaders//" + name + ".frag";
		_path_geom = "Resources//Shaders//" + name + ".geom";
	}

	void bind()
	{
		glUseProgram(_handle);
	}
	bool build()
	{
		// find the vert and frag shaders in the shaders directory by name
		std::string path_v = path_vert();
		std::string path_f = path_frag();
		std::string path_g = path_geom();

		// get their data
		std::string src_v = read_file(path_v);
		std::string src_f = read_file(path_f);
		std::string src_g = read_file(path_g);

		// build the shader
		unsigned int id_v = src_v.size() <= 5 ? 0 : build_module(GL_VERTEX_SHADER, src_v);
		unsigned int id_f = src_f.size() <= 5 ? 0 : build_module(GL_FRAGMENT_SHADER, src_f);
		unsigned int id_g = src_g.size() <= 5 ? 0 : build_module(GL_GEOMETRY_SHADER, src_g);

		_handle = build_shader(
			id_v,
			id_f,
			id_g);

		read_shader_interface();

		return _handle != 0;
	}
	string path_vert() { return _path_vert; }
	string path_frag() { return _path_frag; }
	string path_geom() { return _path_geom; }

	material create_material()
	{
		material ret;
		ret.shader = this;

		// for (auto& x : uniforms)
		// {
		// 	shader_parameter param;
		// 	param.name = x.name;
		// 	param.type = x.type;
		// 	ret.parameters.push_back(param);
		// }

		return ret;
	}


	// utility uniform functions
	// ------------------------------------------------------------------------
	void setBool(const std::string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(_handle, name.c_str()), (int)value);
	}
	// ------------------------------------------------------------------------
	void setInt(const std::string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(_handle, name.c_str()), value);
	}
	// ------------------------------------------------------------------------
	void setFloat(const std::string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(_handle, name.c_str()), value);
	}
	// ------------------------------------------------------------------------
	void setVec2(const std::string& name, const vec2& value) const
	{
		glUniform2fv(glGetUniformLocation(_handle, name.c_str()), 1, &value[0]);
	}
	void setVec2(const std::string& name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(_handle, name.c_str()), x, y);
	}
	// ------------------------------------------------------------------------
	void setVec3(const std::string& name, const vec3& value) const
	{
		glUniform3fv(glGetUniformLocation(_handle, name.c_str()), 1, &value[0]);
	}
	void setVec3(const std::string& name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(_handle, name.c_str()), x, y, z);
	}
	// ------------------------------------------------------------------------
	void setVec4(const std::string& name, const vec4& value) const
	{
		glUniform4fv(glGetUniformLocation(_handle, name.c_str()), 1, &value[0]);
	}
	void setVec4(const std::string& name, float x, float y, float z, float w) const
	{
		glUniform4f(glGetUniformLocation(_handle, name.c_str()), x, y, z, w);
	}
	// ------------------------------------------------------------------------
	void setMat2(const std::string& name, const mat2& mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(_handle, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat3(const std::string& name, const mat3& mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(_handle, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat4(const std::string& name, const mat4& mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(_handle, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
};

struct vertex
{
	vec3 position;
	vec3 normal;
	vec4 color;
	vec2 uv;
};

struct buffer
{
	unsigned int handle = 0; // GPU
	unsigned int type = 0;   // GL_ELEMENT_ARRAY_BUFFER
	unsigned int size = 0;   // 10000
	unsigned int pos = 0;    // 0
};

class component
{
	std::string _name;
public:
	component(std::string name) : _name(name) {}

	const std::string& name() { return _name; }
};
class transform
{
	std::string _name;

	transform* _parent = nullptr;
	std::vector<transform*> _children;

	vec3 _position{ 0,0,0 };
	vec3 _scale{ 1,1,1 };

	vec4 _quat_rotation = { 0,0,0,1 };

	mat4 _local_transform = mat4(1);
	mat4 _global_transform = mat4(1);

	bool _dirty = false;

	std::unordered_map<std::string, component*> _components;
	// recalculates local transform
	// recalculates global transform
	// sets dirty to false
	void recalculate()
	{
		auto m1 = transform_matrix::translate(mat4(1), _position);
		
		auto m3 = transform_matrix::scale(m1, _scale);

		auto glm1 = glm::translate(glm::mat4(1), glm::vec3(_position.x, _position.y, _position.z));
		auto glm3 = glm::scale(glm1, glm::vec3(_scale.x, _scale.y, _scale.z));

		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				if (m3[i][j] != glm3[i][j])
				{
					__debugbreak();
				}
			}
		}

		_local_transform = m3;

		if (_parent)
			_global_transform = _parent->_global_transform * m3;
		else
			_global_transform = _local_transform;


		for (auto* child : _children)
		{
			child->recalculate();
		}
		_dirty = false;
	}
public:
	class mesh* DRAW = nullptr;

	component* get_component(const std::string& name)
	{
		return _components[name];
	}

	const mat4& local_transform()
	{
		if (_dirty) recalculate();
		return _local_transform;
	}
	const mat4& global_transform()
	{
		if (_dirty) recalculate();
		return _global_transform;
	}

	transform* parent()
	{
		return _parent;
	}
	vec3 position()
	{
		return _position;
	}
	vec4 rotation()
	{
		return _quat_rotation;
	}
	vec3 scale()
	{
		return _scale;
	}
	void set_position(vec3 val)
	{
		_position = val;
		_dirty = true;
	}
	void set_rotation(vec4 val)
	{
		_quat_rotation = val;
		_dirty = true;
	}
	void set_scale(vec3 val)
	{
		_scale = val;
		_dirty = true;
	}
	void rename(const std::string& name)
	{
		_name = name;
	}

	void set_parent(transform* t)
	{
		if (_parent)
		{
			_parent->remove_child(this);
		}

		this->_parent = t;
		t->_children.push_back(this);

		_dirty = true;
	}
	
	void add_child(transform* val)
	{
		val->set_parent(this);
	}
	void remove_child(transform* val)
	{
		auto pos = std::find(_children.begin(), _children.end(), val);

		if (pos != _children.end()) 
		{
			_children.erase(pos);
		}
	}

	template<typename T>
	void iterate_depth_first(T&& fn)
	{
		fn(this);

		for (auto& x : _children)
			x->iterate_depth_first(std::forward<T>(fn));
	}
};


// A two dimensional array of pixels encoded as a standardized bitstream, such as PNG.
struct texture_image
{
	unsigned int handle = 0; // GPU
	unsigned int target = 0; // texture_2d
	unsigned int format = 0; // rgba
	unsigned int width = 0;  // 1024
	unsigned int height = 0; // 1024
	unsigned int type = 0;   // unsigned byte

	void create(const char* filename);
};

// An object that controls how image data is sampled.
struct texture_sampler
{
	struct parameter
	{
		unsigned int param = 0;
		unsigned int value = 0;
	};
	std::vector<parameter> parameters;

	void setup(texture_image& img) const;
};

// An object that combines an image and its sampler.
struct texture
{
	std::string _name;
	std::string _path;
	texture_image image;
	texture_sampler sampler;
};



// An object describing the number and the format of data elements stored in a binary buffer.
struct accessor
{
	// describes the format of a single data element stored in a binary buffer
	struct element
	{
		unsigned int count = 0; // 3

		unsigned int format = 0; // float
		unsigned int pointer = 0; // (void*)(offsetof(vertex,position))
	};


	std::vector<element> elements;
	unsigned int stride = 0;

	void setup(buffer& buf) const;
};


// binding a mesh primitive with a material
struct geometry
{
	unsigned int handle_vao = 0;
	unsigned int handle_vbo = 0;
	unsigned int handle_ebo = 0;


	unsigned int count_indices = 0;
	unsigned int count_vertices = 0;
	
	material* material = nullptr;

	unsigned int mode = 0; // GL_TRIANGLES
	unsigned int type = 0; // GL_UNSIGNED_INT



	void create_from(const std::vector<vertex>& vertices, const std::vector<unsigned int>& indices);



	void draw();
};

// a list of mesh primitives
struct mesh
{
	std::vector<geometry> geometry;

	void draw()
	{
		for (auto& x : geometry)
		{
			x.draw();
		}
	}
};

struct node
{
	std::string _name;

	node* _parent = nullptr;
	std::vector<std::shared_ptr<node>> _children;

	mesh _graphics;
	
	transform _transform;

	// any other graphics attributes go here too...
	// like lights, particle systems, etc
	// also scripts
	
	// returns the root node
	transform* instantiate(scene_graph& scene, transform* parent = nullptr);
};


// a list of root nodes that compose a scene
struct scene_graph
{
	std::vector<transform*> _roots;

public:
	transform* create(const std::string& name, transform* parent = nullptr)
	{
		transform* entity = create_transform();
		entity->rename(name);

		if (parent)
		{
			entity->set_parent(parent);
		}
		else
		{
			_roots.push_back(entity);
		}

		return entity;
	}

	template<typename T>
	void iterate_depth_first(T&& fn) const
	{
		for (auto& child : _roots)
			child->iterate_depth_first(fn);
	}
private:
	transform* create_transform()
	{
		// for now just create a new transform. this ensures its always valid.
		return new transform();
	}
};


// a root node of a geometry collection
struct model
{
	std::string _name;
	std::shared_ptr<node> _root;

	transform* instantiate(scene_graph& scene, transform* parent = nullptr)
	{
		// instantiate root node in the current scene
		return _root->instantiate(scene, parent);
	}
};

class window;

class model_importer
{
	bool _success = true;
	std::string directory;
public:
	model try_import(window*,const std::string& filepath);

	
	bool success()
	{
		return _success;
	}

private:
	
	std::shared_ptr<node>  load_node(aiNode* AiNode, node* NodeParent, const std::vector<geometry>& scene_meshes)
	{
		printf("loading node: '%s'\n", AiNode->mName.C_Str());


		std::shared_ptr<node> add = std::make_shared<node>();
		
		add->_name = AiNode->mName.C_Str();
		add->_parent = NodeParent;
		if (NodeParent) NodeParent->_children.push_back(add);

		// get the node transform
		aiVector3D scaling;
		aiVector3D axis_rotation;
		ai_real angle_rotation;
		aiVector3D position;
		AiNode->mTransformation.Decompose(scaling,axis_rotation,angle_rotation,position);
		
		
		// set the node transform
		add->_transform.set_position({ position.x,position.y,position.z });
		{
			double s = sin(angle_rotation / 2.0f);
			double x = axis_rotation.x * s;
			double y = axis_rotation.y * s;
			double z = axis_rotation.z * s;
			double w = cos(angle_rotation / 2.0f);
			add->_transform.set_rotation({ x,y,z,w });
		}
		
		add->_transform.set_scale({ scaling.x,scaling.y,scaling.z });

		printf("\tmeshes: %u\n", AiNode->mNumMeshes);
		printf("\tchildren: %u\n", AiNode->mNumChildren);
		
		// set the node meshes
		for (unsigned int i = 0; i < AiNode->mNumMeshes; ++i)
		{
			add->_graphics.geometry.push_back(scene_meshes[AiNode->mMeshes[i]]);
		}

		// create all children nodes
		for (unsigned int i = 0; i < AiNode->mNumChildren; ++i)
		{
			load_node(AiNode->mChildren[i], add.get(), scene_meshes);
		}
		return add;
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

