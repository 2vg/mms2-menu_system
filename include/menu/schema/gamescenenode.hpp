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


#ifndef _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_GAMESCENENODE_HPP_
#	define _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_GAMESCENENODE_HPP_

#	pragma once

#	include <menu/schema.hpp>

#	include <tier0/dbg.h>
#	include <tier0/platform.h>
#	include <tier0/utlstringtoken.h>

#	define CGAMESCENENODE_CLASS_NAME "CGameSceneNode"

class QAngle;
class Vector;
class CGameSceneNode;

namespace Menu
{
	namespace Schema
	{
		class CGameSceneNode_Helper
		{
		public:
			void AddListeners(CSystem *pSchemaSystemHelper);
			void Clear();

		public:
			SCHEMA_COMPONENT_ACCESSOR_METHOD(GetParentAccessor, CGameSceneNode, CGameSceneNode *, m_aOffsets.m_nParent);
			SCHEMA_COMPONENT_ACCESSOR_METHOD(GetAbsOriginAccessor, CGameSceneNode, Vector, m_aOffsets.m_nAbsOrigin);
			SCHEMA_COMPONENT_ACCESSOR_METHOD(GetAbsRotationAccessor, CGameSceneNode, QAngle, m_aOffsets.m_nAbsRotation);
			SCHEMA_COMPONENT_ACCESSOR_METHOD(GetHierarchyAttachNameAccessor, CGameSceneNode, CUtlStringToken, m_aOffsets.m_nHierarchyAttachName);

		private:
			CSystem::CClass *m_pClass;
			CSystem::CClass::Fields::CListenerCallbacksCollector m_aClassFieldsClassbacks;

			struct
			{
				int m_nParent = INVALID_SCHEMA_FIELD_OFFSET;
				int m_nAbsOrigin = INVALID_SCHEMA_FIELD_OFFSET;
				int m_nAbsRotation = INVALID_SCHEMA_FIELD_OFFSET;
				int m_nHierarchyAttachName = INVALID_SCHEMA_FIELD_OFFSET;
			} m_aOffsets;
		}; // Menu::Schema::CGameSceneNode_Helper
	}; // Menu::Schema
}; // Menu

#endif // _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_GAMESCENENODE_HPP_
