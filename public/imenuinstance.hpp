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


#ifndef _INCLUDE_METAMOD_SOURCE_IMENUINSTANCE_HPP_
#	define _INCLUDE_METAMOD_SOURCE_IMENUINSTANCE_HPP_

#	pragma once

#	include <bitvec.h>
#	include <const.h>
#	include <playerslot.h>
#	include <tier1/utlvector.h>

#	define MENU_MAX_TEXT_LENGTH 512             //< The maximum length of a menu text.
#	define MENU_DEFAULT_ITEMS_COUNT_PER_PAGE 10 //< The default count of menu items per page.

/**
 * @file imenuinstance.hpp
 * @brief Defines the IMenuInstance interface for internal menu management.
 */

class CEntityInstance; // See <entity2/entityinstance.h> of Source SDK.
class IMenuHandler; // See "imenuhandler.hpp".
class IMenuProfile; // See "imenuprofile.hpp".

/**
 * @brief Enumeration representing the menu entity index.
 */
enum MenuEntity_t : uint8
{
	MENU_ENTITY_BACKGROUND_INDEX = 0,   ///< The background layer index.
	MENU_ENTITY_INACTIVE_INDEX = 1,     ///< The inactive layer index.
	MENU_ENTITY_ACTIVE_INDEX = 2,       ///< The active layer index.
	MENU_ENTITY_DISABLED_ACTIVE_INDEX = 3, ///< The disabled active layer index.

	MENU_MAX_ENTITIES,                  ///< Count of menu entities.
};

/**
 * @brief A Menu Instance interface.
 */
class IMenuInstance
{
public:
	/**
	 * @brief Virtual destructor to destroy the menu instance. 
	 *        Used internally, call `ReleaseMenu` instead.
	 */
	virtual ~IMenuInstance() = default;

	/**
	 * @brief Gets a profile of the instance with the menu customization.
	 *
	 * @return Returns a pointer to the IMenuProfile instance.
	 */
	virtual const IMenuProfile *GetProfile() const = 0;

	/**
	 * @brief Applies a profile with the menu customization.
	 *
	 * @param aSlot         The player slot who applies a profile to prereder.
	 *                      `INVALID_PLAYER_SLOT` if the server.
	 * @param pProfile      A pointer to new profile.
	 * 
	 * @return              Returns `true` if the profile was successfully applied;
	 *                      `false` if the profile could not be applied due to an error or invalid input.
	 */
	virtual bool ApplyProfile(CPlayerSlot aSlot, IMenuProfile *pProfile) = 0;

	/**
	 * @brief Gets the menu handler.
	 *
	 * @return Returns a pointer to the handler.
	 */
	virtual IMenuHandler *GetHandler() const = 0;

	/**
	 * @brief Gets the active entities associated with instance.
	 *
	 * @return Returns a reference to a vector of pointers to CEntityInstance objects.
	 */
	virtual const CUtlVector<CEntityInstance *> &GetActiveEntities() const = 0;

	/**
	 * @brief Gets the recipients of players.
	 *
	 * @return Returns a reference to players representing the recipients.
	 */
	virtual const CPlayerBitVec &GetRecipients() const = 0;

	/**
	 * @brief Emits the menu instance for processing.
	 * 
	 * @param vecEntites    List of emit entities.
	 */
	virtual void Emit(const CUtlVector<CEntityInstance *> &vecEntites) = 0;
}; // IMenuInstance

#endif // _INCLUDE_METAMOD_SOURCE_IMENUINSTANCE_HPP_
