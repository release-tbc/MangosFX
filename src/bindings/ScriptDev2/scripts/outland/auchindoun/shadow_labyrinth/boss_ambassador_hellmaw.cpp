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
SDName: Boss_Ambassador_Hellmaw
SD%Complete: 80
SDComment: Enrage spell missing/not known
SDCategory: Auchindoun, Shadow Labyrinth
EndScriptData */

#include "precompiled.h"
#include "shadow_labyrinth.h"
#include "escort_ai.h"

#define SAY_INTRO       -1555000

#define SAY_AGGRO1      -1555001
#define SAY_AGGRO2      -1555002
#define SAY_AGGRO3      -1555003

#define SAY_HELP        -1555004

#define SAY_SLAY1       -1555005
#define SAY_SLAY2       -1555006

#define SAY_DEATH       -1555007

#define SPELL_BANISH            30231
#define SPELL_CORROSIVE_ACID    33551
#define SPELL_FEAR              33547
#define SPELL_ENRAGE            34970

struct MANGOS_DLL_DECL boss_ambassador_hellmawAI : public npc_escortAI
{
    boss_ambassador_hellmawAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        pInstance = pCreature->GetInstanceData();
        m_bIsHeroic = pCreature->GetMap()->IsRegularDifficulty();
        Reset();
    }

    bool m_bIsHeroic;

    uint32 EventCheck_Timer;
    uint32 CorrosiveAcid_Timer;
    uint32 Fear_Timer;
    uint32 Enrage_Timer;
    bool Intro;
    bool IsBanished;
    bool Enraged;

    void Reset()
    {
        EventCheck_Timer = 5000;
        CorrosiveAcid_Timer = urand(5000, 10000);
        Fear_Timer = urand(25000, 30000);
        Enrage_Timer = 180000;
        Intro = false;
        IsBanished = true;
        Enraged = false;

        if (pInstance && me->isAlive())
        {
            if (pInstance->GetData(TYPE_OVERSEER) != DONE)
                me->CastSpell(me, SPELL_BANISH, true);
        }
    }

    void JustReachedHome()
    {
        if (pInstance)
            SetInstanceData(TYPE_HELLMAW, FAIL);
    }

    void WaypointReached(uint32 i)
    {
    }

    void DoIntro()
    {
        if (me->HasAura(SPELL_BANISH,0))
            me->RemoveAurasDueToSpell(SPELL_BANISH);

        IsBanished = false;
        Intro = true;

        if (pInstance)
        {
            if (pInstance->GetData(TYPE_HELLMAW) != FAIL)
            {
                DoScriptText(SAY_INTRO, me);
                Start(true, false, 0, NULL, false, true);
            }

            SetInstanceData(TYPE_HELLMAW, IN_PROGRESS);
        }
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (me->HasAura(SPELL_BANISH,0))
            return;

        npc_escortAI::MoveInLineOfSight(who);
    }

    void Aggro(Unit *who)
    {
        switch(urand(0, 2))
        {
            case 0: DoScriptText(SAY_AGGRO1, me); break;
            case 1: DoScriptText(SAY_AGGRO2, me); break;
            case 2: DoScriptText(SAY_AGGRO3, me); break;
        }
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(urand(0, 1) ? SAY_SLAY1 : SAY_SLAY2, me);
    }

    void JustDied(Unit *victim)
    {
        DoScriptText(SAY_DEATH, me);

        if (pInstance)
            SetInstanceData(TYPE_HELLMAW, DONE);
    }

    void UpdateEscortAI(const uint32 diff)
    {
        if (!Intro && !HasEscortState(STATE_ESCORT_ESCORTING))
        {
            if (EventCheck_Timer < diff)
            {
                if (pInstance)
                {
                    if (pInstance->GetData(TYPE_OVERSEER) == DONE)
                    {
                        DoIntro();
                        return;
                    }
                }
                EventCheck_Timer = 5000;
                return;
            }
            else
            {
                EventCheck_Timer -= diff;
                return;
            }
        }

        if (!CanDoSomething())
            return;

        if (CorrosiveAcid_Timer < diff)
        {
            DoCastVictim(SPELL_CORROSIVE_ACID);
            CorrosiveAcid_Timer = urand(15000, 25000);
        }else CorrosiveAcid_Timer -= diff;

        if (Fear_Timer < diff)
        {
            DoCastMe(SPELL_FEAR);
            Fear_Timer = urand(20000, 35000);
        }else Fear_Timer -= diff;

        if (m_bIsHeroic)
        {
            if (!Enraged && Enrage_Timer < diff)
            {
                DoCastMe(SPELL_ENRAGE);
                Enraged = true;
            }else Enrage_Timer -= diff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_ambassador_hellmaw(Creature* pCreature)
{
    return new boss_ambassador_hellmawAI(pCreature);
}

void AddSC_boss_ambassador_hellmaw()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_ambassador_hellmaw";
    newscript->GetAI = &GetAI_boss_ambassador_hellmaw;
    newscript->RegisterSelf();
}
