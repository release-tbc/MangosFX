/* Copyright (C) 2006 - 2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Boss_Victor_Nefarius
SD%Complete: 75
SDComment: Missing some text, Vael beginning event, and spawns Nef in wrong place
SDCategory: Blackwing Lair
EndScriptData */

#include "precompiled.h"

#define SAY_GAMESBEGIN_1        -1469004
#define SAY_GAMESBEGIN_2        -1469005
#define SAY_VAEL_INTRO          -1469006                    //when he corrupts Vaelastrasz

#define GOSSIP_ITEM_1           "I've made no mistakes."
#define GOSSIP_ITEM_2           "You have lost your mind, Nefarius. You speak in riddles."
#define GOSSIP_ITEM_3           "Please do."

#define CREATURE_BRONZE_DRAKANOID       14263
#define CREATURE_BLUE_DRAKANOID         14261
#define CREATURE_RED_DRAKANOID          14264
#define CREATURE_GREEN_DRAKANOID        14262
#define CREATURE_BLACK_DRAKANOID        14265

#define CREATURE_CHROMATIC_DRAKANOID    14302
#define CREATURE_NEFARIAN               11583

#define ADD_X1 -7591.151855
#define ADD_X2 -7514.598633
#define ADD_Y1 -1204.051880
#define ADD_Y2 -1150.448853
#define ADD_Z1 476.800476
#define ADD_Z2 476.796570

#define NEF_X   -7445
#define NEF_Y   -1332
#define NEF_Z   536

#define HIDE_X  -7592
#define HIDE_Y  -1264
#define HIDE_Z  481

#define SPELL_SHADOWBOLT        21077
#define SPELL_FEAR              26070

//This script is complicated
//Instead of morphing Victor Nefarius we will have him control phase 1
//And then have him spawn "Nefarian" for phase 2
//When phase 2 starts Victor Nefarius will go into hiding and stop attacking
//If Nefarian despawns because he killed the players then this guy will EnterEvadeMode
//and allow players to start the event over
//If nefarian dies then he will kill himself then he will kill himself in his hiding place
//To prevent players from doing the event twice

struct MANGOS_DLL_DECL boss_victor_nefariusAI : public LibDevFSAI
{
    boss_victor_nefariusAI(Creature* pCreature) : LibDevFSAI(pCreature)
    {
		InitIA();
        NefarianGUID = 0;
        srand(time(NULL));
        switch(urand(0, 19))
        {
            case 0:
                DrakType1 = CREATURE_BRONZE_DRAKANOID;
                DrakType2 = CREATURE_BLUE_DRAKANOID;
                break;
            case 1:
                DrakType1 = CREATURE_BRONZE_DRAKANOID;
                DrakType2 = CREATURE_RED_DRAKANOID;
                break;
            case 2:
                DrakType1 = CREATURE_BRONZE_DRAKANOID;
                DrakType2 = CREATURE_GREEN_DRAKANOID;
                break;
            case 3:
                DrakType1 = CREATURE_BRONZE_DRAKANOID;
                DrakType2 = CREATURE_BLACK_DRAKANOID;
                break;
            case 4:
                DrakType1 = CREATURE_BLUE_DRAKANOID;
                DrakType2 = CREATURE_BRONZE_DRAKANOID;
                break;
            case 5:
                DrakType1 = CREATURE_BLUE_DRAKANOID;
                DrakType2 = CREATURE_RED_DRAKANOID;
                break;
            case 6:
                DrakType1 = CREATURE_BLUE_DRAKANOID;
                DrakType2 = CREATURE_GREEN_DRAKANOID;
                break;
            case 7:
                DrakType1 = CREATURE_BLUE_DRAKANOID;
                DrakType2 = CREATURE_BLACK_DRAKANOID;
                break;
            case 8:
                DrakType1 = CREATURE_RED_DRAKANOID;
                DrakType2 = CREATURE_BRONZE_DRAKANOID;
                break;
            case 9:
                DrakType1 = CREATURE_RED_DRAKANOID;
                DrakType2 = CREATURE_BLUE_DRAKANOID;
                break;
            case 10:
                DrakType1 = CREATURE_RED_DRAKANOID;
                DrakType2 = CREATURE_GREEN_DRAKANOID;
                break;
            case 11:
                DrakType1 = CREATURE_RED_DRAKANOID;
                DrakType2 = CREATURE_BLACK_DRAKANOID;
                break;
            case 12:
                DrakType1 = CREATURE_GREEN_DRAKANOID;
                DrakType2 = CREATURE_BRONZE_DRAKANOID;
                break;
            case 13:
                DrakType1 = CREATURE_GREEN_DRAKANOID;
                DrakType2 = CREATURE_BLUE_DRAKANOID;
                break;
            case 14:
                DrakType1 = CREATURE_GREEN_DRAKANOID;
                DrakType2 = CREATURE_RED_DRAKANOID;
                break;
            case 15:
                DrakType1 = CREATURE_GREEN_DRAKANOID;
                DrakType2 = CREATURE_BLACK_DRAKANOID;
                break;
            case 16:
                DrakType1 = CREATURE_BLACK_DRAKANOID;
                DrakType2 = CREATURE_BRONZE_DRAKANOID;
                break;
            case 17:
                DrakType1 = CREATURE_BLACK_DRAKANOID;
                DrakType2 = CREATURE_BLUE_DRAKANOID;
                break;
            case 18:
                DrakType1 = CREATURE_BLACK_DRAKANOID;
                DrakType2 = CREATURE_GREEN_DRAKANOID;
                break;
            case 19:
                DrakType1 = CREATURE_BLACK_DRAKANOID;
                DrakType2 = CREATURE_RED_DRAKANOID;
                break;
        }
        AddEvent(SPELL_SHADOWBOLT,5000,3000,7000);
        AddEvent(SPELL_FEAR,8000,10000,10000);
    }

    uint32 SpawnedAdds;
    uint32 AddSpawnTimer;
    uint32 MindControlTimer;
    uint32 ResetTimer;
    uint32 DrakType1;
    uint32 DrakType2;
    uint64 NefarianGUID;
    uint32 NefCheckTime;

    void Reset()
    {
        SpawnedAdds = 0;
        AddSpawnTimer = 10000;
        ResetTimer = 900000;                                //On official it takes him 15 minutes(900 seconds) to reset. We are only doing 1 minute to make testing easier
        NefarianGUID = 0;
        NefCheckTime = 2000;

        me->SetUInt32Value(UNIT_NPC_FLAGS,1);
        me->setFaction(35);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void BeginEvent(Player* target)
    {
        DoScriptText(SAY_GAMESBEGIN_2, me);

        //MaNGOS::Singleton<MapManager>::Instance().GetMap(me->GetMapId(), me)->GetPlayers().begin();
        /*
        list <Player*>::iterator i = MapManager::Instance().GetMap(me->GetMapId(), me)->GetPlayers().begin();

        for (i = MapManager::Instance().GetMap(me->GetMapId(), me)->GetPlayers().begin(); i != MapManager::Instance().GetMap(me->GetMapId(), me)->GetPlayers().end(); ++i)
        {
        AttackStart((*i));
        }
        */
        me->SetUInt32Value(UNIT_NPC_FLAGS,0);
        me->setFaction(103);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        AttackStart(target);
    }

    void MoveInLineOfSight(Unit *who)
    {
        //We simply use this function to find players until we can use Map->GetPlayers()

        if (who && who->GetTypeId() == TYPEID_PLAYER && me->IsHostileTo(who))
        {
            //Add them to our threat list
            me->AddThreat(who,0.0f);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!CanDoSomething())
            return;

        //Only do this if we haven't spawned nef yet
        if (SpawnedAdds < 42)
        {
            //Add spawning mechanism
            if (AddSpawnTimer < diff)
            {
                //Spawn 2 random types of creatures at the 2 locations
                uint32 CreatureID;
                Creature* Spawned = NULL;
                Unit* target = NULL;

                //1 in 3 chance it will be a chromatic
                if (!urand(0, 2))
                    CreatureID = CREATURE_CHROMATIC_DRAKANOID;
                else CreatureID = DrakType1;

                ++SpawnedAdds;

                //Spawn creature and force it to start attacking a random target
                Spawned = me->SummonCreature(CreatureID,ADD_X1,ADD_Y1,ADD_Z1,5.000,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                target = SelectUnit(SELECT_TARGET_RANDOM,0);
                if (target && Spawned)
                {
                    Spawned->AI()->AttackStart(target);
                    Spawned->setFaction(103);
                }

                //1 in 3 chance it will be a chromatic
                if (!urand(0, 2))
                    CreatureID = CREATURE_CHROMATIC_DRAKANOID;
                else CreatureID = DrakType2;

                ++SpawnedAdds;

                target = NULL;
                Spawned = NULL;
                Spawned = me->SummonCreature(CreatureID,ADD_X2,ADD_Y2,ADD_Z2,5.000,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                target = SelectUnit(SELECT_TARGET_RANDOM,0);
                if (target && Spawned)
                {
                    Spawned->AI()->AttackStart(target);
                    Spawned->setFaction(103);
                }

                //Begin phase 2 by spawning Nefarian and what not
                if (SpawnedAdds >= 42)
                {
                    //Teleport Victor Nefarius way out of the map
                    //MapManager::Instance().GetMap(me->GetMapId(), me)->CreatureRelocation(me,0,0,-5000,0);

                    //Inturrupt any spell casting
                    me->InterruptNonMeleeSpells(false);

                    //Root self
                    DoCastMe(33356);

                    //Make super invis
                    DoCastMe(8149);

                    //Teleport self to a hiding spot (this causes errors in the mangos log but no real issues)
                    me->GetMap()->CreatureRelocation(me, HIDE_X, HIDE_Y, HIDE_Z, 0.0f);
                    me->SendMonsterMove(HIDE_X, HIDE_Y, HIDE_Z, SPLINETYPE_NORMAL, SPLINEFLAG_NONE, 0);

                    //Spawn nef and have him attack a random target
                    Creature* Nefarian = me->SummonCreature(CREATURE_NEFARIAN,NEF_X,NEF_Y,NEF_Z,SPLINETYPE_NORMAL,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,120000);
                    target = SelectUnit(SELECT_TARGET_RANDOM,0);

                    if (target && Nefarian)
                    {
                        Nefarian->AI()->AttackStart(target);
                        Nefarian->setFaction(103);
                        NefarianGUID = Nefarian->GetGUID();
                    }
                    else error_log("SD2: Blackwing Lair: Unable to spawn nefarian properly.");
                }

                AddSpawnTimer = 4000;
            }else AddSpawnTimer -= diff;
            
            UpdateEvent(diff);
        }
        else if (NefarianGUID)
        {
            if (NefCheckTime < diff)
            {
                Creature* pNefarian = (Creature*)Unit::GetUnit((*me),NefarianGUID);

                //If nef is dead then we die to so the players get out of combat
                //and cannot repeat the event
                if (!pNefarian || !pNefarian->isAlive())
                {
                    NefarianGUID = 0;
                    me->ForcedDespawn();
                }

                NefCheckTime = 2000;
            }else NefCheckTime -= diff;
        }
    }
};

CreatureAI* GetAI_boss_victor_nefarius(Creature* pCreature)
{
    return new boss_victor_nefariusAI(pCreature);
}

bool GossipHello_boss_victor_nefarius(Player* pPlayer, Creature* pCreature)
{
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_1 , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    pPlayer->SEND_GOSSIP_MENU(7134, pCreature->GetGUID());
    return true;
}

bool GossipSelect_boss_victor_nefarius(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            pPlayer->SEND_GOSSIP_MENU(7198, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            pPlayer->SEND_GOSSIP_MENU(7199, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            pPlayer->CLOSE_GOSSIP_MENU();
            DoScriptText(SAY_GAMESBEGIN_1, pCreature);
            ((boss_victor_nefariusAI*)pCreature->AI())->BeginEvent(pPlayer);
            break;
    }
    return true;
}

void AddSC_boss_victor_nefarius()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_victor_nefarius";
    newscript->GetAI = &GetAI_boss_victor_nefarius;
    newscript->pGossipHello = &GossipHello_boss_victor_nefarius;
    newscript->pGossipSelect = &GossipSelect_boss_victor_nefarius;
    newscript->RegisterSelf();
}
