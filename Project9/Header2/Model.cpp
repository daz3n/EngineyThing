#include "Model.h"
#include "Header.h"


#include <glad/glad.h>

transform* node::instantiate(scene_graph& scene, transform* parent)
{
	// create a clone of this node in the scene graph
	transform* add = scene.create(_name, parent);

	// todo: add a mesh renderer
	// add->graphics = _graphics; // only create a renderer if the graphics exist in this node. 
	
	add->set_position(_transform.position());
	add->set_rotation(_transform.rotation());
	add->set_scale(_transform.scale());
	add->DRAW = &this->_graphics;

	// components are stored in the owning scene
	// not in the entity
	// this is because i want to loop over all components of a certain type
	// todo:
	// add->add_component("MeshRenderer");
	// component* x = add->get_component("mesh_renderer");
	
	
	
	// create a clone of all children in the scene graph too
	for (auto& child : _children)
	{
		child->instantiate(scene, add);
	}

	return add;
}

bool material::set_uniforms()
{
	if (!shader) return false;

	shader->bind();
	

	glActiveTexture(GL_TEXTURE0);
	if (diffuse)
	{
		glBindTexture(GL_TEXTURE_2D, diffuse->image.handle);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glActiveTexture(GL_TEXTURE1);
	if (specular)
	{
		glBindTexture(GL_TEXTURE_2D, specular->image.handle);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	shader->setInt("diffuse", 0);
	shader->setInt("specular", 1);
	return true;

}

model model_importer::try_import(window* win, const std::string& filepath)
{

	std::string str  = filepath.substr(0, filepath.find_last_of('/'));
	if (str.size() == filepath.size())
	{
		str = filepath.substr(0, filepath.find_last_of('\\'));
	}
	directory = str;

	printf("loading model '%s'\n", filepath.c_str());
	printf("from directory: '%s'\n", directory.c_str());


	model ret;
	ret._name = "<unnamed>";

	std::vector<geometry> scene_meshes;


	Assimp::Importer Assimpimporter;
	const aiScene* Scene = Assimpimporter.ReadFile(filepath.c_str(),
		aiPostProcessSteps::aiProcess_Triangulate |

		aiPostProcessSteps::aiProcess_GenNormals |
		aiPostProcessSteps::aiProcess_GenUVCoords |

		

		aiPostProcessSteps::aiProcess_FlipUVs |
		aiPostProcessSteps::aiProcess_OptimizeMeshes |
		aiPostProcessSteps::aiProcess_JoinIdenticalVertices);

	// todo: check for scene validity	


	// load all materials
	// these are always going to be rendered with the same shader...
	std::vector<material*> GeometryMaterials;

	if (Scene->HasMaterials())
	{
		for (unsigned int i = 0; i < Scene->mNumMaterials; ++i)
		{
			const aiMaterial* material = Scene->mMaterials[i];

			::material* mat = win->resources().find_or_create_material(material->GetName().C_Str(), "geometry");
			GeometryMaterials.push_back(mat);

			printf("creating material '%s'\n", mat->_name.c_str());


			aiString texture_path;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texture_path) == aiReturn::aiReturn_SUCCESS)
			{
				std::string filepath = directory + "/" + texture_path.C_Str();
				texture* tex = win->resources().find_or_create_texture(filepath);


				printf("\ttexture diffuse: (handle=%u) '%s'\n", tex->image.handle, filepath.c_str());
				mat->diffuse = tex;
			}
			if (material->GetTexture(aiTextureType_SPECULAR, 0, &texture_path) == aiReturn::aiReturn_SUCCESS)
			{
				std::string filepath = directory + "/" + texture_path.C_Str();
				texture* tex = win->resources().find_or_create_texture(filepath);


				printf("\ttexture specular: (handle=%u) '%s'\n",tex->image.handle, filepath.c_str());
				mat->specular = tex;
			}


			// todo: load all textures when my shader uses them
			aiTextureType_BASE_COLOR;
			aiTextureType_NORMALS;
			aiTextureType_AMBIENT;
			aiTextureType_AMBIENT_OCCLUSION;

		}
	}

	// load all geometry
	if (Scene->HasMeshes())
	{
		for (unsigned int i = 0; i < Scene->mNumMeshes; ++i)
		{
			const aiMesh* AiMesh = Scene->mMeshes[i];
			const aiMaterial* AiMat = Scene->mMaterials[AiMesh->mMaterialIndex];

			printf("creating mesh '%s'\n", AiMesh->mName.C_Str());

			// prep the geometry buffers
			std::vector<vertex> geometry_vertices;
			std::vector<unsigned int> geometry_indices;

			// reserve required space to prevent reallocation
			geometry_vertices.reserve(AiMesh->mNumVertices);
			geometry_indices.reserve(AiMesh->mNumFaces * 3);


			// load vertices
			for (unsigned int j = 0; j < AiMesh->mNumVertices; ++j)
			{
				const auto& vertex_position = AiMesh->mVertices[j];
				const auto& vertex_normal = AiMesh->mNormals[j];
				// const auto& vertex_color = AiMesh->mColors[0][j];
				const auto& vertex_uvs = AiMesh->mTextureCoords[0][j];

				vertex vert;
				vert.position.x = vertex_position.x;
				vert.position.y = vertex_position.y;
				vert.position.z = vertex_position.z;

				vert.normal.x = vertex_normal.x;
				vert.normal.y = vertex_normal.y;
				vert.normal.z = vertex_normal.z;

				// vert.color.r = vertex_color.r;
				// vert.color.g = vertex_color.g;
				// vert.color.b = vertex_color.b;
				// vert.color.a = vertex_color.a;
				
				vert.color.r = 1;
				vert.color.g = 1;
				vert.color.b = 1;
				vert.color.a = 1;

				vert.uv.x = vertex_uvs.x;
				vert.uv.y = vertex_uvs.y;

				geometry_vertices.push_back(vert);
			}

			// load indices
			for (unsigned int j = 0; j < AiMesh->mNumFaces; ++j)
			{
				auto& face = AiMesh->mFaces[j];

				for (unsigned int k = 0; k < face.mNumIndices; ++k)
				{
					geometry_indices.push_back(face.mIndices[k]);
				}
			}


			printf("\tvertices: %u\n", geometry_vertices.size());
			printf("\tindices:  %u\n", geometry_indices.size());
			printf("\tmaterial: '%s'\n", AiMat->GetName().C_Str());
			// materials are stored in the window.
			geometry add;
			add.create_from(geometry_vertices, geometry_indices);

			// todo: set material 
			// the material is created only once. now i just need to find it...
			add.material = GeometryMaterials[AiMesh->mMaterialIndex];
			scene_meshes.push_back(add);
		}
	}


	// load all nodes
	ret._root = load_node(Scene->mRootNode, nullptr, scene_meshes);

	return ret;
}

void texture_image::create(const char* filename)
{
	int img_width, img_height, nrComponents;
	unsigned char* pixels = stbi_load(filename, &img_width, &img_height, &nrComponents, 0);

	if (nrComponents == 1)
		format = GL_RED;
	else if (nrComponents == 2)
		format = GL_RG;
	else if (nrComponents == 3)
		format = GL_RGB;
	else if (nrComponents == 4)
		format = GL_RGBA;

	width = img_width;
	height = img_height;
	target = GL_TEXTURE_2D;
	type = GL_UNSIGNED_BYTE;

	if (pixels == nullptr) 
	{
		printf("failed to create texture: invalid pixel data\n");
		return;
	}


	glGenTextures(1, &handle);
	glBindTexture(target, handle);

	glTexImage2D(
		target,  // target 
		0,       // level
		format,  // internal format
		width,   // width
		height,  // height
		0,       // border
		format,  // format
		type,    // type
		pixels   // data
	);

	stbi_image_free(pixels);
}

void texture_sampler::setup(texture_image& img) const
{
	glBindTexture(img.target, img.handle);

	for (const auto& x : parameters)
	{
		glTexParameteri(img.target, x.param, x.value);
	}
}

void accessor::setup(buffer& buf) const
{
	glBindBuffer(buf.type, buf.handle);

	int index = 0;

	for (const auto& x : elements)
	{
		glVertexAttribPointer(index,
			x.count,
			x.format,
			GL_FALSE,
			stride,
			(void*)x.pointer
		);

		glEnableVertexAttribArray(index);
		++index;

	}

	glBindBuffer(buf.type, 0);
}

void geometry::create_from(const std::vector<vertex>& vertices, const std::vector<unsigned int>& indices)
{
	mode = GL_TRIANGLES;
	type = GL_UNSIGNED_INT;
	count_indices = indices.size();
	count_vertices = vertices.size();

	glGenVertexArrays(1, &handle_vao);
	glBindVertexArray(handle_vao);

	if (!vertices.empty())
	{
		glGenBuffers(1, &handle_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, handle_vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), vertices.data(), GL_STATIC_DRAW);

		// position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, position));
		glEnableVertexAttribArray(0);
		// normal
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, normal));
		glEnableVertexAttribArray(1);
		// color
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, color));
		glEnableVertexAttribArray(2);
		// uv
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, uv));
		glEnableVertexAttribArray(3);

	}

	if (!indices.empty())
	{
		glGenBuffers(1, &handle_ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
	}
}

void geometry::draw()
{
	if (material == nullptr) 
	{
		return;
	}
	
	material->set_uniforms();
	
	glBindVertexArray(handle_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle_ebo);

	if (count_indices)
	{
		glDrawElements(mode, count_indices, type, 0);
	}
	else if (count_vertices)
	{
		glDrawArrays(mode, 0, count_vertices);
	}
	else
	{
		// no geometry???
		__debugbreak();
	}
}
