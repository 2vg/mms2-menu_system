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
#	include <menu/schema/cpointorient.hpp>
#	include <menu/schema.hpp>

#	define CCSPLAYERPAWN_CLASS_NAME "CCSPlayerPawn"

class CPointOrient;

class CCSPlayerPawn : public CCSPlayerPawnBase
{
};

namespace Menu
{
	namespace Schema
	{
		class CCSPlayerPawn_Helper : virtual public CCSPlayerPawnBase_Helper, virtual public CPointOrient_Helper
		{
		public:
			using Base = CCSPlayerPawnBase_Helper;
			using PointOrientBase = CPointOrient_Helper;

		public:
			void AddListeners(CSystem *pSchemaSystemHelper);
			
			// PointOrient management methods
			void SetPointOrient(CCSPlayerPawn *pCSPlayerPawn, CPointOrient *pOrient);
			CPointOrient* GetPointOrient(CCSPlayerPawn *pCSPlayerPawn);
			void CreatePointOrient(CCSPlayerPawn *pCSPlayerPawn);

		private:
			// Store PointOrient handle per player
			static CHandle<CPointOrient> s_aPlayerPointOrients[64]; // Max 64 players

		// private:
		// 	CSystem::CClass *m_pClass;
		// 	CSystem::CClass::Fields::CListenerCallbacksCollector m_aClassFieldsClassbacks;
		}; // Menu::Schema::CCSPlayerPawn_Helper
	}; // Menu::Schema
}; // Menu

#endif // _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_CSPLAYERPAWN_HPP_
