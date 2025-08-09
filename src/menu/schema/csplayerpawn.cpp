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

#include <menu/schema/csplayerpawn.hpp>
#include <menu/schema/cpointorient.hpp>

#include <entity2/entitysystem.h>
#include <mathlib/vector.h>
#include <mathlib/mathlib.h>

// Static storage for PointOrient handles per player
CHandle<CPointOrient> Menu::Schema::CCSPlayerPawn_Helper::s_aPlayerPointOrients[64];

void Menu::Schema::CCSPlayerPawn_Helper::SetPointOrient(CCSPlayerPawn *pCSPlayerPawn, CPointOrient *pOrient)
{
	if (!pCSPlayerPawn)
		return;

	int nPlayerSlot = pCSPlayerPawn->GetEntityIndex().Get() % 64; // Simple slot calculation
	s_aPlayerPointOrients[nPlayerSlot].Set(pOrient);
}

CPointOrient* Menu::Schema::CCSPlayerPawn_Helper::GetPointOrient(CCSPlayerPawn *pCSPlayerPawn)
{
	if (!pCSPlayerPawn)
		return nullptr;

	int nPlayerSlot = pCSPlayerPawn->GetEntityIndex().Get() % 64; // Simple slot calculation
	return s_aPlayerPointOrients[nPlayerSlot].Get();
}

void Menu::Schema::CCSPlayerPawn_Helper::CreatePointOrient(CCSPlayerPawn *pCSPlayerPawn)
{
	if (!pCSPlayerPawn)
		return;

	// Remove existing PointOrient if any
	CPointOrient* pExistingOrient = GetPointOrient(pCSPlayerPawn);
	if (pExistingOrient)
	{
		pExistingOrient->Remove();
		SetPointOrient(pCSPlayerPawn, nullptr);
	}

	// Create new PointOrient entity
	CPointOrient* pOrient = static_cast<CPointOrient*>(CreateEntityByName("point_orient"));
	if (!pOrient)
		return;

	// Configure PointOrient properties
	CPointOrient_Helper::GetActiveAccessor(pOrient) = true;
	CPointOrient_Helper::GetGoalDirectionAccessor(pOrient) = PointOrientGoalDirectionType_t::eEyesForward;

	// Spawn the entity
	pOrient->DispatchSpawn();
	
	// Store the PointOrient handle
	SetPointOrient(pCSPlayerPawn, pOrient);

	// Position the PointOrient at player's eye position
	Vector vecEyePosition = pCSPlayerPawn->GetEyePosition();
	pOrient->Teleport(&vecEyePosition, nullptr, nullptr);

	// Set parent and target to the player
	variant_t activatorVariant;
	activatorVariant.SetEntity(pCSPlayerPawn);
	
	pOrient->AcceptInput("SetParent", "!activator", pCSPlayerPawn, &activatorVariant, 0);
	pOrient->AcceptInput("SetTarget", "!activator", pCSPlayerPawn, &activatorVariant, 0);
}
