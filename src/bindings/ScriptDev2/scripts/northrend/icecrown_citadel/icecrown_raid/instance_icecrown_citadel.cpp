/* Copyright (C) 2006 - 2010 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Instance_Icecrown_Citadel
SD%Complete: 0
SDComment: Written by K
SDCategory: Icecrown Citadel
EndScriptData */

#include "precompiled.h"
#include "icecrown_citadel.h"

struct MANGOS_DLL_DECL instance_icecrown_citadel : public InstanceData
{
    instance_icecrown_citadel(Map* pMap) : InstanceData(pMap) {Initialize();}

    std::string strInstData;
    uint32 m_auiEncounter[MAX_ENCOUNTER];

    uint64 m_uiMarrowgarGUID;
	uint64 m_uiMarrowgarDoorGUID;
    uint64 m_uiDeathwhisperGUID;
    uint64 m_uiSaurfangGUID;
	uint64 m_uiFestergutGUID;
	uint64 m_uiRotfaceGUID;
	uint64 m_uiPutricideGUID;
	uint64 m_uiPrinceKelesethGUID;
	uint64 m_uiPrinceValanarGUID;
	uint64 m_uiPrinceTaldaramGUID;
	uint64 m_uiLanathelGUID;
	uint64 m_uiDreamWalkerGUID;
	uint64 m_uiSindragosaGUID;
	uint64 m_uiLichKingGUID;

	std::vector<uint64> rotfacePoolsGUIDs;
	std::vector<uint64> vortexGUIDs;

    uint64 m_uiMarrowgarIce1GUID;
    uint64 m_uiMarrowgarIce2GUID;
    uint64 m_uiDeathwhisperGateGUID;
    uint64 m_uiDeathwhisperElevatorGUID;
    uint64 m_uiSaurfangDoorGUID;
	uint64 m_uiFestergutDoorGUID;
	uint64 m_uiRotfaceDoorGUID;
	uint64 m_uiPutricideDoorGUID;
	uint64 m_uiBloodWingDoorGUID;
	uint64 m_uiPrinceCouncilDoorGUID;
	uint64 m_uiLanathelDoorGUID_1;
	uint64 m_uiLanathelDoorGUID_2;
	uint64 m_uiFrostWingDoorGUID;
	uint64 m_uiDreamWalkerDoorGUID;
	uint64 m_uiDreamWalkerExitDoorGUID;
	uint64 m_uiSindragosaDoorGUID;

	uint64 m_uiDreamwalkerEventDoorGUID[4];
	std::vector<uint64> dreamwalkerAddsGUIDs;
	std::vector<uint64> dreamwalkerEventAddGUIDs;
	uint8 m_uiDreamwalkerGuardDone;
	uint64 m_uiDreamwalkerImageGUID;

	uint64 m_uiFordringLKEventGUID;

	uint64 m_uiSaurfangCacheGUID;

	uint64 m_uiPlagueSigilGUID;
	uint64 m_uiBloodSigilGUID;
	uint64 m_uiFrostSigilGUID;

	uint64 m_uiOrangeTubeGUID;
	uint64 m_uiOrangeDoorGUID;

	uint32 checkPlayer_Timer;

    void Initialize()
    {
        memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));

        m_uiMarrowgarGUID               = 0;
		m_uiMarrowgarDoorGUID			= 0;
        m_uiDeathwhisperGUID            = 0;
        m_uiSaurfangGUID                = 0;
		m_uiFestergutGUID				= 0;
		m_uiRotfaceGUID					= 0;
		m_uiPutricideGUID				= 0;
		m_uiPrinceKelesethGUID			= 0;
		m_uiPrinceValanarGUID			= 0;
		m_uiPrinceTaldaramGUID			= 0;
		m_uiLanathelGUID				= 0;
		m_uiDreamWalkerGUID				= 0;
		m_uiSindragosaGUID				= 0;
		// Other IAs
		m_uiLichKingGUID				= 0;

		rotfacePoolsGUIDs.clear();
		vortexGUIDs.clear();

        m_uiMarrowgarIce1GUID           = 0;
        m_uiMarrowgarIce2GUID           = 0;
        m_uiDeathwhisperGateGUID        = 0;
        m_uiDeathwhisperElevatorGUID    = 0;
        m_uiSaurfangDoorGUID            = 0;
		m_uiFestergutDoorGUID			= 0;
		m_uiRotfaceDoorGUID				= 0;
		m_uiPutricideDoorGUID			= 0;
		m_uiPrinceCouncilDoorGUID		= 0;
		m_uiBloodWingDoorGUID			= 0;
		m_uiPrinceCouncilDoorGUID		= 0;
		m_uiLanathelDoorGUID_1			= 0;
		m_uiLanathelDoorGUID_2			= 0;
		m_uiFrostWingDoorGUID			= 0;
		m_uiDreamWalkerDoorGUID			= 0;
		m_uiDreamWalkerExitDoorGUID		= 0;
		m_uiSindragosaDoorGUID			= 0;

		m_uiSaurfangCacheGUID			= 0;

		for(uint8 i=0;i<4;i++)
			m_uiDreamwalkerEventDoorGUID[i] = 0;

		dreamwalkerAddsGUIDs.clear();
		dreamwalkerEventAddGUIDs.clear();
		m_uiDreamwalkerGuardDone = 0;
		m_uiDreamwalkerImageGUID = 0;

		m_uiFordringLKEventGUID = 0;

		m_uiPlagueSigilGUID				= 0;
		m_uiBloodSigilGUID				= 0;
		m_uiFrostSigilGUID				= 0;

		m_uiOrangeTubeGUID				= 0;
		m_uiOrangeDoorGUID				= 0;

		checkPlayer_Timer = 500;
    }

    void OnCreatureCreate(Creature* pCreature)
    {
        switch(pCreature->GetEntry())
        {
            case NPC_MARROWGAR: 
				m_uiMarrowgarGUID = pCreature->GetGUID();
				if(!pCreature->isAlive())
					SetData(TYPE_MARROWGAR,DONE);
				break;
            case NPC_DEATHWHISPER: 
				m_uiDeathwhisperGUID = pCreature->GetGUID();
				if(!pCreature->isAlive())
					SetData(TYPE_DEATHWHISPER,DONE);
				break;
            case NPC_SAURFANG: 
				m_uiSaurfangGUID = pCreature->GetGUID();
				if(!pCreature->isAlive())
				{
					m_auiEncounter[TYPE_SAURFANG] = DONE;
					OpenDoor(m_uiSaurfangDoorGUID);
				}
				break;
			case NPC_FESTERGUT:
				if(isHeroic())	AutoFreeze(pCreature);
				m_uiFestergutGUID = pCreature->GetGUID();
				if(!pCreature->isAlive())
					SetData(TYPE_FESTERGUT,DONE);
				break;
			case NPC_ROTFACE:
				if(isHeroic())	AutoFreeze(pCreature);
				m_uiRotfaceGUID = pCreature->GetGUID();
				if(!pCreature->isAlive())
					SetData(TYPE_ROTFACE,DONE);
				break;
			case NPC_PUTRICIDE:
				m_uiPutricideGUID = pCreature->GetGUID();
				if(!pCreature->isAlive())
					SetData(TYPE_PUTRICIDE,DONE);
				AutoFreeze(pCreature);
				if(isHeroic())	AutoFreeze(pCreature);
				break;
			case NPC_PRINCE_KELESETH:
				m_uiPrinceKelesethGUID = pCreature->GetGUID();
				if(isHeroic())	AutoFreeze(pCreature);
				break;
			case NPC_PRINCE_VALANAR:
				m_uiPrinceValanarGUID = pCreature->GetGUID();
				if(isHeroic())	AutoFreeze(pCreature);
				if(!pCreature->isAlive())
					SetData(TYPE_PRINCE_COUNCIL,DONE);
				break;
			case NPC_PRINCE_TALDARAM:
				m_uiPrinceTaldaramGUID = pCreature->GetGUID();
				if(isHeroic())	AutoFreeze(pCreature);
				break;
			case NPC_LANATHEL:
				m_uiLanathelGUID = pCreature->GetGUID();
				if(isHeroic())	AutoFreeze(pCreature);
				if(!pCreature->isAlive())
					SetData(TYPE_LANATHEL,DONE);
				break;
			case NPC_DREAMWALKER:
				m_uiDreamWalkerGUID = pCreature->GetGUID();
				if(isHeroic())	AutoFreeze(pCreature);
				if(GetData(TYPE_DREAMWALKER) == DONE)
					pCreature->ForcedDespawn(100);
				break;
			case NPC_SINDRAGOSA:
				m_uiSindragosaGUID = pCreature->GetGUID();
				if(isHeroic())	AutoFreeze(pCreature);
				if(!pCreature->isAlive())
					SetData(TYPE_SINDRAGOSA,DONE);
				break;
			case NPC_LICHKING:
				m_uiLichKingGUID = pCreature->GetGUID();
				if(isHeroic())	AutoFreeze(pCreature);
				break;
			case 37006:
				rotfacePoolsGUIDs.push_back(pCreature->GetGUID());
				break;
			case 38422:
			case 38458:
			case 38454:
			case 38369:
				vortexGUIDs.push_back(pCreature->GetGUID());
				break;
			case 38186:
			case 38068:
			case 37934:
			case 37907:
			case 37886:
			case 37863:
			case 36791:
				dreamwalkerAddsGUIDs.push_back(pCreature->GetGUID());
				break;
			case 37868:
				if(pCreature->GetDBTableGUIDLow() == 0)
					dreamwalkerAddsGUIDs.push_back(pCreature->GetGUID());
				else
				{
					if(GetData(TYPE_DREAMWALKER) == DONE)
						pCreature->ForcedDespawn(100);
					dreamwalkerEventAddGUIDs.push_back(pCreature->GetGUID());
				}
				break;
			case NPC_DREAMWALKER_IMAGE:
				pCreature->SetPhaseMask(16,true);
				m_uiDreamwalkerImageGUID = pCreature->GetGUID();
				break;
			case NPC_TIRION_FORDRING:
				m_uiFordringLKEventGUID = pCreature->GetGUID();
				break;

        }
    }

    void OnObjectCreate(GameObject* pGo)
    {
        switch(pGo->GetEntry())
        {
            case GO_MARROWGAR_ICE_1:
                m_uiMarrowgarIce1GUID = pGo->GetGUID();
                if (m_auiEncounter[0] == DONE)
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
            case GO_MARROWGAR_ICE_2:
                m_uiMarrowgarIce2GUID = pGo->GetGUID();
                if (m_auiEncounter[0] == DONE)
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
            case GO_DEATHWHISPER_GATE:
                m_uiDeathwhisperGateGUID = pGo->GetGUID();
                break;
            case GO_DEATHWHISPER_ELEVATOR:
                m_uiDeathwhisperElevatorGUID = pGo->GetGUID();
				if (m_auiEncounter[TYPE_DEATHWHISPER] == DONE)
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
            case GO_SAURFANG_DOOR:
                m_uiSaurfangDoorGUID = pGo->GetGUID();
                if(m_auiEncounter[TYPE_SAURFANG] == DONE)
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
			case GO_MARROWGAR_DOOR:
				m_uiMarrowgarDoorGUID = pGo->GetGUID();
				break;
			case GO_FESTERGUT_DOOR:
				m_uiFestergutDoorGUID = pGo->GetGUID();
				CloseDoor(m_uiFestergutDoorGUID);
				break;
			case GO_ROTFACE_DOOR:
				m_uiRotfaceDoorGUID = pGo->GetGUID();
				CloseDoor(m_uiRotfaceDoorGUID);
				break;
			case GO_PUTRICIDE_DOOR:
				m_uiPutricideDoorGUID = pGo->GetGUID();
				OpenDoor(m_uiPutricideDoorGUID);
				break;
			case GO_BLOODWING_DOOR:
				m_uiBloodWingDoorGUID = pGo->GetGUID();
				if (m_auiEncounter[TYPE_PUTRICIDE] == DONE || m_auiEncounter[TYPE_ROTFACE] == DONE && m_auiEncounter[TYPE_FESTERGUT] == DONE)
					OpenDoor(m_uiBloodWingDoorGUID);
				break;
			case GO_PRINCECOUNCIL_DOOR:
				m_uiPrinceCouncilDoorGUID = pGo->GetGUID();
				OpenDoor(m_uiPrinceCouncilDoorGUID);
				break;
			case GO_LANATHEL_DOOR_1:
				m_uiLanathelDoorGUID_1 = pGo->GetGUID();
				if (m_auiEncounter[TYPE_PRINCE_COUNCIL] == DONE)
                    OpenDoor(m_uiLanathelDoorGUID_1);
				break;
			case GO_LANATHEL_DOOR_2:
				m_uiLanathelDoorGUID_2 = pGo->GetGUID();
				if (m_auiEncounter[TYPE_PRINCE_COUNCIL] == DONE)
                    OpenDoor(m_uiLanathelDoorGUID_2);
				break;
			case GO_FROSTWING_DOOR:
				m_uiFrostWingDoorGUID = pGo->GetGUID();
				if (m_auiEncounter[TYPE_LANATHEL] == DONE)
                    OpenDoor(m_uiFrostWingDoorGUID);
				break;
			case GO_DREAMWALKER_DOOR:
				m_uiDreamWalkerDoorGUID = pGo->GetGUID();
				if (m_auiEncounter[TYPE_LANATHEL] == DONE)
                    OpenDoor(m_uiDreamWalkerDoorGUID);
				break;
			case GO_DREAMWALKER_EXIT_DOOR:
				m_uiDreamWalkerExitDoorGUID = pGo->GetGUID();
				if(m_auiEncounter[TYPE_DREAMWALKER] == DONE)
                    OpenDoor(m_uiDreamWalkerExitDoorGUID);
				break;
			case GO_SINDRAGOSA_DOOR:
				m_uiSindragosaDoorGUID = pGo->GetGUID();
				if(m_auiEncounter[TYPE_DREAMWALKER] == DONE)
                    OpenDoor(m_uiSindragosaDoorGUID);
				break;
			case GO_PLAGUE_SIGIL:
				m_uiPlagueSigilGUID = pGo->GetGUID();
				if(m_auiEncounter[TYPE_PUTRICIDE] == DONE)
                    OpenDoor(m_uiPlagueSigilGUID);
				break;
			case GO_BLOOD_SIGIL:
				m_uiBloodSigilGUID = pGo->GetGUID();
				if(m_auiEncounter[TYPE_LANATHEL] == DONE)
                    OpenDoor(m_uiBloodSigilGUID);
				break;
			case GO_FROST_SIGIL:
				m_uiFrostSigilGUID = pGo->GetGUID();
				if(m_auiEncounter[TYPE_SINDRAGOSA] == DONE)
                    OpenDoor(m_uiFrostSigilGUID);
				break;
			case GO_PLAGUE_ORANGE_TUBE:
				m_uiOrangeTubeGUID = pGo->GetGUID();
				if(m_auiEncounter[TYPE_FESTERGUT] == DONE)
                    OpenDoor(m_uiOrangeTubeGUID);
				break;
			case GO_PLAGUE_ORANGE_DOOR:
				m_uiOrangeDoorGUID = pGo->GetGUID();
				if(m_auiEncounter[TYPE_FESTERGUT] == DONE)
                    OpenDoor(m_uiOrangeDoorGUID);
				break;
			case GO_SAURFANG_CACHE_10:
				m_uiSaurfangCacheGUID = pGo->GetGUID();
				break;
            case GO_SAURFANG_CACHE_25:
				m_uiSaurfangCacheGUID = pGo->GetGUID();
				break;
            case GO_SAURFANG_CACHE_10_H:
				m_uiSaurfangCacheGUID = pGo->GetGUID();
				break;
            case GO_SAURFANG_CACHE_25_H:
				m_uiSaurfangCacheGUID = pGo->GetGUID();
				break;
			case GO_DREAMWALKER_EVENT_DOOR_1:
				m_uiDreamwalkerEventDoorGUID[0] = pGo->GetGUID();
				break;
			case GO_DREAMWALKER_EVENT_DOOR_2:
				m_uiDreamwalkerEventDoorGUID[1] = pGo->GetGUID();
				break;
			case GO_DREAMWALKER_EVENT_DOOR_3:
				m_uiDreamwalkerEventDoorGUID[2] = pGo->GetGUID();
				break;
			case GO_DREAMWALKER_EVENT_DOOR_4:
				m_uiDreamwalkerEventDoorGUID[3] = pGo->GetGUID();
				break;
        }
    }

    bool IsEncounterInProgress() const
    {
        for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
            if (m_auiEncounter[i] == IN_PROGRESS)
                return true;

        return false;
    }

    void SetData(uint32 uiType, uint32 uiData)
    {
        switch(uiType)
        {
            case TYPE_MARROWGAR:
                m_auiEncounter[TYPE_MARROWGAR] = uiData;
                if (uiData == DONE)
                {
                    OpenDoor(m_uiMarrowgarIce1GUID);
                    OpenDoor(m_uiMarrowgarIce2GUID);
					CloseDoor(m_uiMarrowgarDoorGUID);
                }
				else if(uiData == IN_PROGRESS)
					OpenDoor(m_uiMarrowgarDoorGUID);
                break;
            case TYPE_DEATHWHISPER:
                m_auiEncounter[TYPE_DEATHWHISPER] = uiData;
				if(uiData == DONE)
				{
					if(GameObject* go = GetGoInMap(m_uiDeathwhisperElevatorGUID))
						go->SetGoState(GO_STATE_ACTIVE);
				}
                break;
			case TYPE_BATTLE_OF_CANNONS:
				m_auiEncounter[TYPE_BATTLE_OF_CANNONS] = uiData;
				break;
            case TYPE_SAURFANG:
                m_auiEncounter[TYPE_SAURFANG] = uiData;
                if (uiData == DONE)
				{
                    OpenDoor(m_uiSaurfangDoorGUID);
					if(GameObject* pChest = instance->GetGameObject(m_uiSaurfangCacheGUID))
						if (!pChest->isSpawned())
						{
							pChest->SetRespawnTime(7*DAY);
							pChest->UpdateObjectVisibility();
						}
				}
                break;
			case TYPE_FESTERGUT:
				m_auiEncounter[TYPE_FESTERGUT] = uiData;
				if(uiData == DONE)
				{
					CloseDoor(m_uiFestergutDoorGUID);
					OpenDoor(m_uiOrangeTubeGUID);
					OpenDoor(m_uiOrangeDoorGUID);
				}
				else if(uiData == IN_PROGRESS)
					OpenDoor(m_uiFestergutDoorGUID);

				if(m_auiEncounter[TYPE_ROTFACE] == DONE && m_auiEncounter[TYPE_FESTERGUT] == DONE)
				{
					OpenDoor(m_uiBloodWingDoorGUID);
					OpenDoor(m_uiPrinceCouncilDoorGUID);
				}
				break;
			case TYPE_ROTFACE:
				m_auiEncounter[TYPE_ROTFACE] = uiData;
				if(uiData == DONE)
				{
					CloseDoor(m_uiRotfaceDoorGUID);
				}
				else if(uiData == IN_PROGRESS)
					OpenDoor(m_uiRotfaceDoorGUID);
				else if(uiData == FAIL)
					DespawnCreatures(rotfacePoolsGUIDs);

				if(m_auiEncounter[TYPE_ROTFACE] == DONE && m_auiEncounter[TYPE_FESTERGUT] == DONE)
				{
					OpenDoor(m_uiBloodWingDoorGUID);
					OpenDoor(m_uiPrinceCouncilDoorGUID);
				}
				break;
			case TYPE_PUTRICIDE:
				m_auiEncounter[TYPE_PUTRICIDE] = uiData;
				if(uiData == DONE)
				{
					OpenDoor(m_uiPutricideDoorGUID);
					OpenDoor(m_uiBloodWingDoorGUID);
					OpenDoor(m_uiPrinceCouncilDoorGUID);
					OpenDoor(m_uiPlagueSigilGUID);
				}
				else if(uiData == IN_PROGRESS)
					CloseDoor(m_uiPutricideDoorGUID);
				break;
			case TYPE_PRINCE_COUNCIL:
				m_auiEncounter[TYPE_PRINCE_COUNCIL] = uiData;
				if(uiData == DONE)
				{
					OpenDoor(m_uiPrinceCouncilDoorGUID);
					OpenDoor(m_uiLanathelDoorGUID_1);
					OpenDoor(m_uiLanathelDoorGUID_2);
					OpenDoor(m_uiBloodWingDoorGUID);
				}
				else if(uiData == IN_PROGRESS)
					CloseDoor(m_uiPrinceCouncilDoorGUID);
				else if(uiData == FAIL)
				{
					DespawnCreatures(vortexGUIDs);
					OpenDoor(m_uiPrinceCouncilDoorGUID);
				}
				break;
			case TYPE_LANATHEL:
				m_auiEncounter[TYPE_LANATHEL] = uiData;
				if(uiData == FAIL || uiData == DONE)
				{
					OpenDoor(m_uiLanathelDoorGUID_1);
					OpenDoor(m_uiLanathelDoorGUID_2);
					OpenDoor(m_uiPrinceCouncilDoorGUID);
					OpenDoor(m_uiBloodWingDoorGUID);
					OpenDoor(m_uiSaurfangDoorGUID);
				}
				if(uiData == DONE)
				{
					OpenDoor(m_uiFrostWingDoorGUID);
					OpenDoor(m_uiDreamWalkerDoorGUID);
					OpenDoor(m_uiBloodSigilGUID);
				}
				else if(uiData == IN_PROGRESS)
				{
					CloseDoor(m_uiLanathelDoorGUID_1);
					CloseDoor(m_uiLanathelDoorGUID_2);
				}
				break;
			case TYPE_DREAMWALKER:
				m_auiEncounter[TYPE_DREAMWALKER] = uiData;
				if(uiData == DONE)
				{
					OpenDoor(m_uiDreamWalkerDoorGUID);
					OpenDoor(m_uiDreamWalkerExitDoorGUID);
					OpenDoor(m_uiSindragosaDoorGUID);
					DespawnCreatures(dreamwalkerAddsGUIDs);
				}
				else if(uiData == IN_PROGRESS)
				{
					CloseDoor(m_uiDreamWalkerDoorGUID);
					OpenDoor(m_uiDreamwalkerEventDoorGUID[0]);
					OpenDoor(m_uiDreamwalkerEventDoorGUID[3]);
					switch(instance->GetDifficulty())
					{
						case RAID_DIFFICULTY_25MAN_NORMAL:
						case RAID_DIFFICULTY_25MAN_HEROIC:
							OpenDoor(m_uiDreamwalkerEventDoorGUID[1]);
							OpenDoor(m_uiDreamwalkerEventDoorGUID[2]);
							break;
					}
				}
				else
				{
					DespawnCreatures(dreamwalkerAddsGUIDs);
					CloseDoor(m_uiDreamwalkerEventDoorGUID[0]);
					CloseDoor(m_uiDreamwalkerEventDoorGUID[1]);
					CloseDoor(m_uiDreamwalkerEventDoorGUID[2]);
					CloseDoor(m_uiDreamwalkerEventDoorGUID[3]);
					if(uiData == FAIL)
					{
						m_uiDreamwalkerGuardDone = 0;
						for(std::vector<uint64>::iterator itr = dreamwalkerEventAddGUIDs.begin(); itr != dreamwalkerEventAddGUIDs.end(); ++itr)
							if(Creature* add = GetCreatureInMap(*itr))
								add->Respawn();
					}
				}
				break;
			case TYPE_SINDRAGOSA:
				m_auiEncounter[TYPE_SINDRAGOSA] = uiData;
				if(uiData == DONE)
				{
					OpenDoor(m_uiSindragosaDoorGUID);
					OpenDoor(m_uiFrostSigilGUID);
				}
				else if(uiData == IN_PROGRESS)
					CloseDoor(m_uiSindragosaDoorGUID);
				else
					OpenDoor(m_uiSindragosaDoorGUID);
				break;
			case TYPE_LICHKING:
				m_auiEncounter[TYPE_LICHKING] = uiData;
				break;
			case DATA_DW_DOOR_1:
				if(uiData == 0)
					CloseDoor(m_uiDreamwalkerEventDoorGUID[0]);
				else
					OpenDoor(m_uiDreamwalkerEventDoorGUID[0]);
				break;
			case DATA_DW_DOOR_2:
				if(uiData == 0)
					CloseDoor(m_uiDreamwalkerEventDoorGUID[1]);
				else
					OpenDoor(m_uiDreamwalkerEventDoorGUID[1]);
				break;
			case DATA_DW_DOOR_3:
				if(uiData == 0)
					CloseDoor(m_uiDreamwalkerEventDoorGUID[2]);
				else
					OpenDoor(m_uiDreamwalkerEventDoorGUID[2]);
				break;
			case DATA_DW_DOOR_4:
				if(uiData == 0)
					CloseDoor(m_uiDreamwalkerEventDoorGUID[2]);
				else
					OpenDoor(m_uiDreamwalkerEventDoorGUID[2]);
				break;
			case DATA_DREAMWALKER_GUARD:
				m_uiDreamwalkerGuardDone++;
				break;
        }

        if (uiData == DONE)
        {
            OUT_SAVE_INST_DATA;

            std::ostringstream saveStream;
            saveStream << m_auiEncounter[TYPE_MARROWGAR] << " " << m_auiEncounter[TYPE_DEATHWHISPER] << " " << m_auiEncounter[TYPE_BATTLE_OF_CANNONS] << " " << m_auiEncounter[TYPE_SAURFANG];
			saveStream << m_auiEncounter[TYPE_FESTERGUT] << " " << m_auiEncounter[TYPE_ROTFACE] << " " << m_auiEncounter[TYPE_PUTRICIDE] << " " << m_auiEncounter[TYPE_PRINCE_COUNCIL];
			saveStream << m_auiEncounter[TYPE_LANATHEL] << " " << m_auiEncounter[TYPE_DREAMWALKER] << " " << m_auiEncounter[TYPE_SINDRAGOSA] << " " << m_auiEncounter[TYPE_LICHKING];

            strInstData = saveStream.str();

            SaveToDB();
            OUT_SAVE_INST_DATA_COMPLETE;
        }
    }

    const char* Save()
    {
        return strInstData.c_str();
    }

    void Load(const char* chrIn)
    {
        if (!chrIn)
        {
            OUT_LOAD_INST_DATA_FAIL;
            return;
        }

        OUT_LOAD_INST_DATA(chrIn);

        std::istringstream loadStream(chrIn);
        loadStream >> m_auiEncounter[TYPE_MARROWGAR] >> m_auiEncounter[TYPE_DEATHWHISPER] >> m_auiEncounter[TYPE_BATTLE_OF_CANNONS] >> m_auiEncounter[TYPE_SAURFANG];
		loadStream >> m_auiEncounter[TYPE_FESTERGUT] >> m_auiEncounter[TYPE_ROTFACE] >> m_auiEncounter[TYPE_PUTRICIDE] >> m_auiEncounter[TYPE_PRINCE_COUNCIL];
		loadStream >> m_auiEncounter[TYPE_LANATHEL] >> m_auiEncounter[TYPE_DREAMWALKER] >> m_auiEncounter[TYPE_SINDRAGOSA] >> m_auiEncounter[TYPE_LICHKING];

        for(uint8 i = 0; i < MAX_ENCOUNTER; ++i)
        {
            if (m_auiEncounter[i] == IN_PROGRESS)
                m_auiEncounter[i] = NOT_STARTED;
        }

        OUT_LOAD_INST_DATA_COMPLETE;
    }

    uint32 GetData(uint32 uiType)
    {
        switch(uiType)
        {
            case TYPE_MARROWGAR:
                return m_auiEncounter[TYPE_MARROWGAR];
            case TYPE_DEATHWHISPER:
                return m_auiEncounter[TYPE_DEATHWHISPER];
			case TYPE_BATTLE_OF_CANNONS:
                return m_auiEncounter[TYPE_BATTLE_OF_CANNONS];
            case TYPE_SAURFANG:
                return m_auiEncounter[TYPE_SAURFANG];
			case TYPE_FESTERGUT:
                return m_auiEncounter[TYPE_FESTERGUT];
			case TYPE_ROTFACE:
                return m_auiEncounter[TYPE_ROTFACE];
			case TYPE_PUTRICIDE:
                return m_auiEncounter[TYPE_PUTRICIDE];
			case TYPE_PRINCE_COUNCIL:
				return m_auiEncounter[TYPE_PRINCE_COUNCIL];
			case TYPE_LANATHEL:
				return m_auiEncounter[TYPE_LANATHEL];
			case TYPE_DREAMWALKER:
				return m_auiEncounter[TYPE_DREAMWALKER];
			case TYPE_SINDRAGOSA:
				return m_auiEncounter[TYPE_SINDRAGOSA];
			case TYPE_LICHKING:
                return m_auiEncounter[TYPE_LICHKING];
			case DATA_DREAMWALKER_GUARD:
				return m_uiDreamwalkerGuardDone;
			case DATA_NB_BOSS_DOWN:
				uint32 down = 0;
				if(m_auiEncounter[TYPE_MARROWGAR] == DONE)
					down+=1;
				if(m_auiEncounter[TYPE_DEATHWHISPER] == DONE)
					down+=2;
				if(m_auiEncounter[TYPE_BATTLE_OF_CANNONS] == DONE)
					down+=4;
				if(m_auiEncounter[TYPE_SAURFANG] == DONE)
					down+=8;
				if(m_auiEncounter[TYPE_FESTERGUT] == DONE)
					down+=16;
				if(m_auiEncounter[TYPE_ROTFACE] == DONE)
					down+=32;
				if(m_auiEncounter[TYPE_PUTRICIDE] == DONE)
					down+=64;
				if(m_auiEncounter[TYPE_PRINCE_COUNCIL] == DONE)
					down+=128;
				if(m_auiEncounter[TYPE_LANATHEL] == DONE)
					down+=256;
				if(m_auiEncounter[TYPE_DREAMWALKER] == DONE)
					down+=512;
				if(m_auiEncounter[TYPE_SINDRAGOSA] == DONE)
					down+=1024;
				if(m_auiEncounter[TYPE_LICHKING] == DONE)
					down+=2048;
				return down;
        }
        return 0;
    }

    uint64 GetData64(uint32 uiData)
    {
        switch(uiData)
        {
            case TYPE_MARROWGAR:
                return m_uiMarrowgarGUID;
            case TYPE_DEATHWHISPER:
                return m_uiDeathwhisperGUID;
            case TYPE_SAURFANG:
                return m_uiSaurfangGUID;
			case GO_MARROWGAR_DOOR:
				return m_uiMarrowgarDoorGUID;
			case TYPE_FESTERGUT:
				return m_uiFestergutGUID;
			case TYPE_ROTFACE:
				return m_uiRotfaceGUID;
			case TYPE_PUTRICIDE:
				return m_uiPutricideGUID;
			case DATA_PRINCE_VALANAR:
				return m_uiPrinceValanarGUID;
			case DATA_PRINCE_KELESETH:
				return m_uiPrinceKelesethGUID;
			case DATA_PRINCE_TALDARAM:
				return m_uiPrinceTaldaramGUID;
			case TYPE_LANATHEL:
				return m_uiLanathelGUID;
			case TYPE_DREAMWALKER:
				return m_uiDreamWalkerGUID;
			case TYPE_SINDRAGOSA:
				return m_uiSindragosaGUID;
			case TYPE_LICHKING:
				return m_uiLichKingGUID;
			case DATA_FORDRING:
				return m_uiFordringLKEventGUID;
        }
        return 0;
    }

	bool CheckPlayersInMap()
	{
		bool found = false;
		Map::PlayerList const& lPlayers = instance->GetPlayers();

		if (!lPlayers.isEmpty())
			for(Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
				if (Player* pPlayer = itr->getSource())
				{
					if(!pPlayer->isAlive())
						pPlayer->RemoveAurasDueToSpell(69065);

					if(pPlayer->isAlive() && !pPlayer->isGameMaster())
						found = true;
					
					if(GetData(TYPE_SAURFANG) != IN_PROGRESS)
						pPlayer->RemoveAurasDueToSpell(72293);

					if(Creature* Dreamwalker = GetCreatureInMap(m_uiDreamWalkerGUID))
					{
						if(!pPlayer->isGameMaster())
						{
							if(pPlayer->GetTeam() == ALLIANCE)
								Dreamwalker->setFaction(1802);
							else
								Dreamwalker->setFaction(1801);
						}
					}
					if(Creature* Dreamwalker = GetCreatureInMap(m_uiDreamwalkerImageGUID))
					{
						if(!pPlayer->isGameMaster())
						{
							if(pPlayer->GetTeam() == ALLIANCE)
								Dreamwalker->setFaction(1802);
							else
								Dreamwalker->setFaction(1801);
						}
					}
				}
		return found;
	}

	void Update(uint32 diff)
	{
		if(checkPlayer_Timer <= diff)
		{
			if(!CheckPlayersInMap())
			{
				CloseDoor(m_uiMarrowgarDoorGUID);
				CloseDoor(m_uiFestergutDoorGUID);
				CloseDoor(m_uiRotfaceDoorGUID);
				OpenDoor(m_uiPutricideDoorGUID);
				OpenDoor(m_uiPrinceCouncilDoorGUID);
				if(GetData(TYPE_LANATHEL) == DONE)
					OpenDoor(m_uiDreamWalkerDoorGUID);
				if(GetData(TYPE_DREAMWALKER) == DONE)
					OpenDoor(m_uiSindragosaDoorGUID);

				CloseDoor(m_uiDreamwalkerEventDoorGUID[0]);
				CloseDoor(m_uiDreamwalkerEventDoorGUID[1]);
				CloseDoor(m_uiDreamwalkerEventDoorGUID[2]);
				CloseDoor(m_uiDreamwalkerEventDoorGUID[3]);

				if(GetData(TYPE_DREAMWALKER) == IN_PROGRESS)
					SetData(TYPE_DREAMWALKER,FAIL);

				if(Creature* Dreamwalker = GetCreatureInMap(m_uiDreamwalkerImageGUID))
				{
					Dreamwalker->ForcedDespawn(500);
					m_uiDreamwalkerImageGUID = 0;
				}
			}

			if (GetData(TYPE_SAURFANG) == DONE)
	            OpenDoor(m_uiSaurfangDoorGUID);

			if(GetData(TYPE_PRINCE_COUNCIL) == DONE)
			{
				OpenDoor(m_uiLanathelDoorGUID_1);
				OpenDoor(m_uiLanathelDoorGUID_2);
			}

			if(GetData(TYPE_LANATHEL) == DONE)
				OpenDoor(m_uiFrostWingDoorGUID);

			if(GetData(TYPE_ROTFACE) == DONE && GetData(TYPE_FESTERGUT) == DONE || GetData(TYPE_PUTRICIDE) == DONE)
			{
				OpenDoor(m_uiBloodWingDoorGUID);
				OpenDoor(m_uiFrostWingDoorGUID);
			}

			checkPlayer_Timer = 500;
		}
		else
			checkPlayer_Timer -= diff;
	}
};

InstanceData* GetInstanceData_instance_icecrown_citadel(Map* pMap)
{
    return new instance_icecrown_citadel(pMap);
}

void AddSC_instance_icecrown_citadel()
{
    Script* pNewScript;
    pNewScript = new Script;
    pNewScript->Name = "instance_icecrown_citadel";
    pNewScript->GetInstanceData = &GetInstanceData_instance_icecrown_citadel;
    pNewScript->RegisterSelf();
}
