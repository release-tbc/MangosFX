/* Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
 *
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

#include "Common.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Vehicle.h"
#include "Unit.h"
#include "Util.h"
#include "WorldPacket.h"
#include "TemporarySummon.h"
#include "SpellMgr.h"

#include "CreatureAI.h"
#include "ZoneScript.h"

Vehicle::Vehicle(Unit *unit, VehicleEntry const *vehInfo) : Creature(CREATURE_SUBTYPE_VEHICLE), m_vehicleId(0), 
me(unit), m_vehicleInfo(vehInfo),m_vRegenTimer(4000)
{
	m_updateFlag = UPDATEFLAG_NONE;
    m_updateFlag = (UPDATEFLAG_LIVING | UPDATEFLAG_HAS_POSITION | UPDATEFLAG_VEHICLE);
	m_duration = DAY * IN_MILLISECONDS * 7;
    InitSeats();
}

Vehicle::~Vehicle()
{
}

void Vehicle::InitSeats()
{
	m_Seats.clear();
	m_maxSeatsNum = 0;
	for (uint32 i = 0; i < MAX_SEAT; ++i)
    {
        if(uint32 seatId = m_vehicleInfo->m_seatID[i])
            if(VehicleSeatEntry const *veSeat = sVehicleSeatStore.LookupEntry(seatId))
            {
				VehicleSeat vSeat(veSeat);
				vSeat.passenger = NULL;
				vSeat.flags = SEAT_FREE;
                m_Seats.insert(std::make_pair(i, vSeat));
                if(veSeat->IsUsable())
				{
					++m_maxSeatsNum;
				}
            }
    }
}
void Vehicle::Install()
{
    if(Creature *cre = dynamic_cast<Creature*>(me))
    {
        if(m_vehicleInfo->m_powerType == POWER_STEAM)
        {
            me->setPowerType(POWER_ENERGY);
            me->SetMaxPower(POWER_ENERGY, 100);
        }
        else if(m_vehicleInfo->m_powerType == POWER_PYRITE)
        {
            me->setPowerType(POWER_ENERGY);
            me->SetMaxPower(POWER_ENERGY, 50);
        }
        else
        {
            for (uint32 i = 0; i < 6; ++i)
            {
                if(!cre->m_spells[i])
                    continue;

                SpellEntry const *spellInfo = sSpellStore.LookupEntry(cre->m_spells[i]);
                if(!spellInfo)
                    continue;

                if(spellInfo->powerType == POWER_MANA)
                    break;

                if(spellInfo->powerType == POWER_ENERGY)
                {
                    me->setPowerType(POWER_ENERGY);
                    me->SetMaxPower(POWER_ENERGY, 100);
                    break;
                }
            }
        }
    }

    Reset();
}

void Vehicle::InstallAllAccessories()
{
    switch(me->GetEntry())
    {
        case 27850:InstallAccessory(27905,1);break;
        case 28782:InstallAccessory(28768,0,false);break; // Acherus Deathcharger
        case 28312:
			me->setPowerType(POWER_ENERGY);
            me->SetMaxPower(POWER_ENERGY, 100);
            me->ModifyPower(POWER_ENERGY, 100);
			InstallAccessory(28319,7);
			break;
        case 32627:
			me->setPowerType(POWER_ENERGY);
            me->SetMaxPower(POWER_ENERGY, 100);
            me->ModifyPower(POWER_ENERGY, 100);
			InstallAccessory(32629,7);
			break;
        case 32930:
            InstallAccessory(32933,0);
            InstallAccessory(32934,1);
            break;
        case 33109:InstallAccessory(33167,1);break;
        case 33060:InstallAccessory(33067,7);break;
        case 33113: // Flame leviathan
            InstallAccessory(33114,2);
            InstallAccessory(33114,3);
            InstallAccessory(33139,7);
            break;
        /*case 33114:
            InstallAccessory(33143,2); // Overload Control Device
            InstallAccessory(33142,1); // Leviathan Defense Turret
            break;*/
        case 33214:InstallAccessory(33218,1,false);break; // Mechanolift 304-A
        case 35637:InstallAccessory(34705,0,false);break;
        case 35633:InstallAccessory(34702,0,false);break;
        case 35768:InstallAccessory(34701,0,false);break;
        case 34658:InstallAccessory(34657,0,false);break;
        case 35636:InstallAccessory(34703,0,false);break;
        case 35638:InstallAccessory(35572,0,false);break;
        case 35635:InstallAccessory(35569,0,false);break;
        case 35640:InstallAccessory(35571,0,false);break;
        case 35641:InstallAccessory(35570,0,false);break;
        case 35634:InstallAccessory(35617,0,false);break;
    	case 33298:InstallAccessory(35332,0);break; //Darnassian Nightsaber
        case 33416:InstallAccessory(35330,0);break; //Exodar Elekk
        case 33297:InstallAccessory(35328,0);break; //Stormwind Steed
        case 33414:InstallAccessory(35327,0);break; //Forsaken Warhorse
        case 33301:InstallAccessory(35331,0);break; //Gnomeregan Mechanostrider
        case 33408:InstallAccessory(35329,0);break; //Ironforge Ram
        case 33300:InstallAccessory(35325,0);break; //Thunder Bluff Kodo
        case 33409:InstallAccessory(35314,0);break; //Orgrimmar Wolf
        case 33418:InstallAccessory(35326,0);break; //Silvermoon Hawkstrider
        case 33299:InstallAccessory(35323,0);break; //Darkspear Raptor
        case 35491:InstallAccessory(35451,0,false);break; //Black Knight
		case 32633:InstallAccessory(24780,-1);break;
		case 32640:InstallAccessory(24780,-1);break;
		case 28319:
		case 32629:
			me->setPowerType(POWER_ENERGY);
            me->SetMaxPower(POWER_ENERGY, 100);
            me->ModifyPower(POWER_ENERGY, 100);
			break;
		//case 33293: InstallAccessory(33329,-1); break; // XT002 heart
		//case 36476:InstallAccessory(36477,-1);break; Ick & Krick desactivated
    }
}

void Vehicle::Uninstall()
{
    sLog.outDebug("Vehicle::Uninstall %u", me->GetEntry());
    for (SeatMap::iterator itr = m_Seats.begin(); itr != m_Seats.end(); ++itr)
        if(Unit *passenger = itr->second.passenger)
            if(passenger->HasUnitTypeMask(UNIT_MASK_ACCESSORY))
                ((TemporarySummon*)passenger)->UnSummon();
    RemoveAllPassengers();
}

void Vehicle::Update(uint32 diff)
{
	if(m_vRegenTimer <= diff)
	{
		if(me->getPowerType() == POWER_ENERGY && HasAutoRegen())
		{
			uint8 count = me->GetPower(POWER_ENERGY) + 20;
			me->ModifyPower(POWER_ENERGY, count > 100 ? 100 : count);
		}
		m_vRegenTimer = 2500;
	}
	else
		m_vRegenTimer -= diff;

	if(m_duration <= diff)
	{
		Dismiss();
		m_duration = DAY;
	}
	else
		m_duration -= diff;
}

void Vehicle::Die()
{
    sLog.outDebug("Vehicle::Die %u", me->GetEntry());
	for (SeatMap::iterator itr = m_Seats.begin(); itr != m_Seats.end(); ++itr) // Remove everything from a vehicle
        if(Unit *passenger = itr->second.passenger)
            if(passenger->HasUnitTypeMask(UNIT_MASK_ACCESSORY))
                ((TemporarySummon*)passenger)->UnSummon();

    RemoveAllPassengers();
}

void Vehicle::Reset()
{
    sLog.outDebug("Vehicle::Reset");

    if(true || me->GetEntry() == 32930)
    {
        if (me->GetTypeId() == TYPEID_PLAYER)
        {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_PLAYER_VEHICLE);
        }
        else
        {
            InstallAllAccessories();
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        }
    }
}

void Vehicle::RemoveAllPassengers()
{
    for (SeatMap::iterator itr = m_Seats.begin(); itr != m_Seats.end(); ++itr)
        if(Unit *passenger = itr->second.passenger)
        {
            if(passenger->IsVehicle())
                passenger->GetVehicleKit()->RemoveAllPassengers();
            if(passenger->GetVehicle() != this)
			{
                sLog.outError("Vehicle %u has invalid passenger %u.", me->GetEntry(), passenger->GetEntry());
				return;
			}
            passenger->ExitVehicle();
            if(itr->second.passenger)
            {
                sLog.outError("Vehicle %u cannot remove passenger %u. %u is still on it.", me->GetEntry(), passenger->GetEntry(), itr->second.passenger->GetEntry());
                itr->second.passenger = NULL;
            }
        }
	InitSeats();
}

bool Vehicle::HasEmptySeat(int32 seatId)
{
    SeatMap::const_iterator seat = m_Seats.find(seatId);
    if(seat == m_Seats.end()) return false;
    return !seat->second.passenger;
}

Unit *Vehicle::GetPassenger(int8 seatId) const
{
    SeatMap::const_iterator seat = m_Seats.find(seatId);
    if(seat == m_Seats.end()) return NULL;
    return seat->second.passenger;
}

int8 Vehicle::GetNextEmptySeat(int8 seatId, bool next) const
{
    SeatMap::const_iterator seat = m_Seats.find(seatId);
    if(seat == m_Seats.end()) return -1;
    while(seat->second.passenger || !seat->second.seatInfo->IsUsable())
    {
        if(next)
        {
            ++seat;
            if(seat == m_Seats.end())
                seat = m_Seats.begin();
        }
        else
        {
            if(seat == m_Seats.begin())
                seat = m_Seats.end();
            --seat;
        }
        if(seat->first == seatId)
            return -1; // no available seat
    }
    return seat->first;
}

void Vehicle::InstallAccessory(uint32 entry, int8 seatId, bool minion)
{
    if(Unit *passenger = GetPassenger(seatId))
    {
        // already installed
        if(passenger->GetEntry() == entry)
        {
/*            if(me->GetTypeId() == TYPEID_UNIT && ((Creature*)me)->IsInEvadeMode() && ((Creature*)passenger)->IsAIEnabled)
                ((Creature*)passenger)->AI()->EnterEvadeMode();*/
            return;
        }
        passenger->ExitVehicle(); // this should not happen
    }

    //TODO: accessory should be minion
    if(Creature *accessory = me->SummonCreature(entry, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000))
    {
        if(minion)
            accessory->AddUnitTypeMask(UNIT_MASK_ACCESSORY);
        accessory->EnterVehicle(this, seatId);
		accessory->setFaction(me->getFaction());

		if(accessory->isVehicle())
			accessory->BuildVehicleInfo(accessory);
    }
}

bool Vehicle::AddPassenger(Unit *unit, int8 seatId)
{
    SeatMap::iterator seat;
    seat = m_Seats.find(seatId);

    // this should never happen
    if(seat == m_Seats.end())
        return false;

    sLog.outDebug("Unit %s enter vehicle entry %u id %u dbguid %u seat %i, seek seat %i", unit->GetName(), me->GetEntry(), m_vehicleInfo->m_ID, me->GetGUIDLow(), (int32)seat->first,seatId);

	unit->SetVehicleGUID(me->GetGUID());

	if(unit->GetTypeId() == TYPEID_PLAYER)
		if (unit->isInCombat())
			return false;

	seat->second.passenger = unit;
	if(unit->GetTypeId() == TYPEID_UNIT && ((Creature*)unit)->IsVehicle())
    {
		if(unit->GetVehicleKit()->GetEmptySeatsCount(true) == 0)
			ChangeSeatFlag(seatId, SEAT_VEHICLE_FULL);
        else
			ChangeSeatFlag(seatId, SEAT_VEHICLE_FREE);
    }
    else
		ChangeSeatFlag(seatId, SEAT_FULL);

    SetVehicleId(m_vehicleInfo->m_ID);

/*
	if(unit->GetTypeId() == TYPEID_PLAYER)
    {
        WorldPacket data0(SMSG_FORCE_MOVE_ROOT, 10);
        data0.append(unit->GetPackGUID());
        data0 << (uint32)((seat->second.vs_flags & SF_CAN_CAST) ?2 : 0);
        unit->SendMessageToSet(&data0,true);
    }
*/
	unit->m_movementInfo.AddMovementFlag(MOVEFLAG_ROOT);
	unit->m_movementInfo.AddMovementFlag(MOVEFLAG_ONTRANSPORT);

	VehicleEntry const *ve = sVehicleStore.LookupEntry(unit->GetVehicle()->GetVehicleInfo()->m_ID);
    if(!ve)
        return false;

	VehicleSeatEntry const *veSeat = sVehicleSeatStore.LookupEntry(ve->m_seatID[seatId]);
    if(!veSeat)
        return false;
	unit->m_movementInfo.SetTransportData(me->GetGUID(),
		(veSeat->m_attachmentOffsetX),
		(veSeat->m_attachmentOffsetY),
		(veSeat->m_attachmentOffsetZ),
		veSeat->m_passengerYaw, 0/*v->GetCreationTime()*/, seatId, veSeat->m_ID,
		sObjectMgr.GetSeatFlags(veSeat->m_ID), GetVehicleFlags());

	unit->addUnitState(UNIT_STAT_ON_VEHICLE);

	if(me->IsInWorld())
	{

		WorldPacket data(SMSG_MONSTER_MOVE_TRANSPORT, 60);
		data.append(unit->GetPackGUID());
		data.append(me->GetPackGUID());
		data << uint8(seatId);
		data << uint8(0);
		data << me->GetPositionX();
		data << me->GetPositionY();
		data << me->GetPositionZ();
		data << uint32(getMSTime());
		data << uint8(4);
		data << float(0);
		data << uint32(SPLINEFLAG_UNKNOWN5);
		data << uint32(0);// move time
		data << uint32(1);
		data << unit->GetTransOffsetX();
		data << unit->GetTransOffsetY();
		data << unit->GetTransOffsetZ();
		unit->SendMessageToSet(&data, true);

	}

	if((seat->second.vs_flags & SF_MAIN_RIDER /*temp fix*/|| seat->first == 0) && (me->GetTypeId() == TYPEID_UNIT && !((Creature*)me)->isHostileVehicle()))
    {
        if(!(GetVehicleFlags() & VF_MOVEMENT))
		{
			me->GetMotionMaster()->Clear(false);
			me->GetMotionMaster()->MoveIdle();
			me->SetCharmerGUID(unit->GetGUID());
			unit->SetCharm(me);
			if(unit->GetTypeId() == TYPEID_PLAYER)
			{
				((Player*)unit)->SetClientControl(me, 1);
				((Player*)unit)->SetMover(me);
				((Player*)unit)->SetMoverInQueve(me);
				if(((Player*)unit)->GetGroup())
					((Player*)unit)->SetGroupUpdateFlag(GROUP_UPDATE_VEHICLE);
           
				((Player*)unit)->SetFarSightGUID(me->GetGUID());

				BuildVehicleActionBar((Player*)unit);

				if(me->GetTypeId() == TYPEID_UNIT && (((Creature*)me)->GetEntry() == 28781 || ((Creature*)me)->GetEntry() == 33060 || ((Creature*)me)->GetEntry() == 33109
					|| ((Creature*)me)->GetEntry() == 28781 || ((Creature*)me)->GetEntry() == 33062)) // all
				{
					((Player*)unit)->m_movementInfo.AddMovementFlag2(MOVEFLAG2_INTERP_TURNING);
					((Player*)unit)->m_movementInfo.AddMovementFlag2(MOVEFLAG2_INTERP_PITCHING);
				}

				if(me->GetTypeId() == TYPEID_UNIT && (((Creature*)me)->GetEntry() == 28781 || ((Creature*)me)->GetEntry() == 33060 || ((Creature*)me)->GetEntry() == 33109
					|| ((Creature*)me)->GetEntry() == 28781))
				{
					((Player*)unit)->m_movementInfo.AddMovementFlag2(MOVEFLAG2_NO_JUMPING);
				}
			}
			if((me->GetTypeId() == TYPEID_UNIT && ((Creature*)me)->isFlyingVehicle()) || me->HasAuraType(SPELL_AURA_FLY) || me->HasAuraType(SPELL_AURA_MOD_FLIGHT_SPEED_MOUNTED))
            {

                WorldPacket data3(SMSG_MOVE_SET_CAN_FLY, 12);
                data3.append(me->GetPackGUID());
                data3 << (uint32)(0);
                me->SendMessageToSet(&data3,false);
            }
		}
		/*SpellClickInfoMapBounds clickPair = sObjectMgr.GetSpellClickInfoMapBounds(me->GetEntry());
        for(SpellClickInfoMap::const_iterator itr = clickPair.first; itr != clickPair.second; ++itr)
        {
            if (unit->GetTypeId() == TYPEID_UNIT || 
				unit->GetTypeId() == TYPEID_PLAYER && itr->second.IsFitToRequirements((Player*)unit))
            {
                Unit *caster = (itr->second.castFlags & 0x1) ? unit : me;
                Unit *target = (itr->second.castFlags & 0x2) ? unit : me;
				if(caster && target)
					caster->CastSpell(target, itr->second.spellId, true);
            }
        }*/

	}

	if(unit->GetTypeId() == TYPEID_PLAYER) // not right
	{
		if (((Player*)unit)->isAFK())
			((Player*)unit)->ToggleAFK();		
	}

	if(!((Creature*)me)->isHostileVehicle())
	//if(!(GetVehicleFlags() & VF_FACTION))
		me->setFaction(((Player*)unit)->getFaction());

	if(!CanMoveVehicle())
    {
        WorldPacket data2(SMSG_FORCE_MOVE_ROOT, 10);
		data2.append(me->GetPackGUID());
        data2 << (uint32)(2);
        me->SendMessageToSet(&data2,false);
    }
	/*if(!((Creature*)me)->isHostileVehicle())
	//if(GetVehicleFlags() & VF_NON_SELECTABLE)
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);*/
	
	/*if(!((Creature*)me)->isHostileVehicle())
	//if(seat->second.vs_flags & SF_UNATTACKABLE)
        unit->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);*/

    if(me->IsInWorld())
    {
        if(me->GetTypeId() == TYPEID_UNIT)
        {
            /*if(((Creature*)me)->IsAIEnabled)
                ((Creature*)me)->AI()->PassengerBoarded(unit, seat->first, true);*/

            // update all passenger's positions
            //RelocatePassengers(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(),me->GetMap());
        }
    }
	EmptySeatsCountChanged();
    return true;
}

void Vehicle::BuildVehicleActionBar(Player *plr) const
{
	if(me->GetTypeId() != TYPEID_UNIT)
		return;

	// TODO: player spell if he isn't the master of the vehicle
    WorldPacket data(SMSG_PET_SPELLS, 8+2+4+4+4*10+1+1);
    data << uint64(me->GetGUID());
    data << uint16(0x00000000);                     // creature family, not used in vehicles
    data << uint32(0x00000000);                     // unk
    data << uint32(0x00000101);                     // react state

	for (uint32 i = 0; i < MAX_VEHICLE_SPELLS; ++i)
    {
		uint32 spellId = ((Creature*)me)->m_spells[i];
        if(!spellId)
            continue;

        SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
        if(!spellInfo)
            continue;

        if(IsPassiveSpell(spellId))
        {
            me->CastSpell(me, spellId, true);
            data << uint16(0) << uint8(0) << uint8(i+8);
        }
        else
            data << uint16(spellId) << uint8(0) << uint8(i+8);
    }
	for (uint32 i = MAX_VEHICLE_SPELLS-2; i < MAX_VEHICLE_SPELLS; ++i)
        data << uint16(0) << uint8(0) << uint8(i+8);

    data << uint8(0);                               //aditional spells in spellbook, not used in vehicles

    uint8 cooldownsCount = ((Creature*)me)->m_CreatureSpellCooldowns.size() + ((Creature*)me)->m_CreatureCategoryCooldowns.size();
    data << uint8(cooldownsCount);
    time_t curTime = time(NULL);

    for(CreatureSpellCooldowns::const_iterator itr = ((Creature*)me)->m_CreatureSpellCooldowns.begin(); itr != ((Creature*)me)->m_CreatureSpellCooldowns.end(); ++itr)
    {
        time_t cooldown = (itr->second > curTime) ? (itr->second - curTime) * IN_MILLISECONDS : 0;

        data << uint32(itr->first);                         // spellid
        data << uint16(0);                                  // spell category?
        data << uint32(cooldown);                           // cooldown
        data << uint32(0);                                  // category cooldown
    }

    for(CreatureSpellCooldowns::const_iterator itr = ((Creature*)me)->m_CreatureCategoryCooldowns.begin(); itr != ((Creature*)me)->m_CreatureCategoryCooldowns.end(); ++itr)
    {
        time_t cooldown = (itr->second > curTime) ? (itr->second - curTime) * IN_MILLISECONDS : 0;

        data << uint32(itr->first);                         // spellid
        data << uint16(0);                                  // spell category?
        data << uint32(0);                                  // cooldown
        data << uint32(cooldown);                           // category cooldown
    }

    plr->GetSession()->SendPacket(&data);

    data.Initialize(SMSG_PET_GUIDS, 12);
    data << uint32(1);                                        // count
    data << uint64(me->GetGUID());
    plr->GetSession()->SendPacket(&data);
}

void Vehicle::RemovePassenger(Unit *unit)
{
    if(unit->GetVehicle() != this)
        return;

    SeatMap::iterator seat;
    for (seat = m_Seats.begin(); seat != m_Seats.end(); ++seat)
        if(seat->second.passenger == unit)
            break;

    if(seat == m_Seats.end())
		return;

    sLog.outDebug("Unit %s exit vehicle entry %u id %u dbguid %u seat %d", unit->GetName(), me->GetEntry(), m_vehicleInfo->m_ID, me->GetGUIDLow(), (int32)seat->first);

	if((seat->second.flags & (SEAT_FULL | SEAT_VEHICLE_FREE | SEAT_VEHICLE_FULL)) && seat->second.passenger == unit)
    {
		unit->SetVehicleGUID(0);

        if(seat->second.vs_flags & SF_MAIN_RIDER || seat->first == 0)
        {
			me->RemoveSpellsCausingAura(SPELL_AURA_CONTROL_VEHICLE);

            if(unit->GetTypeId() == TYPEID_PLAYER)
            {
                ((Player*)unit)->SetMover(NULL);
				((Player*)unit)->SetClientControl(me, 0);
                ((Player*)unit)->SetClientControl(unit, 1);
                ((Player*)unit)->SetMoverInQueve(NULL);
				((Player*)unit)->RemovePetActionBar();

				((Player*)unit)->SetFarSightGUID(0);
				if(me->GetTypeId() == TYPEID_UNIT && (((Creature*)me)->GetEntry() == 28781 || ((Creature*)me)->GetEntry() == 33060 || ((Creature*)me)->GetEntry() == 33109
					|| ((Creature*)me)->GetEntry() == 28781 || ((Creature*)me)->GetEntry() == 33062))
				{
					((Player*)unit)->m_movementInfo.RemoveMovementFlag2(MOVEFLAG2_INTERP_TURNING);
					((Player*)unit)->m_movementInfo.RemoveMovementFlag2(MOVEFLAG2_INTERP_PITCHING);
				}

				if(me->GetTypeId() == TYPEID_UNIT && (((Creature*)me)->GetEntry() == 28781 || ((Creature*)me)->GetEntry() == 33060 || ((Creature*)me)->GetEntry() == 33109
					|| ((Creature*)me)->GetEntry() == 28781))
				{
					((Player*)unit)->m_movementInfo.RemoveMovementFlag2(MOVEFLAG2_NO_JUMPING);
				}	
            }
            unit->SetCharm(NULL);
            me->SetCharmerGUID(NULL);			
        }
		if(unit->GetTypeId() == TYPEID_UNIT)
			me->setFaction(((Creature*)me)->GetCreatureInfo()->faction_A);
        /*if(GetVehicleFlags() & VF_NON_SELECTABLE)
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        if(seat->second.vs_flags & SF_UNATTACKABLE)
            unit->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);*/
        // restore player control
        if(unit->GetTypeId() == TYPEID_PLAYER)
        {
			if(((Player*)unit)->GetGroup())
				((Player*)unit)->SetGroupUpdateFlag(GROUP_UPDATE_VEHICLE);

            if(seat->second.vs_flags & SF_CAN_CAST)
            {
                WorldPacket data0(SMSG_FORCE_MOVE_UNROOT, 10);
                data0.append(unit->GetPackGUID());
                data0 << (uint32)(2);                        // can rotate
                unit->SendMessageToSet(&data0,true);
            }
            else
            {
                WorldPacket data1(SMSG_FORCE_MOVE_UNROOT, 10);
				data1.append(unit->GetPackGUID());
                data1 << (uint32)(0);                        // cannot rotate
                unit->SendMessageToSet(&data1,true);
            }
        }
		
        seat->second.passenger = NULL;
		ChangeSeatFlag(seat->first, SEAT_FREE);
	
        EmptySeatsCountChanged();
    }
}

void Vehicle::RelocatePassengers(float x, float y, float z, float ang, Map* map)
{
    for (SeatMap::const_iterator itr = m_Seats.begin(); itr != m_Seats.end(); ++itr)
	{
		VehicleSeat seat = itr->second;
		if(seat.flags & SEAT_FULL)
        {
			if (Unit *passengers = seat.passenger)
			{
				float xx = x + passengers->m_movementInfo.GetTransportPos()->x;
				float yy = y + passengers->m_movementInfo.GetTransportPos()->y;
				float zz = z + passengers->m_movementInfo.GetTransportPos()->z;
				float oo = ang + passengers->m_movementInfo.GetTransportPos()->o;

				if(passengers->GetTypeId() == TYPEID_PLAYER)
					((Player*)passengers)->SetPosition(xx, yy, zz, oo);
				else
					map->CreatureRelocation((Creature*)passengers, xx, yy, zz, oo);
			}
		}
		else if(itr->second.flags & (SEAT_VEHICLE_FULL | SEAT_VEHICLE_FREE))
        {
            // passenger cant be NULL here
            Unit *passengers = itr->second.passenger;
            if(!passengers)
				return;

            float xx = x + passengers->m_movementInfo.GetTransportPos()->x;
			float yy = y + passengers->m_movementInfo.GetTransportPos()->y;
			float zz = z + passengers->m_movementInfo.GetTransportPos()->z;
			float oo = ang + passengers->m_movementInfo.GetTransportPos()->o;

            map->CreatureRelocation((Creature*)passengers, xx, yy, zz, oo);
			passengers->GetVehicleKit()->RelocatePassengers(x,y,z,ang,map);
        }
	}
}
void Vehicle::Dismiss()
{
    Uninstall();
    me->SendObjectDeSpawnAnim(me->GetGUID());
    me->CombatStop();
	if(me->GetTypeId() == TYPEID_PLAYER)
		((Creature*)me)->ForcedDespawn(2000);
}

int8 Vehicle::GetEmptySeatsCount(bool force)
{
    int8 count = 0;
    for(SeatMap::iterator itr = m_Seats.begin(); itr != m_Seats.end(); ++itr)
    {
        if(itr->second.flags & (SEAT_FREE | SEAT_VEHICLE_FREE))
        {
            if(!force && (itr->second.vs_flags & SF_UNACCESSIBLE))
                continue;

            count++;
        }
    }

    return count;
}

void Vehicle::EmptySeatsCountChanged()
{
    uint8 m_count = m_maxSeatsNum;
    uint8 p_count = GetEmptySeatsCount(false);
    uint8 u_count = GetEmptySeatsCount(true);

    // seats accesibles by players
    if(p_count > 0)
	{
		if(me->GetTypeId() == TYPEID_PLAYER)
			me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_PLAYER_VEHICLE);
		else
			me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
	}
    else
	{
		if(me->GetTypeId() == TYPEID_PLAYER)
			me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_PLAYER_VEHICLE);
		else
			me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
	}

    if(u_count == m_count)
    {
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
    }
    else
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

    if(uint64 vehicleGUID = GetVehicleGUID())
    {
        if(Unit *vehUnit = Unit::GetUnit(*me, vehicleGUID))
			if(Vehicle *vehicle = vehUnit->GetVehicleKit())
			{
				if(u_count > 0)
					vehicle->ChangeSeatFlag(m_movementInfo.GetTransportSeat(), SEAT_VEHICLE_FREE);
				else
					vehicle->ChangeSeatFlag(m_movementInfo.GetTransportSeat(), SEAT_VEHICLE_FULL);
			}
    }
}

void Vehicle::ChangeSeatFlag(uint8 seat, uint8 flag)
{
    SeatMap::iterator i_seat = m_Seats.find(seat);
    // this should never happen
    if(i_seat == m_Seats.end())
        return;

    if(i_seat->second.flags != flag)
    {
        i_seat->second.flags = flag;
    }
}

Vehicle* Vehicle::FindFreeSeat(int8 *seatid, bool force)
{
    SeatMap::const_iterator i_seat = m_Seats.find(*seatid);
    if(i_seat == m_Seats.end())
	{
        return GetFirstEmptySeat(seatid, force);
	}

    if((i_seat->second.flags & (SEAT_FULL | SEAT_VEHICLE_FULL)) || (!force && (i_seat->second.vs_flags & SF_UNACCESSIBLE)))
	{
        return GetNextEmptySeat(seatid, true, force);
	}
    if(i_seat->second.flags & SEAT_VEHICLE_FREE)
    {
        // this should never be NULL
        if(Vehicle *v = (Vehicle*)i_seat->second.passenger)
            return v->FindFreeSeat(seatid, force);
        return NULL;
    }
    return this;
}

Vehicle* Vehicle::GetNextEmptySeat(int8 *seatId, bool next, bool force)
{
    SeatMap::const_iterator i_seat = m_Seats.find(*seatId);
    if(i_seat == m_Seats.end()) return GetFirstEmptySeat(seatId, force);

    while((i_seat->second.flags & (SEAT_FULL | SEAT_VEHICLE_FULL)) || (!force && (i_seat->second.vs_flags & SF_UNACCESSIBLE)))
    {
        if(next)
        {
            ++i_seat;
            if(i_seat == m_Seats.end())
                i_seat = m_Seats.begin();
        }
        else
        {
            if(i_seat == m_Seats.begin())
                i_seat = m_Seats.end();
            --i_seat;
        }
        if(i_seat->first == *seatId)
            return NULL;
    }
    *seatId = i_seat->first;
    if(i_seat->second.flags & SEAT_VEHICLE_FREE)
    {
        if(Vehicle *v = (i_seat->second.passenger)->GetVehicleKit())
            return v->FindFreeSeat(seatId, force);
        return NULL;
    }

    return this;
}

Vehicle* Vehicle::GetFirstEmptySeat(int8 *seatId, bool force)
{
    for(SeatMap::iterator itr = m_Seats.begin(); itr != m_Seats.end(); ++itr)
    {
        if(itr->second.flags & SEAT_FREE)
        {
            if(!force && (itr->second.vs_flags & SF_UNACCESSIBLE))
                continue;

            *seatId = itr->first;
            return this;
        }
        else if(itr->second.flags & SEAT_VEHICLE_FREE)
        {
            *seatId = itr->first;
            if(Vehicle *v = (itr->second.passenger)->GetVehicleKit())
                return v->FindFreeSeat(seatId, force);
        }
    }

    return NULL;
}

bool Vehicle::HasAutoRegen()
{
	switch(me->GetEntry())
	{
		// Pyrite Vehicles
		case 33109:
			return false;
		default:
			return true;
	}
}

bool Vehicle::CanMoveVehicle()
{
	switch(me->GetEntry())
	{
		case 28833:
		case 27894:
		case 28366:
			return false;
		default:
			return true;
	}
}