#include "Hooks.h"

namespace Hook
{
	struct MainUpdate
	{
		static float GetCurrentGameTime()
		{
			using func_t = decltype(&GetCurrentGameTime);
			REL::Relocation<func_t> funct{ REL::VariantID(56475, 56832, 0x9F3290) };
			return funct();
		}

		static void thunk()
		{
			/*

			This function runs every few ms. We could use a thread instead to just run every second or whatever, but performance shouldn't be effected anyway from just checking some stuff.
			Couldn't find a function that runs on every weather change. We need a function for the time anyway...

			*/

			// Run functions

			func(); // Run original function
		};
		static inline REL::Relocation<decltype(thunk)> func;


		static void Install()
		{
			bool marketplace = REL::Module::get().version() >= SKSE::RUNTIME_SSE_1_6_1130;
			REL::Relocation<std::uintptr_t> target1{ RELOCATION_ID(35565, 36564), REL::Relocate(0x748, (marketplace ? 0xC2b : 0xC26), 0x7EE) };
			stl::write_thunk_call<MainUpdate>(target1.address());
		}
	};

	struct InteriorChange
	{
		static void thunk(bool a_playerIsInInterior) // Runs on cell load
		{
			func(a_playerIsInInterior);

			// Run process function
		};
		static inline REL::Relocation<decltype(thunk)> func;


		static void Install()
		{
			std::array targets{
				std::make_pair(RELOCATION_ID(13171, 13316), REL::Relocate(0x2E6, 0x46D)),
				std::make_pair(RELOCATION_ID(13172, 13317), REL::Relocate(0x2A, 0x1E)),
			};

			for (auto& [id, offset] : targets)
			{
				REL::Relocation<std::uintptr_t> target(id, offset);
				stl::write_thunk_call<InteriorChange>(target.address());
			}

		}
	};



	void Install()
	{
		MainUpdate::Install();
		InteriorChange::Install();

	}
}