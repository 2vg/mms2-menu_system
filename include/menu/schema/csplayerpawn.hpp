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

#ifndef _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_CSPLAYERPAWN_HPP_
#	define _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_CSPLAYERPAWN_HPP_

#	pragma once

#	include <menu/schema/csplayerpawnbase.hpp>
#	include <menu/schema.hpp>

#	define CCSPLAYERPAWN_CLASS_NAME "CCSPlayerPawn"

class CCSPlayer_ViewModelServices;

class CCSPlayerPawn : public CCSPlayerPawnBase
{
};

namespace Menu
{
	namespace Schema
	{
		class CCSPlayerPawn_Helper : virtual public CCSPlayerPawnBase_Helper
		{
		public:
			using Base = CCSPlayerPawnBase_Helper;

		public:
			auto GetViewModelServicesAccessor(CCSPlayerPawn *pCSPlayerPawn)
			{
				return Base::GetViewModelServicesAccessor(pCSPlayerPawn).Cast<CCSPlayer_ViewModelServices *>();
			}

		// private:
		// 	CSystem::CClass *m_pClass;
		// 	CSystem::CClass::Fields::CListenerCallbacksCollector m_aClassFieldsClassbacks;
		}; // Menu::Schema::CCSPlayerPawn_Helper
	}; // Menu::Schema
}; // Menu

#endif // _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_CSPLAYERPAWN_HPP_
