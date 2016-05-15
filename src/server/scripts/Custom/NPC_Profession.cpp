#include "ScriptMgr.h"
#include "Language.h"
#include "Player.h"
#include "Creature.h"

#define TOKEN_PVP 20558
#define QTD_TOKEN 100

class Professions_NPC : public CreatureScript
{
public:
	Professions_NPC() : CreatureScript("Professions_NPC") {}

	void CreatureWhisperBasedOnBool(const char *text, Creature *_creature, Player *pPlayer, bool value)
	{
		if (value)
			_creature->Whisper(text, LANG_UNIVERSAL, NULL);
	}

	uint32 PlayerMaxLevel() const
	{
		return sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
	}

	bool PlayerHasItemOrSpell(const Player *plr, uint32 itemId, uint32 spellId) const
	{
		return plr->HasItemCount(itemId, 1, true) || plr->HasSpell(spellId);
	}

	bool OnGossipHello(Player *pPlayer, Creature* _creature)
	{
		pPlayer->ADD_GOSSIP_ITEM(9, "[Profissões] ->", GOSSIP_SENDER_MAIN, 196);
		pPlayer->PlayerTalkClass->SendGossipMenu(907, _creature->GetGUID());
		return true;
	}

	bool PlayerAlreadyHasTwoProfessions(const Player *pPlayer) const
	{
		uint32 skillCount = 0;

		if (pPlayer->HasSkill(SKILL_MINING))
			skillCount++;
		if (pPlayer->HasSkill(SKILL_SKINNING))
			skillCount++;
		if (pPlayer->HasSkill(SKILL_HERBALISM))
			skillCount++;

		if (skillCount >= 2)
			return true;

		for (uint32 i = 1; i < sSkillLineStore.GetNumRows(); ++i)
		{
			SkillLineEntry const *SkillInfo = sSkillLineStore.LookupEntry(i);
			if (!SkillInfo)
				continue;

			if (SkillInfo->categoryId == SKILL_CATEGORY_SECONDARY)
				continue;

			if ((SkillInfo->categoryId != SKILL_CATEGORY_PROFESSION) || !SkillInfo->canLink)
				continue;

			const uint32 skillID = SkillInfo->id;
			if (pPlayer->HasSkill(skillID))
				skillCount++;

			if (skillCount >= 2)
				return true;
		}
		return false;
	}

	bool LearnAllRecipesInProfession(Player *pPlayer, SkillType skill)
	{
		ChatHandler handler(pPlayer->GetSession());
		char* skill_name;

		SkillLineEntry const *SkillInfo = sSkillLineStore.LookupEntry(skill);
		skill_name = SkillInfo->name[handler.GetSessionDbcLocale()];

		if (!SkillInfo)
		{
			TC_LOG_ERROR("Professions", "Profession NPC: received non-valid skill ID (LearnAllRecipesInProfession)");
			return false;
		}

		LearnSkillRecipesHelper(pPlayer, SkillInfo->id);

		pPlayer->SetSkill(SkillInfo->id, pPlayer->GetSkillStep(SkillInfo->id), 1, 450);


		return true;
	}

	void LearnSkillRecipesHelper(Player *player, uint32 skill_id)
	{
		uint32 classmask = player->getClassMask();

		for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
		{
			SkillLineAbilityEntry const *skillLine = sSkillLineAbilityStore.LookupEntry(j);
			if (!skillLine)
				continue;

			// wrong skill
			if (skillLine->skillId != skill_id)
				continue;

			// not high rank
			if (skillLine->forward_spellid)
				continue;

			// skip racial skills
			if (skillLine->racemask != 0)
				continue;

			// skip wrong class skills
			if (skillLine->classmask && (skillLine->classmask & classmask) == 0)
				continue;

			SpellInfo const * spellInfo = sSpellMgr->GetSpellInfo(skillLine->spellId);
			if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo, player, false))
				continue;

			player->LearnSpell(skillLine->spellId, false);
		}
	}

	bool IsSecondarySkill(SkillType skill) const
	{
		return skill == SKILL_COOKING || skill == SKILL_FIRST_AID;
	}

	void CompleteLearnProfession(Player *pPlayer, Creature *pCreature, SkillType skill)
	{
		if (PlayerAlreadyHasTwoProfessions(pPlayer) && !IsSecondarySkill(skill)){
			pCreature->Whisper("Você já aprendeu essa profissão.", LANG_UNIVERSAL, NULL);
		}
		else
		{
			if (!LearnAllRecipesInProfession(pPlayer, skill))
				pCreature->Whisper("Um erro aconteceu!", LANG_UNIVERSAL, NULL);
		}
	}

	bool OnGossipSelect(Player* pPlayer, Creature* _creature, uint32 uiSender, uint32 uiAction)
	{
		pPlayer->PlayerTalkClass->ClearMenus();

		if (uiSender == GOSSIP_SENDER_MAIN)
		{

			switch (uiAction)
			{
			case 196:
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface\\icons\\trade_alchemy:30|t Alchemy", GOSSIP_SENDER_MAIN, 1);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface\\icons\\INV_Ingot_05:30|t Blacksmithing", GOSSIP_SENDER_MAIN, 2);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface\\icons\\INV_Misc_LeatherScrap_02:30|t Leatherworking", GOSSIP_SENDER_MAIN, 3);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface\\icons\\INV_Fabric_Felcloth_Ebon:30|t Tailoring", GOSSIP_SENDER_MAIN, 4);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface\\icons\\inv_misc_wrench_01:30|t Engineering", GOSSIP_SENDER_MAIN, 5);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface\\icons\\trade_engraving:30|t Enchanting", GOSSIP_SENDER_MAIN, 6);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface\\icons\\inv_misc_gem_01:30|t Jewelcrafting", GOSSIP_SENDER_MAIN, 7);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface\\icons\\INV_Scroll_08:30|t Inscription", GOSSIP_SENDER_MAIN, 8);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface\\icons\\INV_Misc_Herb_07:30|t Herbalism", GOSSIP_SENDER_MAIN, 11);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface\\icons\\inv_misc_pelt_wolf_01:30|t Skinning", GOSSIP_SENDER_MAIN, 12);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface\\icons\\trade_mining:30|t Mining", GOSSIP_SENDER_MAIN, 13);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface\\icons\\inv_misc_food_15:30|tCooking", GOSSIP_SENDER_MAIN, 9);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface\\icons\\spell_holy_sealofsacrifice:30|tFirst Aid", GOSSIP_SENDER_MAIN, 10);

				pPlayer->PlayerTalkClass->SendGossipMenu(907, _creature->GetGUID());
				break;
			case 1:
				if (pPlayer->HasSkill(SKILL_ALCHEMY))
				{
					_creature->Whisper("Você já aprendeu essa profissão.", LANG_UNIVERSAL, NULL);
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_ALCHEMY);
						pPlayer->UpdateSkill(SKILL_ALCHEMY, 450);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();					
						_creature->Whisper("Você não tem o número de Token PvP (100) suficiente para treinar essa profissão.", LANG_UNIVERSAL, NULL);
					}
				}break;
			case 2:
				if (pPlayer->HasSkill(SKILL_BLACKSMITHING))
				{
					_creature->Whisper("Você já aprendeu essa profissão.", LANG_UNIVERSAL, NULL);
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_BLACKSMITHING);
						pPlayer->UpdateSkill(SKILL_BLACKSMITHING, 450);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						_creature->Whisper("Você não tem o número de Token PvP (100) suficiente para treinar essa profissão.", LANG_UNIVERSAL, NULL);
					}
				}break;
			case 3:
				if (pPlayer->HasSkill(SKILL_LEATHERWORKING))
				{
					_creature->Whisper("Você já aprendeu essa profissão.", LANG_UNIVERSAL, NULL);
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_LEATHERWORKING);
						pPlayer->UpdateSkill(SKILL_LEATHERWORKING, 450);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						_creature->Whisper("Você não tem o número de Token PvP (100) suficiente para treinar essa profissão.", LANG_UNIVERSAL, NULL);
					}
				}break;
			case 4:
				if (pPlayer->HasSkill(SKILL_TAILORING))
				{
					_creature->Whisper("Você já aprendeu essa profissão.", LANG_UNIVERSAL, NULL);
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_TAILORING);
						pPlayer->UpdateSkill(SKILL_TAILORING, 450);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						_creature->Whisper("Você não tem o número de Token PvP (100) suficiente para treinar essa profissão.", LANG_UNIVERSAL, NULL);
					}
				}break;
			case 5:
				if (pPlayer->HasSkill(SKILL_ENGINEERING))
				{
					_creature->Whisper("Você já aprendeu essa profissão.", LANG_UNIVERSAL, NULL);
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_ENGINEERING);
						pPlayer->UpdateSkill(SKILL_ENGINEERING, 450);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						_creature->Whisper("Você não tem o número de Token PvP (100) suficiente para treinar essa profissão.", LANG_UNIVERSAL, NULL);
					}
				}break;

			case 6:
				if (pPlayer->HasSkill(SKILL_ENCHANTING))
				{
					_creature->Whisper("Você já aprendeu essa profissão.", LANG_UNIVERSAL, NULL);
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_ENCHANTING);
						pPlayer->UpdateSkill(SKILL_ENCHANTING, 450);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						_creature->Whisper("Você não tem o número de Token PvP (100) suficiente para treinar essa profissão.", LANG_UNIVERSAL, NULL);
					}
				}break;
			case 7:
				if (pPlayer->HasSkill(SKILL_JEWELCRAFTING))
				{
					_creature->Whisper("Você já aprendeu essa profissão.", LANG_UNIVERSAL, NULL);
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_JEWELCRAFTING);
						pPlayer->UpdateSkill(SKILL_JEWELCRAFTING, 450);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						_creature->Whisper("Você não tem o número de Token PvP (100) suficiente para treinar essa profissão.", LANG_UNIVERSAL, NULL);
					}
				}break;
			case 8:
				if (pPlayer->HasSkill(SKILL_INSCRIPTION))
				{
					_creature->Whisper("Você já aprendeu essa profissão.", LANG_UNIVERSAL, NULL);
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_INSCRIPTION);
						pPlayer->UpdateSkill(SKILL_INSCRIPTION, 450);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						_creature->Whisper("Você não tem o número de Token PvP (100) suficiente para treinar essa profissão.", LANG_UNIVERSAL, NULL);
					}
				}break;
			case 9:
				if (pPlayer->HasSkill(SKILL_COOKING))
				{
					_creature->Whisper("Você já aprendeu essa profissão.", LANG_UNIVERSAL, NULL);
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_COOKING);
						pPlayer->UpdateSkill(SKILL_COOKING, 450);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						_creature->Whisper("Você não tem o número de Token PvP (100) suficiente para treinar essa profissão.", LANG_UNIVERSAL, NULL);
					}
				}break;
			case 10:
				if (pPlayer->HasSkill(SKILL_FIRST_AID))
				{
					_creature->Whisper("Você já aprendeu essa profissão.", LANG_UNIVERSAL, NULL);
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_FIRST_AID);
						pPlayer->UpdateSkill(SKILL_FIRST_AID, 450);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						_creature->Whisper("Você não tem o número de Token PvP (100) suficiente para treinar essa profissão.", LANG_UNIVERSAL, NULL);
					}
				}break;
			case 11:
				if (pPlayer->HasSkill(SKILL_HERBALISM))
				{
					_creature->Whisper("Você já aprendeu essa profissão.", LANG_UNIVERSAL, NULL);
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_HERBALISM);
						pPlayer->UpdateSkill(SKILL_HERBALISM, 450);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						_creature->Whisper("Você não tem o número de Token PvP (100) suficiente para treinar essa profissão.", LANG_UNIVERSAL, NULL);
					}
				}break;
			case 12:
				if (pPlayer->HasSkill(SKILL_SKINNING))
				{
					_creature->Whisper("Você já aprendeu essa profissão.", LANG_UNIVERSAL, NULL);
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_SKINNING);
						pPlayer->UpdateSkill(SKILL_SKINNING, 450);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						_creature->Whisper("Você não tem o número de Token PvP (100) suficiente para treinar essa profissão.", LANG_UNIVERSAL, NULL);
					}
				}break;
			case 13:
				if (pPlayer->HasSkill(SKILL_MINING))
				{
					_creature->Whisper("Você já aprendeu essa profissão.", LANG_UNIVERSAL, NULL);
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_MINING);
						pPlayer->UpdateSkill(SKILL_MINING, 450);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						_creature->Whisper("Você não tem o número de Token PvP (100) suficiente para treinar essa profissão.", LANG_UNIVERSAL, NULL);
					}
				}break;
			}
		}
		return true;
	}
};

void AddSC_Professions_NPC()
{
	new Professions_NPC();
}