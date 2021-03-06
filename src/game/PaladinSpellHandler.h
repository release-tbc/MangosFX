#ifndef MANGOS_PALADINSPELLHANDLER_H
#define MANGOS_PALADINSPELLHANDLER_H

#include <Policies/Singleton.h>
#include <Common.h>

class MANGOS_DLL_SPEC PaladinSpellHandler
{
	public:
		void HandleEffectWeaponDamage(Spell* spell, int32 &spell_bonus, bool &weaponDmgMod, float &totalDmgPctMod);
		void HandleSchoolDmg(Spell *spell,int32 &damage,SpellEffectIndex i);
		//void HandleDummyAuraProc(Unit* u, Spell* dummy, uint32 &trig_sp_id);
};

#define sPaladinSpellHandler MaNGOS::Singleton<PaladinSpellHandler>::Instance()
#endif