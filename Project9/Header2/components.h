#pragma once
#include "../utils/math/vector.hpp"

#include <string>
#include <vector>


struct character_controller
{
	void update()
	{

	}
};
struct camera_controller
{

};

// 
struct inventory_item_action
{
	std::string name; // eat
	std::string code; // increase hp 50%

	// bytecode to make things happen.
	// can be much simpler than 

	// increase HP by 50%
	// increase str by 1
	// increase HP by 50
	// decrease str by 1 for 60 seconds
};


struct inventory_item
{
	std::string name;
	std::string icon;
	std::vector<inventory_item_action> actions;

	int stacksize;
	int currstack;
	
	// + loaded icon
	// + mesh if i want my inventory to display that
	
	// this will work.
	// i could assume each item has a certain property like a mesh,
	// but eh.
	// relevant things can be found just by the filesystem hierarchy...
};

struct inventory
{
	std::string owner;

	int capacity;
	std::vector<inventory_item> items;

};



inline void demo()
{
	// create a window
	// create a scene

	// create an entity
	// add controls to it
	// add a script to the camera

	// ... 
}