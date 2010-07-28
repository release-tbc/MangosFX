#ifndef MANGOS_DKSPELLHANDLER_H
#define MANGOS_DKSPELLHANDLER_H

#include <Policies/Singleton.h>
#include <Common.h>

class MANGOS_DLL_SPEC DeathknightSpellHandler
{
	public:
		void HandleEffectWeaponDamage(Spell* spell, int32 &spell_bonus, bool &weaponDmgMod, float &totalDmgPctMod);
};

#define sDeathknightSpellHandler MaNGOS::Singleton<DeathknightSpellHandler>::Instance()
#endif