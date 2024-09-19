#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <set>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <chrono>


#include "Model.h"


#include "../utils/math/vector.hpp"
#include "../utils/math/matrix.hpp"
#include "../utils/event/action.h"

#include "../utils/event/message.h"

// vcpkg
#include <glad/glad.h>
#include <glfw/glfw3.h>
// vcpkg

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class model;
class material;
class shader;
class texture;

// todo:
	// window::resources::load_model()

	// im having issues with binding a shader.
		// i can render things if i dont call bind
		// i cant if i do?


// todo: grass!!!
// https://www.shadertoy.com/view/4dBcDV
// https://www.shadertoy.com/view/dd2cWh
// https://www.reddit.com/r/godot/comments/13sl4o8/open_source_stylized_sky_shader_and_a_simple_day/

/*
	shaders must be put into Resources//Shaders//
	
	file extensions are:
		.vert
		.frag
		.geom

	each shader must be named after its' renderpass.
	the depth pass automatically binds and creates the depth shader. etc.

	
	the shader called compose will be used to draw all of the scene geometry.
	depth, gbuffer and lighting will all be handled automatically.
*/



namespace settings
{
	enum 
	{
		print_graphics = 0
	};
}

struct input_msg_keydown
{
	enum { TYPE = 0 };
	int key;
};
struct input_msg_keyup
{
	enum { TYPE = 1 };
	int key;
};
struct input_msg_key
{
	enum { TYPE = 2 };
	int key;
	int action;
};


struct input_msg_mouse_move
{
	enum { TYPE = 3 };
	vec2 pos;
};

struct input_msg_mouse_drag
{
	enum { TYPE = 4 };
	vec2 start;
	vec2 end;
};
struct input_msg_mouse_down
{
	enum { TYPE = 5 };
	int button;
};
struct input_msg_mouse_up
{
	enum { TYPE = 6 };
	int button;
};
struct input_msg_mouse_button
{
	enum { TYPE = 7 };
	int button;
	int action;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// next steps: 
	// pbr rendering
	// animation
	// shadows
	// scripting
	// idk i cba



class script {};


class window;
struct world;

class scene_stack
{
	window* _root;
	std::vector<world*> scenes;
public:
	scene_stack(window* owner) : _root(owner) {}

	void push(world* top)
	{
		scenes.push_back(top);
	}
	world& current()
	{
		return *scenes.back();
	}
	bool pop(world*)
	{
		return false;
	}
};

class fps_counter
{
	const size_t min_fps = 10; 
	const size_t max_fps = 60; 
	
	// statistics:
		// average fps
		// rolling average
		// last frame time
	const double alpha = 0.75;
	double statistics[3]{};
	size_t frame_index = 0;

	// time tracking
	typedef std::chrono::steady_clock::time_point time;
	time last_frame;
	
public:
	void init()
	{
		last_frame = std::chrono::steady_clock::now();
	}
	void tick()
	{
		time this_frame = std::chrono::steady_clock::now();
		auto delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(this_frame - last_frame);
		tick(delta_time.count());
		last_frame = this_frame;
	}
	double average_fps() const
	{
		// 1.0 / statistics[0] / frame_index;

		const double seconds = statistics[0] / 1000;
		return clamped_fps(double(frame_index) / seconds);
	}
	double rolling_fps() const // sum of frametimes. fps = 1/frametime
	{
		const double seconds = statistics[1] / 1000;
		return clamped_fps(1.0 / seconds);
	}
	double last_frametime() const
	{
		return statistics[2];
	}
private:
	void tick(double delta_time)
	{
		++frame_index;

		// average fps
		statistics[0] += delta_time;
		// last frame time
		statistics[2] = delta_time;


		// rolling average fps
		double rolling_1 = statistics[1] * (1 - alpha);
		double rolling_2 = delta_time * (alpha);
		double rolling_3 = (rolling_1 + rolling_2);
		statistics[1] = rolling_3;
	}
	inline double clamped_fps(double in) const
	{
		return in > max_fps ? max_fps : in < min_fps ? min_fps : in; 
	}
};
struct uniform_buffer
{
	std::string _name;

	unsigned int _slot;
	unsigned int _size;
	unsigned int _handle;
};
struct Buffer_Data_Window
{
	vec2 _size;
	vec2 _resolution;
	
	float _runtime;
	float _playtime;

	int _random;
};
struct Buffer_Data_Camera
{
	vec4 _target;
	vec4 _position;
	mat4 _view;
	mat4 _proj;
	float _fov = 70;
};
struct Buffer_Data_Lights
{
	struct Buffer_Data_Light
	{
		vec4 pos_radius;
		vec4 color_intensity;
	};


	vec4 directional_light_position;
	vec4 directional_light_color_intensity;

	Buffer_Data_Light point_lights[50];
};


class window
{
	inline static bool Initialised = false;
	
private:
	void copy_to_zerod(int& a, int& b);
private:
	void update_resolution(int X, int Y);
	void update_handle();
private:
	std::unordered_map<std::string, uniform_buffer> _buffers;
	template<typename T>
	void create_buffer(unsigned int slot, string name, unsigned int buffertype = GL_UNIFORM_BUFFER)
	{
		unsigned int uboExampleBlock;
		glGenBuffers(1, &uboExampleBlock);

		glBindBuffer(buffertype, uboExampleBlock);
		glBufferData(buffertype, sizeof(T), NULL, GL_STATIC_DRAW); // allocate 152 bytes of memory

		glBindBufferBase(buffertype, slot, uboExampleBlock);
		

		_buffers[name]._name = name;
		_buffers[name]._handle = uboExampleBlock;
		_buffers[name]._size = sizeof(T);
		_buffers[name]._slot = slot;
	}
	

	void update_buffer(string name, int Size, void* data, unsigned int buffertype = GL_UNIFORM_BUFFER);
private:
	void create_handle();
	void init_buffers();
	void init_shaders();
	void init_materials();
	
private:
	void update_buffers();

public:
	window();

	struct subsystem
	{
		subsystem(window* root) : _root(root) {}

		window* _root;
	};
	

	// window
	friend struct draw_context;
	struct draw_context : public subsystem
	{
		using subsystem::subsystem;

		draw_context& target_framerate(int _Hz)
		{
			_dirty = true;
			_framerate = _Hz;
			return *this;
		}
		int target_framerate()
		{
			return _framerate;
		}

		draw_context& output_resolution(int X, int Y)
		{
			_dirty = true;
			_resolution_x = X;
			_resolution_y = Y;
			return *this;
		}
		int output_resolution_x()
		{
			return _resolution_x;
		}
		int output_resolution_y()
		{
			return _resolution_y;
		}


		draw_context& brightness(int _val)
		{
			_dirty = true;
			return *this;
		}
		int brightness()
		{
			return 1;
		}

		draw_context& fullscreen(bool _val)
		{
			_dirty = true;
			_fullscreen = _val;
			return *this;
		}
		bool fullscreen() 
		{
			return _fullscreen;
		}

		int width()
		{
			return _width;
		}
		int height()
		{
			return _height;
		}
		string title()
		{
			return _title;
		}

		draw_context& set_width(int w)
		{
			_dirty = true;
			_width = w;
			return *this;
		}
		draw_context& set_height(int h)
		{
			_dirty = true;
			_height = h;
			return *this;
		}
		draw_context& set_size(int x, int y)
		{
			_dirty = true;
			_width = x;
			_height = y;
			return *this;
		}
		draw_context& set_title(string _str)
		{
			_dirty = true;
			_title = _str;
			return *this;
		}

		void done()
		{
			if (_dirty)
			{
				_root->_graphics = *this;
				_root->update_handle();
			}
		}
	private:
		int _width;
		int _height;
		int _position_x;
		int _position_y;
		string _title;
	
		int _framerate;
		int _resolution_x;
		int _resolution_y;

		bool _fullscreen;

	private:
		bool _dirty = false;
	};
	draw_context& graphics() { return _graphics; }

	// audio
	struct sound_context : public subsystem
	{
		using subsystem::subsystem;
		// updates the settings
		~sound_context(){}

		sound_context& mute(bool) { return *this; }
		sound_context& master(double) { return *this; }
		sound_context& ui(double) { return *this; }
		sound_context& voice(double) { return *this; }
		sound_context& effect(double) { return *this; }
	private:
		double _master = 1;

		double _ui = 1;      // clicks, sliders, etc
		double _voice = 1;   // dialogue
		double _effect = 1; // guns, bombs, swords, fire, ...
	};
	sound_context audio() { return _audio; }

	// loaded resources
	struct resource_context : public subsystem
	{
		using subsystem::subsystem;

		string loading_scene() { return "loading.fbx"; }
		string loading_texture() { return "loading.png"; }
		string title_scene() { return "title.fbx"; }

		bool load_shaders()
		{
			if (not std::filesystem::exists("Resources//Shaders")) return false;

			// find each shader
			std::set<std::string> shadernames;
			for (auto entry : std::filesystem::directory_iterator("Resources//Shaders"))
			{
				auto Path = entry.path();
				std::string P = Path.string();
				std::string S = Path.filename().stem().string();
				shadernames.insert(S);
			}

			// create each shader
			_shaders.reserve(shadernames.size());
			for (const auto& x : shadernames)
			{
				printf("loading shader '%s'\n", x.c_str());
				auto obj = std::make_unique<shader>(x);
				obj->build();
				_shaders.push_back(std::move(obj));
			}

			return true;
		}
		shader* find_shader(const std::string& name)
		{
			for (auto& x : _shaders) if (x->name() == name) return x.get();
			return nullptr;
		}


		bool load_model(const std::string name, const std::string model_path)
		{
			model_importer importer;
			
			model add = importer.try_import(_root, model_path);

			// this is the label used to instantiate the asset, different to the name of the root node.
			add._name = name;

			if (importer.success())
			{
				printf("LOADED MODEL '%s' FROM '%s'\n", name.c_str(), model_path.c_str());
				_models.push_back(std::make_unique<model>(add));
				return true;
			}
			else
			{
				printf("failed to load model '%s' from '%s'\n", name.c_str(), model_path.c_str());
			}
			

			return false;
		}
		bool unload_model(const std::string& name)
		{
			__debugbreak();

			for (auto& x : _models)
			{
				if (x->_name == name)
				{
					// unload
					return true;
				}
			}

			return false;
		}
		model* find_model(const std::string& name) // = "backpack"
		{
			for (auto& x : _models) if (x->_name == name) return x.get();
			return nullptr;
		}
		

		material* find_material(const std::string& name)
		{
			for (auto& x : _materials) if (x->_name == name) return x.get();
			return nullptr;
		}
		material* find_or_create_material(const std::string& name, const std::string& shader_name)
		{
			if (auto* ret = find_material(name)) return ret;

			// find the shader to base the material off of
			shader* s = find_shader(shader_name);
			assert(s);

			// create material based on a shader 
			material mat = s->create_material();
			mat._name = name;

			// store reference to the new material
			_materials.push_back(std::make_unique<material>(mat));
			return _materials.back().get();
		}

		texture* find_texture(const std::string& path)
		{
			for (auto& x : _textures) if (x->_path == path) return x.get();
			return nullptr;
		}

		texture* find_or_create_texture(const std::string& path)
		{
			if (auto* ret = find_texture(path)) return ret;

			texture tex;
			// prepare the sampling settings
			tex.sampler.parameters.push_back({GL_TEXTURE_MIN_FILTER, GL_LINEAR });
			tex.sampler.parameters.push_back({GL_TEXTURE_MAG_FILTER, GL_LINEAR });
			tex.sampler.parameters.push_back({GL_TEXTURE_WRAP_S, GL_CLAMP });
			tex.sampler.parameters.push_back({GL_TEXTURE_WRAP_T, GL_CLAMP });

			// create the actual texture
			tex.image.create(path.c_str());
			tex.sampler.setup(tex.image);

			// save a reference to the textures
			_textures.push_back(std::make_unique<texture>(tex));
			return _textures.back().get();
		}
	private:
		// todo:
		// all of these should be pointers to resources. not just resources.
		// this is so that they are never invalidated.

		// keeps all materials in memory
		std::vector<std::unique_ptr<material>> _materials;
		
		// keeps all shaders in memory
		std::vector<std::unique_ptr<shader>> _shaders;

		// keeps all models in memory
		std::vector<std::unique_ptr<model>> _models;

		// keeps all textures in memory
		std::vector<std::unique_ptr<texture>> _textures;

		// keeps all scripts in memory
		std::vector<std::unique_ptr<script>> _scripts;
	};
	resource_context& resources() { return _resources; }

	// scene loader
	struct load_context : public subsystem
	{
		using subsystem::subsystem;

		scene_stack& active()
		{
			return _root->_scenes;
		}
	};
	load_context scenes() { return this; }

	// hooks
	class hook_context : public subsystem
	{
		std::unordered_map<std::string, std::vector<action_with<double>>> _hooks;
	public:
		using subsystem::subsystem;

		hook_context& add(const std::string& when, action_with<double> _hook)
		{
			_hooks[when].push_back(_hook);
			return *this;
		}
		void call(const std::string& name, double val)
		{
			for (auto& x : _hooks[name]) x(val);
		}
	};
	hook_context& hooks() { return _subsystem_hooks; }

	// timing
	class time_context : public subsystem
	{
		std::chrono::steady_clock::time_point startup_time;
		std::chrono::steady_clock::time_point start_play_time;

	private:
		void hook_on_play(double)
		{
			start_play_time = std::chrono::steady_clock::now();
		}
	public:
		time_context(window* _root) : subsystem(_root)
		{
			startup_time = std::chrono::steady_clock::now();

			action_with<double> Hook;
			Hook.assign<&time_context::hook_on_play>(this);
			_root->hooks().add("play", Hook);
		}
		double since_startup() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startup_time).count(); }
		double since_last_play() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_play_time).count(); }
	};
	time_context& time() { return _time; }

	// renderer specific
	class renderpass
	{
	private:
		template<typename ...T>
		void print_on_update(T&&...args)
		{
			if (settings::print_graphics)
			{
				printf(std::forward<T>(args)...);
			}
		}
		template<typename ...T>
		void print_on_create(T&&...args)
		{
			if (settings::print_graphics)
			{
				printf(std::forward<T>(args)...);
			}
		}
	private:
		void copy_render_textures()
		{
			if (_depth_texture_source.source_handle != 0)
			{
				auto res = _root->pipeline().resolution();

				glBindFramebuffer(GL_READ_FRAMEBUFFER, _depth_texture_source.source_handle);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _handle);
				glBlitFramebuffer(0, 0, res.x, res.y, 0, 0, res.x, res.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
			}
		}
		void clear_render_textures()
		{
			unsigned int flags = 0;

			if (will_clear_color()) flags |= GL_COLOR_BUFFER_BIT;
			if (will_clear_depth()) flags |= GL_DEPTH_BUFFER_BIT;
			if (will_clear_stencil()) flags |= GL_STENCIL_BUFFER_BIT;

			glClearColor(_clear_rgb.r, _clear_rgb.g, _clear_rgb.b, 1.0f);
			glClear(flags);
		}
		void bind_shader_if_exists()
		{
			if (_automatic_shader)
			{
				print_on_update("\tbinding %s shader\n", _automatic_shader->name().c_str());
				_automatic_shader->bind();
			}
		}
		void bind_framebuffer()
		{
			print_on_update("\tbinding %u render textures in %s renderbuffer (id=%u)\n", unsigned(_textures), _name.c_str(), _handle);
			for (auto x : _texture_handles)
				print_on_update("\t(handle=%u)\n", x);

			glBindFramebuffer(GL_FRAMEBUFFER, _handle);
		}
		void set_render_state()
		{
			if (depth()) 
			{
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(_depth_func);
			}
			else 
			{
				glDisable(GL_DEPTH_TEST);
			}
			
			
			if (stencil()) glEnable(GL_STENCIL_TEST);
			else glDisable(GL_STENCIL_TEST);

		}
		
	private:
		unsigned int internal_texture_format() const 
		{
			return 
				_channels == 4 ? GL_RGBA :
				_channels == 3 ? GL_RGB :
				_channels == 2 ? GL_RG :
				GL_RED;
		}
		unsigned int internal_texture_storage() const 
		{
			unsigned int StorageDataType = 0;
			if (_is_hdr) StorageDataType = GL_FLOAT;
			if (!_is_hdr) StorageDataType = GL_UNSIGNED_BYTE;
			return StorageDataType;
		}
		unsigned int internal_texture_sized_format() const
		{
			unsigned int SizedFormat = internal_texture_format();
			if (is_hdr())
			{
				switch (SizedFormat)
				{
				case GL_RED: SizedFormat = GL_R32F; break;
				case GL_RG: SizedFormat = GL_RG32F; break;
				case GL_RGB: SizedFormat = GL_RGB32F; break;
				case GL_RGBA: SizedFormat = GL_RGBA32F; break;
				}
			}
			return SizedFormat;
		}
		unsigned int create_framebuffer_texture(vec2 TargetSize = vec2(0))
		{
			const unsigned int StorageDataType = internal_texture_storage();
			const unsigned int _Format = internal_texture_format();
			const unsigned int SizedFormat = internal_texture_sized_format();
			
			 

			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);

			if (TargetSize.zero())
			{
				TargetSize = _root->pipeline().resolution();
			}

			glTexImage2D(GL_TEXTURE_2D, 0, SizedFormat, TargetSize.x, TargetSize.y, 0, _Format, StorageDataType, NULL);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			print_on_create("created framebuffer texture (index=%i, handle = %u) for %s pass\n", _texture_handles.size(), texture, name().c_str());
			return texture;
		}
		bool create_and_attach_framebuffer_depth_stencil()
		{
			if (_depth > 0)
			{
				unsigned int texture;
				glGenTextures(1, &texture);
				glBindTexture(GL_TEXTURE_2D, texture);

				vec2 TargetResolution = _root->pipeline().resolution();
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, TargetResolution.x, TargetResolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				_depth_handle = texture;
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
			}

			return true;
		}
		bool check_completion()
		{
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
			{
				return true;
			}
			print_on_create("failed to complete framebuffer %s\n", _name.c_str());
			__debugbreak();

			return false;
		}

		bool create_framebuffer()
		{
			glGenFramebuffers(1, &_handle);
			glBindFramebuffer(GL_FRAMEBUFFER, _handle);

			std::vector<unsigned int> draw_buffers;
			
			for (int i = 0; i < _textures; ++i)
			{
				unsigned int tex = create_framebuffer_texture();
				_texture_handles.push_back(tex);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, tex, 0);
				draw_buffers.push_back(GL_COLOR_ATTACHMENT0 + i);				
			}
			glDrawBuffers(draw_buffers.size(), draw_buffers.data());
			
			create_and_attach_framebuffer_depth_stencil();
			return check_completion();
		}
		bool delete_framebuffer()
		{
			// also deletes all textures bound
			glDeleteFramebuffers(1, &_handle);

			return false;
		}
	public:

		renderpass(window* _owner, bool _is_hdr, string _name, int _channels, int _textures, int _depth, int _stencil)
		{
			this->_root = _owner;
			this->_is_hdr = _is_hdr;
			this->_name = _name;
			this->_channels = _channels;
			this->_textures = _textures;
			this->_depth = _depth;
			this->_stencil = _stencil;
			this->_depth_func = GL_LESS;

			this->_enabled = true;
			this->_callback.noop();

			this->_automatic_shader = _owner->resources().find_shader(_name);
			this->_secondary_shader = nullptr;

			clear_flags(true, true, true);
			create_framebuffer();
		}
		void enable(bool _enabled)
		{
			this->_enabled = _enabled;
		}
		bool resize(int X, int Y)
		{
			delete_framebuffer();
			create_framebuffer();
			return true;
		}

		renderpass& function(action_with<const renderpass*, const scene_graph&> _callback)
		{
			this->_callback = _callback;
			return *this;
		}

		void update(scene_graph& entities)
		{
			print_on_update("beginning %s renderpass\n",name().c_str());

			bind_framebuffer();
			set_render_state();
			clear_render_textures();
			copy_render_textures();
			bind_shader_if_exists();

			bind_required_sources();
			_callback(this, entities);
			
			print_on_update("---");
		}

	public: // renderpass behaviours
		renderpass& define_texture_source(const std::string& previous_pass, int texture_from_index, int texture_bind_target)
		{
			required_texture req{};
			req.handle = 0;
			req.source_name = previous_pass;
			req.target = texture_bind_target;
		
			
			req.handle = _root->pipeline().find(previous_pass)->texture_handle(texture_from_index);

			_required_textures.push_back(req);

			return *this;
		}
		renderpass& define_depth_source(const std::string& previous_pass)
		{
			_depth_texture_source.source_name = previous_pass;
			_depth_texture_source.source_handle = _root->pipeline().find(previous_pass)->handle();
			return *this;
		}
	public:
		// immutable state queries.
		bool enabled() const { return _enabled; }
		unsigned int handle() const { return _handle; }
		string name() const { return _name; }
		int channels() const { return _channels; }
		int textures() const { return _textures; }
		int depth() const { return _depth; }
		int stencil() const { return _stencil; }
		bool is_hdr() const { return _is_hdr; }
		bool will_clear_color() const { return _clear_color; }
		bool will_clear_depth() const { return _clear_depth; }
		bool will_clear_stencil() const { return _clear_stencil; }
		bool will_bind_shader() const { return _automatic_shader != nullptr; }
		vec3 clear_color() const { return _clear_rgb; }
		shader* automatic_shader() const { return _automatic_shader; }
		shader* secondary_shader() const { return _secondary_shader; }
		unsigned int depth_func() const { return _depth_func; }

		// mutable state.
		renderpass& clear_color(vec3 col) { _clear_rgb = col; return *this; }
		renderpass& clear_flags(bool color, bool depth, bool stencil)
		{
			_clear_color = color;
			_clear_depth = depth && _depth; // no point clearing depth if there is no depth texture
			_clear_stencil = stencil && _stencil; // no point clearing stencil if there is no stencil texture
			return *this;
		}
		renderpass& depth_func(unsigned int func) { _depth_func = func; return *this; }
		renderpass& set_automatic_shader(const std::string& _name)
		{
			_automatic_shader = _root->resources().find_shader(_name);
			return *this;
		}
		renderpass& set_secondary_shader(const std::string& _name)
		{
			_secondary_shader = _root->resources().find_shader(_name);
			return *this;
		}	
		renderpass& generate_texture_queue(const std::vector<vec2>& scales) // this only works if textures == 1
		{
			assert(_textures == 1);

			
			for (auto scale : scales)
			{
				const vec2 siz = scale * _root->pipeline().resolution();
				const auto ref = create_framebuffer_texture(siz);
				_texture_queue_handles.push_back(ref);
				_texture_queue_sizes.push_back(siz);
			}
			return *this;
		}

		// handle stuff
		unsigned int texture_handle(unsigned int idx) const { return _texture_handles[idx]; }
		unsigned int queued_texture_handle(unsigned int idx) const { return _texture_queue_handles[idx]; }
		unsigned int depth_handle() const { return _depth_handle; }
		unsigned int stencil_handle() const { return _stencil_handle; }
		
		
		
		// utils for the render function
		unsigned int ping(unsigned int iteration) const
		{
			return iteration & 1;
		}
		unsigned int pong(unsigned int iteration) const
		{
			return ++iteration & 1;
		}
		void swap_draw_texture(unsigned int target_handle) const 
		{
			glBindFramebuffer(GL_FRAMEBUFFER, _handle);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target_handle, 0);
		}
		void swap_read_texture(unsigned int target_handle) const
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, target_handle);
		}
		void reset_draw_texture() const
		{
			unsigned int target_handle = texture_handle(0);
			swap_draw_texture(target_handle);
		}
		
	private:
		// frame layout
		bool _enabled;
		bool _is_hdr;
		string _name;
		int _channels;
		int _textures;
		int _depth;
		int _stencil;

	private:
		// operation args
		vec3 _clear_rgb;
		bool _clear_color;
		bool _clear_depth;
		bool _clear_stencil;
		unsigned int _depth_func;
	private:
		// handles
		unsigned int _handle;
		std::vector<unsigned int> _texture_handles;
		std::vector<unsigned int> _texture_queue_handles; // separate vector because its easier.
		std::vector<vec2u> _texture_queue_sizes;

		unsigned int _depth_handle;
		unsigned int _stencil_handle;

	private: // render prep
		struct required_texture
		{
			std::string source_name;
			int handle = 0;
			int target = 0;
		};
		struct prepass_handle
		{
			std::string source_name;
			unsigned int source_handle;
		};
		window* _root;
		shader* _automatic_shader;
		shader* _secondary_shader;
		action_with<const renderpass*, const scene_graph&> _callback;
		std::vector<required_texture> _required_textures;
		prepass_handle _depth_texture_source;

		void bind_required_sources()
		{
			// bind read textures
			for (const auto& req : _required_textures)
			{
				print_on_update("\tbinding texture (handle=%u) from %10s prepass to (texture_slot=%i)\n", req.handle, req.source_name.c_str(), req.target);

				glActiveTexture(GL_TEXTURE0 + req.target);
				glBindTexture(GL_TEXTURE_2D, req.handle);
			}
		}
		void print_shader_uniforms()
		{
			if (_automatic_shader)
			{
				GLuint programID = _automatic_shader->handle();

				// Get the total number of active uniforms in the shader program
				GLint numActiveUniforms = 0;
				glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &numActiveUniforms);

				// Iterate through each uniform
				for (GLuint i = 0; i < numActiveUniforms; ++i) {
					// Buffer to store uniform information
					GLsizei bufSize = 256; // Adjust this buffer size as necessary
					GLchar name[256];
					GLint size;
					GLenum type;
					glGetActiveUniform(programID, i, bufSize, nullptr, &size, &type, name);

					// Get the location of the uniform
					GLint location = glGetUniformLocation(programID, name);

					// Output uniform information
					printf("Uniform #%d: Name=%s, Size=%d, Type=%u, Location=%d\n",
						i, name, size, type, location);
				}
			}
		}
	};
	struct swapchain : public subsystem
	{
		using subsystem::subsystem;

		renderpass& push(bool is_hdr, string _name, int _channels, int _textures, int _depth = 24, int _stencil = 0)
		{
			if (_depth == int(true)) _depth = 24;
			if (_stencil == int(true)) _stencil = 8;

			return _passes.emplace_back(_root, is_hdr, _name, _channels, _textures, _depth, _stencil);
		}

		void resize(int X, int Y)
		{
			if (_width != X || _height != Y)
			{
				_width = X;
				_height = Y;
				for (auto& it : _passes)
				{
					it.resize(X, Y);
				}
			}
		}

		void render(scene_graph& entities)
		{
			if (settings::print_graphics)
			{
				printf("rendering entities\n");
			}

			for (auto& pass : _passes)
			{
				pass.update(entities);
			}

			// the final pass composites into an output texture.
			// if the final pass outputs more than 1 texture, I can switch between which one is displayed by choosing its index here.
			

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT);

			if (not _passes.empty())
			{
				if (settings::print_graphics)
				{
					printf("\n\ndisplaying output (source=%ux%u)\n---\n", unsigned(_width), unsigned(_height));
				}

				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

				glBindFramebuffer(GL_READ_FRAMEBUFFER, _passes.back().handle());

				glBlitFramebuffer(0, 0, _width, _height, 0, 0, _width, _height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}
		}

		renderpass* find(const std::string& name)
		{
			for (auto& x : _passes) if (x.name() == name) return &x;
			return nullptr;
		}

		vec2 resolution() const 
		{
			return vec2(_width, _height);
		}
	private:
		int _width;
		int _height;
		std::vector<renderpass> _passes;
	};
	swapchain& pipeline() { return _pipeline; }

	// scripting
	struct script_context : public subsystem
	{
		using subsystem::subsystem;
		// create scripts in text for each object.

		// a script function continues until the next section header is reached.
		// section headers
		// [action]		= a callable function attached to an object. 
		// [response]   = a listener that is called when an event is recieved. (key, mouse, collision, etc)
		// [lifetime]    = a function attached to an object. called when the age of the object reaches the specified value.
		
		bool start();

	private:
	};
	script_context engine() { return this; }

	// input state
	class input_context : public subsystem
	{
		
	public:
		using subsystem::subsystem;
	};
	input_context& input()
	{
		return _input;
	}

	void tick();
	void play();

	bool should_close();

	fps_counter& fps() { return _fps; }
private:
	fps_counter _fps;
private:
	hook_context _subsystem_hooks{ this };

	draw_context _graphics{this};
	sound_context _audio{ this };
	resource_context _resources{ this };
	load_context _loader{ this };
	swapchain _pipeline{ this };

	time_context _time{ this };

	input_context _input{ this };
private:
	scene_stack _scenes{ this };
private:
	GLFWwindow* handle;
};

struct world
{
	world(window* owner) : 
		_root(owner) 
	{
		_root->scenes().active().push(this);
	}
	~world()
	{
		_root->scenes().active().pop(this);
	}
	

	struct subsystem
	{
		subsystem(world* root) : _root(root) {}

		world* _root;
	};
	window* owner() { return _root; }

	// view
	class view_context : public subsystem
	{
	private:
		void recalculate_view() 
		{
			_data._view = projection::look_at(_data._position, _data._target);
		}
		void recalculate_proj()
		{
			vec2 TargetSize = _root->owner()->pipeline().resolution();
			float aspect = TargetSize.x / TargetSize.y;
			_data._proj = projection::perspective(70, aspect, 0.1, 1000);
		}
	public:
		using subsystem::subsystem;


		view_context& move_to(vec3 position)
		{
			_data._position = position;
			_is_view_dirty = true;
			return *this; 
		}
		view_context& look_at(vec3 target)
		{
			_data._target = target;
			_is_view_dirty = true;
			return *this; 
		}

		// change the fov of the camera
		view_context& zoom(double fov)
		{
			_data._fov = fov;
			_is_proj_dirty = true;
			return *this;
		}

		Buffer_Data_Camera& data()
		{
			if (_is_view_dirty)
			{
				_is_view_dirty = false;
				recalculate_view();
			}
			if (_is_proj_dirty)
			{
				_is_proj_dirty = false;
				recalculate_proj();
			}
			return _data;
		}

	private:
		bool _is_view_dirty = false;
		bool _is_proj_dirty = false;
		
		Buffer_Data_Camera _data{};
	};
	view_context& camera() { return _camera; }

	// misc
	struct environment
	{
		environment& skybox(string name, string extension) { return *this; }
	};
	environment background() { return _background; }

	// loading
	struct load_context
	{
		bool loading();
	};
	load_context load(string scene_path) 
	{

		return {}; 
	}

#if 0
	// entities
	placement_context generate(int start_points) { return {}; }

	object create(string item, vec3 pos) { return {}; }
#endif 

	scene_graph& entities() { return _entities; }
private:
	window* _root;
	scene_graph _entities;
private:
	view_context _camera{ this };
	environment _background;
};

// this will need a rework just to be nice and tidy
struct graphics
{
	window* Window;
public:
	graphics(window* Win, int BloomCount = 10) : Window(Win),Bloom(BloomCount) {}
	inline void draw_geometry_depth_pass(const window::renderpass* pass, const scene_graph& entities)
	{
		// depth framebuffer is already bound
		// depth shader is already bound
		

		// todo:
		// collect all entities with a mesh renderer component
		// render the mesh geometry with the mesh material


		entities.iterate_depth_first([&](transform* entity) 
			{
				pass->automatic_shader()->setMat4("model", entity->global_transform());
				component* graphics = entity->get_component("mesh_renderer");


				// for (const auto& x : entity->geometry->meshes)
				// {
				// 	glBindVertexArray(x.vao);
				// 	glDrawElements(GL_TRIANGLES, x.siz, GL_UNSIGNED_INT, NULL);
				// }
			});
#if 0
		// draws all geometry
		for (const auto& entity : entities)
		{
			pass->automatic_shader()->setMat4("model", entity.transform);
			if (settings::print_graphics)
			{
				printf("drawing %s (meshes = %u)\n", entity.type.c_str(), entity.geometry->meshes.size());
			}

			for (const auto& x : entity.geometry->meshes)
			{
				glBindVertexArray(x.vao);
				glDrawElements(GL_TRIANGLES, x.siz, GL_UNSIGNED_INT, NULL);
			}
		}
#endif 

	}
	inline void draw_geometry_to_buffers(const window::renderpass* pass, const scene_graph& entities)
	{
		// g buffer is bound
		// material shader is already bound

		entities.iterate_depth_first([&](transform* entity)
			{
				pass->automatic_shader()->setMat4("model", entity->global_transform());
				// pass->automatic_shader()->setMat4("model", mat4(1));
				
				if (entity->DRAW)
				{
					entity->DRAW->draw();
				}
			});
#if 0
		for (const auto& entity : entities)
		{
			// entity.geometry->global = transform_matrix::rotate(entity.geometry->global, { 0,1,0 }, 0.5);
			// entity.geometry->global = transform_matrix::scale(entity.geometry->global, { 1.0001,1,1.0001 });

			
			pass->automatic_shader()->setMat4("model", entity.transform);
			pass->automatic_shader()->setVec3("albedo", entity.albedo);
			
			if (settings::print_graphics)
			{
				printf("drawing %s (meshes = %u)\n", entity.type.c_str(), entity.geometry->meshes.size());
			}
			pass->automatic_shader()->setVec3("glow", vec3(0.2, 0.2, 1.0));
			for (const auto& x : entity.geometry->meshes)
			{
				pass->automatic_shader()->setInt("diffuse", 0);
				pass->automatic_shader()->setInt("specular", 1);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, x.Diffuse.handle);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, x.Specular.handle);

				glBindVertexArray(x.vao);
				glDrawElements(GL_TRIANGLES, x.siz, GL_UNSIGNED_INT, NULL);
				// pass->automatic_shader()->setVec3("glow", vec3(0.0));
			}
		}
#endif 
		// draws all geometry
	}
	inline void draw_lighting_to_texture(const window::renderpass* pass,const scene_graph&)
	{
		// lighting buffer is bound
		// lighting shader is already bound
		// textures from g buffer are already bound

		glDrawArrays(GL_TRIANGLES, 0, 6);
		// draws lighting
	}
	inline void apply_effect_bloom(const window::renderpass* pass, const scene_graph&)
	{
		int i = 0;
		// glEnable(GL_BLEND);
		// glBlendFunc(GL_ONE, GL_ONE);
		// glBlendEquation(GL_FUNC_ADD);

		glDisable(GL_BLEND);
		// first pass needs to read the input image
		pass->swap_draw_texture(pass->queued_texture_handle(pass->ping(i)));
		pass->automatic_shader()->setInt("horizontal", 0);
		if (settings::print_graphics)
		{
			printf("---reading from input\n");
		}
		glDrawArrays(GL_TRIANGLES, 0, 6);


		for (i = 0; i < Bloom * 5; ++i)
		{
			// draw to first queued texture
			pass->swap_read_texture(0); // these have to be cleared first?
			pass->swap_draw_texture(0); // these have to be cleared first?
			pass->swap_read_texture(pass->queued_texture_handle(pass->ping(i))); // this has to start at index 0
			pass->swap_draw_texture(pass->queued_texture_handle(pass->pong(i))); // so this has to start at index 1
			pass->automatic_shader()->setInt("horizontal", 1);
			glDrawArrays(GL_TRIANGLES, 0, 6);			


			// draw to second queued texture 
			pass->swap_read_texture(0); // these have to be cleared first?
			pass->swap_draw_texture(0); // these have to be cleared first?
			pass->swap_read_texture(pass->queued_texture_handle(pass->pong(i))); // this has to start at index 0
			pass->swap_draw_texture(pass->queued_texture_handle(pass->ping(i))); // so this has to start at index 1
			pass->automatic_shader()->setInt("horizontal", 0);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		// draw to last bloom
		++i;
		pass->swap_read_texture(0); // these have to be cleared first?
		pass->swap_draw_texture(0); // these have to be cleared first?
		pass->swap_read_texture(pass->queued_texture_handle(0));
		pass->swap_draw_texture(pass->texture_handle(0));
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	inline void compose_to_texture(const window::renderpass* pass, const scene_graph&)
	{
		// compositing buffer is bound (i could just compose into the display framebuffer 0 - i dont want to take screenshots!)
		// compositing shader is bound
		// gbuffer outputs are bounds

		// binds bloom texture
		// binds lighting texture
		// binds other textures?
		// composites to final texture

		if (dirty)
		{
			dirty = false;
			pass->automatic_shader()->setInt("TextureChoice", Choice);
		}
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}


	inline void draw_sky(const window::renderpass* pass, const scene_graph&)
	{
		// set the shader uniforms
		// these arent even used.

		// draw the entire sky to the texture.
		// this gets overlayed when the geometry doesnt exist.

		// i.e:
		// the sky is drawn when the depth is infinity.
		// since the sky has a depth pass, i just need to set the depth to infinity in the sky shader
		// but im using a weird quad? so?

		// 

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}


	void set_output(int key)
	{
		dirty = true;
		
		if (key == '0') Choice = 0;
		if (key == '1') Choice = 1;
		if (key == '2') Choice = 2;
		if (key == '3') Choice = 3;
		if (key == '4') Choice = 4;
		if (key == '5') Choice = 5;
		if (key == '6') Choice = 6;
		if (key == '7') Choice = 7;
		if (key == '8') Choice = 8;
		if (key == '9') Choice = 9;
	}
	action_with<int> get_callback()
	{
		action_with<int> ret;
		ret.assign<&graphics::set_output>(this);
		return ret;
	}

	void set_blur(float val)
	{
		BlurRadius += val * 0.001;
		printf("::::SETTING BLUR RADIUS: %f\n",BlurRadius);
	}
	action_with<float> get_scroll()
	{
		action_with<float> ret;
		ret.assign<&graphics::set_blur>(this);
		return ret;
	}

private:
	int Bloom = 0;

	bool dirty = true;
	int Choice = 0;

	float BlurRadius = 0.02;
};


inline void demo()
{
	bool success = true;

	// creates window
	// creates graphics context
	// creates shaders 
		// todo : loads materials into memory.
	// creates window and camera uniform buffers
	window root;
	
	

	// specifies output settings
	// updates window uniform buffer data
	// root.graphics().output_resolution(800,600).target_framerate(30).fullscreen(false).brightness(10).set_title("Game Title").done();
	root.graphics().output_resolution(1980 - 500,1200 - 500).target_framerate(30).fullscreen(false).brightness(10).set_title("Game Title").done();
	root.audio().master(1).ui(1).voice(1).effect(1);

	int BLOOM = 5;
	graphics gfx(&root, BLOOM);
	action_with<const window::renderpass*, const scene_graph&> depth_pass;
	action_with<const window::renderpass*, const scene_graph&> geometry_pass;
	action_with<const window::renderpass*, const scene_graph&> lighting_pass;
	action_with<const window::renderpass*, const scene_graph&> bloom_pass;
	action_with<const window::renderpass*, const scene_graph&> composite_pass;
	action_with<const window::renderpass*, const scene_graph&> sky_pass;

	depth_pass.assign<&graphics::draw_geometry_depth_pass>(&gfx);
	geometry_pass.assign<&graphics::draw_geometry_to_buffers>(&gfx);
	lighting_pass.assign<&graphics::draw_lighting_to_texture>(&gfx);
	bloom_pass.assign<&graphics::apply_effect_bloom>(&gfx);
	composite_pass.assign<&graphics::compose_to_texture>(&gfx);
	sky_pass.assign<&graphics::draw_sky>(&gfx);
	
	if (true)
	{
		// root.pipeline().push(false, "depth", 0, 0, true).function(depth_pass).depth_func(GL_LESS)
		// 	.clear_color({ 0,0,0 });

		// position
		// normal
		// albedo
		root.pipeline().push(true, "geometry", 3, 3, true).function(geometry_pass)
			// .depth_func(GL_LEQUAL)
			// .define_depth_source("depth")
			.clear_color({ 0,1,0 });

		// lit geometry
		// bloom output
		root.pipeline().push(true, "lighting", 3, 2, false).function(lighting_pass)
			.define_texture_source("geometry", 0, 0)
			.define_texture_source("geometry", 1, 1)
			.define_texture_source("geometry", 2, 2)
			.clear_color({ 0,0,0 });


		// sky shader
		root.pipeline().push(false, "sky", 3, 1, false).function(sky_pass)
			.clear_color({ 0,0,0 });


		// bloom - downsamples bloom!
		root.pipeline().push(true, "bloom", 3, 1, false).function(bloom_pass)
			.define_texture_source("lighting", 1, 0)

			.set_secondary_shader("upsample")
			.generate_texture_queue({ 1, 1 }) // same size as the output. i.e ping pong.
			.clear_color({ 0,0,0 });


		// compose
		root.pipeline().push(false, "compose", 3, 1, false).function(composite_pass)
			.define_texture_source("geometry", 0, 0)
			.define_texture_source("geometry", 1, 1)
			.define_texture_source("geometry", 2, 2)

			.define_texture_source("lighting", 0, 3)
			.define_texture_source("sky", 0, 4)

			.define_texture_source("bloom", 0, 5)


			.clear_color({ 0,0,0 });
	}
	else
	{
		void(*Raytrace)(const window::renderpass*, const scene_graph&) 
			= [](const window::renderpass* pass, const scene_graph& graph)
			{
					// draw a quad. ez
				glDrawArrays(GL_TRIANGLES, 0, 6);
			};

		// desmos grass
		// z\ =\ \left(5.5\ -\ \exp\left(x\ +\ 2\right)\right)\ \cdot\ \left(1-y\right)\left\{-2\ <\ x\ <0\ \right\}\ \left\{-1\ <\ y\ <\ 1\right\}

		// albedo, depth
		// normal, metal
		root.pipeline().push(false, "rt_grid", 3, 1, false).function(Raytrace)
			.clear_color({ 0,0,0 });

		// ive done a cool grid.

		// now i need to do some grass...
		// grass should be done by volume. specify a cube and trace through the entry and exit.

		// grass can be layed out by following a brick pattern. (brick texture is tileable - more grass per grass!)
		// so trace columns up from the center of each brick for each blade.

		// trace function:
			// tile the input uv
			// for each brick in brick texture
			// test collision for grass placed at center of that brick
	}


	// window is complete.
	// shaders are complete.
	// framebuffers are complete.
	
	// todo: models, transforms, scripts
	// this is just a scene.
	// todo: render functions

	// for now:
		// all transforms are only cpu side
		// all materials are only cpu side
	

	// this will just work.
	// its a handler for a basic platform event that persists until the handler is destroyed.
	handler DRAG(input_msg_mouse_drag::TYPE);
	DRAG.assign([](const message& msg) 
		{
			message_data<input_msg_mouse_drag>& Cast = (message_data<input_msg_mouse_drag>&)msg;
			printf("dragging: {%2f,%2f} - {%2f,%2f}\n", 
				Cast.data.start.x,
				Cast.data.start.y,
				Cast.data.end.x,
				Cast.data.end.y
				);
		});



	// creates scene camera
	world scene(&root);
	vec3 zero{0,1.5,0};
	vec3 start{ 0, 2, -10 };


	// sets the skybox and default camera position
	// scene.background().skybox("mysky","png");
	scene.camera().move_to(start).look_at(zero).zoom(1);
	

	printf("\n\nloading backpack model\n\n");
	// load model into memory.
		// load geometry into memory.
		// load textures into memory.
		// load materials into memory.
	stbi_set_flip_vertically_on_load(true);
	root.resources().load_model("backpack", "C:\\Users\\kyecu\\Desktop\\Code\\Resources\\backpack\\backpack.obj");
	// root.resources().load_model("backpack", "C:\\Users\\kyecu\\Desktop\\Code\\Resources\\sponza\\source\\Sponza\\sponza.fbx");


	// render model at root
	root.resources().find_model("backpack")->instantiate(scene.entities());

	// render model with parent
	// auto Entity = scene.entities().create("New Entity");
	// root.resources().find_model("backpack")->instantiate(scene.entities(), Entity);

	

	/*
	std::vector<vertex> cube_verts
	{
		vertex{ .position{ -0.5f, -0.5f, -0.5f }, .normal{ 0.0f,  0.0f, -1.0f },  .uv{ 0.0f, 0.0f } },
		vertex{ .position{  0.5f, -0.5f, -0.5f }, .normal{ 0.0f,  0.0f, -1.0f },  .uv{ 1.0f, 0.0f } },
		vertex{ .position{  0.5f,  0.5f, -0.5f }, .normal{ 0.0f,  0.0f, -1.0f },  .uv{ 1.0f, 1.0f } },
		vertex{ .position{  0.5f,  0.5f, -0.5f }, .normal{ 0.0f,  0.0f, -1.0f },  .uv{ 1.0f, 1.0f } },
		vertex{ .position{ -0.5f,  0.5f, -0.5f }, .normal{ 0.0f,  0.0f, -1.0f },  .uv{ 0.0f, 1.0f } },
		vertex{ .position{ -0.5f, -0.5f, -0.5f }, .normal{ 0.0f,  0.0f, -1.0f },  .uv{ 0.0f, 0.0f } },
		vertex{ .position{ -0.5f, -0.5f,  0.5f }, .normal{ 0.0f,  0.0f,  1.0f },  .uv{ 0.0f, 0.0f } },
		vertex{ .position{  0.5f, -0.5f,  0.5f }, .normal{ 0.0f,  0.0f,  1.0f },  .uv{ 1.0f, 0.0f } },
		vertex{ .position{  0.5f,  0.5f,  0.5f }, .normal{ 0.0f,  0.0f,  1.0f },  .uv{ 1.0f, 1.0f } },
		vertex{ .position{  0.5f,  0.5f,  0.5f }, .normal{ 0.0f,  0.0f,  1.0f },  .uv{ 1.0f, 1.0f } },
		vertex{ .position{ -0.5f,  0.5f,  0.5f }, .normal{ 0.0f,  0.0f,  1.0f },  .uv{ 0.0f, 1.0f } },
		vertex{ .position{ -0.5f, -0.5f,  0.5f }, .normal{ 0.0f,  0.0f,  1.0f },  .uv{ 0.0f, 0.0f } },
		vertex{ .position{ -0.5f,  0.5f,  0.5f }, .normal{ -1.0f, 0.0f,  0.0f },  .uv{ 1.0f, 0.0f } },
		vertex{ .position{ -0.5f,  0.5f, -0.5f }, .normal{ -1.0f, 0.0f,  0.0f },  .uv{ 1.0f, 1.0f } },
		vertex{ .position{ -0.5f, -0.5f, -0.5f }, .normal{ -1.0f, 0.0f,  0.0f },  .uv{ 0.0f, 1.0f } },
		vertex{ .position{ -0.5f, -0.5f, -0.5f }, .normal{ -1.0f, 0.0f,  0.0f },  .uv{ 0.0f, 1.0f } },
		vertex{ .position{ -0.5f, -0.5f,  0.5f }, .normal{ -1.0f, 0.0f,  0.0f },  .uv{ 0.0f, 0.0f } },
		vertex{ .position{ -0.5f,  0.5f,  0.5f }, .normal{ -1.0f, 0.0f,  0.0f },  .uv{ 1.0f, 0.0f } },
		vertex{ .position{  0.5f,  0.5f,  0.5f }, .normal{ 1.0f,  0.0f,  0.0f },  .uv{ 1.0f, 0.0f } },
		vertex{ .position{  0.5f,  0.5f, -0.5f }, .normal{ 1.0f,  0.0f,  0.0f },  .uv{ 1.0f, 1.0f } },
		vertex{ .position{  0.5f, -0.5f, -0.5f }, .normal{ 1.0f,  0.0f,  0.0f },  .uv{ 0.0f, 1.0f } },
		vertex{ .position{  0.5f, -0.5f, -0.5f }, .normal{ 1.0f,  0.0f,  0.0f },  .uv{ 0.0f, 1.0f } },
		vertex{ .position{  0.5f, -0.5f,  0.5f }, .normal{ 1.0f,  0.0f,  0.0f },  .uv{ 0.0f, 0.0f } },
		vertex{ .position{  0.5f,  0.5f,  0.5f }, .normal{ 1.0f,  0.0f,  0.0f },  .uv{ 1.0f, 0.0f } },
		vertex{ .position{ -0.5f, -0.5f, -0.5f }, .normal{ 0.0f, -1.0f,  0.0f },  .uv{ 0.0f, 1.0f } },
		vertex{ .position{  0.5f, -0.5f, -0.5f }, .normal{ 0.0f, -1.0f,  0.0f },  .uv{ 1.0f, 1.0f } },
		vertex{ .position{  0.5f, -0.5f,  0.5f }, .normal{ 0.0f, -1.0f,  0.0f },  .uv{ 1.0f, 0.0f } },
		vertex{ .position{  0.5f, -0.5f,  0.5f }, .normal{ 0.0f, -1.0f,  0.0f },  .uv{ 1.0f, 0.0f } },
		vertex{ .position{ -0.5f, -0.5f,  0.5f }, .normal{ 0.0f, -1.0f,  0.0f },  .uv{ 0.0f, 0.0f } },
		vertex{ .position{ -0.5f, -0.5f, -0.5f }, .normal{ 0.0f, -1.0f,  0.0f },  .uv{ 0.0f, 1.0f } },
		vertex{ .position{ -0.5f,  0.5f, -0.5f }, .normal{ 0.0f,  1.0f,  0.0f },  .uv{ 0.0f, 1.0f } },
		vertex{ .position{  0.5f,  0.5f, -0.5f }, .normal{ 0.0f,  1.0f,  0.0f },  .uv{ 1.0f, 1.0f } },
		vertex{ .position{  0.5f,  0.5f,  0.5f }, .normal{ 0.0f,  1.0f,  0.0f },  .uv{ 1.0f, 0.0f } },
		vertex{ .position{  0.5f,  0.5f,  0.5f }, .normal{ 0.0f,  1.0f,  0.0f },  .uv{ 1.0f, 0.0f } },
		vertex{ .position{ -0.5f,  0.5f,  0.5f }, .normal{ 0.0f,  1.0f,  0.0f },  .uv{ 0.0f, 0.0f } },
		vertex{ .position{ -0.5f,  0.5f, -0.5f }, .normal{ 0.0f,  1.0f,  0.0f },  .uv{ 0.0f, 1.0  } }
	};
	std::vector<unsigned> cube_inds;
	for (int i = 0; i < cube_verts.size(); ++i) 
	{
		cube_inds.push_back(i);
	}
	

	scene.entities().define("cooked cube",cube_verts,cube_inds);
	



	{
		geometry_builder BUILD;
		const float SCALE_PLANE = 2.5;
		BUILD.define_point({ -100 * SCALE_PLANE, 0, -100 * SCALE_PLANE });
		BUILD.define_point({ -100 * SCALE_PLANE, 0,  100 * SCALE_PLANE });
		BUILD.define_point({ 100 * SCALE_PLANE, 0, -100 * SCALE_PLANE });
		BUILD.define_point({ 100 * SCALE_PLANE, 0,  100 * SCALE_PLANE });
		BUILD.define_face(0, 1, 2, 3);


		cube_verts.clear();
		cube_inds.clear();
		BUILD.build(cube_verts, cube_inds);
		scene.entities().define("plane", cube_verts, cube_inds);
	}

	 scene.entities().push("cooked cube", { 0,0,0 });
	 scene.entities().push("cooked cube", { 2,0,2 });
	 scene.entities().push("cooked cube", { -2,0,-2 });
	 scene.entities().push("cooked cube", { 2,0,-2 });
	 scene.entities().push("cooked cube", { -2,0,2 });
	
	 scene.entities().push("plane", {0, -3, 0});

	// a scene file is just a list of names and transforms.
	// the names refer to prefabs. the transforms are the transform of that instance.
	// scripts for each item are located automatically when needed from the relevant directory.


	// scene.entities().load("backpack", "C:\\Users\\kyecu\\Desktop\\Code\\Resources\\backpack\\backpack.obj");
	// to allow for more than 1 scene to be played at a time (ui and game, etc), scenes are added to a stack.
	// each scene is updated from top to bottom with unhandled events.

	// scene.entities().push("backpack", { 0,0,0 });

	// the scene stack is stored in the window.
	// the scripting engine is started at this point to begin gameplay.
	*/

	root.play();
}


#if 0

int a() 
{
	// this was from main
	// ************************************************************************************************* //

	vec3 pos;
	vec3 min_scale;
	vec3 max_scale;

	// loads the ground model from ../models/ground.fbx
	// loads the ground material from ../models/ground.mat
	// loads the ground physics from  ../models/ground.psy
	// check all other systems for their scripts for ground if they exist and add them to the object.
	auto ground = scene.create("ground", pos);

	

	

	scene
		.generate(50)
		.near(pos, 10)
		.visible_from(pos)
		.facing(pos)
		
		.begin_placing()
		.set_scale(min_scale,max_scale)
		.place("grass")

		.attach("grow_grass_over_time");


}
#endif 