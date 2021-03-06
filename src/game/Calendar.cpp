/*
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
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

#include <Policies/SingletonImp.h>
#include "InstanceSaveMgr.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Guild.h"
#include "Opcodes.h"
#include "Calendar.h"
#include "World.h"

INSTANTIATE_SINGLETON_1( CalendarMgr );

void CalendarMgr::Send(Player* plr)
{
	time_t cur_time = time(NULL);

	cEventSet cPlayerEventSet = plr->GetCalendarEvents();
	cEventSet cGuildEventSet;
    if(Guild *guild = plr->getGuild())
		cGuildEventSet = guild->GetCalendarEvents();
	else
		cGuildEventSet.clear();


    WorldPacket data(SMSG_CALENDAR_SEND_CALENDAR,600);

	uint32 invite_count = 0;
	uint32 holiday_count = 0;

    data << (uint32) invite_count;                           //invite node count
    for (int i = 0; i < invite_count; i++)
    {
		uint64 inviteId=0,eventId=0;
		uint8 unk1=0,unk2=0,unk3=0;
		uint64 creatorGuid=0;
		data << inviteId << eventId << unk1 << unk2 << unk3 << creatorGuid;		
	}

	data << uint32(cPlayerEventSet.size() + cGuildEventSet.size());                            //event count

	for (cEventSet::iterator itr = cPlayerEventSet.begin(); itr != cPlayerEventSet.end(); ++itr)
    {
		CalendarEvent* cEvent = sCalendarMgr.getEventById(*itr);
		if(!cEvent)
			continue;
		data << uint64(cEvent->getId());
		data << std::string(cEvent->getTitle());
		data << uint32(cEvent->getType());
		data << uint32(cEvent->getDate());
		data << uint32(EVENT_GUILD); // must be this to work
		data << int32(cEvent->getPveType());
		data.appendPackGUID(cEvent->getCreator());
	}

	for (cEventSet::iterator itr = cGuildEventSet.begin(); itr != cGuildEventSet.end(); ++itr)
    {
		CalendarEvent* cEvent = sCalendarMgr.getEventById(*itr);
		if(!cEvent)
			continue;
		data << uint64(cEvent->getId());
		data << std::string(cEvent->getTitle());
		data << uint32(cEvent->getType());
		data << uint32(cEvent->getDate());
		data << uint32(EVENT_GUILD); // must be this to work
		data << int32(cEvent->getPveType());
		data.appendPackGUID(cEvent->getCreator());
	}

    data << (uint32) 0;                                     //wtf??
    data << (uint32) secsToTimeBitFields(cur_time);         // current time
	
    uint32 counter = 0;
    size_t p_counter = data.wpos();
    data << uint32(counter);                                // instance save count

    for(int i = 0; i < MAX_DIFFICULTY; ++i)
    {
        for (Player::BoundInstancesMap::const_iterator itr = plr->m_boundInstances[i].begin(); itr != plr->m_boundInstances[i].end(); ++itr)
        {
            if(itr->second.perm)
            {
                InstanceSave *save = itr->second.save;
                data << uint32(save->GetMapId());
                data << uint32(save->GetDifficulty());
                data << uint32(save->GetResetTime() - cur_time);
                data << uint64(save->GetInstanceId());      // instance save id as unique instance copy id
                ++counter;
            }
        }
    }
    data.put<uint32>(p_counter,counter);

    data << uint32(1135717200 + sWorld.getConfig(CONFIG_INSTANCE_RESET_TIME_HOUR) * HOUR);
    counter = 0;
    p_counter = data.wpos();
    data << uint32(counter); // Instance reset intervals
    for(MapDifficultyMap::const_iterator itr = sMapDifficultyMap.begin(); itr != sMapDifficultyMap.end(); ++itr)
    {
        uint32 map_diff_pair = itr->first;
        uint32 mapid = PAIR32_LOPART(map_diff_pair);
        Difficulty difficulty = Difficulty(PAIR32_HIPART(map_diff_pair));
        MapDifficulty const* mapDiff = &itr->second;

        if (!mapDiff->resetTime || difficulty != REGULAR_DIFFICULTY)
            continue;

        const MapEntry* map = sMapStore.LookupEntry(mapid);

		uint32 delay = sInstanceSaveMgr.GetMaxResetTimeFor(mapDiff);
		if (delay < DAY) // the reset_delay must be at least one day
			delay = DAY;
        data << uint32(mapid);
        data << uint32(delay);
        data << uint32(map->reset_time);
        ++counter;
    }
    data.put<uint32>(p_counter, counter);

    data << (uint32) holiday_count;                   // unk counter 5
    for (int i = 0; i < holiday_count; i++)
    {
		uint32 unk1=0,unk2=0,unk3=0,unk4=0,unk5=0;
		std::string holidayName ="";
		data << unk1 << unk2 << unk3 << unk4 << unk5;
		for (int j = 0; j < 26; j++)
			data << uint32(0);
		for (int j = 0; j < 10; j++)
			data << uint32(0);
		for (int j = 0; j < 10; j++)
			data << uint32(0);
			
		data << holidayName;
	}
    plr->GetSession()->SendPacket(&data);
}

void CalendarMgr::SendCalendarFlash(Player* plr)
{
	if(!plr || !plr->GetSession())
		return;

	WorldPacket data(SMSG_CALENDAR_SEND_NUM_PENDING, 4);
    data << uint32(1);
	plr->GetSession()->SendPacket(&data);
}

void CalendarMgr::LoadCalendarEvents()
{
	m_calendarEvents.clear();
	uint32 nb = 0;
	if(QueryResult* result = CharacterDatabase.Query("SELECT `id`,`title`,`desc`,`type`,`date`,`ptype`,`flags`,`creator`,`guild` FROM calendar_events"))
	{
		do
		{
			Field *fields = result->Fetch();
			uint64 eventId = fields[0].GetUInt64();
			std::string title = fields[1].GetCppString();
			std::string desc = fields[2].GetCppString();
			uint8 type = fields[3].GetUInt8();
			uint32 date = fields[4].GetUInt32();
			int16 ptype = fields[5].GetInt16();
			uint8 flags = fields[6].GetUInt8();
			uint64 creator = fields[7].GetUInt64();
			uint32 guildid = fields[8].GetUInt32();

			CalendarEvent* cEvent = new CalendarEvent(title,desc,EventType(type),PveType(ptype),date,CalendarEventFlags(flags),creator);
			cEvent->setId(eventId);
			if(guildid > 0)
			{
				if(Guild* guild = sObjectMgr.GetGuildById(guildid))
				{
					cEvent->setGuild(guildid);
					guild->RegisterCalendarEvent(eventId);
				}
				else
					RemoveCalendarEvent(eventId);
			}

			nb++;
		}
		while(result->NextRow());
	}

	sLog.outString("Loaded %u Calendar Events",nb);
}

CalendarEvent* CalendarMgr::CreateEvent(std::string title, std::string desc, EventType type, PveType ptype, uint32 date, CalendarEventFlags flags, uint64 guid)
{
	CalendarEvent* cEvent = new CalendarEvent(title,desc,type,ptype,date,flags,guid);
	uint32 eventId = sObjectMgr.GenerateCalendarEventId();
	cEvent->setId(eventId);
	m_calendarEvents[eventId] = cEvent;

	CharacterDatabase.escape_string(title);
	CharacterDatabase.escape_string(desc);
	CharacterDatabase.PExecute("INSERT INTO calendar_events(`id`,`title`,`desc`,`type`,`date`,`ptype`,`flags`,`creator`) VALUES "
		"('%u','%s','%s','%u','%i','%u','%u','" UI64FMTD "')", eventId, title.c_str(), desc.c_str(), type, date, int32(ptype), flags, guid);

	return cEvent;
}

void CalendarMgr::RemoveCalendarEvent(uint64 eventId)
{
	CalendarEvent* cEvent = sCalendarMgr.getEventById(eventId);
	if(!cEvent)
		return;
	CharacterDatabase.PExecute("DELETE FROM calendar_events WHERE id = '%u'",eventId);
	CharacterDatabase.PExecute("DELETE FROM character_calendar_events WHERE eventid = '%u'",eventId);
	if(Guild* guild = sObjectMgr.GetGuildById(cEvent->getGuild()))
		guild->RemoveCalendarEvent(eventId);
	m_calendarEvents.erase(eventId);
	sWorld.RemoveCalendarEventFromActiveSessions(eventId);
	delete cEvent;
}

CalendarEvent* CalendarMgr::getEventById(uint64 id)
{
	cEventMap::iterator itr = m_calendarEvents.find(id);
	if(itr == m_calendarEvents.end())
		return NULL;

	return itr->second;
}

void CalendarMgr::SendEvent(CalendarEvent* cEvent, Player* plr, bool create)
{
	if(!cEvent || !plr || !plr->IsInWorld())
		return;
	uint32 invites = 1;

	WorldPacket data(SMSG_CALENDAR_SEND_EVENT);
	data << uint8(create ? 1 : 0);
	data.appendPackGUID(cEvent->getCreator());
	data << uint64(cEvent->getId());
	data << std::string(cEvent->getTitle());
	data << std::string(cEvent->getDescription());
	data << uint8(cEvent->getType());
	data << uint8(0);
	data << uint32(MAX_INVITES); // maxinvites
	data << uint32(cEvent->getPveType());
	data << uint32(1); // unk
	data << uint32(cEvent->getDate()); // unk
	data << uint32(/*cEvent->getFlags()*/0);
	// moding
	data << uint32(1);
	data << uint32(cEvent->getMemberList().size());
	for(CalEventMemberList::const_iterator itr = cEvent->getMemberList().begin(); itr != cEvent->getMemberList().end(); ++itr)
	{
		data.appendPackGUID(itr->first);
		data << uint8(plr->getLevel());
		data << uint8(itr->second.status);
		data << uint8(itr->second.status2);
		data << uint8(0x00); // unk
		data << uint8(0x2D) << uint8(urand(1,100)) << uint8(0x71) << uint8(itr->first) << uint32(0); // inviteId ?
		data << uint32(0); // unk
		data << std::string("");
	}
	plr->GetSession()->SendPacket(&data);
}

void CalendarEvent::AddMember(uint64 guid, State st, State2 st2)
{
	CalEventMemberList::iterator itr = m_memberList.find(guid);
	if(itr == m_memberList.end())
	{
		MemberStatus ms;
		ms.status = st;
		ms.status2 = st2;
		m_memberList[guid] = ms;
		CharacterDatabase.PExecute("INSERT INTO character_calendar_events(`guid`,`eventid`,`status`,`status2`) VALUES "
			"('" UI64FMTD "','%u','%u','%u')", guid, getId(), st, st2);
	}
}

void CalendarEvent::DelMember(uint64 guid)
{
	CalEventMemberList::iterator itr = m_memberList.find(guid);
	if(itr != m_memberList.end())
		m_memberList.erase(guid);
	CharacterDatabase.PExecute("DELETE FROM character_calendar_events where guid = '" UI64FMTD "' AND eventid = '%u'",guid,getId());
}

void CalendarEvent::UpdateStatus(uint64 guid, State st, State2 st2)
{
	CalEventMemberList::iterator itr = m_memberList.find(guid);
	if(itr != m_memberList.end())
	{
		itr->second.status = st;
		itr->second.status2 = st2;
	}
}