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

#include <menu/chatcommandsystem.hpp>

#include <tier1/utlrbtree.h>

Menu::CChatCommandSystem::CChatCommandSystem()
 :  CLogger(GetName(), NULL, 0, LV_DEFAULT, MENU_CHATCOMMANDSYSTEM_LOGGINING_COLOR), 
    Base()
{
}

const char *Menu::CChatCommandSystem::GetName()
{
	return "Menu - Chat Command System";
}

const char *Menu::CChatCommandSystem::GetHandlerLowercaseName()
{
	return "chat command";
}

char Menu::CChatCommandSystem::GetPublicTrigger()
{
	return '!';
}

char Menu::CChatCommandSystem::GetSilentTrigger()
{
	return '/';
}

bool Menu::CChatCommandSystem::Handle(const char *pszName, CPlayerSlot aSlot, bool bIsSilent, const CUtlVector<CUtlString> &vecArgs)
{
	if(!aSlot.IsValid())
	{
		CLogger::Message("Type the chat command from root console?\n");

		return false;
	}

	if(!vecArgs.Count())
	{
		if(CLogger::IsChannelEnabled(LS_DETAILED))
		{
			CLogger::Detailed("Chat command arguments is empty\n");
		}

		return false;
	}

	return Base::Handle(pszName, aSlot, bIsSilent, vecArgs);
}

