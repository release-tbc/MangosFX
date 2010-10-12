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

#ifndef __BATTLEGROUND_H
#define __BATTLEGROUND_H

#include "Common.h"
#include "SharedDefines.h"
#include "DBCEnums.h"
#include "ByteBuffer.h"
#include "cClusterMgr.h"
#include "cPacketOpcodes.h"

// magic event-numbers
#define BG_EVENT_NONE 255
// those generic events should get a high event id
#define BG_EVENT_DOOR 254
// only arena event
// cause this buff apears 90sec after start in every bg i implement it here
#define ARENA_BUFF_EVENT 252

class Creature;
class GameObject;
class Group;
class Player;
class WorldPacket;
class BattleGroundMap;

struct PvPDifficultyEntry;
struct WorldSafeLocsEntry;

struct BattleGroundEventIdx
{
    uint8 event1;
    uint8 event2;
};

enum BattleGroundSounds
{
    SOUND_HORDE_WINS                = 8454,
    SOUND_ALLIANCE_WINS             = 8455,
    SOUND_BG_START                  = 3439
};

enum BattleGroundQuests
{
    SPELL_WS_QUEST_REWARD           = 43483,
    SPELL_AB_QUEST_REWARD           = 43484,
    SPELL_AV_QUEST_REWARD           = 43475,
    SPELL_AV_QUEST_KILLED_BOSS      = 23658,
    SPELL_EY_QUEST_REWARD           = 43477,
    SPELL_AB_QUEST_REWARD_4_BASES   = 24061,
    SPELL_AB_QUEST_REWARD_5_BASES   = 24064
};

enum BattleGroundMarks
{
    SPELL_WS_MARK_LOSER             = 24950,
    SPELL_WS_MARK_WINNER            = 24951,
    SPELL_AB_MARK_LOSER             = 24952,
    SPELL_AB_MARK_WINNER            = 24953,
    SPELL_AV_MARK_LOSER             = 24954,
    SPELL_AV_MARK_WINNER            = 24955,
    ITEM_EY_MARK_OF_HONOR           = 29024,
    SPELL_SA_MARK_LOSER             = 61159,
    SPELL_SA_MARK_WINNER            = 61160,
};

enum BattleGroundMarksCount
{
    ITEM_WINNER_COUNT               = 3,
    ITEM_LOSER_COUNT                = 1
};

enum BattleGroundSpells
{
    SPELL_ARENA_PREPARATION         = 32727,                // use this one, 32728 not correct
    SPELL_ALLIANCE_GOLD_FLAG        = 32724,
    SPELL_ALLIANCE_GREEN_FLAG       = 32725,
    SPELL_HORDE_GOLD_FLAG           = 35774,
    SPELL_HORDE_GREEN_FLAG          = 35775,
    SPELL_PREPARATION               = 44521,                // Preparation
    SPELL_RECENTLY_DROPPED_FLAG     = 42792,                // Recently Dropped Flag
    SPELL_AURA_PLAYER_INACTIVE      = 43681,                 // Inactive
	SPELL_AURA_PVP_HEALING          = 74410                 // PvP 10% healing reduction
};

enum BattleGroundTimeIntervals
{
    RESURRECTION_INTERVAL           = 30000,                // ms
    //REMIND_INTERVAL                 = 10000,                // ms
    INVITATION_REMIND_TIME          = 20000,                // ms
    INVITE_ACCEPT_WAIT_TIME         = 40000,                // ms
    TIME_TO_AUTOREMOVE              = 120000,               // ms
    MAX_OFFLINE_TIME                = 300,                  // secs
    RESPAWN_ONE_DAY                 = 86400,                // secs
    RESPAWN_IMMEDIATELY             = 0,                    // secs
    BUFF_RESPAWN_TIME               = 180,                  // secs
    ARENA_SPAWN_BUFF_OBJECTS        = 90000,                // ms - 90sec after start
	ARENA_TIME_LIMIT                = 2820000,               // ms - 47 minutes after start
};

enum BattleGroundStartTimeIntervals
{
    BG_START_DELAY_2M               = 120000,               // ms (2 minutes)
    BG_START_DELAY_1M               = 60000,                // ms (1 minute)
    BG_START_DELAY_30S              = 30000,                // ms (30 seconds)
    BG_START_DELAY_15S              = 15000,                // ms (15 seconds) Used only in arena
    BG_START_DELAY_NONE             = 0,                    // ms
};

enum BattleGroundBuffObjects
{
    BG_OBJECTID_SPEEDBUFF_ENTRY     = 179871,
    BG_OBJECTID_REGENBUFF_ENTRY     = 179904,
    BG_OBJECTID_BERSERKERBUFF_ENTRY = 179905
};

const uint32 Buff_Entries[3] = { BG_OBJECTID_SPEEDBUFF_ENTRY, BG_OBJECTID_REGENBUFF_ENTRY, BG_OBJECTID_BERSERKERBUFF_ENTRY };

enum BattleGroundStatus
{
    STATUS_NONE         = 0,                                // first status, should mean bg is not instance
    STATUS_WAIT_QUEUE   = 1,                                // means bg is empty and waiting for queue
    STATUS_WAIT_JOIN    = 2,                                // this means, that BG has already started and it is waiting for more players
    STATUS_IN_PROGRESS  = 3,                                // means bg is running
    STATUS_WAIT_LEAVE   = 4                                 // means some faction has won BG and it is ending
};

struct BattleGroundPlayer
{
    time_t  OfflineRemoveTime;                              // for tracking and removing offline players from queue after 5 minutes
    uint32  Team;                                           // Player's team
};

struct BattleGroundObjectInfo
{
    BattleGroundObjectInfo() : object(NULL), timer(0), spellid(0) {}

    GameObject  *object;
    int32       timer;
    uint32      spellid;
};

// handle the queue types and bg types separately to enable joining queue for different sized arenas at the same time
enum BattleGroundQueueTypeId
{
    BATTLEGROUND_QUEUE_NONE     = 0,
    BATTLEGROUND_QUEUE_AV       = 1,
    BATTLEGROUND_QUEUE_WS       = 2,
    BATTLEGROUND_QUEUE_AB       = 3,
    BATTLEGROUND_QUEUE_EY       = 4,
    BATTLEGROUND_QUEUE_SA       = 5,
    BATTLEGROUND_QUEUE_IC       = 6,
    BATTLEGROUND_QUEUE_2v2      = 7,
    BATTLEGROUND_QUEUE_3v3      = 8,
    BATTLEGROUND_QUEUE_5v5      = 9,
	BATTLEGROUND_QUEUE_RANDOM	= 10
};
#define MAX_BATTLEGROUND_QUEUE_TYPES 11

enum ScoreType
{
    SCORE_KILLING_BLOWS         = 1,
    SCORE_DEATHS                = 2,
    SCORE_HONORABLE_KILLS       = 3,
    SCORE_BONUS_HONOR           = 4,
    //EY, but in MSG_PVP_LOG_DATA opcode!
    SCORE_DAMAGE_DONE           = 5,
    SCORE_HEALING_DONE          = 6,
    //WS
    SCORE_FLAG_CAPTURES         = 7,
    SCORE_FLAG_RETURNS          = 8,
    //AB
    SCORE_BASES_ASSAULTED       = 9,
    SCORE_BASES_DEFENDED        = 10,
    //AV
    SCORE_GRAVEYARDS_ASSAULTED  = 11,
    SCORE_GRAVEYARDS_DEFENDED   = 12,
    SCORE_TOWERS_ASSAULTED      = 13,
    SCORE_TOWERS_DEFENDED       = 14,
    SCORE_MINES_CAPTURED        = 15,
    SCORE_LEADERS_KILLED        = 16,
    SCORE_SECONDARY_OBJECTIVES  = 17,
	//SA
	SCORE_DESTROYED_DEMOLISHER	= 18,
	SCORE_DESTROYED_WALL		= 19
    // TODO : implement them
};

enum ArenaType
{
    ARENA_TYPE_2v2          = 2,
    ARENA_TYPE_3v3          = 3,
    ARENA_TYPE_5v5          = 5
};

enum BattleGroundType
{
    TYPE_BATTLEGROUND     = 3,
    TYPE_ARENA            = 4
};

enum BattleGroundWinner
{
    WINNER_HORDE            = 0,
    WINNER_ALLIANCE         = 1,
    WINNER_NONE             = 2
};

enum BattleGroundTeamId
{
    BG_TEAM_ALLIANCE        = 0,
    BG_TEAM_HORDE           = 1,
	BG_TEAM_NEUTRAL			= 2
};
#define BG_TEAMS_COUNT  2

enum BattleGroundStartingEvents
{
    BG_STARTING_EVENT_NONE  = 0x00,
    BG_STARTING_EVENT_1     = 0x01,
    BG_STARTING_EVENT_2     = 0x02,
    BG_STARTING_EVENT_3     = 0x04,
    BG_STARTING_EVENT_4     = 0x08
};

enum BattleGroundStartingEventsIds
{
    BG_STARTING_EVENT_FIRST     = 0,
    BG_STARTING_EVENT_SECOND    = 1,
    BG_STARTING_EVENT_THIRD     = 2,
    BG_STARTING_EVENT_FOURTH    = 3
};
#define BG_STARTING_EVENT_COUNT 4

enum GroupJoinBattlegroundResult
{
	// positive values are indexes in BattlemasterList.dbc
	ERR_GROUP_JOIN_BATTLEGROUND_FAIL        = 0,            // Your group has joined a battleground queue, but you are not eligible (showed for non existing BattlemasterList.dbc indexes)
	ERR_BATTLEGROUND_NONE                   = -1,           // not show anything
	ERR_GROUP_JOIN_BATTLEGROUND_DESERTERS   = -2,           // You cannot join the battleground yet because you or one of your party members is flagged as a Deserter.
	ERR_ARENA_TEAM_PARTY_SIZE               = -3,           // Incorrect party size for this arena.
	ERR_BATTLEGROUND_TOO_MANY_QUEUES        = -4,           // You can only be queued for 2 battles at once
	ERR_BATTLEGROUND_CANNOT_QUEUE_FOR_RATED = -5,           // You cannot queue for a rated match while queued for other battles
	ERR_BATTLEDGROUND_QUEUED_FOR_RATED      = -6,           // You cannot queue for another battle while queued for a rated arena match
	ERR_BATTLEGROUND_TEAM_LEFT_QUEUE        = -7,           // Your team has left the arena queue
	ERR_BATTLEGROUND_NOT_IN_BATTLEGROUND    = -8,           // You can't do that in a battleground.
	ERR_BATTLEGROUND_JOIN_XP_GAIN           = -9,           // wtf, doesn't exist in client...
	ERR_BATTLEGROUND_JOIN_RANGE_INDEX       = -10,          // Cannot join the queue unless all members of your party are in the same battleground level range.
	ERR_BATTLEGROUND_JOIN_TIMED_OUT         = -11,          // %s was unavailable to join the queue. (uint64 guid exist in client cache)
	ERR_BATTLEGROUND_JOIN_FAILED            = -12,          // Join as a group failed (uint64 guid doesn't exist in client cache)
	ERR_LFG_CANT_USE_BATTLEGROUND           = -13,          // You cannot queue for a battleground or arena while using the dungeon system.
	ERR_IN_RANDOM_BG                        = -14,          // Can't do that while in a Random Battleground queue.
	ERR_IN_NON_RANDOM_BG                    = -15,          // Can't queue for Random Battleground while in another Battleground queue.
};

class BattleGroundScore
{
    public:
        BattleGroundScore() : KillingBlows(0), Deaths(0), HonorableKills(0),
            BonusHonor(0), DamageDone(0), HealingDone(0)
        {}
        virtual ~BattleGroundScore() {}                     //virtual destructor is used when deleting score from scores map

        uint32 KillingBlows;
        uint32 Deaths;
        uint32 HonorableKills;
        uint32 BonusHonor;
        uint32 DamageDone;
        uint32 HealingDone;
};

/*
This class is used to:
1. Add player to battleground
2. Remove player from battleground
3. some certain cases, same for all battlegrounds
4. It has properties same for all battlegrounds
*/
class BattleGround
{
    friend class BattleGroundMgr;

    public:
        /* Construction */
        BattleGround();
        /*BattleGround(const BattleGround& bg);*/
        virtual ~BattleGround();
        virtual void Update(uint32 diff);                   // must be implemented in BG subclass of BG specific update code, but must in begginning call parent version
        virtual bool SetupBattleGround()                    // must be implemented in BG subclass
        {
            return true;
        }
        virtual void Reset();                               // resets all common properties for battlegrounds, must be implemented and called in BG subclass
        virtual void StartingEventCloseDoors() {}
        virtual void StartingEventOpenDoors() {}
		virtual void OnCreatureCreate(Creature* /*cr*/) {}
		virtual void OnGameObjectCreate(GameObject* /*go*/) {}
		virtual void EventPlayerDamageGO(Player* /*player*/, GameObject* /*target_obj*/, uint32 /*eventId*/) {}

        /* achievement req. */
        virtual bool IsAllNodesConrolledByTeam(uint32 /*team*/) const { return false; }
        bool IsTeamScoreInRange(uint32 team, uint32 minScore, uint32 maxScore) const;

        /* Battleground */
        // Get methods:

        uint32 GetBattlemasterEntry() const;
        uint32 GetBonusHonorFromKill(uint32 kills) const;

        // Set methods:
        void SetTypeID(BattleGroundTypeId TypeID) { }
        void SetRandomTypeID(BattleGroundTypeId TypeID) { }
        //here we can count minlevel and maxlevel for players
        void SetBracket(PvPDifficultyEntry const* bracketEntry);
        
		

        void AddToBGFreeSlotQueue();                        //this queue will be useful when more battlegrounds instances will be available
        void RemoveFromBGFreeSlotQueue();                   //this method could delete whole BG instance, if another free is available

        bool HasFreeSlots();
        uint32 GetFreeSlotsForTeam(uint32 Team) const;

        void StartBattleGround();

        /* Map pointers */
        void SetBgMap(BattleGroundMap* map) { m_Map = map; }
        BattleGroundMap* GetBgMap()
        {
            ASSERT(m_Map);
            return m_Map;
        }

        void SetTeamStartLoc(uint32 TeamID, float X, float Y, float Z, float O);
        void GetTeamStartLoc(uint32 TeamID, float &X, float &Y, float &Z, float &O) const {}

        /* Packet Transfer */
        // method that should fill worldpacket with actual world states (not yet implemented for all battlegrounds!)
        virtual void FillInitialWorldStates(WorldPacket& /*data*/, uint32& /*count*/) {}

        template<class Do>
        void BroadcastWorker(Do& _do);

		void RewardHonorTeamDaily(uint32 TeamID);
        void UpdateWorldState(uint32 Field, uint32 Value,Player *Source = NULL);
        void BlockMovement(Player *plr);

        void SendMessageToAll(int32 entry, ChatMsg type, Player const* source = NULL);
        void SendYellToAll(int32 entry, uint32 language, uint64 const& guid);
        void PSendMessageToAll(int32 entry, ChatMsg type, Player const* source, ...  );

        // specialized version with 2 string id args
        void SendMessage2ToAll(int32 entry, ChatMsg type, Player const* source, int32 strId1 = 0, int32 strId2 = 0);

        /* Raid Group */
        Group *GetBgRaid(uint32 TeamID) const { return TeamID == ALLIANCE ? m_BgRaids[BG_TEAM_ALLIANCE] : m_BgRaids[BG_TEAM_HORDE]; }
        void SetBgRaid(uint32 TeamID, Group *bg_raid);

        virtual void UpdatePlayerScore(Player *Source, uint32 type, uint32 value);

        static BattleGroundTeamId GetTeamIndexByTeamId(uint32 Team) { return Team == ALLIANCE ? BG_TEAM_ALLIANCE : BG_TEAM_HORDE; }
        uint32 GetAlivePlayersCountByTeam(uint32 Team);   // used in arenas to correctly handle death in spirit of redemption / last stand etc. (killer = killed) cases

        // used for rated arena battles
        
        void CheckArenaWinConditions();

        /* Triggers handle */
        // must be implemented in BG subclass
        virtual void HandleAreaTrigger(Player* /*Source*/, uint32 /*Trigger*/) {}
        // must be implemented in BG subclass if need AND call base class generic code
        virtual void HandleKillPlayer(Player *player, Player *killer);
        virtual void HandleKillUnit(Creature* /*unit*/, Player* /*killer*/) { return; };

        /* Battleground events */
        virtual void EventPlayerDroppedFlag(Player* /*player*/) {}
        virtual void EventPlayerClickedOnFlag(Player* /*player*/, GameObject* /*target_obj*/) {}
        virtual void EventPlayerCapturedFlag(Player* /*player*/) {}
        void EventPlayerLoggedIn(Player* player, uint64 plr_guid);
        void EventPlayerLoggedOut(Player* player);

        /* Death related */
        virtual WorldSafeLocsEntry const* GetClosestGraveYard(Player* player);

        virtual void AddPlayer(Player *plr);                // must be implemented in BG subclass

        void AddOrSetPlayerToCorrectBgGroup(Player *plr, uint64 plr_guid, uint32 team);

        virtual void RemovePlayerAtLeave(uint64 guid, bool Transport, bool SendPacket);
                                                            // can be extended in in BG subclass

        /* event related */
        // called when a creature gets added to map (NOTE: only triggered if
        // a player activates the cell of the creature)
        void OnObjectDBLoad(Creature* /*creature*/);
        void OnObjectDBLoad(GameObject* /*obj*/);
        // (de-)spawns creatures and gameobjects from an event
        void SpawnEvent(uint8 event1, uint8 event2, bool spawn);
        
        uint64 GetSingleCreatureGuid(uint8 event1, uint8 event2);

        void OpenDoorEvent(uint8 event1, uint8 event2 = 0);

		/* other things */
        virtual void OnCreatureRespawn(Creature* /*creature*/) {}

        void HandleTriggerBuff(uint64 const& go_guid);

        // TODO: make this protected:
        typedef std::vector<uint64> BGObjects;
        typedef std::vector<uint64> BGCreatures;
        // TODO drop m_BGObjects
        BGObjects m_BgObjects;
		BGCreatures m_BgCreatures;
        void SpawnBGObject(uint64 const& guid, uint32 respawntime);
        bool AddObject(uint32 type, uint32 entry, float x, float y, float z, float o, float rotation0, float rotation1, float rotation2, float rotation3, uint32 respawnTime = 0);
        void SpawnBGCreature(uint64 const& guid, uint32 respawntime);
        bool DelObject(uint32 type);

        void DoorOpen(uint64 const& guid);
        void DoorClose(uint64 const& guid);

        virtual bool HandlePlayerUnderMap(Player * /*plr*/) { return false; }

        // since arenas can be AvA or Hvh, we have to get the "temporary" team of a player
        uint32 GetPlayerTeam(uint64 guid);
        static uint32 GetOtherTeam(uint32 teamId){ return (teamId) ? ((teamId == ALLIANCE) ? HORDE : ALLIANCE) : 0; }
        bool IsPlayerInBattleGround(uint64 guid);

        /* virtual score-array - get's used in bg-subclasses */
        int32 m_TeamScores[BG_TEAMS_COUNT];
  


        // this must be filled first in BattleGroundXY::Reset().. else
        // creatures will get added wrong
        // door-events are automaticly added - but _ALL_ other must be in this vector
        std::map<uint8, uint8> m_ActiveEvents;

		uint32 GetDamageDoneForTeam(uint32 TeamID);

		void UpdateArenaWorldState();
		GameObject* GetBGObject(uint32 type);
		bool DelCreature(uint32 type);
		Creature* AddCreature(uint32 entry, uint32 type, uint32 teamval, float x, float y, float z, float o, uint32 respawntime = 0);
		bool AddSpiritGuide(uint32 type, float x, float y, float z, float o, uint32 team);
		Creature* GetBGCreature(uint32 type);



		/* To handle with cluster */
		void SetInstanceID(uint32 instanceId) { /* TODO : send to server*/ }
		uint32 GetInstanceID() { /* Must Callback from server */return 0; }
		BattleGroundStatus GetStatus() const { return BattleGroundStatus(0);  /* Must Handle With Server */}
		void SetStatus(BattleGroundStatus status) { /* Handle with server*/ }
		void IncreaseInvitedCount(uint32 team) { }
		void DecreaseInvitedCount(uint32 team) { }
		uint32 GetClientInstanceID() { return 0; }
		void SetClientInstanceID(uint32 cId) { }
		uint32 GetStartTime() { return 0; }
		uint32 GetEndTime() { return 0; }
		BattleGroundBracketId GetBracketId() { return BattleGroundBracketId(0); }
		void SetArenaType(uint8 aT) { }
		uint8 GetArenaType() { return 0; }
		void SetArenaorBGType(bool _isArena) { }
		bool isArena() const { return true; }
		bool isBattleGround() { return true; }
		bool GetDeleteThis() { return true; }
		void SetRated(bool rated) { }
		bool isRated()  { return true; }
		uint8 GetWinner() { return 0; }
		void SetMapId(uint32 map) { }
		uint32 GetMapId() const { return 0; }
		bool IsRandomBG() { return true; }
		void SetRandomBG(bool random = true) { }
		void SetBuffChange(bool bChange) { }
		uint32 GetPlayerScoresSize() { return 0; }
		void SendWarningToAll(int32 entry, ...) { }
		void SendWarningToAll(std::string str) { }
		void SetMaxPlayers(uint32 MaxPlayers) { }
        void SetMinPlayers(uint32 MinPlayers) { }
        void SetLevelRange(uint32 min, uint32 max) { }
        void SetMaxPlayersPerTeam(uint32 MaxPlayers) { }
        void SetMinPlayersPerTeam(uint32 MinPlayers) { }
		void SetArenaTeamIdForTeam(uint32 Team, uint32 ArenaTeamId) { }
        uint32 GetArenaTeamIdForTeam(uint32 Team) const { return 0; }
        void SetArenaTeamRatingChangeForTeam(uint32 Team, int32 RatingChange) { }
        int32 GetArenaTeamRatingChangeForTeam(uint32 Team) const { return 0; }
		BattleGroundTypeId GetTypeID(bool GetRandom = false) const { return BattleGroundTypeId(0); }
		uint32 GetMaxPlayers() const        { return 0; }
		uint32 GetMinPlayers() const        { return 0; }
        uint32 GetMinLevel() const          { return 0; }
        uint32 GetMaxLevel() const          { return 0; }
        uint32 GetMaxPlayersPerTeam() const { return 0; }
        uint32 GetMinPlayersPerTeam() const { return 0; }
    protected:
		BattleGroundMap* GetMap() { return m_Map; }

        // must be implemented in BG subclass
        virtual void RemovePlayer(Player * /*player*/, uint64 /*guid*/) {}
    private:
        /* Raid Group */
        Group *m_BgRaids[BG_TEAMS_COUNT];                                // 0 - alliance, 1 - horde

        /* Start location */
        BattleGroundMap* m_Map;
		uint64 m_Id;
};

// helper functions for world state list fill
inline void FillInitialWorldState(ByteBuffer& data, uint32& count, uint32 state, uint32 value)
{
    data << uint32(state);
    data << uint32(value);
    ++count;
}

inline void FillInitialWorldState(ByteBuffer& data, uint32& count, uint32 state, int32 value)
{
    data << uint32(state);
    data << int32(value);
    ++count;
}

inline void FillInitialWorldState(ByteBuffer& data, uint32& count, uint32 state, bool value)
{
    data << uint32(state);
    data << uint32(value?1:0);
    ++count;
}

struct WorldStatePair
{
    uint32 state;
    uint32 value;
};

inline void FillInitialWorldState(ByteBuffer& data, uint32& count, WorldStatePair const* array)
{
    for(WorldStatePair const* itr = array; itr->state; ++itr)
    {
        data << uint32(itr->state);
        data << uint32(itr->value);
        ++count;
    }
}

#endif
