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

#ifndef _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_CPOINTORIENT_HPP_
#	define _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_CPOINTORIENT_HPP_

#	pragma once

#	include <menu/schema/baseentity.hpp>
#	include <menu/schema.hpp>

#	include <tier0/utlsymbol.h>
#	include <entity2/entityhandle.h>

#	define CPOINTORIENT_CLASS_NAME "CPointOrient"

enum PointOrientGoalDirectionType_t : uint32_t
{
	eAbsOrigin = 0,
	eCenter = 1,
	eHead = 2,
	eForward = 3,
	eEyesForward = 4,
};

enum PointOrientConstraint_t : uint32_t
{
	eNone = 0,
	ePreserveUpAxis = 1,
};

class CPointOrient : public CBaseEntity
{
public:
	// Schema fields will be accessed through helper class
};

namespace Menu
{
	namespace Schema
	{
		class CPointOrient_Helper : virtual public CBaseEntity_Helper
		{
		public:
			using Base = CBaseEntity_Helper;

		public:
			void AddListeners(CSystem *pSchemaSystemHelper);
			void Clear();

		public:
			SCHEMA_INSTANCE_ACCESSOR_METHOD(GetSpawnTargetNameAccessor, CPointOrient, CUtlSymbolLarge, m_aOffsets.m_nSpawnTargetName);
			SCHEMA_INSTANCE_ACCESSOR_METHOD(GetTargetAccessor, CPointOrient, CHandle<CBaseEntity>, m_aOffsets.m_nTarget);
			SCHEMA_INSTANCE_ACCESSOR_METHOD(GetActiveAccessor, CPointOrient, bool, m_aOffsets.m_nActive);
			SCHEMA_INSTANCE_ACCESSOR_METHOD(GetGoalDirectionAccessor, CPointOrient, PointOrientGoalDirectionType_t, m_aOffsets.m_nGoalDirection);
			SCHEMA_INSTANCE_ACCESSOR_METHOD(GetConstraintAccessor, CPointOrient, PointOrientConstraint_t, m_aOffsets.m_nConstraint);
			SCHEMA_INSTANCE_ACCESSOR_METHOD(GetMaxTurnRateAccessor, CPointOrient, float32, m_aOffsets.m_nMaxTurnRate);
			SCHEMA_INSTANCE_ACCESSOR_METHOD(GetLastGameTimeAccessor, CPointOrient, GameTime_t, m_aOffsets.m_nLastGameTime);

		private:
			CSystem::CClass *m_pClass;
			CSystem::CClass::Fields::CListenerCallbacksCollector m_aClassFieldsClassbacks;

			struct
			{
				int m_nSpawnTargetName = INVALID_SCHEMA_FIELD_OFFSET;
				int m_nTarget = INVALID_SCHEMA_FIELD_OFFSET;
				int m_nActive = INVALID_SCHEMA_FIELD_OFFSET;
				int m_nGoalDirection = INVALID_SCHEMA_FIELD_OFFSET;
				int m_nConstraint = INVALID_SCHEMA_FIELD_OFFSET;
				int m_nMaxTurnRate = INVALID_SCHEMA_FIELD_OFFSET;
				int m_nLastGameTime = INVALID_SCHEMA_FIELD_OFFSET;
			} m_aOffsets;
		}; // Menu::Schema::CPointOrient_Helper
	}; // Menu::Schema
}; // Menu

#endif // _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_CPOINTORIENT_HPP_
