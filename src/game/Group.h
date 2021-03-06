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

#ifndef MANGOSSERVER_GROUP_H
#define MANGOSSERVER_GROUP_H

#include "GroupReference.h"
#include "GroupRefManager.h"
#include "BattleGround.h"
#include "LootMgr.h"
#include "DBCEnums.h"

#include <map>
#include <vector>

#define MAX_GROUP_SIZE 5
#define MAX_RAID_SIZE 40
#define MAX_RAID_SUBGROUPS (MAX_RAID_SIZE / MAX_GROUP_SIZE)
#define TARGET_ICON_COUNT 8
#define GROUP_MAX_LFG_KICKS 3
#define GROUP_LFG_KICK_VOTES_NEEDED 3

enum RollVote
{
    PASS              = 0,
    NEED              = 1,
    GREED             = 2,
    DISENCHANT        = 3,
    NOT_EMITED_YET    = 4,
    NOT_VALID         = 5
};

enum RollVoteMask
{
	ROLL_VOTE_MASK_PASS       = 0x01,
	ROLL_VOTE_MASK_NEED       = 0x02,
	ROLL_VOTE_MASK_GREED      = 0x04,
	ROLL_VOTE_MASK_DISENCHANT = 0x08,
	
	ROLL_VOTE_MASK_ALL        = 0x0F,
	ROLL_VOTE_MASK_NO_NEED    = ROLL_VOTE_MASK_ALL & ~ROLL_VOTE_MASK_NEED,
};

enum LfgDungeonStatus // Merging
{
    LFG_STATUS_SAVED     = 0,
    LFG_STATUS_NOT_SAVED = 1,
    LFG_STATUS_COMPLETE  = 2,
};

enum GroupMemberOnlineStatus
{
    MEMBER_STATUS_OFFLINE   = 0x0000,
    MEMBER_STATUS_ONLINE    = 0x0001,
    MEMBER_STATUS_PVP       = 0x0002,
    MEMBER_STATUS_UNK0      = 0x0004,                       // dead? (health=0)
    MEMBER_STATUS_UNK1      = 0x0008,                       // ghost? (health=1)
    MEMBER_STATUS_UNK2      = 0x0010,                       // never seen
    MEMBER_STATUS_UNK3      = 0x0020,                       // never seen
    MEMBER_STATUS_UNK4      = 0x0040,                       // appears with dead and ghost flags
    MEMBER_STATUS_UNK5      = 0x0080,                       // never seen
};

enum GroupType                                              // group type flags?
{
    GROUPTYPE_NORMAL = 0x00,
    GROUPTYPE_BG     = 0x01,
    GROUPTYPE_RAID   = 0x02,
    GROUPTYPE_BGRAID = GROUPTYPE_BG | GROUPTYPE_RAID,       // mask
	// will show vote kick option, dungeon guide instead of leader
	GROUPTYPE_RANDOM = 0x04,
	// will show LFG icon at minimap, roles at party portrait, possibly only this one for premade lfg group
	GROUPTYPE_LFD = 0x08,
	// mask, party with only random players have this (checked 3.3.x), not sure about premade
	GROUPTYPE_RNDLFD = GROUPTYPE_RANDOM | GROUPTYPE_LFD
};

enum GroupFlagMask
{
	GROUP_ASSISTANT      = 0x01,
	GROUP_MAIN_ASSISTANT = 0x02,
	GROUP_MAIN_TANK      = 0x04,
};

class BattleGround;

enum GroupUpdateFlags
{
    GROUP_UPDATE_FLAG_NONE              = 0x00000000,       // nothing
    GROUP_UPDATE_FLAG_STATUS            = 0x00000001,       // uint16, flags
    GROUP_UPDATE_FLAG_CUR_HP            = 0x00000002,       // uint32
    GROUP_UPDATE_FLAG_MAX_HP            = 0x00000004,       // uint32
    GROUP_UPDATE_FLAG_POWER_TYPE        = 0x00000008,       // uint8
    GROUP_UPDATE_FLAG_CUR_POWER         = 0x00000010,       // uint16
    GROUP_UPDATE_FLAG_MAX_POWER         = 0x00000020,       // uint16
    GROUP_UPDATE_FLAG_LEVEL             = 0x00000040,       // uint16
    GROUP_UPDATE_FLAG_ZONE              = 0x00000080,       // uint16
    GROUP_UPDATE_FLAG_POSITION          = 0x00000100,       // uint16, uint16
    GROUP_UPDATE_FLAG_AURAS             = 0x00000200,       // uint64 mask, for each bit set uint32 spellid + uint8 unk
    GROUP_UPDATE_FLAG_PET_GUID          = 0x00000400,       // uint64 pet guid
    GROUP_UPDATE_FLAG_PET_NAME          = 0x00000800,       // pet name, NULL terminated string
    GROUP_UPDATE_FLAG_PET_MODEL_ID      = 0x00001000,       // uint16, model id
    GROUP_UPDATE_FLAG_PET_CUR_HP        = 0x00002000,       // uint32 pet cur health
    GROUP_UPDATE_FLAG_PET_MAX_HP        = 0x00004000,       // uint32 pet max health
    GROUP_UPDATE_FLAG_PET_POWER_TYPE    = 0x00008000,       // uint8 pet power type
    GROUP_UPDATE_FLAG_PET_CUR_POWER     = 0x00010000,       // uint16 pet cur power
    GROUP_UPDATE_FLAG_PET_MAX_POWER     = 0x00020000,       // uint16 pet max power
    GROUP_UPDATE_FLAG_PET_AURAS         = 0x00040000,       // uint64 mask, for each bit set uint32 spellid + uint8 unk, pet auras...
    GROUP_UPDATE_FLAG_VEHICLE_SEAT      = 0x00080000,       // uint32 vehicle_seat_id (index from VehicleSeat.dbc)
    GROUP_UPDATE_PET                    = 0x0007FC00,       // all pet flags
    GROUP_UPDATE_VEHICLE                = 0x000FFC00,       // all vehicle flags
    GROUP_UPDATE_FULL                   = 0x000FFFFF,       // all known flags
};

#define GROUP_UPDATE_FLAGS_COUNT          20
                                                                // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,11,12,13,14,15,16,17,18,19
static const uint8 GroupUpdateLength[GROUP_UPDATE_FLAGS_COUNT] = { 0, 2, 2, 2, 1, 2, 2, 2, 2, 4, 8, 8, 1, 2, 2, 2, 1, 2, 2, 8};

class InstanceSave;

class Roll : public LootValidatorRef
{
    public:
        Roll(uint64 _guid, LootItem const& li)
            : itemGUID(_guid), itemid(li.itemid), itemRandomPropId(li.randomPropertyId), itemRandomSuffix(li.randomSuffix), itemCount(li.count),
            totalPlayersRolling(0), totalNeed(0), totalGreed(0), totalPass(0), itemSlot(0) {}
        ~Roll() { }
        void setLoot(Loot *pLoot) { link(pLoot, this); }
        Loot *getLoot() { return getTarget(); }
        void targetObjectBuildLink();

        uint64 itemGUID;
        uint32 itemid;
        int32  itemRandomPropId;
        uint32 itemRandomSuffix;
        uint8 itemCount;
        typedef std::map<uint64, RollVote> PlayerVote;
        PlayerVote playerVote;                              //vote position correspond with player position (in group)
        uint8 totalPlayersRolling;
        uint8 totalNeed;
        uint8 totalGreed;
        uint8 totalPass;
        uint8 itemSlot;
};

struct InstanceGroupBind
{
    InstanceSave *save;
    bool perm;
    /* permanent InstanceGroupBinds exist iff the leader has a permanent
       PlayerInstanceBind for the same instance. */
    InstanceGroupBind() : save(NULL), perm(false) {}
};

/** request member stats checken **/
/** todo: uninvite people that not accepted invite **/
class MANGOS_DLL_SPEC Group
{
    public:
        struct MemberSlot
        {
            uint64      guid;
            std::string name;
            uint8       group;
            bool        assistant;
			uint8       flags;
            uint8       roles;
        };
        typedef std::list<MemberSlot> MemberSlotList;
        typedef MemberSlotList::const_iterator member_citerator;

        typedef UNORDERED_MAP< uint32 /*mapId*/, InstanceGroupBind> BoundInstancesMap;
    protected:
        typedef MemberSlotList::iterator member_witerator;
        typedef std::set<Player*> InvitesList;

        typedef std::vector<Roll*> Rolls;

    public:
        Group();
        ~Group();

        // group manipulation methods
        bool   Create(const uint64 &guid, const char * name);
        bool   LoadGroupFromDB(Field *fields);
        bool   LoadMemberFromDB(uint32 guidLow, uint8 subgroup, bool assistant);
        bool   AddInvite(Player *player);
        uint32 RemoveInvite(Player *player);
        void   RemoveAllInvites();
        bool   AddLeaderInvite(Player *player);
        bool   AddMember(const uint64 &guid, const char* name);
		void   SetRandomInstanceGroup(bool random = true) { random ? (m_groupType = GroupType(uint8(m_groupType) + GROUPTYPE_RNDLFD)) : (m_groupType & ~GROUPTYPE_RNDLFD); }
		bool   IsRandomInstanceGroup() { return (m_groupType & GROUPTYPE_RNDLFD); }
		void   SetWGGroup(bool wgg = true) { WGGroup = wgg; }
		bool   IsWGGroup() { return WGGroup; }
                                                            // method: 0=just remove, 1=kick
        uint32 RemoveMember(const uint64 &guid, const uint8 &method);
        void   ChangeLeader(const uint64 &guid);
        void   SetLootMethod(LootMethod method) { m_lootMethod = method; }
        void   SetLooterGuid(const uint64 &guid) { m_looterGuid = guid; }
        void   UpdateLooterGuid( WorldObject* object, bool ifneed = false );
        void   SetLootThreshold(ItemQualities threshold) { m_lootThreshold = threshold; }
        void   Disband(bool hideDestroy=false);

		// Dungeon Finder //Merging
        void   SetLfgQueued(bool queued) { m_LfgQueued = queued; }
        bool   isLfgQueued() { return m_LfgQueued; }
        void   SetLfgStatus(uint8 status) { m_LfgStatus = status; }
        uint8  GetLfgStatus() { return m_LfgStatus; }
        bool   isLfgDungeonComplete() const { return m_LfgStatus == LFG_STATUS_COMPLETE; }
        void   SetLfgDungeonEntry(uint32 dungeonEntry) { m_LfgDungeonEntry = dungeonEntry; }
        uint32 GetLfgDungeonEntry(bool id = true)
        {
            if (id)
                return (m_LfgDungeonEntry & 0x00FFFFFF);
            else
                return m_LfgDungeonEntry;
        }
        bool   isLfgKickActive() const { return m_LfgkicksActive; }
        void   SetLfgKickActive(bool active) { m_LfgkicksActive = active; }
        uint8  GetLfgKicks() const { return m_Lfgkicks; }
        void   SetLfgKicks(uint8 kicks) { m_Lfgkicks = kicks; }
        void   SetLfgRoles(uint64 guid, const uint8 roles)
        {
            member_witerator slot = _getMemberWSlot(guid);
            if (slot == m_memberSlots.end())
                return;

            slot->roles = roles;
            SendUpdate();
        }
		bool isLFGGroup()  const { return m_groupType & GROUPTYPE_LFD; }

        // properties accessories
		uint32 GetId() const { return m_Id; }
        bool IsFull() const { return (m_groupType == GROUPTYPE_NORMAL) ? (m_memberSlots.size() >= MAX_GROUP_SIZE) : (m_memberSlots.size() >= MAX_RAID_SIZE); }
		bool isRaidGroup() const { return m_groupType & GROUPTYPE_RAID; }
        bool isBGGroup()   const { return m_bgGroup != NULL; }
        bool IsCreated()   const { return GetMembersCount() > 0; }
        const uint64& GetLeaderGUID() const { return m_leaderGuid; }
        const char * GetLeaderName() const { return m_leaderName.c_str(); }
        const uint64& GetGUID() const { return m_guid; }
		uint32 GetLowGUID() const { return GUID_LOPART(m_guid); }
        LootMethod    GetLootMethod() const { return m_lootMethod; }
        const uint64& GetLooterGuid() const { return m_looterGuid; }
        ItemQualities GetLootThreshold() const { return m_lootThreshold; }

        // member manipulation methods
        bool IsMember(const uint64& guid) const { return _getMemberCSlot(guid) != m_memberSlots.end(); }
        bool IsLeader(const uint64& guid) const { return (GetLeaderGUID() == guid); }
        uint64 GetMemberGUID(const std::string& name)
        {
            for(member_citerator itr = m_memberSlots.begin(); itr != m_memberSlots.end(); ++itr)
            {
                if(itr->name == name)
                {
                    return itr->guid;
                }
            }
            return 0;
        }
        bool IsAssistant(uint64 guid) const
        {
            member_citerator mslot = _getMemberCSlot(guid);
            if(mslot==m_memberSlots.end())
                return false;

            return mslot->assistant;
        }
        Player* GetInvited(const uint64& guid) const;
        Player* GetInvited(const std::string& name) const;

        bool SameSubGroup(uint64 guid1,const uint64& guid2) const
        {
            member_citerator mslot2 = _getMemberCSlot(guid2);
            if(mslot2==m_memberSlots.end())
                return false;

            return SameSubGroup(guid1,&*mslot2);
        }

        bool SameSubGroup(uint64 guid1, MemberSlot const* slot2) const
        {
            member_citerator mslot1 = _getMemberCSlot(guid1);
            if(mslot1==m_memberSlots.end() || !slot2)
                return false;

            return (mslot1->group==slot2->group);
        }

        bool HasFreeSlotSubGroup(uint8 subgroup) const
        {
            return (m_subGroupsCounts && m_subGroupsCounts[subgroup] < MAX_GROUP_SIZE);
        }

        bool SameSubGroup(Player const* member1, Player const* member2) const;

        MemberSlotList const& GetMemberSlots() const { return m_memberSlots; }
        GroupReference* GetFirstMember() { return m_memberMgr.getFirst(); }
        uint32 GetMembersCount() const { return m_memberSlots.size(); }
        void GetDataForXPAtKill(Unit const* victim, uint32& count,uint32& sum_level, Player* & member_with_max_level, Player* & not_gray_member_with_max_level);
        uint8  GetMemberGroup(uint64 guid) const
        {
            member_citerator mslot = _getMemberCSlot(guid);
            if(mslot==m_memberSlots.end())
                return MAX_RAID_SUBGROUPS+1;

            return mslot->group;
        }

        // some additional raid methods
		void ConvertToLFG(); // Merging
        void ConvertToRaid();

        void SetBattlegroundGroup(BattleGround *bg) { m_bgGroup = bg; }
        GroupJoinBattlegroundResult  CanJoinBattleGroundQueue(BattleGround const* bgOrTemplate, BattleGroundQueueTypeId bgQueueTypeId, uint32 MinPlayerCount, uint32 MaxPlayerCount, bool isRated, uint32 arenaSlot);

        void ChangeMembersGroup(const uint64 &guid, uint8 group);
        void ChangeMembersGroup(Player *player, uint8 group);

        void SetAssistant(uint64 guid, bool state)
        {
            if(!isRaidGroup())
                return;
            if(_setAssistantFlag(guid, state))
                SendUpdate();
        }
        void SetMainTank(uint64 guid)
        {
            if(!isRaidGroup())
                return;

            if(_setMainTank(guid))
                SendUpdate();
        }
        void SetMainAssistant(uint64 guid)
        {
            if(!isRaidGroup())
                return;

            if(_setMainAssistant(guid))
                SendUpdate();
        }

		void SetTargetIcon(uint8 id, uint64 whoGuid, uint64 targetGuid);

        Difficulty GetDifficulty(bool isRaid) const { return isRaid ? m_raidDifficulty : m_dungeonDifficulty; }
        Difficulty GetDungeonDifficulty() const { return m_dungeonDifficulty; }
        Difficulty GetRaidDifficulty() const { return m_raidDifficulty; }
        void SetDungeonDifficulty(Difficulty difficulty);
        void SetRaidDifficulty(Difficulty difficulty);
        uint16 InInstance();
        bool InCombatToInstance(uint32 instanceId, bool bossOnly = false);
        void ResetInstances(uint8 method, bool isRaid, Player* SendMsgTo);

        // -no description-
        //void SendInit(WorldSession *session);
        void SendTargetIconList(WorldSession *session);
        void SendUpdate();
        void UpdatePlayerOutOfRange(Player* pPlayer);
                                                            // ignore: GUID of player that will be ignored
        void BroadcastPacket(WorldPacket *packet, bool ignorePlayersInBGRaid, int group=-1, uint64 ignore=0);
        void BroadcastReadyCheck(WorldPacket *packet);
        void OfflineReadyCheck();

        /*********************************************************/
        /***                   LOOT SYSTEM                     ***/
        /*********************************************************/

        void SendLootStartRoll(uint32 CountDown, uint32 mapid, const Roll &r);
        void SendLootRoll(const uint64& SourceGuid, const uint64& TargetGuid, uint8 RollNumber, uint8 RollType, const Roll &r);
        void SendLootRollWon(const uint64& SourceGuid, const uint64& TargetGuid, uint8 RollNumber, uint8 RollType, const Roll &r);
        void SendLootAllPassed(uint32 NumberOfPlayers, const Roll &r);
        void GroupLoot(const uint64& playerGUID, Loot *loot, WorldObject* object);
        void NeedBeforeGreed(const uint64& playerGUID, Loot *loot, WorldObject* object);
        void MasterLoot(const uint64& playerGUID, Loot *loot, WorldObject* object);
        Rolls::iterator GetRoll(uint64 Guid)
        {
            Rolls::iterator iter;
            for (iter=RollId.begin(); iter != RollId.end(); ++iter)
            {
                if ((*iter)->itemGUID == Guid && (*iter)->isValid())
                {
                    return iter;
                }
            }
            return RollId.end();
        }
        void CountTheRoll(Rolls::iterator roll, uint32 NumberOfPlayers);
        bool CountRollVote(const uint64& playerGUID, const uint64& Guid, uint32 NumberOfPlayers, uint8 vote);
        void EndRoll();

        void LinkMember(GroupReference *pRef) { m_memberMgr.insertFirst(pRef); }
        void DelinkMember(GroupReference* /*pRef*/ ) { }

        InstanceGroupBind* BindToInstance(InstanceSave *save, bool permanent, bool load = false);
        void UnbindInstance(uint32 mapid, uint8 difficulty, bool unload = false);
        InstanceGroupBind* GetBoundInstance(uint32 mapId, Player* player);
        InstanceGroupBind* GetBoundInstance(Map* aMap, Difficulty difficulty);
        BoundInstancesMap& GetBoundInstances(Difficulty difficulty) { return m_boundInstances[difficulty]; }

    protected:
        bool _addMember(const uint64 &guid, const char* name, bool isAssistant=false);
        bool _addMember(const uint64 &guid, const char* name, bool isAssistant, uint8 group);
        bool _removeMember(const uint64 &guid);             // returns true if leader has changed
        void _setLeader(const uint64 &guid);

        void _removeRolls(const uint64 &guid);

        bool _setMembersGroup(const uint64 &guid, uint8 group);
        bool _setAssistantFlag(const uint64 &guid, const bool &state);
        bool _setMainTank(const uint64 &guid);
        bool _setMainAssistant(const uint64 &guid);

        void _homebindIfInstance(Player *player);

        void _initRaidSubGroupsCounter()
        {
            // Sub group counters initialization
            if (!m_subGroupsCounts)
                m_subGroupsCounts = new uint8[MAX_RAID_SUBGROUPS];

            memset((void*)m_subGroupsCounts, 0, MAX_RAID_SUBGROUPS*sizeof(uint8));

            for (member_citerator itr = m_memberSlots.begin(); itr != m_memberSlots.end(); ++itr)
                ++m_subGroupsCounts[itr->group];
        }

        member_citerator _getMemberCSlot(uint64 Guid) const
        {
            for(member_citerator itr = m_memberSlots.begin(); itr != m_memberSlots.end(); ++itr)
            {
                if (itr->guid == Guid)
                    return itr;
            }
            return m_memberSlots.end();
        }

        member_witerator _getMemberWSlot(uint64 Guid)
        {
            for(member_witerator itr = m_memberSlots.begin(); itr != m_memberSlots.end(); ++itr)
            {
                if (itr->guid == Guid)
                    return itr;
            }
            return m_memberSlots.end();
        }

        void SubGroupCounterIncrease(uint8 subgroup)
        {
            if (m_subGroupsCounts)
                ++m_subGroupsCounts[subgroup];
        }

        void SubGroupCounterDecrease(uint8 subgroup)
        {
            if (m_subGroupsCounts)
                --m_subGroupsCounts[subgroup];
        }
        
        GroupFlagMask GetFlags(MemberSlot const& slot) const
        {
			uint8 flags = 0;
			if (slot.assistant)
				flags |= GROUP_ASSISTANT;
				
			if (slot.guid == m_mainAssistant)
				flags |= GROUP_MAIN_ASSISTANT;
				
			if (slot.guid == m_mainTank)
				flags |= GROUP_MAIN_TANK;
					
			return GroupFlagMask(flags);
		}

		uint32              m_Id;                           // 0 for not created or BG groups
        MemberSlotList      m_memberSlots;
        GroupRefManager     m_memberMgr;
        InvitesList         m_invitees;
        uint64              m_leaderGuid;
        std::string         m_leaderName;
        uint64              m_mainTank;
        uint64              m_mainAssistant;
        GroupType           m_groupType;
        Difficulty          m_dungeonDifficulty;
        Difficulty          m_raidDifficulty;
        BattleGround*       m_bgGroup;
        uint64              m_targetIcons[TARGET_ICON_COUNT];
        LootMethod          m_lootMethod;
        ItemQualities       m_lootThreshold;
        uint64              m_looterGuid;
        Rolls               RollId;
        BoundInstancesMap   m_boundInstances[MAX_DIFFICULTY];
        uint8*              m_subGroupsCounts;
		bool				WGGroup;

		uint64              m_guid; // Add LFG Support
		bool                m_LfgQueued;
        uint8               m_LfgStatus;
        uint32              m_LfgDungeonEntry;
        uint8               m_Lfgkicks;
        bool                m_LfgkicksActive;
};
#endif
