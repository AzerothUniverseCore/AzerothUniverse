/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Totem.h"
#include "Group.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellInfo.h"
#include "TotemPackets.h"

//npcbot
#include "botmgr.h"
#include "ObjectAccessor.h"
//end npcbot

Totem::Totem(SummonPropertiesEntry const* properties, Unit* owner) : Minion(properties, owner, false)
{
    m_unitTypeMask |= UNIT_MASK_TOTEM;
    m_duration = 0;
    m_type = TOTEM_PASSIVE;
}

void Totem::Update(uint32 time)
{
    //npcbot: do not despawn bot totem if master is dead
    Creature const* botOwner = (GetOwner()->GetTypeId() == TYPEID_PLAYER && GetOwner()->ToPlayer()->HaveBot()) ?
        GetOwner()->ToPlayer()->GetBotMgr()->GetBot(GetCreatorGUID()) : nullptr;

    if (botOwner)
    {
        if (!botOwner->IsAlive() || !IsAlive())
        {
            UnSummon();
            return;
        }
    }
    else
    //end npcbot
    if (!GetOwner()->IsAlive() || !IsAlive())
    {
        UnSummon();                                         // remove self
        return;
    }

    if (m_duration <= time)
    {
        UnSummon();                                         // remove self
        return;
    }
    else
        m_duration -= time;

    Creature::Update(time);
}

void Totem::InitStats(uint32 duration)
{
    // client requires SMSG_TOTEM_CREATED to be sent before adding to world and before removing old totem
    if (Player* owner = GetOwner()->ToPlayer())
    {
        uint32 slot = m_Properties->Slot;
        if (slot >= SUMMON_SLOT_TOTEM_FIRE && slot < MAX_TOTEM_SLOT)
        {
            WorldPackets::Totem::TotemCreated data;
            data.Totem = GetGUID();
            data.Slot = slot - SUMMON_SLOT_TOTEM_FIRE;
            data.Duration = duration;
            data.SpellID = GetUInt32Value(UNIT_CREATED_BY_SPELL);
            owner->SendDirectMessage(data.Write());
        }

        // set display id depending on caster's race
        if (uint32 totemDisplayId = sObjectMgr->GetModelForTotem(SummonSlot(slot), Races(owner->GetRace())))
            SetDisplayId(totemDisplayId);
        else
            TC_LOG_DEBUG("misc", "Totem with entry {}, owned by player {} ({} {} {}) in slot {}, created by spell {}, does not have a specialized model. Set to default.",
                         GetEntry(), owner->GetGUID().ToString(), owner->GetLevel(), EnumUtils::ToTitle(Races(owner->GetRace())), EnumUtils::ToTitle(Classes(owner->GetClass())), slot, GetUInt32Value(UNIT_CREATED_BY_SPELL));
    }

    Minion::InitStats(duration);

    // Get spell cast by totem
    if (SpellInfo const* totemSpell = sSpellMgr->GetSpellInfo(GetSpell()))
        if (totemSpell->CalcCastTime())   // If spell has cast time -> its an active totem
            m_type = TOTEM_ACTIVE;

    if (GetEntry() == SENTRY_TOTEM_ENTRY)
        SetReactState(REACT_AGGRESSIVE);

    m_duration = duration;

    SetLevel(GetOwner()->GetLevel());
}

void Totem::InitSummon()
{
    if (m_type == TOTEM_PASSIVE && GetSpell())
    {
        CastSpell(this, GetSpell(), true);
    }

    // Some totems can have both instant effect and passive spell
    if (GetSpell(1))
        CastSpell(this, GetSpell(1), true);
}

void Totem::UnSummon(uint32 msTime)
{
    if (msTime)
    {
        m_Events.AddEvent(new ForcedUnsummonDelayEvent(*this), m_Events.CalculateTime(Milliseconds(msTime)));
        return;
    }

    CombatStop();
    RemoveAurasDueToSpell(GetSpell(), GetGUID());

    // clear owner's totem slot
    for (uint8 i = SUMMON_SLOT_TOTEM_FIRE; i < MAX_TOTEM_SLOT; ++i)
    {
        if (GetOwner()->m_SummonSlot[i] == GetGUID())
        {
            GetOwner()->m_SummonSlot[i].Clear();
            break;
        }
    }

    GetOwner()->RemoveAurasDueToSpell(GetSpell(), GetGUID());

    // Remove Sentry Totem Aura
    if (GetEntry() == SENTRY_TOTEM_ENTRY)
        GetOwner()->RemoveAurasDueToSpell(SENTRY_TOTEM_SPELLID);

    //remove aura all party members too
    if (Player* owner = GetOwner()->ToPlayer())
    {
        owner->SendAutoRepeatCancel(this);

        if (SpellInfo const* spell = sSpellMgr->GetSpellInfo(GetUInt32Value(UNIT_CREATED_BY_SPELL)))
            GetSpellHistory()->SendCooldownEvent(spell, 0, nullptr, false);

        if (Group* group = owner->GetGroup())
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* target = itr->GetSource();
                if (target && target->IsInMap(owner) && group->SameSubGroup(owner, target))
                    target->RemoveAurasDueToSpell(GetSpell(), GetGUID());
            }
        }
    }

    // any totem unsummon look like as totem kill, req. for proper animation
    if (IsAlive())
        setDeathState(DEAD);

    //npcbot: send SummonedCreatureDespawn()
    if (Unit* creator = GetCreator())
        if (creator->IsNPCBot())
            creator->ToCreature()->OnBotDespawn(this);
    //end npcbot

    AddObjectToRemoveList();
}

bool Totem::IsImmunedToSpellEffect(SpellInfo const* spellInfo, SpellEffectInfo const& spellEffectInfo, WorldObject const* caster,
    bool requireImmunityPurgesEffectAttribute /*= false*/) const
{
    // immune to all positive spells, except of stoneclaw totem absorb and sentry totem bind sight
    // totems positive spells have unit_caster target
    if (spellEffectInfo.Effect != SPELL_EFFECT_DUMMY &&
        spellEffectInfo.Effect != SPELL_EFFECT_SCRIPT_EFFECT &&
        spellInfo->IsPositive() && spellEffectInfo.TargetA.GetTarget() != TARGET_UNIT_CASTER &&
        spellEffectInfo.TargetA.GetCheckType() != TARGET_CHECK_ENTRY && spellInfo->Id != SENTRY_STONECLAW_SPELLID && spellInfo->Id != SENTRY_BIND_SIGHT_SPELLID)
        return true;

    switch (spellEffectInfo.ApplyAuraName)
    {
        case SPELL_AURA_PERIODIC_DAMAGE:
        case SPELL_AURA_PERIODIC_LEECH:
        case SPELL_AURA_MOD_FEAR:
        case SPELL_AURA_TRANSFORM:
            return true;
        default:
            break;
    }

    return Creature::IsImmunedToSpellEffect(spellInfo, spellEffectInfo, caster, requireImmunityPurgesEffectAttribute);
}
