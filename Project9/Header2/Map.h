#pragma once
#include <vector>

namespace unfinished_cuz_its_useless
{ 
	template<typename K, auto ... _keys>
	struct static_keys { typedef K key_type; };

	template<typename V, auto ... _vals>
	struct static_vals { typedef V value_type; };

	template<typename Keys, typename Vals>
	struct static_map
	{
		static typename Vals::value_type key(typename Keys::key_type Key)
		{
			return {};
		}
	};

	inline void demo()
	{
		static_map<
			static_keys<int, 1, 2, 3>,
			static_vals<int, 9, 8, 7>>::key(3); // returns 7
	}
}
