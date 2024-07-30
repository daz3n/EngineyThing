#pragma once
#if 0
namespace framework
{
	struct actor
	{
	private:
		// decrease incoming damage by armour, toughness, etc.
		bool apply_damage_reduction(double& inout);
		// decrease current health by an amount
		bool apply_health_reduction(double in);
		// decrease max health by an amount
		bool apply_health_limit(double in);
	public:
		void set_damage_reduction_callback();
		void set_health_reduction_callback();
		void set_health_limit_callback();

		void take_damage(double in)
		{
			apply_damage_reduction(in);
			apply_health_reduction(in);
			apply_health_limit(in);
		}

	};
}
#endif 