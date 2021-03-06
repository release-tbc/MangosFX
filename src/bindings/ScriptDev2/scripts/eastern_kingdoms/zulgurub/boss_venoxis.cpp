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
SDName: Boss_Venoxis
SD%Complete: 100
SDComment:
SDCategory: Zul'Gurub
EndScriptData */

#include "precompiled.h"
#include "zulgurub.h"

enum
{
    NPC_RAZZASHI_COBRA  = 11373,

    SAY_TRANSFORM       = -1309000,
    SAY_DEATH           = -1309001,

    SPELL_HOLY_FIRE     = 23860,
    SPELL_HOLY_WRATH    = 23979,
    SPELL_VENOMSPIT     = 23862,
    SPELL_HOLY_NOVA     = 23858,
    SPELL_POISON_CLOUD  = 23861,
    SPELL_SNAKE_FORM    = 23849,
    SPELL_RENEW         = 23895,
    SPELL_BERSERK       = 23537,
    SPELL_DISPELL       = 23859,
    SPELL_PARASITIC     = 23865,
    SPELL_TRASH         = 3391
};

struct MANGOS_DLL_DECL boss_venoxisAI : public ScriptedAI
{
    boss_venoxisAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = pCreature->GetInstanceData();
        m_fDefaultSize = me->GetFloatValue(OBJECT_FIELD_SCALE_X);
        Reset();
    }

    uint32 m_uiHolyFire_Timer;
    uint32 m_uiHolyWrath_Timer;
    uint32 m_uiVenomSpit_Timer;
    uint32 m_uiRenew_Timer;
    uint32 m_uiPoisonCloud_Timer;
    uint32 m_uiHolyNova_Timer;
    uint32 m_uiDispell_Timer;
    uint32 m_uiParasitic_Timer;
    uint32 m_uiTrash_Timer;

    uint8 m_uiTargetsInRangeCount;

    bool m_bPhaseTwo;
    bool m_bInBerserk;
    
    float m_fDefaultSize;

    void Reset()
    {
        m_uiHolyFire_Timer = 10000;
        m_uiHolyWrath_Timer = 60500;
        m_uiVenomSpit_Timer = 5500;
        m_uiRenew_Timer = 30500;
        m_uiPoisonCloud_Timer = 2000;
        m_uiHolyNova_Timer = 5000;
        m_uiDispell_Timer = 35000;
        m_uiParasitic_Timer = 10000;
        m_uiTrash_Timer = 5000;

        m_uiTargetsInRangeCount = 0;

        me->SetFloatValue(OBJECT_FIELD_SCALE_X, m_fDefaultSize);

        m_bPhaseTwo = false;
        m_bInBerserk = false;
    }

    void JustReachedHome()
    {
        std::list<Creature*> m_lCobras;
        GetCreatureListWithEntryInGrid(m_lCobras, me, NPC_RAZZASHI_COBRA, DEFAULT_VISIBILITY_INSTANCE);

        if (m_lCobras.empty())
            debug_log("SD2: boss_venoxis, no Cobras with the entry %u were found", NPC_RAZZASHI_COBRA);
        else
        {
            for(std::list<Creature*>::iterator iter = m_lCobras.begin(); iter != m_lCobras.end(); ++iter)
            {
                if ((*iter) && !(*iter)->isAlive())
                    (*iter)->Respawn();
            }
        }
    }

    void JustDied(Unit* pKiller)
    {
        DoScriptText(SAY_DEATH, me);

        if (pInstance)
        {
            SetInstanceData(TYPE_VENOXIS, DONE);

            if (Unit* pHakkar = Unit::GetUnit((*me), pInstance->GetData64(DATA_HAKKAR)))
                pHakkar->SetMaxHealth(pHakkar->GetMaxHealth() - 60000);
        }
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        if (!m_bPhaseTwo && (me->GetHealth()+uiDamage)*100 / me->GetMaxHealth() < 50)
        {
            DoScriptText(SAY_TRANSFORM, me);

            me->InterruptNonMeleeSpells(false);
            DoCastMe(SPELL_SNAKE_FORM);

            me->SetFloatValue(OBJECT_FIELD_SCALE_X, m_fDefaultSize*2);
            const CreatureInfo *cinfo = me->GetCreatureInfo();
            me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, (cinfo->mindmg +((cinfo->mindmg/100) * 25)));
            me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, (cinfo->maxdmg +((cinfo->maxdmg/100) * 25)));
            me->UpdateDamagePhysical(BASE_ATTACK);
            DoResetThreat();
            m_bPhaseTwo = true;
        }

        if (m_bPhaseTwo && !m_bInBerserk && (me->GetHealth()+uiDamage)*100 / me->GetMaxHealth() < 11)
        {
            me->InterruptNonMeleeSpells(false);
            DoCastMe( SPELL_BERSERK);
            m_bInBerserk = true;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!CanDoSomething())
            return;

        if (!m_bPhaseTwo)
        {
            if (m_uiDispell_Timer < diff)
            {
                DoCastMe( SPELL_DISPELL);
                m_uiDispell_Timer = urand(15000, 30000);
            }
            else
                m_uiDispell_Timer -= diff;

            if (m_uiRenew_Timer < diff)
            {
                DoCastMe( SPELL_RENEW);
                m_uiRenew_Timer = urand(20000, 30000);
            }
            else
                m_uiRenew_Timer -= diff;

            if (m_uiHolyWrath_Timer < diff)
            {
                DoCastVictim( SPELL_HOLY_WRATH);
                m_uiHolyWrath_Timer = urand(15000, 25000);
            }
            else
                m_uiHolyWrath_Timer -= diff;

            if (m_uiHolyNova_Timer < diff)
            {
                m_uiTargetsInRangeCount = 0;
                for(uint8 i = 0; i < 10; ++i)
                {
                    if (Unit* pTarget = SelectUnit(SELECT_TARGET_TOPAGGRO,i))
                        if (me->IsWithinDistInMap(pTarget, ATTACK_DISTANCE))
                            ++m_uiTargetsInRangeCount;
                }

                if (m_uiTargetsInRangeCount > 1)
                {
                    DoCastVictim(SPELL_HOLY_NOVA);
                    m_uiHolyNova_Timer = 1000;
                }
                else
                {
                    m_uiHolyNova_Timer = 2000;
                }
            }
            else
                m_uiHolyNova_Timer -= diff;

            if (m_uiHolyFire_Timer < diff && m_uiTargetsInRangeCount < 3)
            {
                if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM,0))
                    DoCast(pTarget, SPELL_HOLY_FIRE);

                m_uiHolyFire_Timer = 8000;
            }
            else
                m_uiHolyFire_Timer -= diff;
        }
        else
        {
            if (m_uiPoisonCloud_Timer < diff)
            {
                DoCastVictim( SPELL_POISON_CLOUD);
                m_uiPoisonCloud_Timer = 15000;
            }
            else
                m_uiPoisonCloud_Timer -= diff;

            if (m_uiVenomSpit_Timer < diff)
            {
                if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM,0))
                    DoCast(pTarget, SPELL_VENOMSPIT);

                m_uiVenomSpit_Timer = urand(15000, 20000);
            }
            else
                m_uiVenomSpit_Timer -= diff;

            if (m_uiParasitic_Timer < diff)
            {
                if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM,0))
                    DoCast(pTarget, SPELL_PARASITIC);

                m_uiParasitic_Timer = 10000;
            }
            else
                m_uiParasitic_Timer -= diff;
        }

        if (m_uiTrash_Timer < diff)
        {
            DoCastVictim( SPELL_TRASH);
            m_uiTrash_Timer = urand(10000, 20000);
        }
        else
            m_uiTrash_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_venoxis(Creature* pCreature)
{
    return new boss_venoxisAI(pCreature);
}

void AddSC_boss_venoxis()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_venoxis";
    newscript->GetAI = &GetAI_boss_venoxis;
    newscript->RegisterSelf();
}
