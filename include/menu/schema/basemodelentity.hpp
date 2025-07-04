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

#ifndef _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_BASEMODELENTITY_HPP_
#	define _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_BASEMODELENTITY_HPP_

#	pragma once

#	include <menu/schema/baseentity.hpp>
#	include <menu/schema.hpp>

#	define CBASEMODELENTITY_CLASS_NAME "CBaseModelEntity"

class QAngle;
class Vector;

class CBaseModelEntity : public CBaseEntity
{
public:
	// ...
};

namespace Menu
{
	namespace Schema
	{
		class CBaseModelEntity_Helper : virtual public CBaseEntity_Helper
		{
		public:
			void AddListeners(CSystem *pSchemaSystemHelper);
			void Clear();

		public:
			SCHEMA_INSTANCE_ACCESSOR_METHOD(GetViewOffsetAccessor, CBaseModelEntity, Vector, m_aOffsets.m_nViewOffset);

		private:
			CSystem::CClass *m_pClass;
			CSystem::CClass::Fields::CListenerCallbacksCollector m_aClassFieldsClassbacks;

			struct
			{
				int m_nViewOffset = INVALID_SCHEMA_FIELD_OFFSET;
			} m_aOffsets;
		}; // Menu::Schema::CBaseModelEntity_Helper
	}; // Menu::Schema
}; // Menu

#endif // _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_BASEMODELENTITY_HPP_
