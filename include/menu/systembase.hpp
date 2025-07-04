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

#	include <tier0/platform.h>
#	include <tier1/utlmap.h>
#	include <tier1/utlsymbollarge.h>
#	include <tier1/smartptr.h>

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
		  : CLogger(GetName(), NULL, 0, LV_DEFAULT, MENU_SYSTEMBASE_LOGGINING_COLOR),
		    m_mapCallbacks(DefLessFunc(const CUtlSymbolLarge))
		{
		}

		using Index_t = uint16;
		using OnCallback_t = std::function<bool (const CUtlSymbolLarge &, Args...)>;
		using SmartCallback_t = CSmartPtr<OnCallback_t, CNullRefCountAccessor>;

	public: // ISystemBase
		virtual const char *GetName() { return "Menu - Base System"; }
		virtual const char *GetHandlerLowercaseName() { return ""; }

	public:
		bool IsValidHandler(Index_t idx) { return m_mapCallbacks.InvalidIndex() != idx; }
		Index_t AddHandler(const char *pszName, SmartCallback_t &&fnCallback) { return m_mapCallbacks.Insert(GetSymbol(pszName), Move(fnCallback)); }
		bool RemoveHandler(Index_t idx) { return m_mapCallbacks.RemoveAt(idx); }
		void RemoveAllHandlers() { m_mapCallbacks.Purge(); }

	public:
		Index_t FindHandler(const char *pszName)
		{
			CUtlSymbolLarge sName = FindSymbol(pszName);

			if(!sName.IsValid())
			{
				return m_mapCallbacks.InvalidIndex();
			}

			return m_mapCallbacks.Find(sName);
		}

		bool Call(Index_t iHandler, const char *pszName, Args... args)
		{
			Assert(IsValidHandler(iHandler));

			const SmartCallback_t &it = m_mapCallbacks[iHandler];

			return (*it)(pszName, args...);
		}

	protected:
		virtual bool Handle(const char *pszName, Args... args)
		{
			Index_t iHandler = FindHandler(pszName);

			if(!IsValidHandler(iHandler))
			{
				return false;
			}

			if(CLogger::IsChannelEnabled(LS_DETAILED))
			{
				CLogger::DetailedFormat("Handling \"%s\" %s...\n", pszName, GetHandlerLowercaseName());
			}

			return Call(iHandler, pszName, args...);
		}

	protected:
		CUtlSymbolLarge GetSymbol(const char *pszText) { return m_aSymbolTable.AddString(pszText); }
		CUtlSymbolLarge FindSymbol(const char *pszText) const { return m_aSymbolTable.Find(pszText); }

	private:
		CUtlSymbolTableLarge_CI m_aSymbolTable;

	protected:
		CUtlMap<CUtlSymbolLarge, SmartCallback_t, Index_t> m_mapCallbacks;
	}; // Menu::CSystemBase<>
}; // Menu

#endif // _INCLUDE_METAMOD_SOURCE_MENU_SYSTEMBASE_HPP_
