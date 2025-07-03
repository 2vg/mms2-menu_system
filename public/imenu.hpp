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


#ifndef _INCLUDE_METAMOD_SOURCE_IMENU_HPP_
#	define _INCLUDE_METAMOD_SOURCE_IMENU_HPP_

#	pragma once

#	include "imenuinstance.hpp"
#	include <basetypes.h>
#	include <playerslot.h>
#	if __has_include(<tier0/utlstring.h>)
#		include <tier0/utlstring.h>
#	else // Bcompatibility with HL2SDK
#		include <tier1/utlstring.h>
#	endif // __has_include(<tier0/utlstring.h>)
#	include <tier1/utlvector.h>

/**
 * @file imenu.hpp
 * @brief Defines an IMenu interface for managing in-game menus.
 */

#	define MENU_INVLID_INDEX static_cast<IMenu::Index_t>(-1)            ///< Invalid menu instance index.
#	define MENU_FIRST_ITEM_INDEX static_cast<IMenu::ItemPosition_t>(0)  ///< The menu first item index.
#	define MENU_NO_PAGINATION 0     ///< FUTURE: Menu should not be paginated (10 items max of "default" profile).

/**
 * @brief A Menu interface.
**/
class IMenu : public IMenuInstance
{
public: // The definitions.
	using Index_t = int;
	using ItemPosition_t = int;
	using ItemPositionOnPage_t = int8;
	using Pagination_t = uint8;

	/**
	 * @brief Enumeration representing the style of a menu item.
	 */
	enum ItemStyleFlags_t : uint8
	{
		MENU_ITEM_ACTIVE =      (1 << 0),       ///< Item is drawn selectable.
		MENU_ITEM_HASNUMBER =   (1 << 1),       ///< Item has number.
		MENU_ITEM_CONTROL =     (1 << 2),       ///< Item is control text (back/next/exit).
		MENU_ITEM_DISABLED =    (1 << 3),       ///< Item is disabled (not selectable, shown with disabled color).

		MENU_ITEM_DEFAULT =     (MENU_ITEM_ACTIVE | MENU_ITEM_HASNUMBER),   ///< Item should be drawn normally.
		MENU_ITEM_FULL =        (MENU_ITEM_DEFAULT | MENU_ITEM_CONTROL),    ///< Control item should be drawn normally
		MENU_ITEM_DISABLED_TEXT = (MENU_ITEM_HASNUMBER | MENU_ITEM_DISABLED), ///< Disabled item with number but not selectable.
	};

	/**
	 * @brief Enumeration representing the available menu control flags.
	 */
	enum ItemControlFlags_t : uint8
	{
		MENU_ITEM_CONTROL_FLAG_PANEL = 0,           ///< Just a panel with no control buttons.
		MENU_ITEM_CONTROL_FLAG_BACK = (1 << 0),     ///< Back button flag.
		MENU_ITEM_CONTROL_FLAG_NEXT = (1 << 1),     ///< Next button flag.
		MENU_ITEM_CONTROL_FLAG_EXIT = (1 << 2),     ///< Exit button flag.

		MENU_ITEM_CONTROL_DEFAULT_FLAGS = (MENU_ITEM_CONTROL_FLAG_BACK | MENU_ITEM_CONTROL_FLAG_NEXT | MENU_ITEM_CONTROL_FLAG_EXIT),    //< Default controls.
	};

	/**
	 * @brief Enumeration representing the available menu control indexes.
	 */
	enum ItemControls_t : ItemPosition_t
	{
		MENU_ITEM_CONTROL_BACK_INDEX = -1,
		MENU_ITEM_CONTROL_NEXT_INDEX = -2,
		MENU_ITEM_CONTROL_EXIT_INDEX = -3,
	};

	/**
	 * @brief Enumeration representing the display style flags.
	 */
	enum DisplayFlags_t : uint8
	{
		MENU_DISPLAY_UPDATE_TEXT_NOW = (1 << 0),
		MENU_DISPLAY_RENDER_BASE = (1 << 1),
		MENU_DISPLAY_RERENDER = (1 << 2),

		MENU_DISPLAY_DEFAULT = MENU_DISPLAY_UPDATE_TEXT_NOW,
		MENU_DISPLAY_RENDER_BASE_UPDATE = MENU_DISPLAY_UPDATE_TEXT_NOW | MENU_DISPLAY_RENDER_BASE,
		MENU_DISPLAY_RERENDER_UPDATE = MENU_DISPLAY_UPDATE_TEXT_NOW | MENU_DISPLAY_RERENDER,

		MENU_DISPLAY_BASE_FULL = MENU_DISPLAY_UPDATE_TEXT_NOW | MENU_DISPLAY_RENDER_BASE | MENU_DISPLAY_RERENDER
	};

	/**
	 * @brief A Item Handler for handling menu item actions.
	 */
	class IItemHandler
	{
	public:
		/**
		 * @brief Called when a menu item is selected.
		 *
		 * @param pMenu         Pointer to the menu instance.
		 * @param aSlot         The player slot interacting with the menu.
		 * @param iItem         The selected item position.
		 * @param iItemOnPage   The position of the item on the current page.
		 * @param pData         Additional data associated with the menu item.
		 */
		virtual void OnMenuSelectItem(IMenu *pMenu, CPlayerSlot aSlot, ItemPosition_t iItem, ItemPositionOnPage_t iItemOnPage, void *pData) {}
	};

	struct Title_t
	{
		CUtlString m_sText;                             ///< A title text.

		bool IsEmpty() const
		{
			return m_sText.IsEmpty();
		}

		const char *Get() const
		{
			return m_sText.Get();
		}

		void Set(const char *pszNewValue)
		{
			m_sText.Set(pszNewValue);
		}
	};

	struct Item_t
	{
		ItemStyleFlags_t m_eStyle = MENU_ITEM_DEFAULT;  ///< A style flags of the item.
		CUtlString m_sContent;                          ///< A content of the item.

		IItemHandler *m_pHandler = nullptr;             ///< The handler for item actions.
		void *m_pData = nullptr;                        ///< Additional data passed to item actions.

		Item_t(ItemStyleFlags_t eStyle, const char *pszContent, IItemHandler *pHandler, void *pData)
		 :  m_eStyle(eStyle), 
		    m_sContent(pszContent), 
		    m_pHandler(pHandler), 
		    m_pData(pData)
		{
		}

		Item_t(ItemStyleFlags_t eStyle, const char *pszContent)
		 :  Item_t(eStyle, pszContent, nullptr, nullptr)
		{
		}

		Item_t(const char *pszContent, IItemHandler *pHandler, void *pData = nullptr)
		 :  Item_t(MENU_ITEM_DEFAULT, pszContent, pHandler, pData)
		{
		}

		Item_t(const char *pszContent)
		 :  Item_t(pszContent, nullptr, nullptr)
		{
		}

		bool IsEmpty() const
		{
			return m_sContent.IsEmpty();
		}

		ItemStyleFlags_t GetStyle() const
		{
			return m_eStyle;
		}

		const char *Get() const
		{
			return m_sContent.Get();
		}

		void Set(const char *pszNewValue)
		{
			m_sContent.Set(pszNewValue);
		}
	};
	using Items_t = CUtlVector<Item_t>;

public: // Public methods.
	/**
	 * @brief Gets a reference to the menu title.
	 * 
	 * @return Reference to the menu title.
	 */
	virtual Title_t &GetTitleRef() = 0;

	/**
	 * @brief Gets a reference to the collection of menu items.
	 * 
	 * @return Reference to the collection of menu items.
	 */
	virtual Items_t &GetItemsRef() = 0;

	/**
	 * @brief Gets a reference to the menu controls.
	 * 
	 * @return Reference to the menu controls.
	 */
	virtual ItemControlFlags_t &GetItemControlsRef() = 0;

	/**
	 * @brief Gets the current position of the menu cursor for a specific player.
	 *
	 * @param aSlot         The player slot.
	 * 
	 * @return              The current item position in the menu.
	 */
	virtual ItemPosition_t GetCurrentPosition(CPlayerSlot aSlot) const = 0;

	/**
	 * @brief Displays the menu to a specific player.
	 * NOTE: This method is internal, use IMenuSystem to display.
	 * 
	 * @param aSlot         The player slot.
	 * 
	 * @return              `true` if the menu was displayed successfully,
	 *                      `false` otherwise.
	 */
	inline bool InternalDisplay(CPlayerSlot aSlot)
	{
		return InternalDisplayAt(aSlot, MENU_FIRST_ITEM_INDEX);
	}

	/**
	 * @brief Displays the menu starting from a specific item.
	 * NOTE: This method is internal, use IMenuSystem to display.
	 * 
	 * @param aSlot         The player slot.
	 * @param iStartItem    The starting item position (default: MENU_FIRST_ITEM_INDEX).
	 * @param eFlags        Flags to display.
	 * 
	 * @return              `true` if the menu was displayed successfully, 
	 *                      `false` otherwise.
	 */
	virtual bool InternalDisplayAt(CPlayerSlot aSlot, ItemPosition_t iStartItem = MENU_FIRST_ITEM_INDEX, DisplayFlags_t eFlags = MENU_DISPLAY_DEFAULT) = 0;
}; // IMenuInstance

#endif // _INCLUDE_METAMOD_SOURCE_IMENU_HPP_
