#pragma once
#if 0
#include <string>
#include <vector>
#include <tuple>

using std::string;

struct vec3 {};

template<typename ...>
class action_with {};
typedef action_with<> action;

class shader {};
class texture {};
class script {};

struct window
{
	// window
	struct draw_context
	{
		// updates the gpu uniform buffer.
		// also updates the window if needed.
		~draw_context();

		draw_context& target_framerate(int);
		draw_context& target_framerate(double);

		draw_context& output_resolution(int, int);


		draw_context& brightness(int);
		draw_context& fullscreen(bool);
	private:
		double _framerate;
		int _resolution_x;
		int _resolution_y;

		bool _fullscreen;
	};
	draw_context graphics();

	// audio
	struct sound_context
	{
		// updates the settings
		~sound_context();

		sound_context& mute(bool);
		sound_context& master(double);
		sound_context& ui(double);
		sound_context& voice(double);
		sound_context& effect(double);
	private:
		double _master = 1;

		double _ui = 1;      // clicks, sliders, etc
		double _voice = 1;   // dialogue
		double _effect = 1; // guns, bombs, swords, fire, ...
	};
	sound_context audio();

	// resource locations
	struct resource_context
	{
		string loading_scene() { return "loading.fbx"; }
		string loading_texture() { return "loading.png"; }
		string title_scene() { return "title.fbx"; }

		std::vector<std::tuple<string, string,string,string>> shaders()
		{
			return
			{
				{ "loading","render_image_fullscreen.vert","","render_image_fullscreen.frag" },
				{ "depth","render_geometry_lit.vert","","render_depth_only.frag" },
				{ "geometry","render_geometry_lit.vert","","render_geometry_lit.frag" },
				{ "lighting","render_light_group.vert","","render_light_group.frag" },
			};
		}

		// keeps all shaders in memory
		std::vector<shader> _shaders;

		// keeps all textures in memory
		std::vector<texture> _textures;

		// keeps all scripts in memory
		std::vector<script> _scripts;
	};
	resource_context resources();

	// loaded resources
	struct load_context
	{
		load_context load(string _name);
	};
	load_context loader();


	// renderer specific
	struct renderpass
	{
		void enable(bool);

		renderpass function(action);

	private:
		bool _enabled;
		action _callback;
		
		// frame layout
		string _name;
		int _channels;
		int _textures;
		int _depth;
		int _stencil;
	};
	struct swapchain
	{
		renderpass push(string _name, int _channels, int _textures, int _depth = 24, int _stencil = 0);

	private:
		std::vector<renderpass> _passes;
	};
	swapchain pipeline();



	// scripting
	struct script_context
	{
		// create scripts in text for each object.

		// a script function continues until the next section header is reached.
		// section headers
		// [action]		= a callable function attached to an object. 
		// [response]   = a listener that is called when an event is recieved. (key, mouse, collision, etc)
		// [lifetime]    = a function attached to an object. called when the age of the object reaches the specified value.
		


	private:
	};
	script_context engine();

private:
	draw_context _graphics;
	sound_context _audio;
	resource_context _resources;
	load_context _loader;
	swapchain _pipeline;


};



struct object
{

};

struct placement_context
{
	struct scripting_stream
	{
		scripting_stream attach(string script_name);
	};
	struct placement_stream
	{
		// min
		// max
		// weighting
		placement_stream set_scale(vec3, vec3, float = 0.5);

		placement_stream offset_position(vec3, vec3, float = 0.5);
		placement_stream offset_rotation(vec3, vec3, float = 0.5);
		placement_stream offset_scale(vec3, vec3, float = 0.5);

		scripting_stream place(string item);
		scripting_stream place(size_t item);
		scripting_stream place();

	};

	struct point_stream
	{
		point_stream near(vec3 pos, float distance)
		{
			// generates _point_count points within distance of pos
		}

		point_stream visible_from(vec3 pos)
		{
			// makes sure that the generated points are visible from pos
		}

		point_stream facing(vec3);
		point_stream nearly_facing(vec3, float);

		placement_stream begin_placing();
	};
	
	
	point_stream near(vec3, float distance);
	point_stream within(vec3, vec3);
};

struct world
{
	// view
	struct view_context
	{
		view_context move_to(vec3);
		view_context look_at(vec3);

		// change the fov of the camera
		view_context zoom(double);

		// 


		// updates the gpu uniform buffer.
		~view_context();
	private:
		vec3 _position;
		vec3 _target;
	};
	view_context camera();
	

	
	// entities

	placement_context generate(int start_points);

	object create(string item, vec3 pos);


private:
	view_context _camera;
};

namespace graphics
{
	struct material
	{
		string name;
		string path;
		
		size_t texture_albedo;
		size_t texture_normals;
		size_t texture_metallic;
		size_t texture_roughness;
		size_t texture_ambient_occlusion;
		

		vec3 color;
		double metallic;
		double roughness;
	};
}
inline void draw_geometry_depth_pass();
inline void draw_geometry_to_buffers();
inline void draw_lighting_to_texture();
inline void apply_effect_bloom();
inline void present_to_screen();

struct shader
{
	string name(); // MyShader
	string type(); // geometry
	int targets(); // 3

};

inline void demo()
{
	// creates window
	// creates graphics context
	// creates shaders
	// loads materials into memory
	// creates window and camera uniform buffers
	window root;

	// specifies output settings
	// updates window uniform buffer data
	root.graphics().output_resolution(1980, 1020).target_framerate(60).fullscreen(true).brightness(10);
	root.audio().master(1).ui(1).voice(1).effect(1);

	// creates graphics pipeline from output settings
	root.pipeline().push("depth", 0, 0, true).function(draw_geometry_depth_pass);
	root.pipeline().push("geometry", 3, 3, true).function(draw_geometry_to_buffers);    // position, normal, pbr
	root.pipeline().push("lighting", 3, 1, false).function(draw_lighting_to_texture);
	root.pipeline().push("bloom", 3, 1, false).function(apply_effect_bloom);
	root.pipeline().push("end", 3, 1, false).function(present_to_screen);
	

	// 
	// window is complete.
	// shaders are complete.
	// framebuffers are complete.
	// 
	// todo: models, transforms, scripts (the engine can only spawn prefabs)
	// this is just a scene.
	// 


	// creates camera & updates camera uniform buffer
	// creates transform buffer
	// creates vertex buffer
	world scene;
	vec3 zero;
	vec3 start;
	scene.camera().move_to(start).look_at(zero);


	
}

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