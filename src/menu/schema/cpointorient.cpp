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

#include <menu/schema/cpointorient.hpp>

void Menu::Schema::CPointOrient_Helper::AddListeners(CSystem *pSchemaSystemHelper)
{
	m_pClass = pSchemaSystemHelper->FindSchemaClassBinding(CPOINTORIENT_CLASS_NAME);

	CSystem::CClass::Fields::CListenerCallbacksCollector aCallbacks;

	aCallbacks.Insert(m_pClass->GetFieldSymbol("m_iszSpawnTargetName"), SCHEMA_CLASS_FIELD_SHARED_LAMBDA_CAPTURE(m_aOffsets.m_nSpawnTargetName));
	aCallbacks.Insert(m_pClass->GetFieldSymbol("m_hTarget"), SCHEMA_CLASS_FIELD_SHARED_LAMBDA_CAPTURE(m_aOffsets.m_nTarget));
	aCallbacks.Insert(m_pClass->GetFieldSymbol("m_bActive"), SCHEMA_CLASS_FIELD_SHARED_LAMBDA_CAPTURE(m_aOffsets.m_nActive));
	aCallbacks.Insert(m_pClass->GetFieldSymbol("m_nGoalDirection"), SCHEMA_CLASS_FIELD_SHARED_LAMBDA_CAPTURE(m_aOffsets.m_nGoalDirection));
	aCallbacks.Insert(m_pClass->GetFieldSymbol("m_nConstraint"), SCHEMA_CLASS_FIELD_SHARED_LAMBDA_CAPTURE(m_aOffsets.m_nConstraint));
	aCallbacks.Insert(m_pClass->GetFieldSymbol("m_flMaxTurnRate"), SCHEMA_CLASS_FIELD_SHARED_LAMBDA_CAPTURE(m_aOffsets.m_nMaxTurnRate));
	aCallbacks.Insert(m_pClass->GetFieldSymbol("m_flLastGameTime"), SCHEMA_CLASS_FIELD_SHARED_LAMBDA_CAPTURE(m_aOffsets.m_nLastGameTime));

	m_aClassFieldsClassbacks = aCallbacks;
}

void Menu::Schema::CPointOrient_Helper::Clear()
{
	m_aClassFieldsClassbacks.Clear();
}
