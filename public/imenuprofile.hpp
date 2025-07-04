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

#ifndef _INCLUDE_METAMOD_SOURCE_IMENUPROFILE_HPP_
#define _INCLUDE_METAMOD_SOURCE_IMENUPROFILE_HPP_

#pragma once

#include "imenuprofile.hpp"

#include <color.h>
#include <tier0/utlstring.h>
#include <tier1/utlvector.h>

class CKeyValues3Context;
class CEntityKeyValues;

/**
 * @file imenuprofile.hpp
 * @brief Defines the IMenuProfile interface for menu customization ones.
 */

#define MENU_MAX_FONT_NAME_LENGTH 64
#define MENU_MAX_FONT_BACKGROUND_MATERIAL_NAME_LENGTH 64

/**
 * @brief A Menu Profile interface.
**/
class IMenuProfile
{
public: // Definitions.
	using ItemKeyBind_t = CUtlString;
	using Items_t = CUtlVector<ItemKeyBind_t>;

	struct MatrixOffset_t
	{
		vec_t m_flForward = 0.f;
		vec_t m_flLeft = 0.f;
		vec_t m_flRight = 0.f;
		vec_t m_flUp = 0.f;
	};

public: // Abstract methods.
	/**
	 * @brief Gets the display name of the profile.
	 * 
	 * @return Returns the display name.
	 */
	virtual const CUtlString &GetDisplayName() const = 0;

	/**
	 * @brief Gets the description of the profile.
	 * 
	 * @return Returns the description.
	 */
	virtual const CUtlString &GetDescription() const = 0;

	/**
	 * @brief Gets the items of the profile.
	 * 
	 * @return Returns a pointer to items.
	 */
	virtual const Items_t *GetItems() const = 0;

	/**
	 * @brief Gets the client ConVar name for item verification.
	 * 
	 * @return Returns the client ConVar name.
	 */
	virtual const CUtlString &GetItemsVerificationClientConVarName() const = 0;

	/**
	 * @brief Gets the matrix offset.
	 * 
	 * @return Returns a pointer to matrix offset.
	 */
	virtual const MatrixOffset_t *GetMatrixOffset() const = 0;

	/**
	 * @brief Gets the matrix offset of a previos instance.
	 * 
	 * @return Returns a pointer to matrix offset.
	 */
	virtual const MatrixOffset_t *GetPreviosMatrixOffset() const = 0;

	/**
	 * @brief Gets the background color.
	 * 
	 * @return Returns a pointer to background color.
	 */
	virtual const Color *GetBackgroundColor() const = 0;

	/**
	 * @brief Gets the inactive color.
	 * 
	 * @return Returns a pointer to inactive color.
	 */
	virtual const Color *GetInactiveColor() const = 0;

	/**
	 * @brief Gets the active color.
	 * 
	 * @return Returns a pointer to active color.
	 */
	virtual const Color *GetActiveColor() const = 0;

	/**
	 * @brief Gets the disabled active color.
	 *
	 * @return Returns a pointer to disabled active color.
	 */
	virtual const Color *GetDisabledActiveColor() const = 0;

	/**
	 * @brief Gets the background away units.
	 * 
	 * @return Returns a background away units value.
	 */
	virtual float GetBackgroundAwayUnits() const = 0;

	/**
	 * @brief Gets a list of resources that need to be precached.
	 * 
	 * @return Returns a list of resources.
	 */
	virtual CUtlVector<const char *> GetResources() const = 0;

	/**
	 * @brief Allocates and retrieves entity key values data.
	 * 
	 * @param pAllocator            Optional allocator for entity key values data.
	 * @param bIncludeBackground    Whether to include background-related data.
	 * 
	 * @return                      Returns a pointer to allocated entity key values data. 
	 *                              Must be deleted!
	 */
	virtual CEntityKeyValues *GetAllocactedEntityKeyValues(CKeyValues3Context *pAllocator = nullptr, bool bIncludeBackground = true) const = 0;
}; // IMenuProfile

#endif // _INCLUDE_METAMOD_SOURCE_IMENUPROFILE_HPP_
