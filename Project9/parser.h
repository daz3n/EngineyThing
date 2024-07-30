#pragma once
#if 0
#include <string>
#include <vector>
#include <map>
#include <any>
#include <iostream>
using namespace std;

class behaviour
{
public:
	behaviour(int _min_distance,int _max_distance, string _name);
	
	int begin_distance();
	int end_distance();

	string name();
	bool enabled_at_distance(double _dist);
};



namespace behaviours
{
	behaviour _attack(0, 1, "attack");
	behaviour _avoid(2, 3, "avoid obstacles");
	behaviour _move(4, 5, "move");



	inline void demo()
	{
		double distance_to_actor;

		if (_attack.enabled_at_distance(distance_to_actor))
		{

		}

		
	}
}



// this is simply a manual lookup of words.
// useful for crafting recipes
// royal + man = king.
// cook fish = cooked fish
namespace dictionary
{
	typedef string word;
	
	class hash
	{
	public:
		size_t make_index(string);
	};
	class page
	{
		word words[250][250];
		hash hasher;
	public:
		word find(word a = "fish", word b = "none")
		{
			// [cook][fish] = cooked fish
			// [cooked fish][eat] = "action args"

			auto id_a = hasher.make_index(a);
			auto id_b = hasher.make_index(b);
			return words[id_a][id_b];
		}
	};
	class book
	{
		enum { method_count = 20 };
		page strings[method_count];
		hash hasher;

		// cook
		// gift
		// take
		// read
		// loot
		// page
		// boon
		// increase
		// decrease
	public:

		word find(word method = "cook", word a = "fish", word b = "")
		{
			auto idx = hasher.make_index(method);
			


			return strings[idx].find(a, b);
		}

		word& action(word _method, word _item);
	};

	
	class action_builder
	{
	public:
		// require the calling system to provide an item to the script
		// crafting will require this to be given via ui
		// eating will take this from the inventory
		action_builder& require(string, int);

		// require the calling system to provide an item to the script
		// then destroys the item
		// if the item doesnt exist then the script exits
		action_builder& consume(string, int);


		// increase a stat of the calling actor
		action_builder& increase(string, int);
		// decrease a stat of the calling actor
		action_builder& decrease(string, int);

		// show text on the screen until a button is clicked
		action_builder& page(string);
		
		// learn a skill
		action_builder& learn(string);
		
		// take an item from the calling actor
		action_builder& take(string, int);
		// gift an item to the calling actor
		action_builder& gift(string, int);
		

		// require an item from the inventory
		action_builder& input(int, string);
		
		// append raw code
		action_builder& raw(string);

		string flush();
	};


	class Craft
	{
	public:
		static void call(void* actor, string recipe);
	};


	class KeyValueStore {
	private:
		std::map<std::string, std::any> data;

	public:
		template<typename ValueType>
		void insert(const std::string& key, const ValueType& value) {
			data[key] = value;
		}

		template<typename ValueType>
		ValueType& get(const std::string& key) {
			return std::any_cast<ValueType&>(data[key]);
		}

		bool contains(const std::string& key) const {
			return data.find(key) != data.end();
		}

		void remove(const std::string& key) {
			data.erase(key);
		}

		void print() const {
			for (const auto& pair : data) {
				std::cout << pair.first << ": ";
				try {
					std::cout << std::any_cast<std::string>(pair.second) << std::endl;
				}
				catch (const std::bad_any_cast&) {
					try {
						std::cout << std::any_cast<int>(pair.second) << std::endl;
					}
					catch (const std::bad_any_cast&) {
						try {
							std::cout << std::any_cast<double>(pair.second) << std::endl;
						}
						catch (const std::bad_any_cast&) {
							std::cout << "Unsupported type" << std::endl;
						}
					}
				}
			}
		}
	};


	class actor : public KeyValueStore
	{
	public:
		
	};

	class action_provider
	{
	public:
		enum class action_args 
		{
			always,
			sometimes,
			
			integer,
			name,
		};
		struct arg
		{
			string value;
			bool is_int;
			bool is_string;

			int Int();
			string String();
		};
		struct arglist
		{
			actor* self;
			vector<arg> values;
			bool is_valid = false;

			void must_be_valid(bool require_self = true, string errmsg = "args were invalid") const;
		};
		class action_info
		{
		public:
			string name;
			void(*callback)(arglist args);
			vector<action_args> required;
		};
		
	};



	// an action provider can register static actions that are taken on an actor.
	// an actor calling "[gift] 10 iron" will pass themself into the context.
	class inventory : public action_provider
	{
		static int gift_count(arglist);
		static string gift_item(arglist);
		static void do_gift(arglist);

		static int take_count(arglist);
		static string take_item(arglist);
		static void do_take(arglist);
	public:
		
		static void gift(arglist args)
		{
			args.must_be_valid();
			
			do_gift(args);
		}
		static void take(arglist args)
		{
			// remove item from items
			args.must_be_valid();
			
			auto& items = args.self->get<vector<string>>("inventory.items");
			do_take(args);
		}

		// script actions get registered with the scripting engine. 
		// an actor calls these to modify their own stats.
		void actions(vector<action_provider::action_info>& out_actions)
		{
			out_actions.push_back({ 
				"gift", gift,
					{
						action_provider::action_args::always, action_provider::action_args::integer,
						action_provider::action_args::always, action_provider::action_args::name 
					}
				});

			out_actions.push_back({ 
				"take", take,
					{
						action_provider::action_args::always, action_provider::action_args::integer,
						action_provider::action_args::always, action_provider::action_args::name 
					}
				});
		}
	};

	

	class script_engine
	{
	public:
		void add(action_provider*);
		void add(vector<action_provider::action_info>&);
		void run(string _command, action_provider::arglist&);
	};

	namespace actions
	{
		void eat(action_provider::arglist list);
		// inventory
		void gift(action_provider::arglist list);
		void take(action_provider::arglist list);
		// dialogue
		void talk(action_provider::arglist list);
		void page(action_provider::arglist list);
		// stats
		void increase(action_provider::arglist list);
		void decrease(action_provider::arglist list);
		// damage
		void recover(action_provider::arglist list);
		// loot
		void loot();
	}

	inline void demo()
	{
		{
			vector<action_provider::action_info> script_actions;

			// these actions can also be registered from plugins.
			// this lets me add more systems that use these actions from plugins.


			// inventory
			// stats
			// damage
			// loot
			// dialogue
			script_actions.push_back({
				"eat", actions::eat,
					{
						action_provider::action_args::always, action_provider::action_args::name
					}
				});

			script_actions.push_back({
				"gift", actions::gift,
					{
						action_provider::action_args::always, action_provider::action_args::integer,
						action_provider::action_args::always, action_provider::action_args::name
					}
				});
			
			script_actions.push_back({
				"take", actions::take,
					{
						action_provider::action_args::always, action_provider::action_args::integer,
						action_provider::action_args::always, action_provider::action_args::name
					}
				});

			script_engine engine;



			actor player;
			


			// player is browsing their inventory
			// they click on cooked fish
			// read its' scripts 
			// this displays all the scripts associated with cooked fish.
			// they select eat

			// inventory.open()
			// # they click on the cooked fish icon
			// for each script in find_scripts_for("cooked fish")
			// if (click button) -> book.find(script.name,"cooked fish")
			
			// 

			action_provider::arg arg1;
			arg1.value = "cooked fish";

			action_provider::arglist args;
			args.self = &player;
			args.values.push_back(arg1);

			engine.run("eat", args);
			// searches all plugins for one that provides an eat command
			// this happens to be a builtin plugin which just parses the string into a subscript
			// for each command in the eat script, it calls that command recursively.
			// it can do this because the engine passes itself into the plugin args.
			// args.context.call("recover",{"50" "health"});

			// all of these scripts are the interactions only.

			// to load a monster, a level calls load_actor("actor name")
			// this returns the actor behaviour and mesh/etc.
		}


		book recipes;
		action_builder builder;

		// "[increase] 50 health"
		recipes.action("eat","fish") = builder.increase("health", 50).flush();
		
		// "[consume] 50 iron \n [gift] 25 nails"
		recipes.action("craft", "nails") = builder.consume("iron", 5).gift("nails", 25).flush(); 


		// "(any metal[10]) \n [consume] (arg1).name \n [gift] (arg1).name + ' sword'"
		recipes.action("craft","sword") = builder.require("any metal", 10).consume("(arg1).name", 10).gift("(arg1).name + ' sword'", 1).flush();


		Craft::call(nullptr, recipes.find("craft", "sword"));
			// parses the recipe using the common parser
			// checks for requirements
			// "10 any metal"
			// asks for requirements via the ui
			// calls parser.replace(arg1, item)
			// begins with the script
			// "consume 10 iron"
			// "gift 1 iron sword"


	}
}

#endif 