#if 0
#pragma once
#include <string>
#include <vector>
using namespace std;

class DefinitionParser
{
public:
	class Definition
	{
	public:
		bool is_label();
		bool is_text();

		string text();
	};


	 bool read(string);
	 bool replace_arg(int, string);

	 typedef vector<Definition> block;

	 vector<block> text(vector<string> headings = {});
};

class Stats
{
public:
	int HP;
	int SP;
	int MP;

	int strength;
	int dexterity;
	int intelligence;
	int vitality;
	int armour;

	bool exist();

	static Stats For(string actor_type);
};

// goblin, orc, player, etc.
// every actor will have stats
class Actor
{
public:
	string name();
	string type();

	static bool gift(Actor&, string);
	static bool take(Actor&, string); 

	// these are all to do with stats.
	// only some actors will do this.
	static bool recover(Actor& a, string resource)
	{
		if (a.stats.exist())
		{
			a.stats.HP += 50;
			

			return true;
		}
		return false;
	}
	static bool increase(Actor&, string);
	static bool decrease(Actor&, string);
	static bool page(Actor&, string);
	static bool learn(Actor&, string);
public:
	vector<string> inventory;
	Stats stats;
};

class ActionBinding
{
public:
	ActionBinding(string,bool(*callback)(Actor&, string));
	string text();

	static bool process(DefinitionParser::block);
};
namespace ActionBindings
{
	// give the caller an item
	ActionBinding gift("gift",Actor::gift);
	// remove an item from the caller
	ActionBinding take("take", Actor::take);
	// recover a resource of the caller
	ActionBinding recover("recover", Actor::recover);
	// increase a stat of the caller
	ActionBinding increase("increase", Actor::increase);
	// decrease a stat of the caller
	ActionBinding decrease("decrease", Actor::decrease);
	// show a page of text and wait for input
	ActionBinding page("page", Actor::page);
	// learn a skill
	ActionBinding learn("learn", Actor::learn);
}



bool button(string);

bool window(string);

// player opens their inventory
void on_inventory(Actor& player)
{
	if (window("inventory"))
	{
		for (auto item : player.inventory)
		{
			// show the item and its icon
			// icon is located in ../icons/itemName.png
		}


		// player selects an Apple. now show the available actions 
		// the actions for Apple are located here
			// ../items/Apple.item

		DefinitionParser parser;

		// parse the item definition
		parser.read("Apple.item");

		// retrieve all inventory actions
		auto txt = parser.text({ "action", "Tier 1", "Tier 2", "Tier 3" });

		// display their text
		for (auto x : txt)
		{
			if (button(x[0].text()))
			{
				// process the action!
				ActionBinding::process(x);
			}
		}


	}
}

#endif 