/**
 * vim: set ts=4 sw=4 tw=99 noet :
 * ======================================================
 * Metamod:Source Menu System
 * Written by Wend4r & komashchenko (Vladimir Ezhikov & Borys Komashchenko).
 * ======================================================

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "menusystem_plugin.hpp"
#include <menusystem_exports.h>

#include <algorithm>
#include <utility>
#include <map>

class CMenuWrapper : public IMenuHandler, public IMenu::IItemHandler
{
public:
	using Key_t = std::pair<IMenuItemPosition_t, IMenu_t *>;
	using Value_t = IMenuItemHandler_t;

public: // IMenuHandler
	void OnMenuDestroy(IMenu *pMenu) override
	{
		RemoveHandlers(pMenu);
	}

public: // IMenu::IItemHandler
	void OnMenuSelectItem(IMenu *pMenu, CPlayerSlot aSlot, IMenu::ItemPosition_t iItem, IMenu::ItemPositionOnPage_t iItemOnPage, void *pData) override
	{
		auto itFound = m_mapHandlers.find({iItem, pMenu});

		if(itFound == m_mapHandlers.cend())
		{
			return;
		}

		auto *pHandler = itFound->second;

		if(!pHandler)
		{
			return;
		}

		pHandler(pMenu, aSlot, iItem, iItemOnPage, pData);
	}

public:
	void AddHandler(const Key_t &aKey, const Value_t &aValue)
	{
		m_mapHandlers.emplace(aKey, aValue);
	}

	void RemoveHandlers(const IMenu_t *pMenu)
	{
		auto &mapHandlers = m_mapHandlers;

		for(auto it = mapHandlers.begin(); it != mapHandlers.end();)
		{
			if(it->first.second == pMenu)
			{
				it = mapHandlers.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

private:
	std::map<Key_t, Value_t> m_mapHandlers;
} g_aMenuWrapper;

// The menu system functions.

MENU_DLL_EXPORT IMenuSystem_t *MenuSystem()
{
	return static_cast<IMenuSystem *>(g_pMenuPlugin);
}

MENU_DLL_EXPORT IMenuProfileSystem_t *MenuSystem_GetProfiles(IMenuSystem_t *pSystem)
{
	return pSystem->GetProfiles();
}

MENU_DLL_EXPORT IMenu_t *MenuSystem_CreateInstance(IMenuSystem_t *pSystem, IMenuProfile_t *pProfile)
{
	return pSystem->CreateInstance(pProfile, static_cast<IMenuHandler *>(&g_aMenuWrapper));
}

MENU_DLL_EXPORT bool MenuSystem_DisplayInstanceToPlayer(IMenuSystem_t *pSystem, IMenu_t *pMenuInstance, CPlayerSlot aSlot, IMenuItemPosition_t iStartItem, int nManyTimes)
{
	return pSystem->DisplayInstanceToPlayer(pMenuInstance, aSlot, iStartItem, nManyTimes);
}

MENU_DLL_EXPORT bool MenuSystem_CloseInstance(IMenuSystem_t *pSystem, IMenu_t *pMenuInstance)
{
	return pSystem->CloseInstance(pMenuInstance);
}

// The menu profile system functions.

MENU_DLL_EXPORT IMenuProfile_t *MenuProfileSystem_Get(IMenuProfileSystem_t *pProfileSystem, const char *pszName)
{
	return pProfileSystem->Get(pszName);
}

// The menu instance functions.

MENU_DLL_EXPORT const char *Menu_GetTitle(IMenu_t *pMenu)
{
	return pMenu->GetTitleRef().Get();
}

MENU_DLL_EXPORT void Menu_SetTitle(IMenu_t *pMenu, const char *pszNewText)
{
	return pMenu->GetTitleRef().Set(pszNewText);
}

MENU_DLL_EXPORT IMenuItemStyleFlags_t Menu_GetItemStyles(IMenu_t *pMenu, IMenuItemPosition_t iItem)
{
	return pMenu->GetItemsRef().Element(iItem).GetStyle();
}

MENU_DLL_EXPORT const char *Menu_GetItemContent(IMenu_t *pMenu, IMenuItemPosition_t iItem)
{
	return pMenu->GetItemsRef().Element(iItem).Get();
}

MENU_DLL_EXPORT IMenuItemPosition_t Menu_AddItem(IMenu_t *pMenu, IMenuItemStyleFlags_t eFlags, const char *pszContent, IMenuItemHandler_t pfnItemHandler, void *pData)
{
	auto &vecItems = pMenu->GetItemsRef();

	g_aMenuWrapper.AddHandler(std::make_pair(vecItems.Count(), pMenu), pfnItemHandler);

	return vecItems.AddToTail({eFlags, pszContent, static_cast<IMenu::IItemHandler *>(&g_aMenuWrapper), pData});
}

MENU_DLL_EXPORT void Menu_RemoveItem(IMenu_t *pMenu, IMenuItemPosition_t iItem)
{
	pMenu->GetItemsRef().Remove(iItem);
}

MENU_DLL_EXPORT IMenuItemControlFlags_t Menu_GetItemControls(IMenu_t *pMenu)
{
	return pMenu->GetItemControlsRef();
}

MENU_DLL_EXPORT void Menu_SetItemControls(IMenu_t *pMenu, IMenuItemControlFlags_t eNewControls)
{
	pMenu->GetItemControlsRef() = eNewControls;
}

MENU_DLL_EXPORT IMenuItemPosition_t Menu_GetCurrentPosition(IMenu_t *pMenu, CPlayerSlot aSlot)
{
	return pMenu->GetCurrentPosition(aSlot);
}
MENU_DLL_EXPORT IMenu_t *Menu_GetPlayerActiveMenu(IMenuSystem_t *pSystem, CPlayerSlot aSlot)
{
	if (!pSystem)
	{
		return nullptr;
	}

	auto *pPlayer = pSystem->GetPlayer(aSlot);
	if (!pPlayer)
	{
		return nullptr;
	}

	auto iActiveMenu = pPlayer->GetActiveMenuIndex();
	if (iActiveMenu < 0)
	{
		return nullptr;
	}

	auto &vecMenus = pPlayer->GetMenus();
	if (iActiveMenu >= vecMenus.Count())
	{
		return nullptr;
	}

	return vecMenus[iActiveMenu].m_pInstance;
}
