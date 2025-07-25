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


#ifndef _INCLUDE_METAMOD_SOURCE_IMENU_ISAMPLE_HPP_
#	define _INCLUDE_METAMOD_SOURCE_IMENU_ISAMPLE_HPP_

#	pragma once

#	include <playerslot.h>
#	include <igamesystem.h>

#	include <tier1/utlvector.h>
#	include <tier1/utlstringmap.h>

class CGameEntitySystem;
class CBaseGameSystemFactory;
class CGameSystemEventDispatcher;
class CServerSideClient;
class IGameEventManager2;
struct AddedGameSystem_t;

/**
 * @brief A Sample-legacy interface.
**/
class ISample
{
public:
	/**
	 * @brief Gets a game entity system.
	 * 
	 * @return              A double pointer to a game entity system.
	 */
	virtual CGameEntitySystem **GetGameEntitySystemPointer() const = 0;

	/**
	 * @brief Gets a first game system.
	 * 
	 * @return              A double pointer to a first game system.
	 */
	virtual CBaseGameSystemFactory **GetFirstGameSystemPointer() const = 0;

	/**
	 * @brief Gets game system factories.
	 * 
	 * @return              A pointer to a string map of game system factories.
	 */
	virtual CUtlStringMap<IGameSystem::FactoryInfo_t> *GetGameSystemFactoriesPointer() const = 0;

	/**
	 * @brief Gets game systems.
	 * 
	 * @return              A pointer to a list of game systems.
	 */
	virtual CUtlVector<AddedGameSystem_t> *GetGameSystemsPointer() const = 0;

	/**
	 * @brief Gets a game system event dispatcher.
	 * 
	 * @return              A double pointer to a game system event dispatcher.
	 */
	virtual CGameSystemEventDispatcher **GetGameSystemEventDispatcherPointer() const = 0;

	/**
	 * @brief Gets a out of game system event dispatcher.
	 * 
	 * @return              A pointer to an out of game system event dispatcher.
	 */
	virtual CGameSystemEventDispatcher *GetOutOfGameEventDispatcher() const = 0;

	/**
	 * @brief Gets a game event manager.
	 * 
	 * @return              A double pointer to a game event manager.
	 */
	virtual IGameEventManager2 **GetGameEventManagerPointer() const = 0;

public: // Language ones.
	/**
	 * @brief A player data language interface.
	**/
	class ILanguage
	{
	public:
		/**
		 * @brief Gets a name of a language.
		 * 
		 * @return              Returns a language name.
		 */
		virtual const char *GetName() const = 0;

		/**
		 * @brief Gets a country code of a language.
		 * 
		 * @return              Returns a country code of a language.
		 */
		virtual const char *GetCountryCode() const = 0;
	}; // ISample::ILanguage

public: // Player ones.
	class IPlayerLanguageListener
	{
	public:
		/**
		 * @brief Calls then a player language are changed.
		 * 
		 * @param aSlot         A player slot (index).
		 * @param pLanguage     A language pointer, who changed.
		 */
		virtual void OnPlayerLanguageChanged(CPlayerSlot aSlot, const ILanguage *pLanguage) = 0;
	}; // ISample::IPlayerLanguageListener

	/**
	 * @brief A player language interface.
	**/
	class IPlayerLanguageCallbacks
	{
	public:
		/**
		 * @brief Add a language listener.
		 * 
		 * @param pListener     A listener, who will be called when language has received.
		 * 
		 * @return              Returns true if this has listenen.
		 */
		virtual bool AddLanguageListener(IPlayerLanguageListener *pListener) = 0;

		/**
		 * @brief Removes a language listener.
		 * 
		 * @param fnCallback    A listener to remove.
		 * 
		 * @return              Returns "true" if this has removed, 
		 *                      otherwise "false" if not exists.
		 */
		virtual bool RemoveLanguageListener(IPlayerLanguageListener *pListener) = 0;
	}; // ISample::IPlayerLanguageCallbacks

	/**
	 * @brief A player language interface.
	**/
	class IPlayerLanguage : public IPlayerLanguageCallbacks
	{
	public:
		/**
		 * @brief Gets a language.
		 * 
		 * @return              Returns a language, 
		 *                      otherwise "nullptr" that not been received.
		 */
		virtual const ILanguage *GetLanguage() const = 0;

		/**
		 * @brief Sets a language to player.
		 * 
		 * @param pData         A language to set.
		 */
		virtual void SetLanguage(const ILanguage *pData) = 0;
	}; // ISample::IPlayerLanguage

	/**
	 * @brief A player base interface.
	**/
	class IPlayerBase : public IPlayerLanguage
	{
	public:
		/**
		 * @brief Gets a connection status of a player.
		 * 
		 * @return              A vector of menu entities.
		 */
		virtual bool IsConnected() const = 0;

		/**
		 * @brief Gets a server side client of the player.
		 * 
		 * @return              A server server side client pointer.
		 */
		virtual CServerSideClient *GetServerSideClient() = 0;
	}; // ISample::IPlayerBase

	/**
	 * @brief Gets a server language.
	 * 
	 * @return              Returns a server language.
	 */
	virtual const ILanguage *GetServerLanguage() const = 0;

	/**
	 * @brief Gets a language by a name.
	 * 
	 * @param psz           A case insensitive language name.
	 * 
	 * @return              Returns a found language, otherwise
	 *                      "nullptr".
	 */
	virtual const ILanguage *GetLanguageByName(const char *psz) const = 0;

	/**
	 * @brief Gets a player base data.
	 * 
	 * @param aSlot         A player slot.
	 * 
	 * @return              Returns a player data.
	 */
	virtual IPlayerBase *GetPlayerBase(const CPlayerSlot &aSlot) = 0;
}; // ISample

#endif // _INCLUDE_METAMOD_SOURCE_IMENU_ISAMPLE_HPP_
