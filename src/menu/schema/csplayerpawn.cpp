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
 * GNU General Public License for more details.pbaseentity

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <menu/schema/baseentity.hpp>
#include <menu/schema/csplayerpawn.hpp>
#include <menu/schema/cpointorient.hpp>
#include <menu/schema.hpp>
#include <menu/provider.hpp>
#include <menusystem_plugin.hpp>
#include <globals.hpp>

#include <entity_manager.hpp>
#include <entity_manager/provider/entitysystem.hpp>
#include <entity2/entitysystem.h>
#include <entity2/entitykeyvalues.h>
#include <entity2/entityidentity.h>
#include <mathlib/vector.h>
#include <mathlib/mathlib.h>
#include <variant.h>
#include <ehandle.h>

// Static storage for PointOrient handles per player
CHandle<CPointOrient> Menu::Schema::CCSPlayerPawn_Helper::s_aPlayerPointOrients[64];

void Menu::Schema::CCSPlayerPawn_Helper::AddListeners(CSystem *pSchemaSystemHelper)
{
	// Call both base class AddListeners methods
	CCSPlayerPawnBase_Helper::AddListeners(pSchemaSystemHelper);
	CPointOrient_Helper::AddListeners(pSchemaSystemHelper);
}

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

	// Use global entity manager provider agent
	extern EntityManager::ProviderAgent *g_pEntityManagerProviderAgent;
	if (!g_pEntityManagerProviderAgent)
		return;

	// Remove existing PointOrient if any
	CPointOrient* pExistingOrient = GetPointOrient(pCSPlayerPawn);
	if (pExistingOrient)
	{
		// Use entity manager to queue entity for destruction
		g_pEntityManagerProviderAgent->PushDestroyQueue(pExistingOrient);
		g_pEntityManagerProviderAgent->ExecuteDestroyQueued();
		SetPointOrient(pCSPlayerPawn, nullptr);
	}

	// Get the entity system provider
	auto *pEntitySystem = static_cast<EntityManager::CEntitySystemProvider*>(g_pEntityManagerProviderAgent->GetSystem());
	if (!pEntitySystem)
		return;

	// Create new PointOrient entity using entity manager
	CEntityKeyValues* pKeyValues = new CEntityKeyValues();
	pKeyValues->SetString("classname", "point_orient");
	
	// Create entity using the proper entity manager API
	CEntityInstance* pEntityInstance = pEntitySystem->CreateEntity(INVALID_SPAWN_GROUP, "point_orient", EntityNetworkingMode_t::ENTITY_NETWORKING_MODE_NEVER, CEntityIndex(-1), -1, false);
	if (!pEntityInstance)
	{
		delete pKeyValues;
		return;
	}

	CPointOrient* pOrient = static_cast<CPointOrient*>(pEntityInstance);

	// Configure PointOrient properties
	CPointOrient_Helper::GetActiveAccessor(pOrient) = true;
	CPointOrient_Helper::GetGoalDirectionAccessor(pOrient) = PointOrientGoalDirectionType_t::eEyesForward;

	// Queue the entity for spawning using entity manager
	pEntitySystem->QueueSpawnEntity(pEntityInstance->m_pEntity, pKeyValues);
	pEntitySystem->ExecuteQueuedCreation();
	
	// Store the PointOrient handle
	SetPointOrient(pCSPlayerPawn, pOrient);

	// Get provider for proper API calls
	extern MenuSystem_Plugin *g_pMenuPlugin;
	auto &aBaseEntity = g_pMenuPlugin->GetGameDataStorage().GetBaseEntity();
	auto &aBasePlayerPawn = g_pMenuPlugin->GetGameDataStorage().GetBasePlayerPawn();

	// Position the PointOrient at player's eye position
	Vector vecEyePosition = aBasePlayerPawn.GetEyePosition(pCSPlayerPawn);
	aBaseEntity.Teleport(pOrient, vecEyePosition);

	// Set parent and target to the player using proper variant_t
	variant_t activatorVariant;
	CEntityHandle playerHandle;
	playerHandle.Set(pCSPlayerPawn);
	activatorVariant = playerHandle;
	
	aBaseEntity.AcceptInput(pOrient, "SetParent", pCSPlayerPawn, pCSPlayerPawn, &activatorVariant, 0);
	aBaseEntity.AcceptInput(pOrient, "SetTarget", pCSPlayerPawn, pCSPlayerPawn, &activatorVariant, 0);
}
