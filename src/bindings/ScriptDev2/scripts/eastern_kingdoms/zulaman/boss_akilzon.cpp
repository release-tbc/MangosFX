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
SDName: Boss_Akilzon
SD%Complete: 50
SDComment: TODO: Correct timers, correct details, remove hack for eagles
SDCategory: Zul'Aman
EndScriptData */

#include "precompiled.h"
#include "zulaman.h"

enum
{
    SAY_EVENT1              = -1568024,
    SAY_EVENT2              = -1568025,
    SAY_AGGRO               = -1568026,
    SAY_SUMMON              = -1568027,
    SAY_SUMMON_ALT          = -1568028,
    SAY_ENRAGE              = -1568029,
    SAY_SLAY1               = -1568030,
    SAY_SLAY2               = -1568031,
    SAY_DEATH               = -1568032,
    EMOTE_STORM             = -1568033,

    SPELL_STATIC_DISRUPTION = 43622,
    SPELL_STATIC_VISUAL     = 45265,

    SPELL_CALL_LIGHTNING    = 43661,
    SPELL_GUST_OF_WIND      = 43621,

    SPELL_ELECTRICAL_STORM  = 43648,
    SPELL_STORMCLOUD_VISUAL = 45213,

    SPELL_BERSERK           = 45078,

    NPC_SOARING_EAGLE       = 24858,
    MAX_EAGLE_COUNT         = 6,

    //SE_LOC_X_MAX            = 400,
    //SE_LOC_X_MIN            = 335,
    //SE_LOC_Y_MAX            = 1435,
    //SE_LOC_Y_MIN            = 1370
};

struct MANGOS_DLL_DECL boss_akilzonAI : public ScriptedAI
{
    boss_akilzonAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = pCreature->GetInstanceData();
        Reset();
    }

    uint32 m_uiStaticDisruptTimer;
    uint32 m_uiCallLightTimer;
    uint32 m_uiGustOfWindTimer;
    uint32 m_uiStormTimer;
    uint32 m_uiSummonEagleTimer;
    uint32 m_uiBerserkTimer;
    bool m_bIsBerserk;

    void Reset()
    {
        m_uiStaticDisruptTimer = urand(7000, 14000);
        m_uiCallLightTimer = urand(15000, 25000);
        m_uiGustOfWindTimer = urand(20000, 30000);
        m_uiStormTimer = 50000;
        m_uiSummonEagleTimer = 65000;
        m_uiBerserkTimer = MINUTE*8*IN_MILLISECONDS;
        m_bIsBerserk = false;
    }

    void Aggro(Unit* pWho)
    {
        DoScriptText(SAY_AGGRO, me);
        me->SetInCombatWithZone();
    }

    void KilledUnit(Unit* pVictim)
    {
        DoScriptText(urand(0, 1) ? SAY_SLAY1 : SAY_SLAY2, me);
    }

    void JustDied(Unit* pKiller)
    {
        DoScriptText(SAY_DEATH, me);

        if (!pInstance)
            return;

        SetInstanceData(TYPE_AKILZON, DONE);
    }

    void JustSummoned(Creature* pSummoned)
    {
        if (pSummoned->GetEntry() == NPC_SOARING_EAGLE)
            pSummoned->SetInCombatWithZone();
    }

    void DoSummonEagles()
    {
        for(uint32 i = 0; i < MAX_EAGLE_COUNT; ++i)
        {
            float fX, fY, fZ;
            me->GetRandomPoint(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ()+15.0f, 30.0f, fX, fY, fZ);

            me->SummonCreature(NPC_SOARING_EAGLE, fX, fY, fZ, me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1000);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!CanDoSomething())
            return;

        if (m_uiCallLightTimer < diff)
        {
            me->CastSpell(me->getVictim(), SPELL_CALL_LIGHTNING, false);
            m_uiCallLightTimer = urand(15000, 25000);
        }else m_uiCallLightTimer -= diff;

        if (m_uiStaticDisruptTimer < diff)
        {
            if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM, 1))
                me->CastSpell(pTarget, SPELL_STATIC_DISRUPTION, false);

            m_uiStaticDisruptTimer = urand(7000, 14000);
        }else m_uiStaticDisruptTimer -= diff;

        if (m_uiStormTimer < diff)
        {
            if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0))
            {
                if (me->IsNonMeleeSpellCasted(false))
                    me->InterruptNonMeleeSpells(false);

                DoScriptText(EMOTE_STORM, me);
                me->CastSpell(pTarget, SPELL_ELECTRICAL_STORM, false);
            }

            m_uiStormTimer = 60000;
        }else m_uiStormTimer -= diff;

        if (m_uiGustOfWindTimer < diff)
        {
            if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM, 1))
                me->CastSpell(pTarget, SPELL_GUST_OF_WIND, false);

            m_uiGustOfWindTimer = urand(20000, 30000);
        }else m_uiGustOfWindTimer -= diff;

        if (m_uiSummonEagleTimer < diff)
        {
            DoScriptText(urand(0,1) ? SAY_SUMMON : SAY_SUMMON_ALT, me);
            DoSummonEagles();
            m_uiSummonEagleTimer = 60000;
        }else m_uiSummonEagleTimer -= diff;

        if (!m_bIsBerserk && m_uiBerserkTimer < diff)
        {
            DoScriptText(SAY_ENRAGE, me);
            me->CastSpell(me, SPELL_BERSERK, true);
            m_bIsBerserk = true;
        }else m_uiBerserkTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_akilzon(Creature* pCreature)
{
    return new boss_akilzonAI(pCreature);
}

enum
{
    SPELL_EAGLE_SWOOP       = 44732,
    POINT_ID_RANDOM         = 1
};

struct MANGOS_DLL_DECL mob_soaring_eagleAI : public ScriptedAI
{
    mob_soaring_eagleAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = pCreature->GetInstanceData();
        Reset();
    }

    uint32 m_uiEagleSwoopTimer;
    uint32 m_uiReturnTimer;
    bool m_bCanMoveToRandom;
    bool m_bCanCast;

    void Reset()
    {
        m_uiEagleSwoopTimer = urand(2000, 6000);
        m_uiReturnTimer = 800;
        m_bCanMoveToRandom = false;
        m_bCanCast = true;

    }

    void AttackStart(Unit* pWho)
    {
        if (!pWho)
            return;

        if (me->Attack(pWho, false))
        {
            me->AddThreat(pWho, 0.0f);
            me->SetInCombatWith(pWho);
            pWho->SetInCombatWith(me);
        }
    }

    void MovementInform(uint32 uiType, uint32 uiPointId)
    {
        if (uiType != POINT_MOTION_TYPE)
            return;

        m_bCanCast = true;
    }

    void DoMoveToRandom()
    {
        if (!pInstance)
            return;

        if (Creature* pAzkil = pInstance->instance->GetCreature(pInstance->GetData64(DATA_AKILZON)))
        {
            float fX, fY, fZ;
            pAzkil->GetRandomPoint(pAzkil->GetPositionX(), pAzkil->GetPositionY(), pAzkil->GetPositionZ()+15.0f, 30.0f, fX, fY, fZ);

            if (me->HasSplineFlag(SPLINEFLAG_WALKMODE))
                me->RemoveSplineFlag(SPLINEFLAG_WALKMODE);

            me->GetMotionMaster()->MovePoint(POINT_ID_RANDOM, fX, fY, fZ);

            m_bCanMoveToRandom = false;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!CanDoSomething())
            return;

        if (m_bCanMoveToRandom)
        {
            if (m_uiReturnTimer < diff)
            {
                DoMoveToRandom();
                m_uiReturnTimer = 800;
            }else m_uiReturnTimer -= diff;
        }

        if (!m_bCanCast)
            return;

        if (m_uiEagleSwoopTimer < diff)
        {
            if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM,0))
            {
                DoCast(pTarget,SPELL_EAGLE_SWOOP);

                m_bCanMoveToRandom = true;
                m_bCanCast = false;
            }

            m_uiEagleSwoopTimer = urand(4000, 6000);
        }else m_uiEagleSwoopTimer -= diff;
    }
};

CreatureAI* GetAI_mob_soaring_eagle(Creature* pCreature)
{
    return new mob_soaring_eagleAI(pCreature);
}

void AddSC_boss_akilzon()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_akilzon";
    newscript->GetAI = &GetAI_boss_akilzon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_soaring_eagle";
    newscript->GetAI = &GetAI_mob_soaring_eagle;
    newscript->RegisterSelf();
}
