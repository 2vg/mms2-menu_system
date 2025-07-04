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

#ifndef _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_CSOBSERVERPAWN_HPP_
#	define _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_CSOBSERVERPAWN_HPP_

#	pragma once

#	include <menu/schema/csplayerpawnbase.hpp>
#	include <menu/schema.hpp>

#	define CCSOBERVERPAWN_CLASS_NAME "CCSObserverPawn"

class CCSPlayer_ObserverServices;

class CCSObserverPawn : public CCSPlayerPawnBase
{
public:
	// ...
};

namespace Menu
{
	namespace Schema
	{
		class CCSObserverPawn_Helper : virtual public CCSPlayerPawnBase_Helper
		{
		public:
			using Base = CCSPlayerPawnBase_Helper;

		public:
			auto GetObserverServicesAccessor(CCSObserverPawn *pCSObserverPawn)
			{
				return Base::GetObserverServicesAccessor(pCSObserverPawn).Cast<CCSPlayer_ObserverServices *>();
			}

		// private:
			// CSystem::CClass *m_pClass;
			// CSystem::CClass::Fields::CListenerCallbacksCollector m_aClassFieldsClassbacks;
		}; // Menu::Schema::CCSObserverPawn_Helper
	}; // Menu::Schema
}; // Menu

#endif // _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_CSOBSERVERPAWN_HPP_
