#pragma once
#if 0
#include "../utils/math/vector.hpp"

#include <vector>
struct cell;
struct object
{
	vec2u bounds[2]{};
	float radius;

	std::vector<cell*> is_inside_of_these_cells;

	void* userdata;
};
struct object_list
{

};
struct cell
{
	object_list entities;
};


struct grid
{
	vec2u size;
	vec2u cell_size;
	
	std::vector<std::vector<cell>> cells;
	std::vector<void*> entity_handles;

	size_t register_entity(float xpos, float ypos)
	{
		// add an entity to the grid.

		// each entity holds:
			// a reference to the grid cell(s) it is in - entities can be in more than 1 cell because their bounding box is used, not their transform.
			// this is ok because generally larger entities wont move.
			// so it just saves time on collision detection.

		// each 
	}

	vec2u get_cell_index(float xpos, float ypos)
	{
		unsigned int Idx_x = xpos / cell_size.x;
		unsigned int Idx_y = ypos / cell_size.y;


		return vec2u(Idx_x, Idx_y);
	}
	object_list collect_cell_entities(const std::vector<vec2u>& cell_index_list)
	{

	}

	object_list get_collision_tests(float xpos, float ypos)
	{
		vec2u Center = get_cell_index(xpos, ypos);
		vec2u N      = get_cell_index(xpos, ypos + cell_size.y);
		vec2u S      = get_cell_index(xpos, ypos - cell_size.y);
		vec2u E      = get_cell_index(xpos + cell_size.x, ypos);
		vec2u W      = get_cell_index(xpos - cell_size.x, ypos);
		vec2u NE     = get_cell_index(xpos + cell_size.x, ypos + cell_size.y);
		vec2u NW     = get_cell_index(xpos + cell_size.x, ypos - cell_size.y);
		vec2u SE     = get_cell_index(xpos - cell_size.x, ypos + cell_size.y);
		vec2u SW     = get_cell_index(xpos - cell_size.x, ypos - cell_size.y);

		// return all entities in all of these cells! that's a lot of entities!
		return collect_cell_entities({ N,S,E,W,NE,NW,SE,SW });
	}
};

#endif 