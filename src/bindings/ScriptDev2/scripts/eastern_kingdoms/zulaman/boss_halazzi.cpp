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
SDName: Boss_Halazzi
SD%Complete: 70
SDComment: Details and timers need check.
SDCategory: Zul'Aman
EndScriptData */

#include "precompiled.h"
#include "zulaman.h"
#include "ObjectMgr.h"

enum
{
    SAY_AGGRO                       = -1568034,
    SAY_SPLIT                       = -1568035,
    SAY_MERGE                       = -1568036,
    SAY_SABERLASH1                  = -1568037,
    SAY_SABERLASH2                  = -1568038,
    SAY_BERSERK                     = -1568039,
    SAY_KILL1                       = -1568040,
    SAY_KILL2                       = -1568041,
    SAY_DEATH                       = -1568042,
    SAY_EVENT1                      = -1568043,
    SAY_EVENT2                      = -1568044,

    SPELL_DUAL_WIELD                = 42459,
    SPELL_SABER_LASH                = 43267,
    SPELL_FRENZY                    = 43139,
    SPELL_FLAMESHOCK                = 43303,
    SPELL_EARTHSHOCK                = 43305,
    SPELL_BERSERK                   = 45078,

    //SPELL_TRANSFORM_TO_ORIGINAL     = 43311,

    //SPELL_TRANSFIGURE               = 44054,

    SPELL_TRANSFIGURE_TO_TROLL      = 43142,
    //SPELL_TRANSFIGURE_TO_TROLL_TRIGGERED = 43573,

    SPELL_TRANSFORM_TO_LYNX_75      = 43145,
    SPELL_TRANSFORM_TO_LYNX_50      = 43271,
    SPELL_TRANSFORM_TO_LYNX_25      = 43272,

    SPELL_SUMMON_LYNX               = 43143,
    SPELL_SUMMON_TOTEM              = 43302,

    NPC_TOTEM                       = 24224
};

enum HalazziPhase
{
    PHASE_SINGLE        = 0,
    PHASE_TOTEM         = 1,
    PHASE_FINAL         = 2
};

struct MANGOS_DLL_DECL boss_halazziAI : public ScriptedAI
{
    boss_halazziAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = pCreature->GetInstanceData();
        Reset();
    }

    uint32 m_uiPhase;
    uint32 m_uiPhaseCounter;
    uint32 m_uiFrenzyTimer;
    uint32 m_uiSaberLashTimer;
    uint32 m_uiShockTimer;
    uint32 m_uiTotemTimer;
    uint32 m_uiCheckTimer;
    uint32 m_uiBerserkTimer;
    bool m_bIsBerserk;

    void Reset()
    {
        m_uiPhase = PHASE_SINGLE;                           // reset phase
        m_uiPhaseCounter = 3;

        m_uiCheckTimer = IN_MILLISECONDS;
        m_uiFrenzyTimer = 16*IN_MILLISECONDS;
        m_uiSaberLashTimer = 20*IN_MILLISECONDS;
        m_uiShockTimer = 10*IN_MILLISECONDS;
        m_uiTotemTimer = 12*IN_MILLISECONDS;
        m_uiBerserkTimer = 10*MINUTE*IN_MILLISECONDS;
        m_bIsBerserk = false;

        me->SetMaxHealth(me->GetCreatureInfo()->maxhealth);

        if (pInstance)
        {
            if (Creature* pSpiritLynx = pInstance->instance->GetCreature(pInstance->GetData64(DATA_SPIRIT_LYNX)))
                pSpiritLynx->ForcedDespawn();
        }
    }

    void JustReachedHome()
    {
        SetInstanceData(TYPE_HALAZZI, NOT_STARTED);
    }

    void Aggro(Unit* pWho)
    {
        DoScriptText(SAY_AGGRO, me);
        me->SetInCombatWithZone();

        if (pInstance)
            SetInstanceData(TYPE_HALAZZI, IN_PROGRESS);
    }

    void KilledUnit(Unit* pVictim)
    {
        if (pVictim->GetTypeId() != TYPEID_PLAYER)
            return;

        DoScriptText(urand(0, 1) ? SAY_KILL1 : SAY_KILL2, me);
    }

    void JustDied(Unit* pKiller)
    {
        DoScriptText(SAY_DEATH, me);

        if (pInstance)
            SetInstanceData(TYPE_HALAZZI, DONE);
    }

    void JustSummoned(Creature* pSummoned)
    {
        if (pSummoned->GetEntry() == NPC_SPIRIT_LYNX)
            pSummoned->SetInCombatWithZone();
    }

    void DoUpdateStats(const CreatureInfo* pInfo)
    {
        me->SetMaxHealth(pInfo->maxhealth);

        if (m_uiPhase == PHASE_SINGLE)
        {
            me->SetHealth(me->GetMaxHealth()/4*m_uiPhaseCounter);
            --m_uiPhaseCounter;
        }
    }

    void SpellHit(Unit* pCaster, const SpellEntry* pSpell)
    {
        if (pSpell->EffectApplyAuraName[0] != SPELL_AURA_TRANSFORM)
            return;

        // possibly hack and health should be set by Aura::HandleAuraTransform()
        if (const CreatureInfo* pInfo = GetCreatureTemplateStore(pSpell->EffectMiscValue[0]))
            DoUpdateStats(pInfo);

        if (m_uiPhase == PHASE_TOTEM)
            DoCastMe( SPELL_SUMMON_LYNX);
    }

    void PhaseChange()
    {
        if (m_uiPhase == PHASE_SINGLE)
        {
            if ((me->GetHealth()*100 / me->GetMaxHealth()) <= 25*m_uiPhaseCounter)
            {
                if (!m_uiPhaseCounter)
                {
                    // final phase
                    m_uiPhase = PHASE_FINAL;
                    m_uiFrenzyTimer = 16*IN_MILLISECONDS;
                    m_uiSaberLashTimer = 20*IN_MILLISECONDS;
                }
                else
                {
                    m_uiPhase = PHASE_TOTEM;
                    m_uiShockTimer = 10*IN_MILLISECONDS;
                    m_uiTotemTimer = 12*IN_MILLISECONDS;

                    DoScriptText(SAY_SPLIT, me);
                    me->CastSpell(me, SPELL_TRANSFIGURE_TO_TROLL, false);
                }
            }
        }
        else
        {
            Creature* pSpiritLynx = pInstance->instance->GetCreature(pInstance->GetData64(DATA_SPIRIT_LYNX));

            if (me->GetHealth()*10 < me->GetMaxHealth() ||
                (pSpiritLynx && pSpiritLynx->GetHealth()*10 < pSpiritLynx->GetMaxHealth()))
            {
                m_uiPhase = PHASE_SINGLE;

                DoScriptText(SAY_MERGE, me);

                uint32 uiSpellId;

                switch(m_uiPhaseCounter)
                {
                    case 3: uiSpellId = SPELL_TRANSFORM_TO_LYNX_75; break;
                    case 2: uiSpellId = SPELL_TRANSFORM_TO_LYNX_50; break;
                    case 1: uiSpellId = SPELL_TRANSFORM_TO_LYNX_25; break;
                }

                me->CastSpell(me, uiSpellId, false);

                if (pSpiritLynx)
                    pSpiritLynx->ForcedDespawn();

                m_uiFrenzyTimer = 16*IN_MILLISECONDS;
                m_uiSaberLashTimer = 20*IN_MILLISECONDS;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!CanDoSomething())
            return;

        if (!m_bIsBerserk)
        {
            if (m_uiBerserkTimer < diff)
            {
                DoScriptText(SAY_BERSERK, me);
                DoCastMe( SPELL_BERSERK,true);
                m_bIsBerserk = true;
            }
            else
                m_uiBerserkTimer -= diff;
        }

        if (m_uiPhase != PHASE_FINAL)
        {
            if (m_uiCheckTimer < diff)
            {
                if (pInstance)
                    PhaseChange();
                else
                    m_uiPhase = PHASE_FINAL;

                m_uiCheckTimer = IN_MILLISECONDS;
            }
            else
                m_uiCheckTimer -= diff;
        }

        if (m_uiPhase == PHASE_FINAL || m_uiPhase == PHASE_SINGLE)
        {
            if (m_uiFrenzyTimer < diff)
            {
                DoCastMe( SPELL_FRENZY);
                m_uiFrenzyTimer = 16*IN_MILLISECONDS;
            }
            else
                m_uiFrenzyTimer -= diff;

            if (m_uiSaberLashTimer < diff)
            {
                DoScriptText(urand(0, 1) ? SAY_SABERLASH1 : SAY_SABERLASH2, me);

                DoCastVictim( SPELL_SABER_LASH);
                m_uiSaberLashTimer = 20*IN_MILLISECONDS;
            }
            else
                m_uiSaberLashTimer -= diff;
        }

        if (m_uiPhase == PHASE_FINAL || m_uiPhase == PHASE_TOTEM)
        {
            if (m_uiTotemTimer < diff)
            {
                DoCastMe( SPELL_SUMMON_TOTEM);
                m_uiTotemTimer = 20*IN_MILLISECONDS;
            }
            else
                m_uiTotemTimer -= diff;

            if (m_uiShockTimer < diff)
            {
                if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM,0))
                {
                    if (pTarget->IsNonMeleeSpellCasted(false))
                        DoCast(pTarget, SPELL_EARTHSHOCK);
                    else
                        DoCast(pTarget, SPELL_FLAMESHOCK);

                    m_uiShockTimer = urand(10000, 14000);
                }
            }
            else
                m_uiShockTimer -= diff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_halazzi(Creature* pCreature)
{
    return new boss_halazziAI(pCreature);
}

enum
{
    SPELL_LYNX_FRENZY       = 43290,
    SPELL_SHRED_ARMOR       = 43243
};

struct MANGOS_DLL_DECL boss_spirit_lynxAI : public ScriptedAI
{
    boss_spirit_lynxAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = pCreature->GetInstanceData();
        Reset();
    }

    uint32 m_uiFrenzyTimer;
    uint32 m_uiShredArmorTimer;

    void Reset()
    {
        m_uiFrenzyTimer = urand(10000, 20000);              //first frenzy after 10-20 seconds
        m_uiShredArmorTimer = 4000;
    }

    void Aggro(Unit* pWho)
    {
        me->SetInCombatWithZone();
    }

    void KilledUnit(Unit* pVictim)
    {
        if (!pInstance)
            return;

        if (Creature* pHalazzi = pInstance->instance->GetCreature(pInstance->GetData64(DATA_HALAZZI)))
            pHalazzi->AI()->KilledUnit(pVictim);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!CanDoSomething())
            return;

        if (m_uiFrenzyTimer < diff)
        {
            DoCastMe( SPELL_LYNX_FRENZY);
            m_uiFrenzyTimer = urand(20000, 30000);          //subsequent frenzys casted every 20-30 seconds
        }
        else
            m_uiFrenzyTimer -= diff;

        if (m_uiShredArmorTimer < diff)
        {
            DoCastVictim( SPELL_SHRED_ARMOR);
            m_uiShredArmorTimer = 4000;
        }
        else
            m_uiShredArmorTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_spirit_lynx(Creature* pCreature)
{
    return new boss_spirit_lynxAI(pCreature);
}

void AddSC_boss_halazzi()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "boss_halazzi";
    newscript->GetAI = &GetAI_boss_halazzi;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_spirit_lynx";
    newscript->GetAI = &GetAI_boss_spirit_lynx;
    newscript->RegisterSelf();
}
