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

#ifndef _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_CSWEAPONBASEVDATA_HPP_
#	define _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_CSWEAPONBASEVDATA_HPP_

#	pragma once

#	include <menu/schema/baseplayerweaponvdata.hpp>
#	include <menu/schema.hpp>

#	define CCSWEAPONBASEVDATA_CLASS_NAME "CCSWeaponBaseVData"

enum gear_slot_t : int32
{
	GEAR_SLOT_INVALID = -1,

	GEAR_SLOT_RIFLE = 0,
	GEAR_SLOT_PISTOL = 1,
	GEAR_SLOT_KNIFE = 2,
	GEAR_SLOT_GRENADES = 3,
	GEAR_SLOT_C4 = 4,
	GEAR_SLOT_RESERVED_SLOT6 = 5,
	GEAR_SLOT_RESERVED_SLOT7 = 6,
	GEAR_SLOT_RESERVED_SLOT8 = 7,
	GEAR_SLOT_RESERVED_SLOT9 = 8,
	GEAR_SLOT_RESERVED_SLOT10 = 9,
	GEAR_SLOT_RESERVED_SLOT11 = 10,
	GEAR_SLOT_BOOSTS = 12,
	GEAR_SLOT_UTILITY = 13,
	GEAR_SLOT_COUNT,

	GEAR_SLOT_FIRST = 0,
	GEAR_SLOT_LAST = GEAR_SLOT_COUNT - 1,
};

class CCSWeaponBaseVData : public CBasePlayerWeaponVData
{
public:
	// ...
};

namespace Menu
{
	namespace Schema
	{
		class CCSWeaponBaseVData_Helper : virtual public CBasePlayerWeaponVData_Helper
		{
		public:
			void AddListeners(CSystem *pSchemaSystemHelper);
			void Clear();

		public:
			SCHEMA_COMPONENT_ACCESSOR_METHOD(GetGearSlotAccessor, CCSWeaponBaseVData, gear_slot_t, m_aOffsets.m_nGearSlot);

		private:
			CSystem::CClass *m_pClass;
			CSystem::CClass::Fields::CListenerCallbacksCollector m_aClassFieldsClassbacks;

			struct
			{
				int m_nGearSlot = INVALID_SCHEMA_FIELD_OFFSET;
			} m_aOffsets;
		}; // Menu::Schema::CCSWeaponBaseVData_Helper
	}; // Menu::Schema
}; // Menu

#endif // _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_CSWEAPONBASEVDATA_HPP_
