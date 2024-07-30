#include "Header.h"

#include "../utils/event/message.h"

static bool is_dragging = false;
static bool drag_state = false;
static vec2 drag_start = 0;


void window::copy_to_zerod(int& a, int& b)
{
	if (a == 0) a = b;
	if (b == 0) b = a;
}

void window::update_resolution(int X, int Y)
{
	pipeline().resize(X, Y);
}

void window::update_handle()
{
	int Width = graphics().width();
	int Height = graphics().height();
	string Title = graphics().title();
	bool Fullscreen = graphics().fullscreen();


	// if these are different then i need to recreate all the framebuffers too...
	int ResolutionX = graphics().output_resolution_x();
	int ResolutionY = graphics().output_resolution_y();
	int RefreshRate = graphics().target_framerate();
	copy_to_zerod(ResolutionX, Width);
	copy_to_zerod(ResolutionY, Height);


	glViewport(0, 0, Width, Height);

	GLFWmonitor* Monitor = glfwGetPrimaryMonitor();

	int MonitorXPos = 0;
	int MonitorYPos = 0;
	int MonitorXSiz = 0;
	int MonitorYSiz = 0;
	glfwGetMonitorWorkarea(Monitor, &MonitorXPos, &MonitorYPos, &MonitorXSiz, &MonitorYSiz);



	int PosX = MonitorXPos + ((MonitorXSiz - Width) * 0.5);
	int PosY = MonitorYPos + ((MonitorYSiz - Height) * 0.5);
	glfwSetWindowPos(handle, PosX, PosY);
	glfwSetWindowSize(handle, Width, Height);
	glfwSetWindowTitle(handle, Title.c_str());


	if (Fullscreen)
	{
		glfwSetWindowMonitor(handle, Monitor, PosX, PosY, Width, Height, RefreshRate);
	}



	// recreate the framebuffers too...but only if the target resolution has changed.
	update_resolution(ResolutionX, ResolutionY);
}

void window::update_buffer(string name, int Size, void* data, unsigned int buffertype)
{
	glBindBuffer(buffertype, _buffers[name]._handle);
	glBufferSubData(buffertype, 0, Size, data);
}


void window::create_handle()
{
	glfwInit();

	handle = glfwCreateWindow(800, 600, "", 0, 0);
	glfwMakeContextCurrent(handle);
	glfwShowWindow(handle);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glfwSetWindowUserPointer(handle, this);

	glfwSetKeyCallback(handle, [](GLFWwindow* win, int a,int b,int c,int d)
		{
			window* self = (window*)glfwGetWindowUserPointer(win);
			
			message_data<input_msg_key> msg;
			msg.type = input_msg_key::TYPE;
			msg.data.action = c;
			msg.data.key = a;
			msg.source = nullptr;
			post_message(msg);



			// this only does so many per second. not enough to be smooth.
			// i will need to make a script if i want things to be processed per frame instead of per message.
			if (c == GLFW_PRESS)
			{
				message_data<input_msg_keydown> msg;
				msg.type = input_msg_keydown::TYPE;
				msg.data.key = a;
				msg.source = nullptr;
				post_message(msg);
			}
			else if (c == GLFW_RELEASE)
			{
				message_data<input_msg_keyup> msg;
				msg.type = input_msg_keyup::TYPE;
				msg.data.key = a;
				msg.source = nullptr;
				post_message(msg);
			}
		});
	glfwSetScrollCallback(handle, [](GLFWwindow* win, double x, double y) 
		{
			window* self = (window*)glfwGetWindowUserPointer(win);
			// self->input().on_keypress._scroll_callback(y);
		});
	glfwSetCursorPosCallback(handle, [](GLFWwindow* win, double x, double y)
		{
			window* self = (window*)glfwGetWindowUserPointer(win);

			{
				message_data<input_msg_mouse_move> msg;
				msg.type = input_msg_mouse_move::TYPE;
				msg.data.pos = { x,y };
				msg.source = nullptr;
				post_message(msg);
			}

			if (is_dragging && drag_state == false)
			{
				drag_start = { x,y };
			}

			// if dragging, post a drag event too
			if (is_dragging)
			{
				if (drag_state)
				{
					message_data<input_msg_mouse_drag> msg;
					msg.type = input_msg_mouse_drag::TYPE;
					msg.data.end = { x,y };
					msg.data.start = drag_start;
					msg.source = nullptr;
					post_message(msg);
				}
			}


			drag_state = is_dragging;
		});
	glfwSetMouseButtonCallback(handle, [](GLFWwindow* win, int a, int b, int c)
		{
			window* self = (window*)glfwGetWindowUserPointer(win);
		
			message_data<input_msg_mouse_button> msg;
			msg.type = input_msg_mouse_button::TYPE;
			msg.data.action = b;
			msg.data.button = a;
			msg.source = nullptr;
			post_message(msg);


			if (b == GLFW_PRESS)
			{
				if (a == GLFW_MOUSE_BUTTON_1)
				{
					is_dragging = true;
				}

				message_data<input_msg_mouse_down> msg;
				msg.type = input_msg_mouse_down::TYPE;
				msg.data.button = a;
				msg.source = nullptr;
				post_message(msg);
			}
			else if (b == GLFW_RELEASE)
			{
				if (a == GLFW_MOUSE_BUTTON_1)
				{
					is_dragging = false;
				}
				message_data<input_msg_mouse_up> msg;
				msg.type = input_msg_mouse_up::TYPE;
				msg.data.button = a;
				msg.source = nullptr;
				post_message(msg);
			}
		});
}

void window::init_buffers()
{
	create_buffer<Buffer_Data_Window>(0, "Window", GL_UNIFORM_BUFFER);
	create_buffer<Buffer_Data_Camera>(1, "Camera", GL_UNIFORM_BUFFER);
	create_buffer<Buffer_Data_Lights>(2, "Lights", GL_SHADER_STORAGE_BUFFER);
}

void window::init_shaders()
{
	resources().load_shaders();
}

void window::init_materials()
{
	// todo
}

void window::update_buffers()
{
	{
		Buffer_Data_Window data;
		data._size.x = graphics().width();
		data._size.y = graphics().height();
		data._resolution.x = graphics().output_resolution_x();
		data._resolution.y = graphics().output_resolution_y();

		data._runtime = time().since_startup();
		data._playtime = time().since_last_play();
		data._random = rand();

		update_buffer("Window", sizeof(data), &data);
	}

	{
		Buffer_Data_Camera& data = _scenes.current().camera().data();

		update_buffer("Camera", sizeof(data), &data);
	}


	{
		Buffer_Data_Lights data{};
		// data.directional_light_position = vec3(5 * sin(2 * glfwGetTime()), 0, 5 * cos(2 * glfwGetTime())).normalized();
		// data.directional_light_color_intensity = vec4(1, 1, 1, 5);
		// 
		float Ypos = 4;
		float MOVE = 0.1;
		float INTENSE = 10;

		// data.point_lights[0].pos_radius = vec4(15 * sin(MOVE * 2 * glfwGetTime()), Ypos, -2, 5);
		// data.point_lights[0].color_intensity = vec4(1, 0, 0, INTENSE * 5);
		//  
		// data.point_lights[1].pos_radius = vec4(14 * cos(MOVE * 3 * glfwGetTime()), Ypos, 0, 5);
		// data.point_lights[1].color_intensity = vec4(0, 1, 0, INTENSE * 4);
		//  
		// data.point_lights[2].pos_radius = vec4(3 * sin(MOVE * 14 * glfwGetTime()), Ypos, 2, 5);
		// data.point_lights[2].color_intensity = vec4(0, 0, 1, INTENSE * 3);
		 
		// data.point_lights[3].pos_radius = vec4(2 * cos(MOVE * 15 * glfwGetTime()), Ypos - 1, 1, 5);
		// data.point_lights[3].color_intensity = vec4(0, 1, 1, INTENSE * 2);


		// data.directional_light_position = vec3(1,15,0);
		// data.directional_light_color_intensity = vec4(1, 1, 1, 10);

		// data.point_lights[0].pos_radius = vec4(0, 15, 0, 1);
		// data.point_lights[0].color_intensity = vec4(0.2, 0.2, 1, 55); 
		// data.point_lights[1].pos_radius = vec4(15, 15, 0, 1);
		// data.point_lights[1].color_intensity = vec4(0.5, 0.5, 1, 55); 

		data.point_lights[0].pos_radius = vec4(3, 9, 0, 1);
		data.point_lights[0].color_intensity = vec4(1, 1, 1, 100);

		update_buffer("Lights", sizeof(data), &data);
	}
}

window::window()
{
	create_handle();
	update_handle();

	init_buffers();
	init_shaders();
	init_materials(); // todo
}

void window::tick()
{
	glfwPollEvents();
	glClearColor(1, 1, 1, 1);

	hooks().call("update", fps().last_frametime());
	this->scenes().active().current().camera().on_update()();


	// update the window and camera uniform blocks
	update_buffers();

	// update physics
	// update scripts


	// double draw_when_faster_than = 1.0 / 30.0;
	double draw_when_faster_than = 1.0 / 15.0;

	if (fps().last_frametime() <= draw_when_faster_than)
	{
		// draw the scene 
		_pipeline.render(scenes().active().current().entities());
		glfwSwapBuffers(handle);
	}


}

void window::play()
{
	fps().init();

	while (not glfwWindowShouldClose(handle))
	{
		tick();
		fps().tick();

		float ypos = 2;
		float radius = 5;

		vec3 zero{};
		// vec3 zero{0, -15 * sin(0.5 * glfwGetTime()),0};

		// vec3 start{ 2 - (4 * cos(0.2 * glfwGetTime())), 4, -6 + (12 * sin(0.2 * glfwGetTime()))};
		// vec3 start{ (radius * cos(0.1 * glfwGetTime())), 3 + sin(0.2 * glfwGetTime()) * 0.3, (radius * sin(0.1 * glfwGetTime()))};
		// vec3 start{ (radius * cos(0.1 * glfwGetTime())), ypos + sin(0.2 * glfwGetTime()) * 0.5, (radius * sin(0.1 * glfwGetTime()))};
		// vec3 start{ (radius * cos(0.1 * glfwGetTime())), ypos, (radius * sin(0.1 * glfwGetTime()))};

		// static float A = 0;
		// vec3 start{ 10, 2, 10};
		// vec3 start;
		// start.x = 0; // lr
		// start.y = -15; // fb
		// start.z = 25; // tb
		// scenes().active().current().camera().move_to(start).look_at(zero).zoom(1);
	}
}

bool window::should_close()
{
	return glfwWindowShouldClose(handle);
}

std::string read_file(const std::string& filename)
{
	std::ifstream t(filename);
	std::stringstream buffer;
	buffer << t.rdbuf();
	return buffer.str();
}
