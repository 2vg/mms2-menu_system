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

#ifndef _INCLUDE_METAMOD_SOURCE_MENU_SYSTEMBASE_HPP_
#	define _INCLUDE_METAMOD_SOURCE_MENU_SYSTEMBASE_HPP_

#	pragma once

#	include <functional>
#	include <memory>

#	include <tier1/utlmap.h>
#	include <tier1/utlsymbollarge.h>

#	include <logger.hpp>

#	define MENU_SYSTEMBASE_LOGGINING_COLOR {255, 255, 255, 191}

namespace Menu
{
	class ISystemBase
	{
	public:
		virtual const char *GetName() = 0;
		virtual const char *GetHandlerLowercaseName() = 0;
	}; // Menu::ISystemBase

	template<typename... Args>
	class CSystemBase : public ISystemBase, virtual public CLogger
	{
	public:
		CSystemBase()
		  : CLogger(GetName(), NULL, 0, LV_OFF, MENU_SYSTEMBASE_LOGGINING_COLOR),
		    m_mapCallbacks(DefLessFunc(const CUtlSymbolLarge))
		{
		}

		using OnCallback_t = std::function<bool (const CUtlSymbolLarge &, Args...)>;
		using OnCallbackShared_t = std::shared_ptr<OnCallback_t>;

		class SharedCallback
		{
		public:
			SharedCallback()
			 :  m_pCallback(std::make_shared<OnCallback_t>(nullptr))
			{
			}

			SharedCallback(const OnCallbackShared_t &funcSharedCallback)
			 :  m_pCallback(funcSharedCallback)
			{
			}

			SharedCallback(const OnCallback_t &funcCallback)
			 :  m_pCallback(std::make_shared<OnCallback_t>(funcCallback))
			{
			}

			operator OnCallbackShared_t() const
			{
				return m_pCallback;
			}

			operator OnCallback_t() const
			{
				return *m_pCallback;
			}

		private:
			OnCallbackShared_t m_pCallback;
		}; // Menu::CSystemBase::SharedCallback

	public: // ISystemBase
		virtual const char *GetName()
		{
			return "Menu - Base System";
		}

		virtual const char *GetHandlerLowercaseName()
		{
			return "";
		}

	public:
		bool AddHandler(const char *pszName, const SharedCallback &fnCallback)
		{
			return m_mapCallbacks.Insert(GetSymbol(pszName), fnCallback) != m_mapCallbacks.InvalidIndex();
		}

		bool RemoveHandler(const char *pszName)
		{
			return m_mapCallbacks.Remove(FindSymbol(pszName));
		}

		void RemoveAllHandlers()
		{
			m_mapCallbacks.Purge();
		}

	protected:
		virtual bool Handle(const char *pszName, Args... args)
		{
			CUtlSymbolLarge sName = FindSymbol(pszName);

			if(!sName.IsValid())
			{
				return false;
			}

			auto iFound = m_mapCallbacks.Find(sName);

			if(iFound == m_mapCallbacks.InvalidIndex())
			{
				return false;
			}

			if(CLogger::IsChannelEnabled(LS_DETAILED))
			{
				CLogger::DetailedFormat("Handling \"%s\" %s...\n", pszName, GetHandlerLowercaseName());
			}

			OnCallback_t it = m_mapCallbacks[iFound];

			return it(pszName, args...);
		}

	protected:
		CUtlSymbolLarge GetSymbol(const char *pszText)
		{
			return m_aSymbolTable.AddString(pszText);
		}

		CUtlSymbolLarge FindSymbol(const char *pszText) const
		{
			return m_aSymbolTable.Find(pszText);
		}

	private:
		CUtlSymbolTableLarge_CI m_aSymbolTable;

	protected:
		CUtlMap<CUtlSymbolLarge, SharedCallback> m_mapCallbacks;
	}; // Menu::CSystemBase<>
}; // Menu

#endif // _INCLUDE_METAMOD_SOURCE_MENU_SYSTEMBASE_HPP_
