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

#ifndef _INCLUDE_METAMOD_SOURCE_MENUALLOCATOR_HPP_
#	define _INCLUDE_METAMOD_SOURCE_MENUALLOCATOR_HPP_

#	pragma once

#	include <menu.hpp>

#	include <basetypes.h>
#	include <const.h>
#	include <tier0/rawallocator.h>
#	include <tier0/memblockallocator.h>
#	include <tier1/utlrbtree.h>
#	include <tier1/utlvector.h>

#	define MENU_MEMBLOCK_RESERVED_FREE_BIT (1 << 30)
#	define MENU_MEMBLOCK_RESERVED_INSTANCE_THISOFFSET_MASK (~MENU_MEMBLOCK_RESERVED_FREE_BIT)

class IMenuProfile;

template<uintp PAGE_SIZE = sizeof(CMenu)>
class CMenuAllocator
{
public:
	using CInstance_t = CMenu;
	using Interface_t = IMenu;

	CMenuAllocator(int nGrowSize = 0, int nInitSize = 16, RawAllocatorType_t eAllocatorType = RawAllocator_Standard)
	 :  m_vecMemBlocks(nInitSize, nGrowSize),
	    m_MemBlockAllocator((nInitSize > 0) ? ABSOLUTE_PLAYER_LIMIT : 0, PAGE_SIZE, eAllocatorType)
	{
	}

public:
	struct MemBlock_t
	{
		int m_nReserved;
		MemBlockHandle_t m_Handle;

		MemBlock_t(const MemBlockHandle_t &hBlock)
		 :  m_nReserved(0), 
		    m_Handle(hBlock)
		{
		}

		bool IsFree() const
		{
			return !!(m_nReserved & MENU_MEMBLOCK_RESERVED_FREE_BIT);
		}

		int GetInstanceThisOffset() const
		{
			return m_nReserved & MENU_MEMBLOCK_RESERVED_INSTANCE_THISOFFSET_MASK;
		}

		MemBlockHandle_t GetHandle() const
		{
			return m_Handle;
		}

		bool operator==(const MemBlockHandle_t &hRight) const
		{
			return m_Handle == hRight;
		}

		bool operator!=(const MemBlockHandle_t &hRight) const
		{
			return m_Handle != hRight;
		}

		void MarkFree()
		{
			m_nReserved |= MENU_MEMBLOCK_RESERVED_FREE_BIT;
		}

		void SetThisOffset(CInstance_t *pInstance)
		{
			m_nReserved = reinterpret_cast<uintp>(static_cast<Interface_t *>(pInstance)) - reinterpret_cast<uintp>(pInstance) & MENU_MEMBLOCK_RESERVED_INSTANCE_THISOFFSET_MASK;
		}
	};

	template<class T>
	inline T *GetByHandle(MemBlockHandle_t hMemBlock, int nThisOffset = 0)
	{
		return reinterpret_cast<T *>(reinterpret_cast<uintp>(m_MemBlockAllocator.GetBlock(hMemBlock)) + nThisOffset);
	}

	inline auto *GetInstanceByMemBlock(const MemBlock_t *pMemBlock)
	{
		return GetByHandle<CInstance_t>(pMemBlock->GetHandle(), pMemBlock->GetInstanceThisOffset());
	}

	MemBlock_t *FindLastFreeMemBlock()
	{
		MemBlockHandle_t hBlock = MEMBLOCKHANDLE_INVALID;

		FOR_EACH_VEC_BACK(m_vecMemBlocks, i)
		{
			auto &aElm = m_vecMemBlocks[i];

			if(aElm.IsFree())
			{
				return &aElm;
			}
		}

		return nullptr;
	}

	MemBlock_t *FindMemBlock(Interface_t *pMenu)
	{
		FOR_EACH_VEC_BACK(m_vecMemBlocks, i)
		{
			auto &aElm = m_vecMemBlocks[i];

			if((reinterpret_cast<uintp>(pMenu) + aElm.GetInstanceThisOffset()) == reinterpret_cast<uintp>(m_MemBlockAllocator.GetBlock(aElm.GetHandle())))
			{
				return &aElm;
			}
		}

		return nullptr;
	}

public:
	CInstance_t *CreateInstance(const CMenu::CPointWorldText_Helper *pCtorSchemaHelper, const CMenu::CGameData_BaseEntity *pCtorGameData, IMenuProfile *pCtorProfile, IMenuHandler *pCtorHandler = nullptr, CMenuData_t::ControlItems_t *pCtorControls = nullptr)
	{
		MemBlock_t *pMemBlock = FindLastFreeMemBlock();

		MemBlockHandle_t hMemBlock {};

		if(pMemBlock)
		{
			hMemBlock = pMemBlock->GetHandle();
		}
		else
		{
			hMemBlock = m_MemBlockAllocator.Alloc(sizeof(CInstance_t));
			pMemBlock = &m_vecMemBlocks.Element(m_vecMemBlocks.AddToTail(MemBlock_t(hMemBlock)));
		}

		auto *pResult = GetByHandle<CInstance_t>(hMemBlock);

		pMemBlock->SetThisOffset(pResult);

		return pResult ? Construct(pResult, pCtorSchemaHelper, pCtorGameData, pCtorProfile, pCtorHandler, pCtorControls) : nullptr;
	}

	CInstance_t *FindAndUpperCast(Interface_t *pMenu)
	{
		MemBlock_t *pFoundBlock = FindMemBlock(pMenu);

		if(pFoundBlock)
		{
			return GetInstanceByMemBlock(pFoundBlock);
		}

		return nullptr;
	}

	void ReleaseByMemBlock(MemBlock_t *pMemBlock)
	{
		auto *pInstance = GetInstanceByMemBlock(pMemBlock);

		Destruct(pInstance);
		pMemBlock->MarkFree();
	}

	bool ReleaseByInterface(Interface_t *pMenu)
	{
		MemBlock_t *pFoundBlock = FindMemBlock(pMenu);

		if(pFoundBlock)
		{
			ReleaseByMemBlock(pFoundBlock);

			return true;
		}

		return false;
	}

	void RemoveAll()
	{
		m_vecMemBlocks.RemoveAll();
		m_MemBlockAllocator.RemoveAll();
	}

	void Purge()
	{
		m_vecMemBlocks.Purge();
		m_MemBlockAllocator.Purge();
	}

	void PurgeAndDeleteElements()
	{
		for(const auto &aMemBlock : m_vecMemBlocks)
		{
			if(aMemBlock.IsFree())
			{
				continue;
			}

			Destruct(GetInstanceByMemBlock(&aMemBlock));
		}

		m_vecMemBlocks.Purge();
		m_MemBlockAllocator.Purge();
	}

private:
	using MemBlocksVec_t = CUtlVector_RawAllocator<MemBlock_t>;

	MemBlocksVec_t m_vecMemBlocks;
	CUtlMemoryBlockAllocator<CInstance_t> m_MemBlockAllocator;
};

#endif // _INCLUDE_METAMOD_SOURCE_MENUALLOCATOR_HPP_
