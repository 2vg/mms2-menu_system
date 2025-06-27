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

#ifndef _INCLUDE_METAMOD_SOURCE_MENUSYSTEM_EXPORTS_H_
#	define _INCLUDE_METAMOD_SOURCE_MENUSYSTEM_EXPORTS_H_

#	ifndef __cplusplus
#		include <stdbool.h>
#	endif // !__cplusplus

#	include <stddef.h>

#	ifdef __cplusplus
#		define MENU_DLL_EXTERN_C extern "C"
#	else
#		define MENU_DLL_EXTERN_C extern
#	endif // __cplusplus

#	ifdef _WIN32
#		define MENU_DLL_EXPORT MENU_DLL_EXTERN_C __declspec( dllexport )
#		define MENU_DLL_IMPORT MENU_DLL_EXTERN_C __declspec( dllimport )
#	else
#		define MENU_DLL_DECLARATION_DEFAULT_VISIBILITY __attribute__(( visibility( "default" ) ))
#		define MENU_DLL_EXPORT MENU_DLL_EXTERN_C MENU_DLL_DECLARATION_DEFAULT_VISIBILITY
#		define MENU_DLL_IMPORT MENU_DLL_EXTERN_C
#	endif // _WIN32

#	ifdef __cplusplus
#		include "imenusystem.hpp"

using IMenuSystem_t = IMenuSystem;
using IMenuProfileSystem_t = IMenuProfileSystem;
using IMenu_t = IMenu;
using IMenuItemPosition_t = IMenu::ItemPosition_t;
using IMenuItemStyleFlags_t = IMenu::ItemStyleFlags_t;
using IMenuItemHandler_t = void (*)(IMenu_t *pMenu, CPlayerSlot aSlot, IMenuItemPosition_t iItem, IMenuItemPosition_t iItemOnPage, void *pData);
using IMenuItemControlFlags_t = IMenu::ItemControlFlags_t;
using IMenuProfile_t = IMenuProfile;
#	else
typedef void IMenuSystem_t;
typedef void IMenuProfileSystem_t;
typedef void IMenu_t;
typedef int CPlayerSlot;
typedef int IMenuItemPosition_t;
enum IMenuItemStyleFlags_t
{
	MENU_ITEM_ACTIVE =      (1 << 0),
	MENU_ITEM_HASNUMBER =   (1 << 1),
	MENU_ITEM_CONTROL =     (1 << 2),
};
typedef void (*IMenuItemHandler_t)(IMenu_t *pMenu, CPlayerSlot aSlot, IMenuItemPosition_t iItem, IMenuItemPosition_t iItemOnPage, void *pData);
enum IMenuItemControlFlags_t
{
	MENU_ITEM_CONTROL_FLAG_PANEL = 0,
	MENU_ITEM_CONTROL_FLAG_BACK = (1 << 0),
	MENU_ITEM_CONTROL_FLAG_NEXT = (1 << 1),
	MENU_ITEM_CONTROL_FLAG_EXIT = (1 << 2),
};
typedef void IMenuProfile_t;
#	endif // __cplusplus

/// The menu system.
MENU_DLL_EXPORT IMenuSystem_t *MenuSystem(); // Gets a main pointer to menu system.
MENU_DLL_EXPORT IMenuProfileSystem_t *MenuSystem_GetProfiles(IMenuSystem_t *pSystem); // See IMenuSystem::GetProfiles.
MENU_DLL_EXPORT IMenu_t *MenuSystem_CreateInstance(IMenuSystem_t *pSystem, IMenuProfile_t *pProfile); // See IMenuSystem::CreateInstance.
MENU_DLL_EXPORT bool MenuSystem_DisplayInstanceToPlayer(IMenuSystem_t *pSystem, IMenu_t *pMenuInstance, CPlayerSlot aSlot, IMenuItemPosition_t iStartItem = 0, int nManyTimes = 0); // See IMenuSystem::DisplayInstanceToPlayer.
MENU_DLL_EXPORT bool MenuSystem_CloseInstance(IMenuSystem_t *pSystem, IMenu_t *pMenuInstance); // See IMenuSystem::CloseInstance.

/// The menu profile system.
MENU_DLL_EXPORT IMenuProfile_t *MenuProfileSystem_Get(IMenuProfileSystem_t *pProfileSystem, const char *pszName = "default");

/// The menu instance.

// See IMenu::GetTitleRef
MENU_DLL_EXPORT const char *Menu_GetTitle(IMenu_t *pMenu);
MENU_DLL_EXPORT void Menu_SetTitle(IMenu_t *pMenu, const char *pszNewText);

// See IMenu::GetItemsRef.
MENU_DLL_EXPORT IMenuItemStyleFlags_t Menu_GetItemStyles(IMenu_t *pMenu, IMenuItemPosition_t iItem);
MENU_DLL_EXPORT const char *Menu_GetItemContent(IMenu_t *pMenu, IMenuItemPosition_t iItem);
MENU_DLL_EXPORT IMenuItemPosition_t Menu_AddItem(IMenu_t *pMenu, IMenuItemStyleFlags_t eFlags, const char *pszContent, IMenuItemHandler_t pfnItemHandler = NULL, void *pData = NULL);
MENU_DLL_EXPORT void Menu_RemoveItem(IMenu_t *pMenu, IMenuItemPosition_t iItem);

// See IMenu::GetItemControlsRef.
MENU_DLL_EXPORT IMenuItemControlFlags_t Menu_GetItemControls(IMenu_t *pMenu);
MENU_DLL_EXPORT void Menu_SetItemControls(IMenu_t *pMenu, IMenuItemControlFlags_t eNewControls);

// See IMenu::GetCurrentPosition.
MENU_DLL_EXPORT IMenuItemPosition_t Menu_GetCurrentPosition(IMenu_t *pMenu, CPlayerSlot aSlot);

// Get the currently active menu for a player.
MENU_DLL_EXPORT IMenu_t *Menu_GetPlayerActiveMenu(IMenuSystem_t *pSystem, CPlayerSlot aSlot);


#endif // _INCLUDE_METAMOD_SOURCE_MENUSYSTEM_EXPORTS_H_
