#pragma once

#include "../utils/math/vector.hpp"
#include "../utils/math/matrix.hpp"
#include "../utils/event/action.h"

#include <vector>

struct ray
{
	vec3 ro;
	vec3 rd;

	double hit_t = 0;
};
struct camera
{
	vec2 resolution;
	vec2 fov = { 90,90 };
	
	std::vector<ray> generate_rays(vec3 view_pos, vec3 view_fwd)
	{
		std::vector<ray> ret;
		view_fwd.normalize();


		ret.reserve(resolution.x * resolution.y);


		vec3 Right = cross(view_fwd, vec3(0,1,0));
		vec3 Up = cross(view_fwd, Right);

		for (int y = 0; y < resolution.y; ++y)
		{
			for (int x = 0; x < resolution.x; ++x)
			{
				vec3 origin = view_pos;

				int X = x - (resolution.x * 0.5);
				int Y = y - (resolution.y * 0.5);
				
				vec3 direction = (Right * X) + (Up * Y);
				
				ret.push_back({origin,direction});
			}
		}
		return ret;
	}
};

struct world
{
	// transform rays
	void hit_rays(std::vector<ray>& rays)
	{
		// for each ray
		// if ray hits something
		// set t
		// else
		// remove ray from the hit list
	}
};


inline void demo()
{

}