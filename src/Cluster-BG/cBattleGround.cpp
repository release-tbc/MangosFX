#include <ObjectMgr.h>
#include <WorldPacket.h>
#include <BattleGround.h>
#include <GridNotifiers.h>
#include <Language.h>
#include "cBattleGround.h"
#include "cBattleGroundMgr.h"
#include "ClusterSpec.h"

namespace ClusterFX
{
    class BattleGroundChatBuilder
    {
        public:
            BattleGroundChatBuilder(ChatMsg msgtype, int32 textId, Player const* source, va_list* args = NULL)
                : i_msgtype(msgtype), i_textId(textId), i_source(source), i_args(args) {}
            void operator()(WorldPacket& data, int32 loc_idx)
            {
                char const* text = sObjectMgr.GetMangosString(i_textId,loc_idx);

                if (i_args)
                {
                    // we need copy va_list before use or original va_list will corrupted
                    va_list ap;
                    va_copy(ap,*i_args);

                    char str [2048];
                    vsnprintf(str,2048,text, ap );
                    va_end(ap);

                    do_helper(data,&str[0]);
                }
                else
                    do_helper(data,text);
            }
        private:
            void do_helper(WorldPacket& data, char const* text)
            {
                uint64 target_guid = i_source  ? i_source ->GetGUID() : 0;

                data << uint8(i_msgtype);
                data << uint32(LANG_UNIVERSAL);
                data << uint64(target_guid);                // there 0 for BG messages
                data << uint32(0);                          // can be chat msg group or something
                data << uint64(target_guid);
                data << uint32(strlen(text)+1);
                data << text;
                data << uint8(i_source ? i_source->chatTag() : uint8(0));
            }

            ChatMsg i_msgtype;
            int32 i_textId;
            Player const* i_source;
            va_list* i_args;
    };

    class BattleGroundYellBuilder
    {
        public:
            BattleGroundYellBuilder(uint32 language, int32 textId, Creature const* source, va_list* args = NULL)
                : i_language(language), i_textId(textId), i_source(source), i_args(args) {}
            void operator()(WorldPacket& data, int32 loc_idx)
            {
                char const* text = sObjectMgr.GetMangosString(i_textId,loc_idx);

                if(i_args)
                {
                    // we need copy va_list before use or original va_list will corrupted
                    va_list ap;
                    va_copy(ap,*i_args);

                    char str [2048];
                    vsnprintf(str,2048,text, ap );
                    va_end(ap);

                    do_helper(data,&str[0]);
                }
                else
                    do_helper(data,text);
            }
        private:
            void do_helper(WorldPacket& data, char const* text)
            {
                //copyied from BuildMonsterChat
                data << (uint8)CHAT_MSG_MONSTER_YELL;
                data << (uint32)i_language;
                data << (uint64)i_source->GetGUID();
                data << (uint32)0;                                     //2.1.0
                data << (uint32)(strlen(i_source->GetName())+1);
                data << i_source->GetName();
                data << (uint64)0;                            //Unit Target - isn't important for bgs
                data << (uint32)strlen(text)+1;
                data << text;
                data << (uint8)0;                                      // ChatTag - for bgs allways 0?
            }

            uint32 i_language;
            int32 i_textId;
            Creature const* i_source;
            va_list* i_args;
    };


    class BattleGround2ChatBuilder
    {
        public:
            BattleGround2ChatBuilder(ChatMsg msgtype, int32 textId, Player const* source, int32 arg1, int32 arg2)
                : i_msgtype(msgtype), i_textId(textId), i_source(source), i_arg1(arg1), i_arg2(arg2) {}
            void operator()(WorldPacket& data, int32 loc_idx)
            {
                char const* text = sObjectMgr.GetMangosString(i_textId,loc_idx);
                char const* arg1str = i_arg1 ? sObjectMgr.GetMangosString(i_arg1,loc_idx) : "";
                char const* arg2str = i_arg2 ? sObjectMgr.GetMangosString(i_arg2,loc_idx) : "";

                char str [2048];
                snprintf(str,2048,text, arg1str, arg2str );

                uint64 target_guid = i_source  ? i_source ->GetGUID() : 0;

                data << uint8(i_msgtype);
                data << uint32(LANG_UNIVERSAL);
                data << uint64(target_guid);                // there 0 for BG messages
                data << uint32(0);                          // can be chat msg group or something
                data << uint64(target_guid);
                data << uint32(strlen(str)+1);
                data << str;
                data << uint8(i_source ? i_source->chatTag() : uint8(0));
            }
        private:

            ChatMsg i_msgtype;
            int32 i_textId;
            Player const* i_source;
            int32 i_arg1;
            int32 i_arg2;
    };

    class BattleGround2YellBuilder
    {
        public:
            BattleGround2YellBuilder(uint32 language, int32 textId, Creature const* source, int32 arg1, int32 arg2)
                : i_language(language), i_textId(textId), i_source(source), i_arg1(arg1), i_arg2(arg2) {}
            void operator()(WorldPacket& data, int32 loc_idx)
            {
                char const* text = sObjectMgr.GetMangosString(i_textId,loc_idx);
                char const* arg1str = i_arg1 ? sObjectMgr.GetMangosString(i_arg1,loc_idx) : "";
                char const* arg2str = i_arg2 ? sObjectMgr.GetMangosString(i_arg2,loc_idx) : "";

                char str [2048];
                snprintf(str,2048,text, arg1str, arg2str );
                //copyied from BuildMonsterChat
                data << (uint8)CHAT_MSG_MONSTER_YELL;
                data << (uint32)i_language;
                data << (uint64)i_source->GetGUID();
                data << (uint32)0;                                     //2.1.0
                data << (uint32)(strlen(i_source->GetName())+1);
                data << i_source->GetName();
                data << (uint64)0;                            //Unit Target - isn't important for bgs
                data << (uint32)strlen(str)+1;
                data << str;
                data << (uint8)0;                                      // ChatTag - for bgs allways 0?
            }
        private:

            uint32 i_language;
            int32 i_textId;
            Creature const* i_source;
            int32 i_arg1;
            int32 i_arg2;
    };
}                                                           // namespace ClusterFX

void cBattleGround::SendWarningToAll(int32 entry, ...)
{
    const char *format = sObjectMgr.GetMangosStringForDBCLocale(entry);
    va_list ap;
    char str [1024];
    va_start(ap, entry);
    vsnprintf(str,1024,format, ap);
    va_end(ap);
    std::string msg = (std::string)str;

    WorldPacket data(SMSG_MESSAGECHAT, 200);

    data << (uint8)CHAT_MSG_RAID_BOSS_EMOTE;
    data << (uint32)LANG_UNIVERSAL;
    data << (uint64)0;
    data << (uint32)0;                                     // 2.1.0
    data << (uint32)1;
    data << (uint8)0; 
    data << (uint64)0;
    data << (uint32)(strlen(msg.c_str())+1);
    data << msg.c_str();
    data << (uint8)0;
    /*for (BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        if (Player *plr = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(itr->first, 0, HIGHGUID_PLAYER)))
            if (plr->GetSession())
                plr->GetSession()->SendPacket(&data);*/
}

void cBattleGround::SendWarningToAll(std::string msg)
{
    WorldPacket data(SMSG_MESSAGECHAT, 200);

    data << (uint8)CHAT_MSG_RAID_BOSS_EMOTE;
    data << (uint32)LANG_UNIVERSAL;
    data << (uint64)0;
    data << (uint32)0;                                     // 2.1.0
    data << (uint32)1;
    data << (uint8)0; 
    data << (uint64)0;
    data << (uint32)(strlen(msg.c_str())+1);
    data << msg.c_str();
    data << (uint8)0;
    /*for (BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        if (Player *plr = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(itr->first, 0, HIGHGUID_PLAYER)))
            if (plr->GetSession())
                plr->GetSession()->SendPacket(&data);*/
}

cBattleGround::cBattleGround(): m_Id(0)
{
	m_TypeID            = BattleGroundTypeId(0);
    m_RandomTypeID      = BattleGroundTypeId(0);

	m_InstanceID        = 0;
	m_ClientInstanceID  = 0;

	m_InvitedAlliance   = 0;
    m_InvitedHorde      = 0;

	m_Events            = 0;

	m_BuffChange        = false;

	m_Status            = STATUS_NONE;
	m_BracketId         = BG_BRACKET_ID_FIRST;

	m_ArenaType         = 0;
	m_StartTime         = 0;
	m_EndTime           = 0;

	m_IsArena           = false;
	m_IsRated           = false;

	m_ArenaTeamIds[BG_TEAM_ALLIANCE]   = 0;
    m_ArenaTeamIds[BG_TEAM_HORDE]      = 0;

	m_ArenaTeamRatingChanges[BG_TEAM_ALLIANCE]   = 0;
    m_ArenaTeamRatingChanges[BG_TEAM_HORDE]      = 0;

	m_PlayersCount[BG_TEAM_ALLIANCE] = 0;
	m_PlayersCount[BG_TEAM_HORDE] = 0;

	m_TeamScores[BG_TEAM_ALLIANCE]      = 0;
    m_TeamScores[BG_TEAM_HORDE]         = 0;
	
	m_LevelMin          = 0;
    m_LevelMax          = 0;

	m_MaxPlayersPerTeam = 0;
    m_MaxPlayers        = 0;
    m_MinPlayersPerTeam = 0;
    m_MinPlayers        = 0;

	m_TeamStartLocX[BG_TEAM_ALLIANCE]   = 0;
    m_TeamStartLocX[BG_TEAM_HORDE]      = 0;

    m_TeamStartLocY[BG_TEAM_ALLIANCE]   = 0;
    m_TeamStartLocY[BG_TEAM_HORDE]      = 0;

    m_TeamStartLocZ[BG_TEAM_ALLIANCE]   = 0;
    m_TeamStartLocZ[BG_TEAM_HORDE]      = 0;

    m_TeamStartLocO[BG_TEAM_ALLIANCE]   = 0;
    m_TeamStartLocO[BG_TEAM_HORDE]      = 0;

	m_InBGFreeSlotQueue = false;
    m_SetDeleteThis     = false;
	m_RandomBG			= false;

	m_MapId             = 0;

	m_PrematureCountDown = false;
	m_TimerArenaDone = false;
    m_PrematureCountDown = 0;

	m_StartDelayTimes[BG_STARTING_EVENT_FIRST]  = BG_START_DELAY_2M;
    m_StartDelayTimes[BG_STARTING_EVENT_SECOND] = BG_START_DELAY_1M;
    m_StartDelayTimes[BG_STARTING_EVENT_THIRD]  = BG_START_DELAY_30S;
    m_StartDelayTimes[BG_STARTING_EVENT_FOURTH] = BG_START_DELAY_NONE;
    //we must set to some default existing values
    m_StartMessageIds[BG_STARTING_EVENT_FIRST]  = 0;
    m_StartMessageIds[BG_STARTING_EVENT_SECOND] = LANG_BG_WS_START_ONE_MINUTE;
    m_StartMessageIds[BG_STARTING_EVENT_THIRD]  = LANG_BG_WS_START_HALF_MINUTE;
    m_StartMessageIds[BG_STARTING_EVENT_FOURTH] = LANG_BG_WS_HAS_BEGUN;

	m_Players.clear();
}

cBattleGround::~cBattleGround()
{
	// remove objects and creatures
    // (this is done automatically in mapmanager update, when the instance is reset after the reset time)

    int size = m_BgObjects.size();
    for(int i = 0; i < size; ++i)
        DelObject(i);

    if (GetInstanceID())                                    // not spam by useless queries in case BG templates
    {
        // delete creature and go respawn times
        /*WorldDatabase.PExecute("DELETE FROM creature_respawn WHERE instance = '%u'",GetInstanceID());
        WorldDatabase.PExecute("DELETE FROM gameobject_respawn WHERE instance = '%u'",GetInstanceID());
        // delete instance from db
        CharacterDatabase.PExecute("DELETE FROM instance WHERE id = '%u'",GetInstanceID());*/
        // remove from battlegrounds
    }

    sClusterBGMgr.RemoveBattleGround(GetInstanceID(), GetTypeID());

    // unload map
    // map can be null at bg destruction
    /*if (m_Map)
        m_Map->SetUnload();*/

    // remove from bg free slot queue
    RemoveFromBGFreeSlotQueue();

    for(BattleGroundScoreMap::const_iterator itr = m_PlayerScores.begin(); itr != m_PlayerScores.end(); ++itr)
        delete itr->second;
}

void cBattleGround::Update(uint32 diff)
{
	if (!GetPlayersSize())
    {
        // BG is empty
        // if there are no players invited, delete BG
        // this will delete arena or bg object, where any player entered
        // [[   but if you use battleground object again (more battles possible to be played on 1 instance)
        //      then this condition should be removed and code:
        //      if (!GetInvitedCount(HORDE) && !GetInvitedCount(ALLIANCE))
        //          this->AddToFreeBGObjectsQueue(); // not yet implemented
        //      should be used instead of current
        // ]]
        // BattleGround Template instance cannot be updated, because it would be deleted
        if (!GetInvitedCount(HORDE) && !GetInvitedCount(ALLIANCE))
            m_SetDeleteThis = true;
        return;
    }

    // remove offline players from bg after 5 minutes
    if (!m_OfflineQueue.empty())
    {
        BattleGroundPlayerMap::iterator itr = m_Players.find(*(m_OfflineQueue.begin()));
        if (itr != m_Players.end())
        {
            if(itr->second.OfflineRemoveTime <= sClusterBG.GetGameTime())
            {
				RemovePlayerAtLeave(itr->first, true, true);// remove player from BG
                m_OfflineQueue.pop_front();                 // remove from offline queue
                //do not use itr for anything, because it is erased in RemovePlayerAtLeave()
            }
        }
    }

    /*********************************************************/
    /***           BATTLEGROUND BALLANCE SYSTEM            ***/
    /*********************************************************/

    // if less then minimum players are in on one side, then start premature finish timer
    if (GetStatus() == STATUS_IN_PROGRESS && !isArena() && sClusterBGMgr.GetPrematureFinishTime() && (GetPlayersCountByTeam(ALLIANCE) < GetMinPlayersPerTeam() || GetPlayersCountByTeam(HORDE) < GetMinPlayersPerTeam()))
    {
        if (!m_PrematureCountDown)
        {
            m_PrematureCountDown = true;
            m_PrematureCountDownTimer = sClusterBGMgr.GetPrematureFinishTime();
        }
        else if (m_PrematureCountDownTimer < diff)
        {
            // time's up!
            uint32 winner = 0;
            if (GetPlayersCountByTeam(ALLIANCE) >= GetMinPlayersPerTeam())
                winner = ALLIANCE;
            else if (GetPlayersCountByTeam(HORDE) >= GetMinPlayersPerTeam())
                winner = HORDE;

            EndBattleGround(winner);
            m_PrematureCountDown = false;
        }
        else
        {
            uint32 newtime = m_PrematureCountDownTimer - diff;
            // announce every minute
            if (newtime > (MINUTE * IN_MILLISECONDS))
            {
                if (newtime / (MINUTE * IN_MILLISECONDS) != m_PrematureCountDownTimer / (MINUTE * IN_MILLISECONDS))
                    PSendMessageToAll(LANG_BATTLEGROUND_PREMATURE_FINISH_WARNING, CHAT_MSG_SYSTEM, NULL, (uint32)(m_PrematureCountDownTimer / (MINUTE * IN_MILLISECONDS)));
            }
            else
            {
                //announce every 15 seconds
                if (newtime / (15 * IN_MILLISECONDS) != m_PrematureCountDownTimer / (15 * IN_MILLISECONDS))
                    PSendMessageToAll(LANG_BATTLEGROUND_PREMATURE_FINISH_WARNING_SECS, CHAT_MSG_SYSTEM, NULL, (uint32)(m_PrematureCountDownTimer / IN_MILLISECONDS));
            }
            m_PrematureCountDownTimer = newtime;
        }

    }
    else if (m_PrematureCountDown)
        m_PrematureCountDown = false;

    /*********************************************************/
    /***           ARENA BUFF OBJECT SPAWNING              ***/
    /*********************************************************/
    if (isArena() && !m_ArenaBuffSpawned)
    {
        // 60 seconds after start the buffobjects in arena should get spawned
        if (m_StartTime > uint32(m_StartDelayTimes[BG_STARTING_EVENT_FIRST] + ARENA_SPAWN_BUFF_OBJECTS))
        {
            SpawnEvent(ARENA_BUFF_EVENT, 0, true);
            m_ArenaBuffSpawned = true;
        }
    }

    /*********************************************************/
    /***           BATTLEGROUND STARTING SYSTEM            ***/
    /*********************************************************/

    if (GetStatus() == STATUS_WAIT_JOIN && GetPlayersSize())
    {
        ModifyStartDelayTime(diff);

        if (!(m_Events & BG_STARTING_EVENT_1))
        {
            m_Events |= BG_STARTING_EVENT_1;

            // setup here, only when at least one player has ported to the map
            if (!SetupBattleGround())
            {
                EndNow();
                return;
            }

            StartingEventCloseDoors();
            SetStartDelayTime(m_StartDelayTimes[BG_STARTING_EVENT_FIRST]);
            //first start warning - 2 or 1 minute, only if defined
            if (m_StartMessageIds[BG_STARTING_EVENT_FIRST])
                SendMessageToAll(m_StartMessageIds[BG_STARTING_EVENT_FIRST], CHAT_MSG_BG_SYSTEM_NEUTRAL);
        }
        // After 1 minute or 30 seconds, warning is signalled
        else if (GetStartDelayTime() <= m_StartDelayTimes[BG_STARTING_EVENT_SECOND] && !(m_Events & BG_STARTING_EVENT_2))
        {
            m_Events |= BG_STARTING_EVENT_2;
            SendMessageToAll(m_StartMessageIds[BG_STARTING_EVENT_SECOND], CHAT_MSG_BG_SYSTEM_NEUTRAL);
        }
        // After 30 or 15 seconds, warning is signalled
        else if (GetStartDelayTime() <= m_StartDelayTimes[BG_STARTING_EVENT_THIRD] && !(m_Events & BG_STARTING_EVENT_3))
        {
            m_Events |= BG_STARTING_EVENT_3;
            SendMessageToAll(m_StartMessageIds[BG_STARTING_EVENT_THIRD], CHAT_MSG_BG_SYSTEM_NEUTRAL);
        }
        // delay expired (atfer 2 or 1 minute)
        else if (GetStartDelayTime() <= 0 && !(m_Events & BG_STARTING_EVENT_4))
        {
            m_Events |= BG_STARTING_EVENT_4;

            StartingEventOpenDoors();

            SendMessageToAll(m_StartMessageIds[BG_STARTING_EVENT_FOURTH], CHAT_MSG_BG_SYSTEM_NEUTRAL);
            SetStatus(STATUS_IN_PROGRESS);
            SetStartDelayTime(m_StartDelayTimes[BG_STARTING_EVENT_FOURTH]);

            //remove preparation
            if (isArena())
            {
                //TODO : add arena sound PlaySoundToAll(SOUND_ARENA_START);
                for(BattleGroundPlayerMap::const_iterator itr = GetPlayers().begin(); itr != GetPlayers().end(); ++itr)
                    if(/*Player *plr = sObjectMgr.GetPlayer(*/itr->first/*)*/)
					{
						WorldPacket status;
						BattleGroundQueueTypeId bgQueueTypeId = sClusterBGMgr.BGQueueTypeId(GetTypeID(), GetArenaType());
						uint32 queueSlot = 0/*plr->GetBattleGroundQueueIndex(bgQueueTypeId)*/;
						sClusterBGMgr.BuildBattleGroundStatusPacket(&status, this, queueSlot, GetStatus(), 0, GetStartTime(), GetArenaType());
						/*plr->GetSession()->SendPacket(&status);
						plr->RemoveAurasDueToSpell(SPELL_ARENA_PREPARATION);*/
					} 

                CheckArenaWinConditions();
            }
            else
            {
                PlaySoundToAll(SOUND_BG_START);
               /*for(BattleGroundPlayerMap::const_iterator itr = GetPlayers().begin(); itr != GetPlayers().end(); ++itr)
                  if (Player* plr = sObjectMgr.GetPlayer(itr->first))
                      plr->RemoveAurasDueToSpell(SPELL_PREPARATION);*/
            }
        }
    }

    /*********************************************************/
    /***           BATTLEGROUND ENDING SYSTEM              ***/
    /*********************************************************/

    if (GetStatus() == STATUS_WAIT_LEAVE)
    {
        // remove all players from battleground after 2 minutes
        m_EndTime -= diff;
        if (m_EndTime <= 0)
        {
            m_EndTime = 0;
            BattleGroundPlayerMap::iterator itr, next;
            for(itr = m_Players.begin(); itr != m_Players.end(); itr = next)
            {
                next = itr;
                ++next;
                //itr is erased here!
                RemovePlayerAtLeave(itr->first, true, true);// remove player from BG
                // do not change any battleground's private variables
            }
        }
    }

	if(isArena())
	{
		if(m_StartTime > uint32(ARENA_TIME_LIMIT) && !m_TimerArenaDone)
		{
			uint32 winner;
			if(GetDamageDoneForTeam(ALLIANCE) > GetDamageDoneForTeam(HORDE))
				winner = ALLIANCE;
			else if (GetDamageDoneForTeam(HORDE) > GetDamageDoneForTeam(ALLIANCE))
				winner = HORDE;
			else
				winner = 0;

			m_TimerArenaDone = true;
			
			EndBattleGround(winner);
		}
	}

    //update start time
    m_StartTime += diff;
}

void cBattleGround::SetTeamStartLoc(uint32 TeamID, float X, float Y, float Z, float O)
{
    BattleGroundTeamId idx = GetTeamIndexByTeamId(TeamID);
    m_TeamStartLocX[idx] = X;
    m_TeamStartLocY[idx] = Y;
    m_TeamStartLocZ[idx] = Z;
    m_TeamStartLocO[idx] = O;
}

void cBattleGround::SendPacketToAll(WorldPacket *packet)
{
	for(BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.OfflineRemoveTime)
            continue;
        /*Player *plr = sObjectMgr.GetPlayer(/*itr->first*itr);
        if (plr)
            plr->GetSession()->SendPacket(packet);
        else
            sLog.outError("BattleGround:SendPacketToAll: Player (GUID: %u) not found!", GUID_LOPART(/*itr->first*itr));*/
    }

}

void cBattleGround::SendPacketToTeam(uint32 TeamID, WorldPacket *packet, Player *sender, bool self)
{
	for(BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.OfflineRemoveTime)
            continue;
        /*Player *plr = sObjectMgr.GetPlayer(/*itr->firs/*itr);
        if (!plr)
        {
            sLog.outError("BattleGround:SendPacketToTeam: Player (GUID: %u) not found!", GUID_LOPART(/*itr->first*itr));
            continue;
        }

        if (!self && sender == plr)
            continue;

		// todo: handle team
        uint32 team = GetPlayerTeam(*itr)/*itr->second.Team;
        if(!team) team = plr->GetTeam();

        if (team == TeamID)
            plr->GetSession()->SendPacket(packet);*/
    }
}

void cBattleGround::PlaySoundToAll(uint32 SoundID)
{
	WorldPacket data;
    sClusterBGMgr.BuildPlaySoundPacket(&data, SoundID);
    SendPacketToAll(&data);
}

void cBattleGround::PlaySoundToTeam(uint32 SoundID, uint32 TeamID)
{
	for(BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.OfflineRemoveTime)
            continue;
        /*Player *plr = sObjectMgr.GetPlayer(/*itr->first*itr);

        if (!plr)
        {
            sLog.outError("BattleGround:PlaySoundToTeam: Player (GUID: %u) not found!", GUID_LOPART(/*itr->first*itr));
            continue;
        }

		// TODO : utiliser la retransmission directe via le cluster
        uint32 team = GetPlayerTeam(*itr)/*itr->second.Team;
        if(!team) team = plr->GetTeam();

        if (team == TeamID)
        {
            sClusterBGMgr.BuildPlaySoundPacket(&data, SoundID);
            plr->GetSession()->SendPacket(&data);
        }*/
    }
}

void cBattleGround::CastSpellOnTeam(uint32 SpellID, uint32 TeamID)
{
	for(BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.OfflineRemoveTime)
            continue;
        /*Player *plr = sObjectMgr.GetPlayer(/*itr->first*itr);

        if (!plr)
        {
            sLog.outError("BattleGround:CastSpellOnTeam: Player (GUID: %u) not found!", GUID_LOPART(/*itr->first**itr));
            continue;
        }

        uint32 team = GetPlayerTeam(*itr)/*itr->second.Team;
        if(!team) team = plr->GetTeam();

        if (team == TeamID)
            plr->CastSpell(plr, SpellID, true);*/
    }
}

void cBattleGround::RewardHonorToTeam(uint32 Honor, uint32 TeamID)
{
	for(BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.OfflineRemoveTime)
            continue;
        /*Player *plr = sObjectMgr.GetPlayer(/*itr->first*itr);

        if (!plr)
        {
            sLog.outError("BattleGround:RewardHonorToTeam: Player (GUID: %u) not found!", GUID_LOPART(/*itr->first*itr));
            continue;
        }

        uint32 team = GetPlayerTeam(*itr)/*itr->second.Team;
        if(!team) team = plr->GetTeam();

        if (team == TeamID)
            UpdatePlayerScore(plr, SCORE_BONUS_HONOR, Honor);*/
    }
}

void cBattleGround::RewardHonorTeamDaily(uint32 WinningTeamID)
{
	for(BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
	{
		if (itr->second.OfflineRemoveTime)
			continue;
		
		/*Player *plr = sObjectMgr.GetPlayer(/*itr->first*itr);
		
		if (!plr)
			continue;
		
		uint32 team = GetPlayerTeam(*itr)/*itr->second.Team;
		if(!team) 
			team = plr->GetTeam();
		plr->RewardHonorEndBattlegroud(team == WinningTeamID);*/
	}
}

void cBattleGround::RewardReputationToTeam(uint32 faction_id, uint32 Reputation, uint32 TeamID)
{
	FactionEntry const* factionEntry = sFactionStore.LookupEntry(faction_id);

    if (!factionEntry)
        return;

    for(BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.OfflineRemoveTime)
            continue;
        /*Player *plr = sObjectMgr.GetPlayer(/*itr->first*itr);

        if (!plr)
        {
            sLog.outError("BattleGround:RewardReputationToTeam: Player (GUID: %u) not found!", GUID_LOPART(/*itr->first*itr));
            continue;
        }

        uint32 team = GetPlayerTeam(*itr)/*itr->second.Team;
        if(!team) team = plr->GetTeam();

        if (team == TeamID)
            plr->GetReputationMgr().ModifyReputation(factionEntry, Reputation);*/
    }
}

void cBattleGround::RewardXpToTeam(uint32 Xp, float percentOfLevel, uint32 TeamID)
{
	for(BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.OfflineRemoveTime)
            continue;
        /*Player *plr = sObjectMgr.GetPlayer(/*itr->first*itr);

        if (!plr)
        {
            sLog.outError("BattleGround:RewardXpToTeam: Player (GUID: %u) not found!", GUID_LOPART(/*itr->first*itr));
            continue;
        }

        uint32 team = GetPlayerTeam(*itr)/*itr->second.Team;
        if(!team) team = plr->GetTeam();

        if (team == TeamID)
        {
            uint32 gain = Xp;
            if(gain == 0 && percentOfLevel != 0)
            {
                percentOfLevel = percentOfLevel / 100;
                gain = uint32(float(plr->GetUInt32Value(PLAYER_NEXT_LEVEL_XP))*percentOfLevel);
            }
            plr->GiveXP(gain, NULL);
        }*/
    }
}

void cBattleGround::UpdateWorldState(uint32 Field, uint32 Value,Player *Source)
{
	WorldPacket data;
    sClusterBGMgr.BuildUpdateWorldStatePacket(&data, Field, Value);
	/*if(Source)
		Source->GetSession()->SendPacket(&data);
	else*/
		SendPacketToAll(&data);
}

void cBattleGround::EndBattleGround(uint32 winner)
{
	RemoveFromBGFreeSlotQueue();

    ArenaTeam * winner_arena_team = NULL;
    ArenaTeam * loser_arena_team = NULL;
    uint32 loser_rating = 0;
    uint32 winner_rating = 0;
    WorldPacket data;
    int32 winmsg_id = 0;

    if (winner == ALLIANCE)
    {
        winmsg_id = isBattleGround() ? LANG_BG_A_WINS : LANG_ARENA_GOLD_WINS;

        PlaySoundToAll(SOUND_ALLIANCE_WINS);                // alliance wins sound

        SetWinner(WINNER_ALLIANCE);
    }
    else if (winner == HORDE)
    {
        winmsg_id = isBattleGround() ? LANG_BG_H_WINS : LANG_ARENA_GREEN_WINS;

        PlaySoundToAll(SOUND_HORDE_WINS);                   // horde wins sound

        SetWinner(WINNER_HORDE);
    }
    else
    {
        SetWinner(3);
    }

    SetStatus(STATUS_WAIT_LEAVE);
    //we must set it this way, because end time is sent in packet!
    m_EndTime = TIME_TO_AUTOREMOVE;

    // arena rating calculation
    if (isArena() && isRated() && winner)
    {
        /*winner_arena_team = sObjectMgr.GetArenaTeamById(GetArenaTeamIdForTeam(winner));
        loser_arena_team = sObjectMgr.GetArenaTeamById(GetArenaTeamIdForTeam(GetOtherTeam(winner)));*/
        if (winner_arena_team && loser_arena_team)
        {
            /*loser_rating = loser_arena_team->GetStats().rating;
            winner_rating = winner_arena_team->GetStats().rating;
            int32 winner_change = winner_arena_team->WonAgainst(loser_rating);*/
			
			/*switch(winner_arena_team->GetType())
			{
				case 2:
					if(winner_rating + winner_change >= 1550)
						RewardAchievementToTeam(winner,399);
					if(winner_rating + winner_change >= 1750)
						RewardAchievementToTeam(winner,400);
					if(winner_rating + winner_change >= 2000)
						RewardAchievementToTeam(winner,401);
					if(winner_rating + winner_change >= 2200)
						RewardAchievementToTeam(winner,1159);
					break;
				case 3:
					if(winner_rating + winner_change >= 1550)
						RewardAchievementToTeam(winner,402);
					if(winner_rating + winner_change >= 1750)
						RewardAchievementToTeam(winner,403);
					if(winner_rating + winner_change >= 2000)
						RewardAchievementToTeam(winner,405);
					if(winner_rating + winner_change >= 2200)
						RewardAchievementToTeam(winner,1160);
					break;
				case 5:
					if(winner_rating + winner_change >= 1550)
						RewardAchievementToTeam(winner,406);
					if(winner_rating + winner_change >= 1750)
						RewardAchievementToTeam(winner,407);
					if(winner_rating + winner_change >= 2000)
						RewardAchievementToTeam(winner,404);
					if(winner_rating + winner_change >= 2200)
						RewardAchievementToTeam(winner,1161);
					break;
			}*/
            //int32 loser_change = loser_arena_team->LostAgainst(winner_rating);
            //sLog.outDebug("--- Winner rating: %u, Loser rating: %u, Winner change: %u, Losser change: %u ---", winner_rating, loser_rating, winner_change, loser_change);
            /*SetArenaTeamRatingChangeForTeam(winner, winner_change);
            SetArenaTeamRatingChangeForTeam(GetOtherTeam(winner), loser_change);*/
        }
        else
        {
            SetArenaTeamRatingChangeForTeam(ALLIANCE, 0);
            SetArenaTeamRatingChangeForTeam(HORDE, 0);
        }
    }

    for(BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
		uint32 team = itr->second.Team;

        if (itr->second.OfflineRemoveTime)
        {
            //if rated arena match - make member lost!
            if (isArena() && isRated() && winner_arena_team && loser_arena_team)
            {
                /*if (team == winner)
                    winner_arena_team->OfflineMemberLost(itr->first*itr, loser_rating);
                else
                    loser_arena_team->OfflineMemberLost(itr->first*itr, winner_rating);*/
            }
            continue;
        }
        /*
		Player *plr = sObjectMgr.GetPlayer(/*itr->first*itr);
        if (!plr)
        {
            sLog.outError("BattleGround:EndBattleGround Player (GUID: %u) not found!", GUID_LOPART(/*itr->first*itr));
            continue;
        }

        // should remove spirit of redemption
        if (plr->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
            plr->RemoveSpellsCausingAura(SPELL_AURA_MOD_SHAPESHIFT);

        if (!plr->isAlive())
        {
            plr->ResurrectPlayer(1.0f);
            plr->SpawnCorpseBones();
        }
        else
        {
            //needed cause else in av some creatures will kill the players at the end
            plr->CombatStop();
            plr->getHostileRefManager().deleteReferences();
        }*/

        // per player calculation
        if (isArena() && isRated() && winner_arena_team && loser_arena_team)
        {
            if (team == winner)
            {
                // update achievement BEFORE personal rating update
                /*ArenaTeamMember* member = winner_arena_team->GetMember(plr->GetGUID());
                if (member)
                    plr->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA, member->personal_rating);*/

                //winner_arena_team->MemberWon(plr,loser_rating);
            }
            else
            {
                //loser_arena_team->MemberLost(plr,winner_rating);

                // Arena lost => reset the win_rated_arena having the "no_loose" condition
                //plr->GetAchievementMgr().ResetAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA, ACHIEVEMENT_CRITERIA_CONDITION_NO_LOOSE);
            }
        }

        /*if (team == winner)
            RewardQuestComplete(plr);*/

        //plr->CombatStopWithPets(true);

        //BlockMovement(plr);

        /*sBattleGroundMgr.BuildPvpLogDataPacket(&data, this);
        plr->GetSession()->SendPacket(&data);

        BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(GetTypeID(), GetArenaType());
        sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, this, plr->GetBattleGroundQueueIndex(bgQueueTypeId), STATUS_IN_PROGRESS, TIME_TO_AUTOREMOVE, GetStartTime(), GetArenaType());
        plr->GetSession()->SendPacket(&data);
        plr->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_BATTLEGROUND, 1);*/
    }

    if (isArena() && isRated() && winner_arena_team && loser_arena_team)
    {
        // update arena points only after increasing the player's match count!
        //obsolete: winner_arena_team->UpdateArenaPointsHelper();
        //obsolete: loser_arena_team->UpdateArenaPointsHelper();
        // save the stat changes
        /*winner_arena_team->SaveToDB();
        loser_arena_team->SaveToDB();
        // send updated arena team stats to players
        // this way all arena team members will get notified, not only the ones who participated in this match
        winner_arena_team->NotifyStatsChanged();
        loser_arena_team->NotifyStatsChanged();*/
    }

    if (winmsg_id)
        SendMessageToAll(winmsg_id, CHAT_MSG_BG_SYSTEM_NEUTRAL);
}

uint32 cBattleGround::GetBonusHonorFromKill(uint32 kills) const
{
    //variable kills means how many honorable kills you scored (so we need kills * honor_for_one_kill)
    return ClusterFX::Honor::hk_honor_at_level(GetMaxLevel(), kills);
}

uint32 cBattleGround::GetBattlemasterEntry() const
{
	switch(GetTypeID(true))
    {
        case BATTLEGROUND_AV: return 15972;
        case BATTLEGROUND_WS: return 14623;
        case BATTLEGROUND_AB: return 14879;
        case BATTLEGROUND_EY: return 22516;
        case BATTLEGROUND_NA: return 20200;
        default:              return 0;
    }
}

void cBattleGround::RewardSpellCast(Player *plr, uint32 spell_id)
{
	// 'Inactive' this aura prevents the player from gaining honor points and battleground tokens
    /*if (plr->GetDummyAura(SPELL_AURA_PLAYER_INACTIVE))
        return;

    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spell_id);
    if(!spellInfo)
    {
        sLog.outError("Battleground reward casting spell %u not exist.",spell_id);
        return;
    }

    plr->CastSpell(plr, spellInfo, true);*/
}

void cBattleGround::RewardItem(Player *plr, uint32 item_id, uint32 count)
{
	// 'Inactive' this aura prevents the player from gaining honor points and battleground tokens
    /*if (plr->GetDummyAura(SPELL_AURA_PLAYER_INACTIVE))
        return;

    ItemPosCountVec dest;
    uint32 no_space_count = 0;
    uint8 msg = plr->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, item_id, count, &no_space_count );

    if( msg == EQUIP_ERR_ITEM_NOT_FOUND)
    {
        sLog.outErrorDb("Battleground reward item (Entry %u) not exist in `item_template`.",item_id);
        return;
    }

    if( msg != EQUIP_ERR_OK )                               // convert to possible store amount
        count -= no_space_count;

    if( count != 0 && !dest.empty())                        // can add some
        if (Item* item = plr->StoreNewItem( dest, item_id, true, 0))
            plr->SendNewItem(item,count,true,false);

    if (no_space_count > 0)
        SendRewardMarkByMail(plr,item_id,no_space_count);*/
}

void cBattleGround::RewardQuestComplete(Player *plr)
{
	uint32 quest;
    switch(GetTypeID(true))
    {
        case BATTLEGROUND_AV:
            quest = SPELL_AV_QUEST_REWARD;
            break;
        case BATTLEGROUND_WS:
            quest = SPELL_WS_QUEST_REWARD;
            break;
        case BATTLEGROUND_AB:
            quest = SPELL_AB_QUEST_REWARD;
            break;
        case BATTLEGROUND_EY:
            quest = SPELL_EY_QUEST_REWARD;
            break;
        default:
            return;
    }

    RewardSpellCast(plr, quest);
}

void cBattleGround::BlockMovement(Player *plr)
{
    //plr->SetClientControl(plr, 0);                          // movement disabled NOTE: the effect will be automatically removed by client when the player is teleported from the battleground, so no need to send with uint8(1) in RemovePlayerAtLeave()
}

void cBattleGround::RemovePlayerAtLeave(uint64 guid, bool Transport, bool SendPacket)
{
	uint32 team = GetPlayerTeam(guid);
	bool participant = IsPlayerInBattleGround(guid);

	BattleGroundPlayerMap::iterator itr = m_Players.find(guid);
    if (itr != m_Players.end())
    {
        UpdatePlayersCountByTeam(team, true);               // -1 player
        m_Players.erase(itr);
	}

	BattleGroundScoreMap::iterator itr2 = m_PlayerScores.find(guid);
    if (itr2 != m_PlayerScores.end())
    {
        delete itr2->second;                                // delete player's score
        m_PlayerScores.erase(itr2);
    }

    /*Player *plr = sObjectMgr.GetPlayer(guid);

    // should remove spirit of redemption
    if (plr && plr->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
        plr->RemoveSpellsCausingAura(SPELL_AURA_MOD_SHAPESHIFT);

    if(plr && !plr->isAlive())                              // resurrect on exit
    {
        plr->ResurrectPlayer(1.0f);
        plr->SpawnCorpseBones();
    }*/

    //RemovePlayer(plr, guid);                                // BG subclass specific code

    if(participant) // if the player was a match participant, remove auras, calc rating, update queue
    {
        BattleGroundTypeId bgTypeId = GetTypeID();
        BattleGroundQueueTypeId bgQueueTypeId = sClusterBGMgr.BGQueueTypeId(GetTypeID(), GetArenaType());
        /*if (plr)
        {
            plr->ClearAfkReports();

            if(!team) team = plr->GetTeam();

			plr->RemoveAurasDueToSpell(SPELL_AURA_PVP_HEALING); 

            // if arena, remove the specific arena auras
            if (isArena())
            {
                plr->RemoveArenaAuras(true);                // removes debuffs / dots etc., we don't want the player to die after porting out
                bgTypeId=BATTLEGROUND_AA;                   // set the bg type to all arenas (it will be used for queue refreshing)

                // unsummon current and summon old pet if there was one and there isn't a current pet
                plr->RemovePet(NULL, PET_SAVE_NOT_IN_SLOT);
                plr->ResummonPetTemporaryUnSummonedIfAny();

                if (isRated() && GetStatus() == STATUS_IN_PROGRESS)
                {
                    //left a rated match while the encounter was in progress, consider as loser
                    ArenaTeam * winner_arena_team = sObjectMgr.GetArenaTeamById(GetArenaTeamIdForTeam(GetOtherTeam(team)));
                    ArenaTeam * loser_arena_team = sObjectMgr.GetArenaTeamById(GetArenaTeamIdForTeam(team));
                    if (winner_arena_team && loser_arena_team)
                        loser_arena_team->MemberLost(plr,winner_arena_team->GetRating());
                }
            }
            if (SendPacket)
            {
                WorldPacket data;
                sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, this, plr->GetBattleGroundQueueIndex(bgQueueTypeId), STATUS_NONE, 0, 0, 0);
                plr->GetSession()->SendPacket(&data);
            }

            // this call is important, because player, when joins to battleground, this method is not called, so it must be called when leaving bg
            plr->RemoveBattleGroundQueueId(bgQueueTypeId);
        }
        else
        // removing offline participant
        {
            if (isRated() && GetStatus() == STATUS_IN_PROGRESS)
            {
                //left a rated match while the encounter was in progress, consider as loser
                ArenaTeam * others_arena_team = sObjectMgr.GetArenaTeamById(GetArenaTeamIdForTeam(GetOtherTeam(team)));
                ArenaTeam * players_arena_team = sObjectMgr.GetArenaTeamById(GetArenaTeamIdForTeam(team));
                if (others_arena_team && players_arena_team)
                    players_arena_team->OfflineMemberLost(guid, others_arena_team->GetRating());
            }
        }*/

        // remove from raid group if player is member
        /*if (Group *group = GetBgRaid(team))
        {
            if( !group->RemoveMember(guid, 0) )             // group was disbanded
            {
                SetBgRaid(team, NULL);
                delete group;
            }
        }*/
        DecreaseInvitedCount(team);
        //we should update battleground queue, but only if bg isn't ending
        if (isBattleGround() && GetStatus() < STATUS_WAIT_LEAVE)
        {
            // a player has left the battleground, so there are free slots -> add to queue
            AddToBGFreeSlotQueue();
            //sBattleGroundMgr.ScheduleQueueUpdate(0, 0, bgQueueTypeId, bgTypeId, GetBracketId());
        }

        // Let others know
        WorldPacket data;
        sClusterBGMgr.BuildPlayerLeftBattleGroundPacket(&data, guid);
        //SendPacketToTeam(team, &data, plr, false);
    }

    /*if (plr)
    {
        // Do next only if found in battleground
        plr->SetBattleGroundId(0, BATTLEGROUND_TYPE_NONE);  // We're not in BG.
        // reset destination bg team
        plr->SetBGTeam(0);

        if (Transport)
            plr->TeleportToBGEntryPoint();

        sLog.outDetail("BATTLEGROUND: Removed player %s from BattleGround.", plr->GetName());
    }*/

    //battleground object will be deleted next BattleGround::Update() call
}

void cBattleGround::Reset()
{
	SetWinner(WINNER_NONE);
    SetStatus(STATUS_WAIT_QUEUE);
    SetStartTime(0);
    SetEndTime(0);
    SetArenaType(0);
    SetRated(false);

    m_Events = 0;

    // door-event2 is always 0
    m_ActiveEvents[BG_EVENT_DOOR] = 0;
    if (isArena())
    {
        m_ActiveEvents[ARENA_BUFF_EVENT] = BG_EVENT_NONE;
        m_ArenaBuffSpawned = false;
    }

    if(m_InvitedAlliance > 0 || m_InvitedHorde > 0)
        sLog.outError("BattleGround system: bad counter, m_InvitedAlliance: %d, m_InvitedHorde: %d", m_InvitedAlliance, m_InvitedHorde);

    m_InvitedAlliance = 0;
    m_InvitedHorde = 0;
    m_InBGFreeSlotQueue = false;

    // need do the same
	m_Players.clear();

    for(BattleGroundScoreMap::const_iterator itr = m_PlayerScores.begin(); itr != m_PlayerScores.end(); ++itr)
        delete itr->second;
    m_PlayerScores.clear();
}

void cBattleGround::StartBattleGround()
{
	SetStartTime(0);

    // add BG to free slot queue
    AddToBGFreeSlotQueue();

    // add bg to update list
    // This must be done here, because we need to have already invited some players when first BG::Update() method is executed
    // and it doesn't matter if we call StartBattleGround() more times, because m_BattleGrounds is a map and instance id never changes
	sClusterBGMgr.AddBattleGround(GetInstanceID(), IsRandomBG() ? BATTLEGROUND_RB : GetTypeID(), this);
}

void cBattleGround::AddPlayer(Player *plr)
{
	/*// remove afk from player
    if (plr->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_AFK))
        plr->ToggleAFK();

    // score struct must be created in inherited class

    uint64 guid = plr->GetGUID();
    uint32 team = plr->GetBGTeam();

    BattleGroundPlayer bp;
    bp.OfflineRemoveTime = 0;
    bp.Team = team;

    m_Players[guid] = bp;

    //UpdatePlayersCountByTeam(team, false);                  // +1 player

    WorldPacket data;
    sClusterBGMgr.BuildPlayerJoinedBattleGroundPacket(&data, plr);
    SendPacketToTeam(team, &data, plr, false);

	plr->CastSpell(plr, SPELL_AURA_PVP_HEALING,true); 
	plr->Unmount();
	plr->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);*/

    // add arena specific auras
    if (isArena())
    {
       /* plr->RemoveArenaSpellCooldowns();
        plr->RemoveArenaAuras();
        plr->RemoveAllEnchantments(TEMP_ENCHANTMENT_SLOT);
        if(team == ALLIANCE)                                // gold
        {
            if (plr->GetTeam() == HORDE)
                plr->CastSpell(plr, SPELL_HORDE_GOLD_FLAG,true);
            else
                plr->CastSpell(plr, SPELL_ALLIANCE_GOLD_FLAG,true);
        }
        else                                                // green
        {
            if (plr->GetTeam() == HORDE)
                plr->CastSpell(plr, SPELL_HORDE_GREEN_FLAG,true);
            else
                plr->CastSpell(plr, SPELL_ALLIANCE_GREEN_FLAG,true);
        }

        plr->DestroyConjuredItems(true);
        plr->UnsummonPetTemporaryIfAny();*/

        if(GetStatus() == STATUS_WAIT_JOIN)                 // not started yet
        {
           /* plr->CastSpell(plr, SPELL_ARENA_PREPARATION, true);

            plr->SetHealth(plr->GetMaxHealth());
            plr->SetPower(POWER_MANA, plr->GetMaxPower(POWER_MANA));*/
        }

		/*WorldPacket data(SMSG_ARENA_OPPONENT_UPDATE, 8);
		data << uint64(plr->GetGUID());
		SendPacketToTeam(team, &data, plr, true);*/
    }
    else
    {
        /*if(GetStatus() == STATUS_WAIT_JOIN)                 // not started yet
            plr->CastSpell(plr, SPELL_PREPARATION, true);   // reduces all mana cost of spells.*/
    }

    /*plr->GetAchievementMgr().ResetAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_HEALING_DONE, ACHIEVEMENT_CRITERIA_CONDITION_MAP, GetMapId());
    plr->GetAchievementMgr().ResetAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DAMAGE_DONE, ACHIEVEMENT_CRITERIA_CONDITION_MAP, GetMapId());*/

    // setup BG group membership
    PlayerAddedToBGCheckIfBGIsRunning(plr);
    //AddOrSetPlayerToCorrectBgGroup(plr, guid, team);

    // Log
    sLog.outDetail("BATTLEGROUND: Player %s joined the battle.", plr->GetName());
}

void cBattleGround::AddOrSetPlayerToCorrectBgGroup(Player *plr, uint64 plr_guid, uint32 team)
{
	/*Group* group = GetBgRaid(team);
    if(!group)                                      // first player joined
    {
        group = new Group;
        SetBgRaid(team, group);
        group->Create(plr_guid, plr->GetName());
    }
    else                                            // raid already exist
    {
        if (group->IsMember(plr_guid))
        {
            uint8 subgroup = group->GetMemberGroup(plr_guid);
            plr->SetBattleGroundRaid(group, subgroup);
        }
        else
        {
            group->AddMember(plr_guid, plr->GetName());
            if (Group* originalGroup = plr->GetOriginalGroup())
                if (originalGroup->IsLeader(plr_guid))
                    group->ChangeLeader(plr_guid);
        }
    }*/
}

void cBattleGround::EventPlayerLoggedIn(Player* player, uint64 plr_guid)
{
	for(std::deque<uint64>::iterator itr = m_OfflineQueue.begin(); itr != m_OfflineQueue.end(); ++itr)
    {
        if (*itr == plr_guid)
        {
            m_OfflineQueue.erase(itr);
            break;
        }
    }

    m_Players[plr_guid].OfflineRemoveTime = 0;
    PlayerAddedToBGCheckIfBGIsRunning(player);
    // if battleground is starting, then add preparation aura
    // we don't have to do that, because preparation aura isn't removed when player logs out
}

void cBattleGround::EventPlayerLoggedOut(Player* player)
{
	// player is correct pointer, it is checked in WorldSession::LogoutPlayer()
    m_OfflineQueue.push_back(player->GetGUID());

    //m_Players[player->GetGUID()].OfflineRemoveTime = sWorld.GetGameTime() + MAX_OFFLINE_TIME;
    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        // drop flag and handle other cleanups
        RemovePlayer(player, player->GetGUID());

        // 1 player is logging out, if it is the last, then end arena!
        /*if (isArena())
            if (GetAlivePlayersCountByTeam(player->GetTeam()) <= 1 && GetPlayersCountByTeam(GetOtherTeam(player->GetTeam())))
                EndBattleGround(GetOtherTeam(player->GetTeam()));*/

    }
}

void cBattleGround::AddToBGFreeSlotQueue()
{
	// make sure to add only once
    if (!m_InBGFreeSlotQueue && isBattleGround())
    {
        //sBattleGroundMgr.BGFreeSlotQueue[GetTypeID()].push_front(this);
        m_InBGFreeSlotQueue = true;
    }
}

void cBattleGround::RemoveFromBGFreeSlotQueue()
{
	// set to be able to re-add if needed
    //m_InBGFreeSlotQueue = false;
	BattleGroundTypeId typeId = GetTypeID();
    // uncomment this code when battlegrounds will work like instances
    /*for (BGFreeSlotQueueType::iterator itr = sBattleGroundMgr.BGFreeSlotQueue[typeId].begin(); itr != sBattleGroundMgr.BGFreeSlotQueue[typeId].end(); ++itr)
    {
        if ((*itr)->GetInstanceID() == m_InstanceID)
        {
            sBattleGroundMgr.BGFreeSlotQueue[typeId].erase(itr);
            return;
        }
    }*/
}

uint32 cBattleGround::GetFreeSlotsForTeam(uint32 Team) const
{
	//return free slot count to MaxPlayerPerTeam
    if (GetStatus() == STATUS_WAIT_JOIN || GetStatus() == STATUS_IN_PROGRESS)
        return /*(GetInvitedCount(Team) < GetMaxPlayersPerTeam()) ? GetMaxPlayersPerTeam() - GetInvitedCount(Team) :*/ 0;

	return 0;
}

bool cBattleGround::HasFreeSlots()
{
	return GetPlayersSize() < GetMaxPlayers();
}

void cBattleGround::UpdatePlayerScore(Player *Source, uint32 type, uint32 value)
{
	/*BattleGroundScoreMap::const_iterator itr = m_PlayerScores.find(Source->GetGUID());

    if(itr == m_PlayerScores.end())                         // player not found...
        return;

    switch(type)
    {
        case SCORE_KILLING_BLOWS:                           // Killing blows
            itr->second->KillingBlows += value;
            break;
        case SCORE_DEATHS:                                  // Deaths
            itr->second->Deaths += value;
            break;
        case SCORE_HONORABLE_KILLS:                         // Honorable kills
            itr->second->HonorableKills += value;
			if(itr->second->HonorableKills >= 0)
				RewardAchievementToPlayer(Source,229);
            break;
        case SCORE_BONUS_HONOR:                             // Honor bonus
            // do not add honor in arenas
            if (isBattleGround())
            {
                // reward honor instantly
                if (Source->RewardHonor(NULL, 1, value))
                    itr->second->BonusHonor += value;
            }
            break;
            //used only in EY, but in MSG_PVP_LOG_DATA opcode
        case SCORE_DAMAGE_DONE:                             // Damage Done
            itr->second->DamageDone += value;
            break;
        case SCORE_HEALING_DONE:                            // Healing Done
            itr->second->HealingDone += value;
            break;
        default:
            sLog.outError("BattleGround: Unknown player score type %u", type);
            break;
    }*/
}

uint32 cBattleGround::GetPlayerScore(Player *Source, uint32 type)
{
	BattleGroundScoreMap::const_iterator itr = m_PlayerScores.find(Source->GetGUID());

    if(itr == m_PlayerScores.end()) // player not found...
        return 0;

    switch(type)
    {
        case SCORE_KILLING_BLOWS: // Killing blows
            return itr->second->KillingBlows;
        case SCORE_DEATHS: // Deaths
            return itr->second->Deaths;
        case SCORE_DAMAGE_DONE: // Damage Done
            return itr->second->DamageDone;
        case SCORE_HEALING_DONE: // Healing Done
            return itr->second->HealingDone;
        default:
            sLog.outError("BattleGround: Unknown player score type %u", type);
            return 0;
   }
}

uint32 cBattleGround::GetDamageDoneForTeam(uint32 TeamID)
{
	uint32 finaldamage = 0;
	for(BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
	{
		uint32 team = itr->second.Team;
		/*Player *plr = sObjectMgr.GetPlayer(/*itr->first*itr);
		if (!plr)
			continue; 
		if(!team) team = plr->GetTeam();
		if(team == TeamID)
			finaldamage += GetPlayerScore(plr, SCORE_DAMAGE_DONE);*/
	}
	return finaldamage;
}

bool cBattleGround::AddObject(uint32 type, uint32 entry, float x, float y, float z, float o, float rotation0, float rotation1, float rotation2, float rotation3, uint32 respawnTime)
{
	return true;
}

void cBattleGround::DoorClose(uint64 const& guid)
{
	/*GameObject *obj = GetBgMap()->GetGameObject(guid);
    if (obj)
    {
        //if doors are open, close it
        if (obj->getLootState() == GO_ACTIVATED && obj->GetGoState() != GO_STATE_READY)
        {
            //change state to allow door to be closed
            obj->SetLootState(GO_READY);
            obj->UseDoorOrButton(RESPAWN_ONE_DAY);
        }
    }
    else
    {
        sLog.outError("BattleGround: Door object not found (cannot close doors)");
    }*/
}

void BattleGround::DoorOpen(uint64 const& guid)
{
	/*GameObject *obj = GetBgMap()->GetGameObject(guid);
    if (obj)
    {
        //change state to be sure they will be opened
        obj->SetLootState(GO_READY);
        obj->UseDoorOrButton(RESPAWN_ONE_DAY);
    }
    else
    {
        sLog.outError("BattleGround: Door object not found! - doors will be closed.");
    }*/
}

void cBattleGround::OnObjectDBLoad(Creature* creature)
{
    const BattleGroundEventIdx eventId = sClusterBGMgr.GetCreatureEventIndex(creature->GetDBTableGUIDLow());
    if (eventId.event1 == BG_EVENT_NONE)
        return;
    m_EventObjects[MAKE_PAIR32(eventId.event1, eventId.event2)].creatures.push_back(creature->GetGUID());
    if (!IsActiveEvent(eventId.event1, eventId.event2))
        SpawnBGCreature(creature->GetGUID(), RESPAWN_ONE_DAY);
}

uint64 cBattleGround::GetSingleCreatureGuid(uint8 event1, uint8 event2)
{
	BGCreatures::const_iterator itr = m_EventObjects[MAKE_PAIR32(event1, event2)].creatures.begin();
    if (itr != m_EventObjects[MAKE_PAIR32(event1, event2)].creatures.end())
        return *itr;

	return 0;
}

void cBattleGround::OnObjectDBLoad(GameObject* obj)
{
	const BattleGroundEventIdx eventId = sClusterBGMgr.GetGameObjectEventIndex(obj->GetDBTableGUIDLow());
    if (eventId.event1 == BG_EVENT_NONE)
        return;
    m_EventObjects[MAKE_PAIR32(eventId.event1, eventId.event2)].gameobjects.push_back(obj->GetGUID());
    if (!IsActiveEvent(eventId.event1, eventId.event2))
    {
        SpawnBGObject(obj->GetGUID(), RESPAWN_ONE_DAY);
    }
    else
    {
        // it's possible, that doors aren't spawned anymore (wsg)
        if (GetStatus() >= STATUS_IN_PROGRESS && IsDoor(eventId.event1, eventId.event2))
            DoorOpen(obj->GetGUID());
    }
}

bool cBattleGround::IsDoor(uint8 event1, uint8 event2)
{
	if (event1 == BG_EVENT_DOOR)
    {
        if (event2 > 0)
        {
            sLog.outError("BattleGround too high event2 for event1:%i", event1);
            return false;
        }
        return true;
    }
    return false;
}

void cBattleGround::OpenDoorEvent(uint8 event1, uint8 event2 /*=0*/)
{
	if (!IsDoor(event1, event2))
    {
        sLog.outError("BattleGround:OpenDoorEvent this is no door event1:%u event2:%u", event1, event2);
        return;
    }
    /*if (!IsActiveEvent(event1, event2))                 // maybe already despawned (eye)
    {
        sLog.outError("BattleGround:OpenDoorEvent this event isn't active event1:%u event2:%u", event1, event2);
        return;
    }
    BGObjects::const_iterator itr = m_EventObjects[MAKE_PAIR32(event1, event2)].gameobjects.begin();
    for(; itr != m_EventObjects[MAKE_PAIR32(event1, event2)].gameobjects.end(); ++itr)
        DoorOpen(*itr);*/
}

void cBattleGround::SpawnEvent(uint8 event1, uint8 event2, bool spawn)
{
	// stop if we want to spawn something which was already spawned
    // or despawn something which was already despawned
    /*if (event2 == BG_EVENT_NONE || (spawn && m_ActiveEvents[event1] == event2)
        || (!spawn && m_ActiveEvents[event1] != event2))
        return;

    if (spawn)
    {
        // if event gets spawned, the current active event mus get despawned
        SpawnEvent(event1, m_ActiveEvents[event1], false);
        m_ActiveEvents[event1] = event2;                    // set this event to active
    }
    else
        m_ActiveEvents[event1] = BG_EVENT_NONE;             // no event active if event2 gets despawned

    BGCreatures::const_iterator itr = m_EventObjects[MAKE_PAIR32(event1, event2)].creatures.begin();
    for(; itr != m_EventObjects[MAKE_PAIR32(event1, event2)].creatures.end(); ++itr)
        SpawnBGCreature(*itr, (spawn) ? RESPAWN_IMMEDIATELY : RESPAWN_ONE_DAY);
    BGObjects::const_iterator itr2 = m_EventObjects[MAKE_PAIR32(event1, event2)].gameobjects.begin();
    for(; itr2 != m_EventObjects[MAKE_PAIR32(event1, event2)].gameobjects.end(); ++itr2)
        SpawnBGObject(*itr2, (spawn) ? RESPAWN_IMMEDIATELY : RESPAWN_ONE_DAY);*/
}

void cBattleGround::SpawnBGObject(uint64 const& guid, uint32 respawntime)
{
	/*Map* map = GetBgMap();

    GameObject *obj = map->GetGameObject(guid);
    if(!obj)
        return;
    if (respawntime == 0)
    {
        //we need to change state from GO_JUST_DEACTIVATED to GO_READY in case battleground is starting again
        if (obj->getLootState() == GO_JUST_DEACTIVATED)
            obj->SetLootState(GO_READY);
        obj->SetRespawnTime(0);
        map->Add(obj);
    }
    else
    {
        map->Add(obj);
        obj->SetRespawnTime(respawntime);
        obj->SetLootState(GO_JUST_DEACTIVATED);
    }*/
}

void cBattleGround::SpawnBGCreature(uint64 const& guid, uint32 respawntime)
{
	/*Map* map = GetBgMap();

    Creature* obj = map->GetCreature(guid);
    if (!obj)
        return;
    if (respawntime == 0)
    {
        obj->Respawn();
        map->Add(obj);
    }
    else
    {
        map->Add(obj);
        obj->setDeathState(JUST_DIED);
        obj->SetRespawnDelay(respawntime);
        obj->RemoveCorpse();
    }*/
}

bool cBattleGround::DelObject(uint32 type)
{
	/*if (!m_BgObjects[type])
        return true;

    GameObject *obj = GetBgMap()->GetGameObject(m_BgObjects[type]);
    if (!obj)
    {
        sLog.outError("Can't find gobject guid: %u",GUID_LOPART(m_BgObjects[type]));
        return false;
    }

    obj->SetRespawnTime(0);                                 // not save respawn time
    obj->Delete();
    m_BgObjects[type] = 0;*/
    return true;
}

template<class Do>
void cBattleGround::BroadcastWorker(Do& _do)
{
    for(BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        if (Player *plr = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(itr->first, 0, HIGHGUID_PLAYER)))
            _do(plr);
}

void cBattleGround::SendMessageToAll(int32 entry, ChatMsg type, Player const* source)
{
	ClusterFX::BattleGroundChatBuilder bg_builder(type, entry, source);
    MaNGOS::LocalizedPacketDo<ClusterFX::BattleGroundChatBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);
}

void cBattleGround::SendYellToAll(int32 entry, uint32 language, uint64 const& guid)
{
	/*Creature* source = GetBgMap()->GetCreature(guid);
    if(!source)
        return;
    ClusterFX::BattleGroundYellBuilder bg_builder(language, entry, source);
    MaNGOS::LocalizedPacketDo<ClusterFX::BattleGroundYellBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);*/
}

void cBattleGround::PSendMessageToAll(int32 entry, ChatMsg type, Player const* source, ...)
{
	va_list ap;
    va_start(ap, source);

    ClusterFX::BattleGroundChatBuilder bg_builder(type, entry, source, &ap);
    MaNGOS::LocalizedPacketDo<ClusterFX::BattleGroundChatBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);

    va_end(ap);
}

void cBattleGround::SendMessage2ToAll(int32 entry, ChatMsg type, Player const* source, int32 arg1, int32 arg2)
{
	ClusterFX::BattleGround2ChatBuilder bg_builder(type, entry, source, arg1, arg2);
    MaNGOS::LocalizedPacketDo<ClusterFX::BattleGround2ChatBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);
}

void cBattleGround::SendYell2ToAll(int32 entry, uint32 language, uint64 const& guid, int32 arg1, int32 arg2)
{
	/*Creature* source = GetBgMap()->GetCreature(guid);
    if(!source)
        return;
    ClusterFX::BattleGround2YellBuilder bg_builder(language, entry, source, arg1, arg2);
    MaNGOS::LocalizedPacketDo<ClusterFX::BattleGround2YellBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);*/
}

void cBattleGround::EndNow()
{
	RemoveFromBGFreeSlotQueue();
    SetStatus(STATUS_WAIT_LEAVE);
    SetEndTime(0);
}

void cBattleGround::HandleTriggerBuff(uint64 const& go_guid)
{
	/*GameObject *obj = GetBgMap()->GetGameObject(go_guid);
    if (!obj || obj->GetGoType() != GAMEOBJECT_TYPE_TRAP || !obj->isSpawned())
        return;*/

    // static buffs are already handled just by database and don't need
    // battleground code
    if (!m_BuffChange)
    {
        /*obj->SetLootState(GO_JUST_DEACTIVATED);             // can be despawned or destroyed
        return;*/
    }

    // change buff type, when buff is used:
    // TODO this can be done when poolsystem works for instances
    int32 index = m_BgObjects.size() - 1;
    while (index >= 0 && m_BgObjects[index] != go_guid)
        index--;
    if (index < 0)
    {
        //sLog.outError("BattleGround (Type: %u) has buff gameobject (Guid: %u Entry: %u Type:%u) but it hasn't that object in its internal data",GetTypeID(),GUID_LOPART(go_guid),obj->GetEntry(),obj->GetGoType());
        return;
    }

    //randomly select new buff
    uint8 buff = urand(0, 2);
    /*uint32 entry = obj->GetEntry();
    if (m_BuffChange && entry != Buff_Entries[buff])
    {
        //despawn current buff
        SpawnBGObject(m_BgObjects[index], RESPAWN_ONE_DAY);
        //set index for new one
        for (uint8 currBuffTypeIndex = 0; currBuffTypeIndex < 3; ++currBuffTypeIndex)
        {
            if (entry == Buff_Entries[currBuffTypeIndex])
            {
                index -= currBuffTypeIndex;
                index += buff;
            }
        }
    }

    SpawnBGObject(m_BgObjects[index], BUFF_RESPAWN_TIME);*/
}

void cBattleGround::HandleKillPlayer(Player *player, Player *killer)
{
	// add +1 deaths
    /*UpdatePlayerScore(player, SCORE_DEATHS, 1);

    // add +1 kills to group and +1 killing_blows to killer
    if (killer)
    {
        UpdatePlayerScore(killer, SCORE_HONORABLE_KILLS, 1);
        UpdatePlayerScore(killer, SCORE_KILLING_BLOWS, 1);

        for(BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        {
            Player *plr = sObjectMgr.GetPlayer(itr->first);

            if (!plr || plr == killer)
                continue;

            if (plr->GetTeam() == killer->GetTeam() && plr->IsAtGroupRewardDistance(player))
                UpdatePlayerScore(plr, SCORE_HONORABLE_KILLS, 1);
        }
    }

    // to be able to remove insignia -- ONLY IN BattleGrounds
    if (!isArena())
        player->SetFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE );*/
}

uint32 cBattleGround::GetPlayerTeam(uint64 guid)
{
    BattleGroundPlayerMap::const_iterator itr = m_Players.find(guid);
    if (itr!=m_Players.end())
        return itr->second.Team;
    return 0;
}

bool cBattleGround::IsPlayerInBattleGround(uint64 guid)
{
    BattleGroundPlayerMap::const_iterator itr = m_Players.find(guid);
    if (itr != m_Players.end())
        return true;
    return false;
}

void cBattleGround::PlayerAddedToBGCheckIfBGIsRunning(Player* plr)
{
	if (GetStatus() != STATUS_WAIT_LEAVE)
        return;

    WorldPacket data;
    BattleGroundQueueTypeId bgQueueTypeId = sClusterBGMgr.BGQueueTypeId(GetTypeID(), GetArenaType());

    /*BlockMovement(plr);

    sBattleGroundMgr.BuildPvpLogDataPacket(&data, this);
    plr->GetSession()->SendPacket(&data);*/

    sClusterBGMgr.BuildBattleGroundStatusPacket(&data, this, plr->GetBattleGroundQueueIndex(bgQueueTypeId), STATUS_IN_PROGRESS, GetEndTime(), GetStartTime(), GetArenaType());
    //plr->GetSession()->SendPacket(&data);
}

uint32 cBattleGround::GetAlivePlayersCountByTeam(uint32 Team)
{
	uint32 count = 0;
	for(BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.Team == Team)
        {
            /*Player * pl = sObjectMgr.GetPlayer(itr->first);
            if (pl && pl->isAlive())
                ++count;*/
        }
    }
    return count;
}

void cBattleGround::CheckArenaWinConditions()
{
	if (!GetAlivePlayersCountByTeam(ALLIANCE) && GetPlayersCountByTeam(HORDE))
        EndBattleGround(HORDE);
    else if (GetPlayersCountByTeam(ALLIANCE) && !GetAlivePlayersCountByTeam(HORDE))
        EndBattleGround(ALLIANCE);
}

void cBattleGround::SetBgRaid( uint32 TeamID, Group *bg_raid )
{
	/*Group* &old_raid = TeamID == ALLIANCE ? m_BgRaids[BG_TEAM_ALLIANCE] : m_BgRaids[BG_TEAM_HORDE];
    if(old_raid) old_raid->SetBattlegroundGroup(NULL);
    if(bg_raid) bg_raid->SetBattlegroundGroup(this);
    old_raid = bg_raid;*/
}

WorldSafeLocsEntry const* BattleGround::GetClosestGraveYard( Player* player )
{
	//return sObjectMgr.GetClosestGraveYard( player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId(), player->GetTeam() );
	return NULL;
}

bool cBattleGround::IsTeamScoreInRange(uint32 team, uint32 minScore, uint32 maxScore) const
{
    BattleGroundTeamId team_idx = GetTeamIndexByTeamId(team);
    uint32 score = (m_TeamScores[team_idx] < 0) ? 0 : uint32(m_TeamScores[team_idx]);
    return score >= minScore && score <= maxScore;
}

void cBattleGround::SetBracket( PvPDifficultyEntry const* bracketEntry )
{
	m_BracketId  = bracketEntry->GetBracketId();
    SetLevelRange(bracketEntry->minLevel,bracketEntry->maxLevel);
}

void cBattleGround::UpdateArenaWorldState()
{
	UpdateWorldState(0xe10, GetAlivePlayersCountByTeam(HORDE));
    UpdateWorldState(0xe11, GetAlivePlayersCountByTeam(ALLIANCE));
}

GameObject* cBattleGround::GetBGObject(uint32 type)
{
    /*GameObject *obj = GetBgMap()->GetGameObject(m_BgObjects[type]);
    if(!obj)
        sLog.outError("couldn't get gameobject %i",type);
    return obj;*/
	return NULL;
}

bool cBattleGround::DelCreature(uint32 type)
{
    /*if (!m_BgCreatures[type])
        return true;

    Creature *cr = GetBgMap()->GetCreature(m_BgCreatures[type]);
    if (!cr)
    {
        sLog.outError("Can't find creature guid: %u",GUID_LOPART(m_BgCreatures[type]));
        return false;
    }
    cr->AddObjectToRemoveList();
    m_BgCreatures[type] = 0;*/
    return true;
}

Creature* cBattleGround::AddCreature(uint32 entry, uint32 type, uint32 teamval, float x, float y, float z, float o, uint32 respawntime)
{
    /*Map* map = GetBgMap();
    if (!map)*/
        return NULL;

    /*Creature* pCreature = new Creature;
    if (!pCreature->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_UNIT), map, PHASEMASK_NORMAL, entry, teamval, 0))
    {
        sLog.outError("Can't create creature entry: %u",entry);
        delete pCreature;
        return NULL;
    }

	pCreature->Relocate(x, y, z, o);
    pCreature->SetSummonPoint(x, y, z, o);
    //pCreature->SetHomePosition(x, y, z, o);

    //pCreature->SetDungeonDifficulty(0);

    map->Add(pCreature);
    m_BgCreatures[type] = pCreature->GetGUID();

    return  pCreature;*/
}

bool cBattleGround::AddSpiritGuide(uint32 type, float x, float y, float z, float o, uint32 team)
{
	uint32 entry = 0;

    if (team == ALLIANCE)
        entry = 13116;
    else
        entry = 13117;

    /*Creature* pCreature = AddCreature(entry,type,team,x,y,z,o);
    if (!pCreature)
    {
        sLog.outError("Can't create Spirit guide. BattleGround not created!");
        EndNow();
        return false;
    }

    pCreature->setDeathState(DEAD);

    pCreature->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, pCreature->GetGUID());
    // aura
    //TODO: Fix display here
    //pCreature->SetVisibleAura(0, SPELL_SPIRIT_HEAL_CHANNEL);
    // casting visual effect
    pCreature->SetUInt32Value(UNIT_CHANNEL_SPELL, 22011);
    // correct cast speed
    pCreature->SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);*/

    //pCreature->CastSpell(pCreature, SPELL_SPIRIT_HEAL_CHANNEL, true);

    return true;
}

Creature* cBattleGround::GetBGCreature(uint32 type)
{
	/*Creature *creature = GetBgMap()->GetCreature(m_BgCreatures[type]);
    if(!creature)
        sLog.outError("couldn't get creature %i",type);
    return creature;*/
    return NULL;
}

void cBattleGround::RewardAchievementToPlayer(Player* plr, uint32 entry)
{
	/*if(!plr)
		return;

	AchievementEntry const* pAE = GetAchievementStore()->LookupEntry(entry);
	if (!pAE)
    {
        sLog.outError("DoCompleteAchievement called for not existing achievement %u", entry);
        return;
    }
	
	plr->GetAchievementMgr().DoCompleteAchivement(pAE);*/
}

void cBattleGround::RewardAchievementToTeam(uint32 team, uint32 entry)
{
	for(BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.OfflineRemoveTime)
            continue;
        
		/*Player *plr = sObjectMgr.GetPlayer(itr->first);

        if (!plr)
			continue;

		AchievementEntry const* pAE = GetAchievementStore()->LookupEntry(entry);
		if (!pAE)
		{
			sLog.outError("DoCompleteAchievement called for not existing achievement %u", entry);
			continue;
		}*/

        uint32 TeamID = itr->second.Team;
        /*if(!TeamID) TeamID = plr->GetTeam();

        if (team == TeamID)
            plr->GetAchievementMgr().DoCompleteAchivement(pAE);*/
    }
}

// Cluster special functions

std::vector<uint64> cBattleGround::getPlayerList()
{
	std::vector<uint64> players;
	players.clear();

	for(BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.OfflineRemoveTime)
            continue;
		players.push_back(itr->first);
	}
	return players;
}

uint32 cBattleGround::GetPlayerOfflineTime(uint64 guid)
{
	BattleGroundPlayerMap::const_iterator itr = m_Players.find(guid);
    if (itr != m_Players.end())
        return uint32(itr->second.OfflineRemoveTime);
    return 0;
}



void cBattleGround::SetPlayerValues(uint64 guid, uint32 offlineTime, uint32 team)
{
	if(!IsPlayerInBattleGround(guid))
		UpdatePlayersCountByTeam(team,false);

	cBattleGroundPlayer bp;
    bp.OfflineRemoveTime = offlineTime;
    bp.Team = team;

	m_Players[guid] = bp;
}



