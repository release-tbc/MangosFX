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
SDName: Boss_Shade_of_Akama
SD%Complete: 85
SDComment: Seems to be complete. Some little details/cosmetics left (see next comment section).
SDCategory: Black Temple
EndScriptData */

/* ToDo:
(1) After start event Akama should walk a bit towards Shade of Akama, then stop (between the two pillars) and begin to channel.
(2) Some minor changes to post event (after killing Shade of Akama):
(2.1) After Shade of Akama is dead Akama should roar in direction to the door (he must turn around if he reached the stage).
(2.2) Positioning of broken NPCs.
(3) The channelers are casting their spell somestimes even if they are daed (move out of view distance and then move in - they are dead but they channel - maybe some clientspecific issue?).
(4) Unbanish Shade of Akama if a ashtongue sorcerer is spawned but not reached Shade of Akama and channels his spell?
*/

#include "precompiled.h"
#include "black_temple.h"

#define GOSSIP_ITEM                 "We are ready to fight alongside you, Akama"

// Spells
enum
{
    SAY_DEATH                       = -1564013,
    SAY_LOW_HEALTH                  = -1564014,
    // Ending cinematic text
    SAY_FREE                        = -1564015,
    SAY_BROKEN_FREE_01              = -1564016,
    SAY_BROKEN_FREE_02              = -1564017,

    SPELL_VERTEX_SHADE_BLACK        = 39833,
    SPELL_SHADE_SOUL_CHANNEL        = 40401,
    SPELL_DESTRUCTIVE_POISON        = 40874,
    SPELL_LIGHTNING_BOLT            = 42024,
    SPELL_AKAMA_SOUL_CHANNEL        = 40447,
    SPELL_AKAMA_SOUL_RETRIEVE       = 40902,

    NPC_AKAMA                       = 22990,
    NPC_ASH_CHANNELER               = 23421,
    NPC_ASH_SORCERER                = 23215,
    NPC_ASH_DEFENDER                = 23216,
    NPC_ASH_BROKEN                  = 23319,
    NPC_ASH_ELEMENTAL               = 23523,
    NPC_ASH_ROGUE                   = 23318,
    NPC_ASH_SPIRITBIND              = 23524,

    //akama's phases (used as point id's)
    //PHASE_CHANNEL                   = 1,
    //PHASE_BELOW_PLATFORM            = 2,
    //PHASE_ON_PLATFORM               = 3
};

const uint32 m_auiRandSpawnEntry[]=
{
    NPC_ASH_ELEMENTAL,
    NPC_ASH_ROGUE,
    NPC_ASH_SPIRITBIND
};

const float LOC_RAND_TO_CENTER_X        = 482.793182;
const float LOC_RAND_TO_CENTER_Y        = 401.270172;
const float LOC_RAND_TO_CENTER_Z        = 112.783928;

const float LOC_PLATFORM_Z              = 118.537;
const float LOC_LOW_Z                   = 112.784;

struct Location
{
    float m_fX, m_fY, m_fZ, m_fO;
};

Location m_afSpawnLoc[]=
{
    {498.652740, 461.728119, LOC_LOW_Z, 0.0},
    {498.505003, 339.619324, LOC_LOW_Z, 0.0}
};

Location m_afAkamaWP[]=
{
    //{516.885193, 400.836060, LOC_LOW_Z_SPAWN, 0.0},       //not used yet, he moves to here before start channel
    {482.352448, 401.162720, LOC_LOW_Z, 0.0},
    {469.597443, 402.264404, LOC_PLATFORM_Z, 0.0}
};

Location m_afBrokenSpawnLoc[]=
{
    {541.375916, 401.439575, LOC_LOW_Z, M_PI},              // The place where Akama channels
    {534.130005, 352.394531, LOC_LOW_Z, 2.164150},          // Behind a 'pillar' which is behind the east alcove
    {499.621185, 341.534729, LOC_LOW_Z, 1.652856},          // East Alcove
    {499.151093, 461.036438, LOC_LOW_Z, 4.770888}           // West Alcove
};

Location m_afBrokenWP[]=
{
    {492.491638, 400.744690, LOC_LOW_Z, 3.122336},
    {494.335724, 382.221771, LOC_LOW_Z, 2.676230},
    {489.555939, 373.507202, LOC_LOW_Z, 2.416263},
    {491.136353, 427.868774, LOC_LOW_Z, 3.519748}
};

struct MANGOS_DLL_DECL boss_shade_of_akamaAI : public ScriptedAI
{
    boss_shade_of_akamaAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = pCreature->GetInstanceData();
        m_lChannelersGUIDList.clear();
        m_lSorcerersGUIDList.clear();
        Reset();
    }

    std::list<uint64> m_lChannelersGUIDList;
    std::list<uint64> m_lSorcerersGUIDList;

    uint64 m_uiAkamaGUID;

    uint32 m_uiSorcererCount;
    uint32 m_uiDeathCount;

    uint32 m_uiReduceHealthTimer;
    uint32 m_uiSummonTimer;
    uint32 m_uiResetTimer;
    uint32 m_uiDefenderTimer;                                   // They are on a flat 15 second timer, independant of the other summon creature timer.

    bool m_bIsBanished;
    bool m_bHasKilledAkama;

    void Reset()
    {
        m_uiSorcererCount = 0;
        m_uiDeathCount = 0;

        m_uiSummonTimer = 10000;
        m_uiReduceHealthTimer = 0;
        m_uiResetTimer = 60000;
        m_uiDefenderTimer = 15000;

        m_bIsBanished = true;
        m_bHasKilledAkama = false;

        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

        me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_STUN);
    }

    void AttackStart(Unit* pWho)
    {
        if (!pWho || m_bIsBanished)
            return;

        ScriptedAI::AttackStart(pWho);
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (m_bIsBanished)
            return;

        ScriptedAI::MoveInLineOfSight(pWho);
    }

    void JustReachedHome()
    {
        if (pInstance)
            SetInstanceData(TYPE_SHADE, NOT_STARTED);

        RespawnChannelersIfDeadOrEvade();
    }

    void IncrementDeathCount(uint64 uiGuid = 0)               // If guid is set, will remove it from list of sorcerer
    {
        debug_log("SD2: Increasing Death Count for Shade of Akama encounter");
        ++m_uiDeathCount;

        if (uiGuid)
        {
            if (m_lSorcerersGUIDList.empty())
                error_log("SD2: boss_shade_of_akamaAI attempt to remove guid %u from Sorcerers list but list is already empty", uiGuid);
            else
                m_lSorcerersGUIDList.remove(uiGuid);
        }
    }

    void SummonCreature()
    {
        uint32 uiRand = sizeof(m_afSpawnLoc)/sizeof(Location);

        // max of 6 sorcerers can be summoned
        if (!urand(0, 2) && (m_uiDeathCount > 0) && (m_uiSorcererCount < 7))
        {
            if (Creature* pSorcerer = me->SummonCreature(NPC_ASH_SORCERER,
                m_afSpawnLoc[uiRand].m_fX, m_afSpawnLoc[uiRand].m_fY, m_afSpawnLoc[uiRand].m_fZ, m_afSpawnLoc[uiRand].m_fO,
                TEMPSUMMON_DEAD_DESPAWN, 0))
            {
                pSorcerer->RemoveSplineFlag(SPLINEFLAG_WALKMODE);
                pSorcerer->GetMotionMaster()->MovePoint(0, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                pSorcerer->SetUInt64Value(UNIT_FIELD_TARGET, me->GetGUID());

                m_lSorcerersGUIDList.push_back(pSorcerer->GetGUID());

                --m_uiDeathCount;
                ++m_uiSorcererCount;
            }
        }
        else
        {
            int iSize = (sizeof(m_auiRandSpawnEntry) / sizeof(uint32));

            for(uint8 i = 0; i < iSize; ++i)
            {
                if (Creature* pSpawn = me->SummonCreature(m_auiRandSpawnEntry[i],
                    m_afSpawnLoc[uiRand].m_fX, m_afSpawnLoc[uiRand].m_fY, m_afSpawnLoc[uiRand].m_fZ, m_afSpawnLoc[uiRand].m_fO,
                    TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 25000))
                {
                    pSpawn->RemoveSplineFlag(SPLINEFLAG_WALKMODE);
                    pSpawn->GetMotionMaster()->MovePoint(0, LOC_RAND_TO_CENTER_X, LOC_RAND_TO_CENTER_Y, LOC_RAND_TO_CENTER_Z);
                }
            }
        }
    }

    void DespawnSorceres()
    {
        if (!m_lSorcerersGUIDList.empty() && pInstance)
        {
            for(std::list<uint64>::iterator itr = m_lSorcerersGUIDList.begin(); itr != m_lSorcerersGUIDList.end(); ++itr)
            {
                if (Creature* pSorcerer = pInstance->instance->GetCreature(*itr))
                {
                    if (pSorcerer->isAlive())
                        pSorcerer->ForcedDespawn();
                }
            }
        }
    }

    void RespawnChannelersIfDeadOrEvade()
    {
        if (!m_lChannelersGUIDList.empty() && pInstance)
        {
            for(std::list<uint64>::iterator itr = m_lChannelersGUIDList.begin(); itr != m_lChannelersGUIDList.end(); ++itr)
            {
                if (Creature* pChanneler = pInstance->instance->GetCreature(*itr))
                {
                    if (!pChanneler->isAlive())
                        pChanneler->Respawn();
                    else
                        pChanneler->AI()->EnterEvadeMode();
                }
            }
        }
        else
            error_log("SD2: boss_shade_of_akamaAI not able to respawn channelers, list is empty.");
    }

    void PrepareChannelers()
    {
        std::list<Creature*> lChannelerList;
        GetCreatureListWithEntryInGrid(lChannelerList,me, NPC_ASH_CHANNELER, 50.0f);

        if (!lChannelerList.empty())
        {
            //clear this, we want a clean start
            m_lChannelersGUIDList.clear();

            for(std::list<Creature*>::iterator itr = lChannelerList.begin(); itr != lChannelerList.end(); ++itr)
            {
                m_lChannelersGUIDList.push_back((*itr)->GetGUID());
                debug_log("SD2: boss_shade_of_akamaAI found channeler %u. Adding to list", (*itr)->GetGUID());

                (*itr)->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            }
        }
        else
            error_log("SD2: boss_shade_of_akamaAI unable to find any channelers.");
    }

    void KilledUnit(Unit* pVictim)
    {
        if (pVictim->GetEntry() == NPC_AKAMA)
            EnterEvadeMode();
    }

    void JustDied(Unit* pKiller)
    {
        if (pInstance)
            SetInstanceData(TYPE_SHADE, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!me->isInCombat())
            return;

        if (m_bIsBanished)
        {
            // Akama is set in the threatlist so when we reset, we make sure that he is not included in our check
            if (me->getThreatManager().getThreatList().size() < 2)
                ScriptedAI::EnterEvadeMode();

            if (m_uiDefenderTimer < diff)
            {
                uint32 uiRand = sizeof(m_afSpawnLoc)/sizeof(Location);

                if (Creature* pDefender = me->SummonCreature(NPC_ASH_DEFENDER,
                    m_afSpawnLoc[uiRand].m_fX, m_afSpawnLoc[uiRand].m_fY, m_afSpawnLoc[uiRand].m_fZ, m_afSpawnLoc[uiRand].m_fO,
                    TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 25000))
                {
                    if (Unit* pAkama = Unit::GetUnit(*me, pInstance->GetData64(DATA_AKAMA_SHADE)))
                        pDefender->AI()->AttackStart(pAkama);
                }

                m_uiDefenderTimer = 15000;
            }
            else
                m_uiDefenderTimer -= diff;

            if (m_uiSummonTimer < diff)
            {
                SummonCreature();
                m_uiSummonTimer = 35000;
            }
            else
                m_uiSummonTimer -= diff;

            if (m_uiDeathCount >= 6)
            {
                if (Unit* pAkama = Unit::GetUnit(*me, pInstance->GetData64(DATA_AKAMA_SHADE)))
                {
                    if (pAkama && pAkama->isAlive())
                    {
                        m_bIsBanished = false;

                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                        // Shade should move to Akama, not the other way around
                        AttackStart(pAkama);

                        // Crazy amount of threat
                        me->AddThreat(pAkama, 10000000.0f);
                        pAkama->AddThreat(me, 10000000.0f);
                    }
                }
            }
        }
        else                                                // No longer banished, let's fight Akama now
        {
            if (m_uiReduceHealthTimer < diff)
            {
                if (Unit* pAkama = Unit::GetUnit(*me, pInstance->GetData64(DATA_AKAMA_SHADE)))
                {
                    if (pAkama->isAlive())
                    {
                        // 10 % less health every few seconds.
                        me->DealDamage(pAkama, pAkama->GetMaxHealth()/10, NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                        m_uiReduceHealthTimer = 12000;
                    }
                    else
                    {
                        m_bHasKilledAkama = true;           // Akama is dead, we stop fighting and disappear
                        EnterEvadeMode();
                        return;
                    }
                }
            }
            else
                m_uiReduceHealthTimer -= diff;

            if (m_bHasKilledAkama)
            {
                if (m_uiResetTimer < diff)
                {
                    EnterEvadeMode();                       // Reset a little while after killing Akama
                    return;
                }
                else
                    m_uiResetTimer -= diff;
            }

            DoMeleeAttackIfReady();
        }
    }
};

struct MANGOS_DLL_DECL npc_akamaAI : public ScriptedAI
{
    npc_akamaAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_bIsShadeDead = false;
        m_bCanStartCombat = false;
        pInstance = pCreature->GetInstanceData();
        Reset();
    }

    uint32 m_uiDestructivePoisonTimer;
    uint32 m_uiLightningBoltTimer;
    uint32 m_uiCheckTimer;
    uint32 m_uiCastSoulRetrieveTimer;
    uint32 m_uiSoulRetrieveTimer;
    uint32 m_uiSummonBrokenTimer;
    uint32 m_uiEndingTalkCount;
    uint32 m_uiWayPointId;
    uint32 m_uiBrokenSummonIndex;

    std::list<uint64> m_lBrokenGUIDList;

    bool m_bIsEventBegun;
    bool m_bIsShadeDead;
    bool m_bCanStartCombat;
    bool m_bHasYelledOnce;

    void Reset()
    {
        SetCombatMovement(false);

        m_uiDestructivePoisonTimer = 15000;
        m_uiLightningBoltTimer = 10000;
        m_uiCheckTimer = 2000;
        m_uiCastSoulRetrieveTimer = 0;
        m_uiSoulRetrieveTimer = 0;
        m_uiSummonBrokenTimer = 0;
        m_uiEndingTalkCount = 0;
        m_uiWayPointId = 0;
        m_uiBrokenSummonIndex = 0;

        m_lBrokenGUIDList.clear();

        m_bIsEventBegun = false;
        m_bHasYelledOnce = false;

        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    }

    void BeginEvent()
    {
        if (!pInstance)
            return;

        if (Creature* pShade = pInstance->instance->GetCreature(pInstance->GetData64(DATA_SHADEOFAKAMA)))
        {
            if (boss_shade_of_akamaAI* pShadeAI = dynamic_cast<boss_shade_of_akamaAI*>(pShade->AI()))
                pShadeAI->PrepareChannelers();

            // Prevent players from trying to restart event
            SetInstanceData(TYPE_SHADE, IN_PROGRESS);

            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

            pShade->AddThreat(me, 1000000.0f);
            pShade->SetInCombatWith(me);
            me->SetInCombatWith(pShade);

            pShade->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_NONE);
            pShade->SetUInt64Value(UNIT_FIELD_TARGET, me->GetGUID());

            pShade->SetInCombatWithZone();

            m_bIsEventBegun = true;
        }
    }

    void MovementInform(uint32 uiMoveType, uint32 uiPointId)
    {
        if (uiMoveType != POINT_MOTION_TYPE || !pInstance)
            return;

        switch(uiPointId)
        {
            case 0:
                ++m_uiWayPointId;
                break;
            case 1:
                if (Unit* pShade = Unit::GetUnit(*me, pInstance->GetData64(DATA_SHADEOFAKAMA)))
                {
                    DoCast(pShade, SPELL_AKAMA_SOUL_RETRIEVE);
                    m_uiEndingTalkCount = 0;
                    m_uiSoulRetrieveTimer = 16000;
                }
                break;
        }
    }

    void JustDied(Unit* pKiller)
    {
        DoScriptText(SAY_DEATH, me);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_bIsEventBegun || !pInstance)
            return;

        if (!m_bCanStartCombat)
        {
            if (Creature* pShade = pInstance->instance->GetCreature(pInstance->GetData64(DATA_SHADEOFAKAMA)))
            {
                if (!pShade->isAlive())
                {
                    EnterEvadeMode();
                    return;
                }

                if (boss_shade_of_akamaAI* pShadeAI = dynamic_cast<boss_shade_of_akamaAI*>(pShade->AI()))
                {
                    if (pShadeAI->m_bIsBanished)
                    {
                        if (m_uiCastSoulRetrieveTimer < diff)
                        {
                            DoCast(pShade, SPELL_AKAMA_SOUL_CHANNEL);
                            m_uiCastSoulRetrieveTimer = 500;
                        }
                        else
                            m_uiCastSoulRetrieveTimer -= diff;
                    }
                    else
                    {
                        me->InterruptNonMeleeSpells(false);
                        m_bCanStartCombat = true;
                    }
                }
            }
        }

        if (m_bIsShadeDead && (m_uiWayPointId == 1))
        {
            me->GetMotionMaster()->MovePoint(m_uiWayPointId, m_afAkamaWP[1].m_fX, m_afAkamaWP[1].m_fY, m_afAkamaWP[1].m_fZ);
            ++m_uiWayPointId;
        }

        if (!m_bIsShadeDead && m_bCanStartCombat)
        {
            if (m_uiCheckTimer < diff)
            {
                if (Unit* pShade = Unit::GetUnit(*me, pInstance->GetData64(DATA_SHADEOFAKAMA)))
                {
                    if (!pShade->isAlive())
                    {
                        m_bIsShadeDead = true;
                        m_uiWayPointId = 0;
                        me->AddSplineFlag(SPLINEFLAG_WALKMODE);
                        me->GetMotionMaster()->MovePoint(m_uiWayPointId, m_afAkamaWP[0].m_fX, m_afAkamaWP[0].m_fY, m_afAkamaWP[0].m_fZ);
                    }
                }
                m_uiCheckTimer = 5000;
            }
            else
                m_uiCheckTimer -= diff;
        }

        if (m_uiSummonBrokenTimer && m_uiBrokenSummonIndex < 4)
        {
            if (m_uiSummonBrokenTimer <= diff)
            {
                for(uint8 i = 0; i < 4; ++i)
                {
                    float x = m_afBrokenSpawnLoc[m_uiBrokenSummonIndex].m_fX + (i*5);
                    float y = m_afBrokenSpawnLoc[m_uiBrokenSummonIndex].m_fY + (1*5);
                    float z = m_afBrokenSpawnLoc[m_uiBrokenSummonIndex].m_fZ;
                    float o = m_afBrokenSpawnLoc[m_uiBrokenSummonIndex].m_fO;

                    if (Creature* pBroken = me->SummonCreature(NPC_ASH_BROKEN, x, y, z, o, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 360000))
                    {
                        float wx = m_afBrokenWP[m_uiBrokenSummonIndex].m_fX + (i*5);
                        float wy = m_afBrokenWP[m_uiBrokenSummonIndex].m_fY + (i*5);
                        float wz = m_afBrokenWP[m_uiBrokenSummonIndex].m_fZ;

                        pBroken->GetMotionMaster()->MovePoint(0, wx, wy, wz);
                        pBroken->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

                        m_lBrokenGUIDList.push_back(pBroken->GetGUID());
                    }
                }

                ++m_uiBrokenSummonIndex;
                m_uiSummonBrokenTimer = 1000;
            }
            else
                m_uiSummonBrokenTimer -= diff;
        }

        if (m_uiSoulRetrieveTimer)
        {
            if (m_uiSoulRetrieveTimer <= diff)
            {
                switch(m_uiEndingTalkCount)
                {
                    case 0:
                        me->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
                        ++m_uiEndingTalkCount;
                        m_uiSoulRetrieveTimer = 2000;
                        m_uiSummonBrokenTimer = 1;
                        break;
                    case 1:
                        DoScriptText(SAY_FREE, me);
                        ++m_uiEndingTalkCount;
                        m_uiSoulRetrieveTimer = 25000;
                        break;
                    case 2:
                        if (!m_lBrokenGUIDList.empty())
                        {
                            bool bYelled = false;

                            for(std::list<uint64>::iterator itr = m_lBrokenGUIDList.begin(); itr != m_lBrokenGUIDList.end(); ++itr)
                            {
                                if (Unit* pUnit = Unit::GetUnit(*me, *itr))
                                {
                                    if (!bYelled)
                                    {
                                        DoScriptText(SAY_BROKEN_FREE_01, pUnit);
                                        bYelled = true;
                                    }
                                    pUnit->HandleEmoteCommand(EMOTE_ONESHOT_KNEEL);
                                }
                            }
                        }
                        ++m_uiEndingTalkCount;
                        m_uiSoulRetrieveTimer = 1500;
                        break;
                    case 3:
                        if (!m_lBrokenGUIDList.empty())
                        {
                            for(std::list<uint64>::iterator itr = m_lBrokenGUIDList.begin(); itr != m_lBrokenGUIDList.end(); ++itr)
                            {
                                // This is the incorrect spell, but can't seem to find the right one.
                                if (Unit* pUnit = Unit::GetUnit(*me, *itr))
                                    pUnit->CastSpell(pUnit, 39656, true);
                            }
                        }
                        ++m_uiEndingTalkCount;
                        m_uiSoulRetrieveTimer = 5000;
                        break;
                    case 4:
                        if (!m_lBrokenGUIDList.empty())
                        {
                            for(std::list<uint64>::iterator itr = m_lBrokenGUIDList.begin(); itr != m_lBrokenGUIDList.end(); ++itr)
                            {
                                if (Unit* pUnit = Unit::GetUnit((*me), *itr))
                                    DoScriptText(SAY_BROKEN_FREE_02, pUnit);
                            }
                        }
                        m_uiSoulRetrieveTimer = 0;
                        break;
                }
            }
            else
                m_uiSoulRetrieveTimer -= diff;
        }

        if (!me->getVictim() || !me->SelectHostileTarget())
            return;

        if (!m_bHasYelledOnce && (me->GetHealth()*100 / me->GetMaxHealth()) < 15)
        {
            DoScriptText(SAY_LOW_HEALTH, me);
            m_bHasYelledOnce = true;
        }

        if (m_uiDestructivePoisonTimer < diff)
        {
            if (Unit* pShade = Unit::GetUnit(*me, pInstance->GetData64(DATA_SHADEOFAKAMA)))
            {
                if (pShade->isAlive())
                    DoCast(pShade, SPELL_DESTRUCTIVE_POISON);
            }

            m_uiDestructivePoisonTimer = 15000;
        }
        else
            m_uiDestructivePoisonTimer -= diff;

        if (m_uiLightningBoltTimer < diff)
        {
            DoCastVictim( SPELL_LIGHTNING_BOLT);
            m_uiLightningBoltTimer = 10000;
        }
        else
            m_uiLightningBoltTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

bool GossipHello_npc_akama(Player* pPlayer, Creature* pCreature)
{
    if (InstanceData* pInstance = pCreature->GetInstanceData())
    {
        if (pInstance->GetData(TYPE_SHADE) != DONE)
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    }

    pPlayer->SEND_GOSSIP_MENU(907, pCreature->GetGUID());
    return true;
}

bool GossipSelect_npc_akama(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)               //Fight time
    {
        pPlayer->CLOSE_GOSSIP_MENU();

        if (npc_akamaAI* pAkamaAI = dynamic_cast<npc_akamaAI*>(pCreature->AI()))
            pAkamaAI->BeginEvent();
    }

    return true;
}

struct MANGOS_DLL_DECL mob_ashtongue_channelerAI : public ScriptedAI
{
    mob_ashtongue_channelerAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = pCreature->GetInstanceData();
        Reset();
    }

    void Reset()
    {
        if (pInstance)
        {
            //self-resurrect if encounter not done and we are dead
            if (!me->isAlive() && pInstance->GetData(TYPE_SHADE) != DONE)
                me->Respawn();

            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }
    }

    void AttackStart(Unit* pWho) {}
    void MoveInLineOfSight(Unit* pWho) {}

    void JustDied(Unit* pKiller)
    {
        if (!pInstance)
            return;

        if (Creature* pShade = pInstance->instance->GetCreature(pInstance->GetData64(DATA_SHADEOFAKAMA)))
        {
            if (pShade->isAlive())
            {
                if (boss_shade_of_akamaAI* pShadeAI = dynamic_cast<boss_shade_of_akamaAI*>(pShade->AI()))
                    pShadeAI->IncrementDeathCount();
                else
                    error_log("SD2: mob_ashtongue_channelerAI dead but unable to increment DeathCount for Shade of Akama.");
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!me->isAlive())
            return;

        //start channel (not nice way to start channeling)
        if (!me->IsNonMeleeSpellCasted(false) && !me->getVictim() && pInstance)
        {
            if (Creature* pShade = pInstance->instance->GetCreature(pInstance->GetData64(DATA_SHADEOFAKAMA)))
                me->CastSpell(pShade, SPELL_SHADE_SOUL_CHANNEL, false);
        }
    }
};

struct MANGOS_DLL_DECL mob_ashtongue_sorcererAI : public ScriptedAI
{
    mob_ashtongue_sorcererAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = pCreature->GetInstanceData();
        Reset();
    }

    uint32 m_uiCheckTimer;
    bool m_bStartBanishing;

    void Reset()
    {
        m_uiCheckTimer = 5000;
        m_bStartBanishing = false;
    }

    void AttackStart(Unit* pWho) {}
    void MoveInLineOfSight(Unit* pWho) {}

    void JustDied(Unit* pKiller)
    {
        if (!pInstance)
            return;

        if (Creature* pShade = pInstance->instance->GetCreature(pInstance->GetData64(DATA_SHADEOFAKAMA)))
        {
            if (pShade->isAlive())
            {
                if (boss_shade_of_akamaAI* pShadeAI = dynamic_cast<boss_shade_of_akamaAI*>(pShade->AI()))
                    pShadeAI->IncrementDeathCount(me->GetGUID());
                else
                    error_log("SD2: mob_ashtongue_sorcererAI dead but unable to increment DeathCount for Shade of Akama.");
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (m_bStartBanishing || !pInstance)
            return;

        if (m_uiCheckTimer < diff)
        {
            Unit* pShade = Unit::GetUnit(*me, pInstance->GetData64(DATA_SHADEOFAKAMA));

            if (pShade && pShade->isAlive() && me->isAlive())
            {
                if (me->IsWithinDist(pShade, 20.0f, false))
                {
                    me->GetMotionMaster()->Clear(false);
                    me->GetMotionMaster()->MoveIdle();

                    DoCast(pShade, SPELL_SHADE_SOUL_CHANNEL, true);

                    m_bStartBanishing = true;
                }
            }
            m_uiCheckTimer = 2000;
        }
        else
            m_uiCheckTimer -= diff;
    }
};

CreatureAI* GetAI_boss_shade_of_akama(Creature* pCreature)
{
    return new boss_shade_of_akamaAI(pCreature);
}

CreatureAI* GetAI_npc_akama_shade(Creature* pCreature)
{
    return new npc_akamaAI(pCreature);
}

CreatureAI* GetAI_mob_ashtongue_channeler(Creature* pCreature)
{
    return new mob_ashtongue_channelerAI(pCreature);
}

CreatureAI* GetAI_mob_ashtongue_sorcerer(Creature* pCreature)
{
    return new mob_ashtongue_sorcererAI(pCreature);
}

void AddSC_boss_shade_of_akama()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_shade_of_akama";
    newscript->GetAI = &GetAI_boss_shade_of_akama;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_akama_shade";
    newscript->GetAI = &GetAI_npc_akama_shade;
    newscript->pGossipHello = &GossipHello_npc_akama;
    newscript->pGossipSelect = &GossipSelect_npc_akama;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_ashtongue_channeler";
    newscript->GetAI = &GetAI_mob_ashtongue_channeler;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_ashtongue_sorcerer";
    newscript->GetAI = &GetAI_mob_ashtongue_sorcerer;
    newscript->RegisterSelf();
}
