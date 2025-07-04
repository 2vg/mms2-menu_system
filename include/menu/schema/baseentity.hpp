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

#ifndef _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_BASEENTITY_HPP_
#	define _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_BASEENTITY_HPP_

#	pragma once

#	include <menu/schema.hpp>

#	include <entity2/entityinstance.h>
#	include <tier0/utlstringtoken.h>

#	define CBASEENTITY_CLASS_NAME "CBaseEntity"

class QAngle;
class Vector;
class CBodyComponent;
class CEntitySubclassVDataBase;

class CBaseEntity : public CEntityInstance
{
public:
	// ...
};

namespace Menu
{
	namespace Schema
	{
		class CBaseEntity_Helper
		{
		public:
			void AddListeners(CSystem *pSchemaSystemHelper);
			void Clear();

		public:
			SCHEMA_INSTANCE_ACCESSOR_METHOD(GetBodyComponentAccessor, CBaseEntity, CBodyComponent *, m_aOffsets.m_nBodyComponent);
			SCHEMA_INSTANCE_ACCESSOR_METHOD(GetSubclassIDAccessor, CBaseEntity, CUtlStringToken, m_aOffsets.m_nSubclassID);
			SCHEMA_INSTANCE_ACCESSOR_METHOD2(GetEntitySubclassVDataAccessor, CBaseEntity, CEntitySubclassVDataBase *, m_aOffsets.m_nSubclassID, sizeof(CUtlStringToken));
			SCHEMA_INSTANCE_ACCESSOR_METHOD(GetTeamNumAccessor, CBaseEntity, uint8, m_aOffsets.m_nTeamNum);
			SCHEMA_INSTANCE_ACCESSOR_METHOD(GetOwnerEntityAccessor, CBaseEntity, CBaseEntity *, m_aOffsets.m_nOwnerEntity);
			SCHEMA_INSTANCE_ACCESSOR_METHOD(GetEffectsAccessor, CBaseEntity, uint, m_aOffsets.m_nEffects);
			SCHEMA_INSTANCE_ACCESSOR_METHOD(GetEFlagsAccessor, CBaseEntity, int, m_aOffsets.m_nEFlags);

		private:
			CSystem::CClass *m_pClass;
			CSystem::CClass::Fields::CListenerCallbacksCollector m_aClassFieldsClassbacks;

			struct
			{
				int m_nBodyComponent = INVALID_SCHEMA_FIELD_OFFSET;
				int m_nSubclassID = INVALID_SCHEMA_FIELD_OFFSET;
				int m_nTeamNum = INVALID_SCHEMA_FIELD_OFFSET;
				int m_nOwnerEntity = INVALID_SCHEMA_FIELD_OFFSET;
				int m_nEffects = INVALID_SCHEMA_FIELD_OFFSET;
				int m_nEFlags = INVALID_SCHEMA_FIELD_OFFSET;
			} m_aOffsets;
		}; // Menu::Schema::CBaseEntity_Helper
	}; // Menu::Schema
}; // Menu

#endif // _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_BASEENTITY_HPP_
