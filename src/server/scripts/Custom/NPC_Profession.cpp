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
		pPlayer->ADD_GOSSIP_ITEM(9, "[Profiss�es] ->", GOSSIP_SENDER_MAIN, 196);
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
			pPlayer->GetSession()->SendNotification("Voc� j� sabe duas profiss�es!");
			pCreature->Whisper("Voc� j� sabe duas profiss�es!", LANG_UNIVERSAL, NULL);
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
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Alchemy", GOSSIP_SENDER_MAIN, 1);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Blacksmithing", GOSSIP_SENDER_MAIN, 2);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Leatherworking", GOSSIP_SENDER_MAIN, 3);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Tailoring", GOSSIP_SENDER_MAIN, 4);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Engineering", GOSSIP_SENDER_MAIN, 5);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Enchanting", GOSSIP_SENDER_MAIN, 6);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Jewelcrafting", GOSSIP_SENDER_MAIN, 7);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Inscription", GOSSIP_SENDER_MAIN, 8);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Herbalism", GOSSIP_SENDER_MAIN, 11);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Skinning", GOSSIP_SENDER_MAIN, 12);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Mining", GOSSIP_SENDER_MAIN, 13);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Cooking", GOSSIP_SENDER_MAIN, 9);
				pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "First Aid", GOSSIP_SENDER_MAIN, 10);

				pPlayer->PlayerTalkClass->SendGossipMenu(907, _creature->GetGUID());
				break;
			case 1:
				if (pPlayer->HasSkill(SKILL_ALCHEMY))
				{
					pPlayer->GetSession()->SendNotification("Voc� j� aprendeu essa profiss�o.");
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_ALCHEMY);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						pPlayer->GetSession()->SendNotification("Voc� n�o tem o n�mero de Token PvP (100) suficiente para treinar essa profiss�o");
					}
				}break;
			case 2:
				if (pPlayer->HasSkill(SKILL_BLACKSMITHING))
				{
					pPlayer->GetSession()->SendNotification("Voc� j� aprendeu essa profiss�o.");
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_BLACKSMITHING);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						pPlayer->GetSession()->SendNotification("Voc� n�o tem o n�mero de Token PvP (100) suficiente para treinar essa profiss�o");
					}
				}break;
			case 3:
				if (pPlayer->HasSkill(SKILL_LEATHERWORKING))
				{
					pPlayer->GetSession()->SendNotification("Voc� j� aprendeu essa profiss�o.");
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_LEATHERWORKING);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						pPlayer->GetSession()->SendNotification("Voc� n�o tem o n�mero de Token PvP (100) suficiente para treinar essa profiss�o");
					}
				}break;
			case 4:
				if (pPlayer->HasSkill(SKILL_TAILORING))
				{
					pPlayer->GetSession()->SendNotification("Voc� j� aprendeu essa profiss�o.");
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_TAILORING);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						pPlayer->GetSession()->SendNotification("Voc� n�o tem o n�mero de Token PvP (100) suficiente para treinar essa profiss�o");
					}
				}break;
			case 5:
				if (pPlayer->HasSkill(SKILL_ENGINEERING))
				{
					pPlayer->GetSession()->SendNotification("Voc� j� aprendeu essa profiss�o.");
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_ENGINEERING);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						pPlayer->GetSession()->SendNotification("Voc� n�o tem o n�mero de Token PvP (100) suficiente para treinar essa profiss�o");
					}
				}break;

			case 6:
				if (pPlayer->HasSkill(SKILL_ENCHANTING))
				{
					pPlayer->GetSession()->SendNotification("Voc� j� aprendeu essa profiss�o.");
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_ENCHANTING);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						pPlayer->GetSession()->SendNotification("Voc� n�o tem o n�mero de Token PvP (100) suficiente para treinar essa profiss�o");
					}
				}break;
			case 7:
				if (pPlayer->HasSkill(SKILL_JEWELCRAFTING))
				{
					pPlayer->GetSession()->SendNotification("Voc� j� aprendeu essa profiss�o.");
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_JEWELCRAFTING);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						pPlayer->GetSession()->SendNotification("Voc� n�o tem o n�mero de Token PvP (100) suficiente para treinar essa profiss�o");
					}
				}break;
			case 8:
				if (pPlayer->HasSkill(SKILL_INSCRIPTION))
				{
					pPlayer->GetSession()->SendNotification("Voc� j� aprendeu essa profiss�o.");
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_INSCRIPTION);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						pPlayer->GetSession()->SendNotification("Voc� n�o tem o n�mero de Token PvP (100) suficiente para treinar essa profiss�o");
					}
				}break;
			case 9:
				if (pPlayer->HasSkill(SKILL_COOKING))
				{
					pPlayer->GetSession()->SendNotification("Voc� j� aprendeu essa profiss�o.");
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_COOKING);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						pPlayer->GetSession()->SendNotification("Voc� n�o tem o n�mero de Token PvP (100) suficiente para treinar essa profiss�o");
					}
				}break;
			case 10:
				if (pPlayer->HasSkill(SKILL_FIRST_AID))
				{
					pPlayer->GetSession()->SendNotification("Voc� j� aprendeu essa profiss�o.");
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_FIRST_AID);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						pPlayer->GetSession()->SendNotification("Voc� n�o tem o n�mero de Token PvP (100) suficiente para treinar essa profiss�o");
					}
				}break;
			case 11:
				if (pPlayer->HasSkill(SKILL_HERBALISM))
				{
					pPlayer->GetSession()->SendNotification("Voc� j� aprendeu essa profiss�o.");
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_HERBALISM);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						pPlayer->GetSession()->SendNotification("Voc� n�o tem o n�mero de Token PvP (100) suficiente para treinar essa profiss�o");
					}
				}break;
			case 12:
				if (pPlayer->HasSkill(SKILL_SKINNING))
				{
					pPlayer->GetSession()->SendNotification("Voc� j� aprendeu essa profiss�o.");
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_SKINNING);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						pPlayer->GetSession()->SendNotification("Voc� n�o tem o n�mero de Token PvP (100) suficiente para treinar essa profiss�o");
					}
				}break;
			case 13:
				if (pPlayer->HasSkill(SKILL_MINING))
				{
					pPlayer->GetSession()->SendNotification("Voc� j� aprendeu essa profiss�o.");
					pPlayer->PlayerTalkClass->SendCloseGossip();
					break;
				}
				else
				{
					if (pPlayer->HasItemCount(TOKEN_PVP, QTD_TOKEN, false))
					{
						CompleteLearnProfession(pPlayer, _creature, SKILL_MINING);
						pPlayer->DestroyItemCount(TOKEN_PVP, QTD_TOKEN, true);
						pPlayer->PlayerTalkClass->SendCloseGossip();
					}
					else
					{
						pPlayer->PlayerTalkClass->SendCloseGossip();
						pPlayer->GetSession()->SendNotification("Voc� n�o tem o n�mero de Token PvP (100) suficiente para treinar essa profiss�o");
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