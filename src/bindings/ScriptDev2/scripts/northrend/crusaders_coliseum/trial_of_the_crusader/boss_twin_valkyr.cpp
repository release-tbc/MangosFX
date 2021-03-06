#include "precompiled.h"
#include "trial_of_the_crusader.h"
enum 
{
	SPELL_BERSERK = 64238,
	SPELL_LIGHT_VORTEX = 66046,
	SPELL_POWER_OF_THE_TWINS = 65916,
	SPELL_SHIELD_OF_LIGHTS = 65858,
	SPELL_SURGE_OF_LIGHT = 65766,
	SPELL_TOUCH_OF_LIGHT = 65950,
	SPELL_TWIN_SPIKE = 66075,
	SPELL_TWINS_PACT2 = 65876,
};

enum 
{
	SPELL_BERSERK_10 = 64238,
	SPELL_BERSERK_25 = 68378,
	SPELL_DARK_VORTEX = 66058,
	SPELL_SHIELD_OF_DARKNESS = 65874,
	SPELL_SURGE_OF_DARKNESS = 65768,
	SPELL_TOUCH_OF_DARKNESS = 66001,
	SPELL_TWIN_SPIKE2 = 66069,
	SPELL_TWINS_PACT = 65875,
};

struct MANGOS_DLL_DECL boss_Eydis_DarkbaneAI : public LibDevFSAI
{
    boss_Eydis_DarkbaneAI(Creature* pCreature) : LibDevFSAI(pCreature)
    {
        InitInstance();
		AddEventOnMe(SPELL_POWER_OF_THE_TWINS,120000,8000,20000);
		AddEvent(SPELL_TOUCH_OF_DARKNESS,20000,20000);
		AddEventOnTank(SPELL_TWIN_SPIKE2,18000,21000,1000);
		switch(m_difficulty)
		{
			case RAID_DIFFICULTY_10MAN_NORMAL:
				AddEventMaxPrioOnMe(SPELL_BERSERK_10,360000,10000);
				break;
			case RAID_DIFFICULTY_25MAN_NORMAL:
				AddEventMaxPrioOnMe(SPELL_BERSERK_25,360000,10000);
				break;
			case RAID_DIFFICULTY_10MAN_HEROIC:
				AddEventMaxPrioOnMe(SPELL_BERSERK_10,360000,10000);
				break;
			case RAID_DIFFICULTY_25MAN_HEROIC:
				AddEventMaxPrioOnMe(SPELL_BERSERK_25,360000,10000);
				break;
		}
    }

	bool HealEvent;
	uint32 HealCast_Timer;
	uint32 Ball_Timer;

    void Reset()
    {
		ResetTimers();
		HealEvent = false;
		HealCast_Timer = 1000;
		Ball_Timer = 8000;
		DoCastMe(SPELL_SURGE_OF_DARKNESS);
		SetEquipmentSlots(false, 45990, 47470, 47267);
		ActivateTimeDown(240000);
    }

	void DamageDeal(Unit *pDoneTo, uint32 &uiDamage)
	{
		if(pDoneTo->GetTypeId() == TYPEID_UNIT)
			if(pDoneTo->GetEntry() == 34496 || pDoneTo->GetEntry() == 34497)
				uiDamage = 0;
	}

	void DamageTaken(Unit* pDoneBy, uint32 &dmg)
	{
		if (Creature* Fjola = GetInstanceCreature(TYPE_Fjola_Lightbane))
			if(Fjola->isAlive())
				pDoneBy->DealDamage(Fjola,dmg,NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
	}

	void HealBy(Unit* pHealer,uint32& heal)
	{
		if (Creature* Fjola = GetInstanceCreature(TYPE_Fjola_Lightbane))
			if(Fjola->isAlive())
				Fjola->SetHealth(Fjola->GetHealth() + heal);
	}

    void JustDied(Unit *victim)
    {
        if (pInstance)
            SetInstanceData(TYPE_Eydis_Darkbane, DONE);
            
		if (Creature* Fjola = GetInstanceCreature(TYPE_Fjola_Lightbane))
			if(!Fjola->isAlive())
			{
				SetInstanceData(TYPE_VALKYRS,DONE);
				if (Creature* Ann = GetInstanceCreature(DATA_ANNOUNCER))
					((npc_toc10_announcerAI*)Ann->AI())->StartEvent(NULL,EVENT_TYPE_VALKYR_OUTRO);

				me->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
				Fjola->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
			}
			else
				me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

		switch(m_difficulty)
		{
			case RAID_DIFFICULTY_10MAN_NORMAL:
				GiveEmblemsToGroup(CONQUETE,2);
				break;
			case RAID_DIFFICULTY_25MAN_NORMAL:
				GiveEmblemsToGroup(TRIOMPHE,2);
				break;
			case RAID_DIFFICULTY_10MAN_HEROIC:
				GiveEmblemsToGroup(TRIOMPHE,3);
				break;
			case RAID_DIFFICULTY_25MAN_HEROIC:
				GiveEmblemsToGroup(TRIOMPHE,3);
				break;
		}
		if(pInstance && TimeDownSucceed())
		{
			switch(m_difficulty)
			{
				case RAID_DIFFICULTY_10MAN_NORMAL:
				case RAID_DIFFICULTY_10MAN_HEROIC:
					pInstance->CompleteAchievementForGroup(3799);
					break;
				case RAID_DIFFICULTY_25MAN_NORMAL:
				case RAID_DIFFICULTY_25MAN_HEROIC:
					pInstance->CompleteAchievementForGroup(3815);
					break;
			}
		}
		me->ForcedDespawn(TEN_MINS*1000);
    }

	void Aggro(Unit* pWho)
    {
        me->SetInCombatWithZone();

        if (pInstance)
            SetInstanceData(TYPE_Eydis_Darkbane, IN_PROGRESS);
    }

	void DoEvent()
	{
		me->CastStop();
		if(urand(0,1))
			DoCastVictim(SPELL_DARK_VORTEX);
		else
		{
			HealEvent = true;
			DoCastMe(SPELL_SHIELD_OF_DARKNESS);
		}
	}

	void KilledUnit(Unit* victim)
	{
		if(victim->GetTypeId() == TYPEID_PLAYER)
			SetInstanceData(TYPE_TRY,1);
	}

    void UpdateAI(const uint32 diff)
    {
		if (!CanDoSomething())
            return;

		if(HealEvent)
		{
			if(HealCast_Timer <= diff)
			{
				DoCastMe(SPELL_TWINS_PACT);
				HealEvent = false;
				HealCast_Timer = 1000;
			}
			else
				HealCast_Timer -= diff;
		}

		if(Ball_Timer <= diff)
		{
			if(Creature* ball = CallCreature(34630,THREE_MINS*3,NEAR_30M))
				if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0))
					ball->GetMotionMaster()->MoveChase(target);
			Ball_Timer = urand(3000,5000);
		}
		else
			Ball_Timer -= diff;

		UpdateEvent(diff);

        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_boss_Eydis_Darkbane(Creature* pCreature)
{
    return new boss_Eydis_DarkbaneAI(pCreature);
}

struct MANGOS_DLL_DECL boss_Fjola_LightbaneAI : public LibDevFSAI
{
    boss_Fjola_LightbaneAI(Creature* pCreature) : LibDevFSAI(pCreature)
    {
        InitInstance();
		switch(m_difficulty)
		{
			case RAID_DIFFICULTY_10MAN_NORMAL:
				AddEventMaxPrioOnMe(SPELL_BERSERK_10,360000,10000);
				break;
			case RAID_DIFFICULTY_25MAN_NORMAL:
				AddEventMaxPrioOnMe(SPELL_BERSERK_25,360000,10000);
				break;
			case RAID_DIFFICULTY_10MAN_HEROIC:
				AddEventMaxPrioOnMe(SPELL_BERSERK_10,360000,10000);
				break;
			case RAID_DIFFICULTY_25MAN_HEROIC:
				AddEventMaxPrioOnMe(SPELL_BERSERK_25,360000,10000);
				break;
		}
		AddEventOnMe(SPELL_POWER_OF_THE_TWINS,120000,8000,20000);
		AddEvent(SPELL_TOUCH_OF_LIGHT,20000,20000);
		AddEventOnTank(SPELL_TWIN_SPIKE,18000,21000,1000,TARGET_MAIN);
    }

	bool HealEvent;
	uint32 Ball_Timer;
	uint32 HealCast_Timer;
	uint32 Event_Timer;

    void Reset()
    {
		ResetTimers();
		HealEvent = false;
		HealCast_Timer = 1000;
		Ball_Timer = 8000;
		DoCastMe(SPELL_SURGE_OF_LIGHT);
		switch(m_difficulty)
		{
			case RAID_DIFFICULTY_10MAN_NORMAL:
			case RAID_DIFFICULTY_25MAN_NORMAL:
				Event_Timer = 40000;
				break;
			case RAID_DIFFICULTY_10MAN_HEROIC:
			case RAID_DIFFICULTY_25MAN_HEROIC:
				Event_Timer = 30000;
				break;
		}
		
		SetEquipmentSlots(false, 49303, 47146, 47267);
    }

	void DamageTaken(Unit* pDoneBy, uint32 &dmg)
	{
		if (Creature* Eydis = GetInstanceCreature(TYPE_Eydis_Darkbane))
			if(Eydis->isAlive())
				pDoneBy->DealDamage(Eydis,dmg,NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
	}

    void JustDied(Unit *victim)
    {
        if (pInstance)
            SetInstanceData(TYPE_Fjola_Lightbane, DONE);
            
		if (Creature* Eydis = GetInstanceCreature(TYPE_Eydis_Darkbane))
			if(!Eydis->isAlive())
			{
				SetInstanceData(TYPE_VALKYRS,DONE);
				if (Creature* Ann = GetInstanceCreature(DATA_ANNOUNCER))
					((npc_toc10_announcerAI*)Ann->AI())->StartEvent(NULL,EVENT_TYPE_VALKYR_OUTRO);
					
				me->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
				Eydis->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
			}
			else
				me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
		me->ForcedDespawn(TEN_MINS*1000);
    }

	void KilledUnit(Unit* victim)
	{
		if(victim->GetTypeId() == TYPEID_PLAYER)
			SetInstanceData(TYPE_TRY,1);
	}

    void Aggro(Unit* pWho)
    {
        me->SetInCombatWithZone();

        if (pInstance)
            SetInstanceData(TYPE_Fjola_Lightbane, IN_PROGRESS);
    }

	void DamageDeal(Unit *pDoneTo, uint32 &uiDamage)
	{
		if(pDoneTo->GetTypeId() == TYPEID_UNIT)
			if(pDoneTo->GetEntry() == 34496 || pDoneTo->GetEntry() == 34497)
				uiDamage = 0;
	}

	void HealBy(Unit* pHealer,uint32& heal)
	{
		if (Creature* Eydis = GetInstanceCreature(TYPE_Eydis_Darkbane))
			if(Eydis->isAlive())
				Eydis->SetHealth(me->GetHealth());
	}

	void DoEvent()
	{
		me->CastStop();
		if(urand(0,1))
			DoCastVictim(SPELL_LIGHT_VORTEX);
		else
		{
			DoCastMe(SPELL_SHIELD_OF_LIGHTS);
			HealEvent = true;
		}
	}

    void UpdateAI(const uint32 diff)
    {
		if (!CanDoSomething())
            return;

		if(HealEvent)
		{
			if(HealCast_Timer <= diff)
			{
				DoCastMe(SPELL_TWINS_PACT2);
				HealEvent = false;
				HealCast_Timer = 1000;
			}
			else
				HealCast_Timer -= diff;
		}

		if(Event_Timer <= diff)
		{
			if(urand(0,1))
			{
				if (Creature* Eydis = GetInstanceCreature(TYPE_Eydis_Darkbane))
					if(Eydis->isAlive())
						((boss_Eydis_DarkbaneAI*)Eydis->AI())->DoEvent();
			}
			else
				DoEvent();

			Event_Timer = 40000;
		}
		else
			Event_Timer -= diff;

		if(Ball_Timer <= diff)
		{
			if(Creature* ball = CallCreature(34628,THREE_MINS*3,NEAR_30M))
				if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0))
					ball->GetMotionMaster()->MoveChase(target);
			Ball_Timer = urand(3000,5000);
		}
		else
			Ball_Timer -= diff;

		UpdateEvent(diff);

        DoMeleeAttackIfReady();

    }

};

CreatureAI* GetAI_boss_Fjola_Lightbane(Creature* pCreature)
{
    return new boss_Fjola_LightbaneAI(pCreature);
}

struct MANGOS_DLL_DECL Valkyr_BallAI : public LibDevFSAI
{
    Valkyr_BallAI(Creature* pCreature) : LibDevFSAI(pCreature)
    {
        InitInstance();
    }

	uint32 move_Timer;
	uint32 checkpDist_Timer;

    void Reset()
    {
		me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
		SetCombatMovement(false);
		move_Timer = 15000;
		checkpDist_Timer = 1000;
    }

	void CheckDist()
	{
		bool Event = false;
		Map::PlayerList const& lPlayers = me->GetMap()->GetPlayers();
		if (!lPlayers.isEmpty())
			for(Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
				if (Player* pPlayer = itr->getSource())
					if(pPlayer->isAlive() && !pPlayer->isGameMaster() && pPlayer->GetDistance2d(me) < 7.0f)
						Event = true;

		if (Creature* Darkbane = GetInstanceCreature(TYPE_Eydis_Darkbane))
			if(Darkbane->isAlive() && Darkbane->GetDistance2d(me) < 7.0f)
				Event = true;
		if (Creature* Lightbane = GetInstanceCreature(TYPE_Fjola_Lightbane))
			if(Lightbane->isAlive() && Lightbane->GetDistance2d(me) < 7.0f)
				Event = true;

		if(Event)
			DoEvent();
	}

	void DoEvent()
	{
		Map::PlayerList const& lPlayers = me->GetMap()->GetPlayers();
		if (!lPlayers.isEmpty())
			for(Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
				if (Player* pPlayer = itr->getSource())
					if(pPlayer->isAlive() && !pPlayer->isGameMaster() && pPlayer->GetDistance2d(me) < 7.0f)
					{
						if(pPlayer->HasAura(65686))
						{
							if(me->GetEntry() == 34630)
								UpdateStacks(pPlayer);
						}
						else if(pPlayer->HasAura(65684))
						{
							if(me->GetEntry() == 34628)
								UpdateStacks(pPlayer);
						}
					}

		if (Creature* Darkbane = GetInstanceCreature(TYPE_Eydis_Darkbane))
			if(Darkbane->isAlive() && Darkbane->GetDistance2d(me) < 7.0f)
				if(me->GetEntry() == 34628)
					UpdateStacks(Darkbane);
		if (Creature* Lightbane = GetInstanceCreature(TYPE_Fjola_Lightbane))
			if(Lightbane->isAlive() && Lightbane->GetDistance2d(me) < 7.0f)
				if(me->GetEntry() == 34630)
					UpdateStacks(Lightbane);

		if(me->GetEntry() == 34630)
			DoCastMe(65795);
		else if(me->GetEntry() == 34628)
			DoCastMe(65808);

		me->RemoveFromWorld();
	}

	void UpdateStacks(Unit* u)
	{
		uint32 stk = 0;
		if(u->HasAura(67590))
			stk = u->GetAura(67590,0)->GetStackAmount();
		if((stk + 7) >= 100)
		{
			u->RemoveAurasDueToSpell(67590);
			if(urand(0,1))
				ModifyAuraStack(65724,1,u);
			else
				ModifyAuraStack(65748,1,u);
		}
		else
			ModifyAuraStack(67590,stk + 7,u);
	}

    void UpdateAI(const uint32 diff)
    {
		if(move_Timer <= diff)
		{
			if(Unit* u = SelectUnit(SELECT_TARGET_RANDOM,0))
				me->GetMotionMaster()->MovePoint(0,u->GetPositionX(),u->GetPositionY(),u->GetPositionZ());
			move_Timer = 8000;
		}
		else
			move_Timer -= diff;

		if(checkpDist_Timer <= diff)
		{
			CheckDist();
			checkpDist_Timer = 1000;
		}
		else
			checkpDist_Timer -= diff;
    }

};

CreatureAI* GetAI_Valkyr_Ball(Creature* pCreature)
{
    return new Valkyr_BallAI(pCreature);
}

void AddSC_boss_twin_valkyr()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_Eydis_Darkbane";
    newscript->GetAI = &GetAI_boss_Eydis_Darkbane;
    newscript->RegisterSelf();

	newscript = new Script;
    newscript->Name = "boss_Fjola_Lightbane";
    newscript->GetAI = &GetAI_boss_Fjola_Lightbane;
    newscript->RegisterSelf();

	newscript = new Script;
    newscript->Name = "valkyr_ball";
    newscript->GetAI = &GetAI_Valkyr_Ball;
    newscript->RegisterSelf();
}
