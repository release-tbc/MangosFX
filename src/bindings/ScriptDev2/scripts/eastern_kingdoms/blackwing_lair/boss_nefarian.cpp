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
SDName: Boss_Nefarian
SD%Complete: 80
SDComment: Some issues with class calls effecting more than one class
SDCategory: Blackwing Lair
EndScriptData */

#include "precompiled.h"

#define SAY_AGGRO               -1469007
#define SAY_XHEALTH             -1469008
#define SAY_SHADOWFLAME         -1469009
#define SAY_RAISE_SKELETONS     -1469010
#define SAY_SLAY                -1469011
#define SAY_DEATH               -1469012

#define SAY_MAGE                -1469013
#define SAY_WARRIOR             -1469014
#define SAY_DRUID               -1469015
#define SAY_PRIEST              -1469016
#define SAY_PALADIN             -1469017
#define SAY_SHAMAN              -1469018
#define SAY_WARLOCK             -1469019
#define SAY_HUNTER              -1469020
#define SAY_ROGUE               -1469021

#define SPELL_SHADOWFLAME_INITIAL   22972
#define SPELL_SHADOWFLAME           22539
#define SPELL_BELLOWINGROAR         22686
#define SPELL_VEILOFSHADOW          7068
#define SPELL_CLEAVE                20691
#define SPELL_TAILLASH              23364
#define SPELL_BONECONTRUST          23363                   //23362, 23361

#define SPELL_MAGE                  23410                   //wild magic
#define SPELL_WARRIOR               23397                   //beserk
#define SPELL_DRUID                 23398                   // cat form
#define SPELL_PRIEST                23401                   // corrupted healing
#define SPELL_PALADIN               23418                   //syphon blessing
#define SPELL_SHAMAN                23425                   //totems
#define SPELL_WARLOCK               23427                   //infernals
#define SPELL_HUNTER                23436                   //bow broke
#define SPELL_ROGUE                 23414                   //Paralise

struct MANGOS_DLL_DECL boss_nefarianAI : public LibDevFSAI
{
    boss_nefarianAI(Creature* pCreature) : LibDevFSAI(pCreature) 
    {
		InitIA();
		AddEventOnTank(SPELL_SHADOWFLAME,12000,12000);
		AddEventOnTank(SPELL_BELLOWINGROAR,30000,30000);
		AddEventOnTank(SPELL_VEILOFSHADOW,15000,15000);
		AddEventOnTank(SPELL_CLEAVE,7000,7000);
		AddEventOnMe(SPELL_TAILLASH,10000,10000);
    }

    uint32 TailLash_Timer;
    uint32 ClassCall_Timer;
    bool Phase3;

    void Reset()
    {
		ResetTimers();
        TailLash_Timer = 10000;
        ClassCall_Timer = 35000;                            //35-40 seconds
        Phase3 = false;
    }

    void KilledUnit(Unit* Victim)
    {
        if (urand(0, 4))
            return;

        DoScriptText(SAY_SLAY, me, Victim);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, me);
    }

    void Aggro(Unit* pWho)
    {
        switch(urand(0, 3))
        {
            case 0: DoScriptText(SAY_XHEALTH, me); break;
            case 1: DoScriptText(SAY_AGGRO, me); break;
            case 2: DoScriptText(SAY_SHADOWFLAME, me); break;
        }

        DoCast(pWho,SPELL_SHADOWFLAME_INITIAL);

        me->SetInCombatWithZone();
    }

    void UpdateAI(const uint32 diff)
    {
        if (!CanDoSomething())
            return;

        //ClassCall_Timer
        if (ClassCall_Timer < diff)
        {
            //Cast a random class call
            //On official it is based on what classes are currently on the hostil list
            //but we can't do that yet so just randomly call one

            switch(urand(0, 8))
            {
                case 0:
                    DoScriptText(SAY_MAGE, me);
                    DoCastMe(SPELL_MAGE);
                    break;
                case 1:
                    DoScriptText(SAY_WARRIOR, me);
                    DoCastMe(SPELL_WARRIOR);
                    break;
                case 2:
                    DoScriptText(SAY_DRUID, me);
                    DoCastMe(SPELL_DRUID);
                    break;
                case 3:
                    DoScriptText(SAY_PRIEST, me);
                    DoCastMe(SPELL_PRIEST);
                    break;
                case 4:
                    DoScriptText(SAY_PALADIN, me);
                    DoCastMe(SPELL_PALADIN);
                    break;
                case 5:
                    DoScriptText(SAY_SHAMAN, me);
                    DoCastMe(SPELL_SHAMAN);
                    break;
                case 6:
                    DoScriptText(SAY_WARLOCK, me);
                    DoCastMe(SPELL_WARLOCK);
                    break;
                case 7:
                    DoScriptText(SAY_HUNTER, me);
                    DoCastMe(SPELL_HUNTER);
                    break;
                case 8:
                    DoScriptText(SAY_ROGUE, me);
                    DoCastMe(SPELL_ROGUE);
                    break;
            }

            ClassCall_Timer = urand(35000, 40000);
        }else ClassCall_Timer -= diff;

        //Phase3 begins when we are below X health
        if (!Phase3 && CheckPercentLife(20))
        {
            Phase3 = true;
            DoScriptText(SAY_RAISE_SKELETONS, me);
        }
        
        UpdateEvent(diff);

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_nefarian(Creature* pCreature)
{
    return new boss_nefarianAI(pCreature);
}

void AddSC_boss_nefarian()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_nefarian";
    newscript->GetAI = &GetAI_boss_nefarian;
    newscript->RegisterSelf();
}
