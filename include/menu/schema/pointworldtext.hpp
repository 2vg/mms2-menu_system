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


#ifndef _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_POINTWORLDTEXT_HPP_
#	define _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_POINTWORLDTEXT_HPP_

#	pragma once

#	include <menu/schema.hpp>
#	include <menu/schema/basemodelentity.hpp>

#	include <color.h>
#	include <tier0/dbg.h>
#	include <tier0/platform.h>
#	include <tier0/utlstringtoken.h>

#	define CPOINTWORLDTEXT_CLASS_NAME "CPointWorldText"

enum PointWorldTextJustifyHorizontal_t
{
	POINT_WORLD_TEXT_JUSTIFY_HORIZONTAL_LEFT = 0,
	POINT_WORLD_TEXT_JUSTIFY_HORIZONTAL_CENTER = 1,
	POINT_WORLD_TEXT_JUSTIFY_HORIZONTAL_RIGHT = 2,
};

enum PointWorldTextJustifyVertical_t
{
	POINT_WORLD_TEXT_JUSTIFY_VERTICAL_BOTTOM = 0,
	POINT_WORLD_TEXT_JUSTIFY_VERTICAL_CENTER = 1,
	POINT_WORLD_TEXT_JUSTIFY_VERTICAL_TOP = 2,
};

enum PointWorldTextReorientMode_t
{
	POINT_WORLD_TEXT_REORIENT_NONE = 0,
	POINT_WORLD_TEXT_REORIENT_AROUND_UP = 1,
};

class CPointWorldText : public CBaseModelEntity // < CModelPointEntity
{
public:
	// ...
};

namespace Menu
{
	namespace Schema
	{
		class CPointWorldText_Helper : virtual public CBaseModelEntity_Helper
		{
		public:
			void AddListeners(CSystem *pSchemaSystemHelper);
			void Clear();

		public:
			SCHEMA_INSTANCE_ARRAY_ACCESSOR_METHOD(GetMessageTextAccessor, CPointWorldText, char, m_aOffsets.m_aMessageText.nValue, m_aOffsets.m_aMessageText.nArraySize);
			SCHEMA_INSTANCE_ARRAY_ACCESSOR_METHOD(GetBackgroundMaterialNameAccessor, CPointWorldText, char, m_aOffsets.m_aBackgroundMaterialName.nValue, m_aOffsets.m_aBackgroundMaterialName.nArraySize);
			SCHEMA_INSTANCE_ACCESSOR_METHOD(GetJustifyHorizontalAccessor, CPointWorldText, PointWorldTextJustifyHorizontal_t, m_aOffsets.m_nJustifyHorizontal);
			SCHEMA_INSTANCE_ACCESSOR_METHOD(GetJustifyVerticalAccessor, CPointWorldText, PointWorldTextJustifyVertical_t, m_aOffsets.m_nJustifyVertical);
			SCHEMA_INSTANCE_ACCESSOR_METHOD(GetReorientModeAccessor, CPointWorldText, PointWorldTextReorientMode_t, m_aOffsets.m_nReorientMode);
			SCHEMA_INSTANCE_ACCESSOR_METHOD(GetColorAccessor, CPointWorldText, Color, m_aOffsets.m_nColor);

			SCHEMA_FORCEINLINE int GetMessageTextSize() const
			{
				int nArraySize = m_aOffsets.m_aMessageText.nArraySize;

				Assert(nArraySize != INVALID_SCHEMA_FIELD_ARRAY_SIZE);

				return nArraySize;
			}

		private:
			CSystem::CClass *m_pClass;
			CSystem::CClass::Fields::CListenerCallbacksCollector m_aClassFieldsClassbacks;

			struct
			{
				struct
				{
					int nValue = INVALID_SCHEMA_FIELD_OFFSET;
					int nArraySize = INVALID_SCHEMA_FIELD_ARRAY_SIZE;
				} m_aMessageText;

				struct
				{
					int nValue = INVALID_SCHEMA_FIELD_OFFSET;
					int nArraySize = INVALID_SCHEMA_FIELD_ARRAY_SIZE;
				} m_aBackgroundMaterialName;

				int m_nJustifyHorizontal = INVALID_SCHEMA_FIELD_OFFSET;
				int m_nJustifyVertical = INVALID_SCHEMA_FIELD_OFFSET;
				int m_nReorientMode = INVALID_SCHEMA_FIELD_OFFSET;
				int m_nColor = INVALID_SCHEMA_FIELD_OFFSET;
			} m_aOffsets;
		}; // Menu::Schema::CPointWorldText_Helper
	}; // Menu::Schema
}; // Menu

#endif // _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_POINTWORLDTEXT_HPP_
