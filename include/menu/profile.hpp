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

#ifndef _INCLUDE_METAMOD_SOURCE_MENU_PROFILE_HPP_
#	define _INCLUDE_METAMOD_SOURCE_MENU_PROFILE_HPP_

#	pragma once

#	include <imenuprofile.hpp>
#	include <menu/system.hpp>

#	include <tier0/platform.h>

#	include <entity2/entitykeyvalues.h>
#	include <playerslot.h>
#	include <tier0/bufferstring.h>
#	include <tier0/utlstring.h>
#	include <tier1/utlmap.h>
#	include <tier1/utlsymbollarge.h>
#	include <tier1/utlvector.h>
#	include <tier1/keyvalues3.h>

#	include <logger.hpp>

namespace Menu
{
	class CProfileSystem; // See "profilesystem.hpp".
	class CProfile;

	struct MenuProfile_t
	{
		template<class T>
		struct MetadataBase_t
		{
			bool m_bHidden = false; // "hidden"

			// "inherits"
			using Bases_t = CUtlVector<const T *>;
			Bases_t m_vecBases;

			bool IsHidden() const
			{
				return m_bHidden;
			}

			const Bases_t GetBases() const
			{
				return m_vecBases;
			}

			Bases_t GetBaseline() const
			{
				Bases_t vecResult;

				for(const auto &pBase : m_vecBases)
				{
					Assert(pBase);
					vecResult.AddToTail(pBase);

					Bases_t vecSubresult = pBase->m_aMetadata.GetBaseline();

					vecResult.AddMultipleToTail(vecSubresult.Count(), vecSubresult.Base());
				}

				return vecResult;
			}
		};

		using Metadata_t = MetadataBase_t<CProfile>;
		Metadata_t m_aMetadata;

		CUtlString m_sDisplayName; // "display_name"
		CUtlString m_sDescription; // "description"

		IMenuProfile::Items_t *m_pItems = nullptr; // "items"
		CUtlString m_sItemsVerificationClientConVarName; // "items_verification_client_convar_name"

		IMenuProfile::MatrixOffset_t *m_pMatrixOffset = nullptr; // "matrix_offset"
		IMenuProfile::MatrixOffset_t *m_pMatrixOffset_Previos = nullptr; // "matrix_offset-previos"

		Color *m_pBackgroundColor = nullptr; // "background_color"
		Color *m_pInactiveColor = nullptr; // "inactive_color"
		Color *m_pActiveColor = nullptr; // "active_color"
		Color *m_pTitleColor = nullptr; // "title_color"
		Color *m_pDisabledColor = nullptr; // "disabled_color"

		float m_flBackgroundAwayUnits = 0.f; // "background_away_units"

		using ResourcesBase_t = CUtlVector<CUtlString>;

		struct Resources_t : ResourcesBase_t
		{
			using Base_t = ResourcesBase_t;

			CUtlVector<const char *> GetExports() const
			{
				CUtlVector<const char *> vecResult(Base_t::Count());

				for(const auto &sResource : *this)
				{
					vecResult.AddToTail(sResource.Get());
				}

				return vecResult;
			}
		};

		Resources_t m_vecResources;
		CEntityKeyValues *m_pData = nullptr; // Other elements.
	};

	enum ProfileLoadFlags_t : uint8
	{
		PROFILE_LOAD_NONE_FLAGS = 0,
		PROFILE_LOAD_FLAG_METADATA = (1 << 0),
		PROFILE_LOAD_FLAG_DONT_REMOVE_STATIC_MEMBERS = (1 << 1),
		PROFILE_LOAD_ENTITYKEYVALUES = (1 << 2),

		PROFILE_LOAD_STEP1 = PROFILE_LOAD_FLAG_DONT_REMOVE_STATIC_MEMBERS,
		PROFILE_LOAD_STEP2 = (PROFILE_LOAD_FLAG_METADATA | PROFILE_LOAD_ENTITYKEYVALUES),
	};

	class CProfile : public IMenuProfile, public MenuProfile_t
	{
	public:
		~CProfile();

		using Base_t = MenuProfile_t;

	public:
		bool Load(CProfileSystem *pSystem, KeyValues3 *pData, ProfileLoadFlags_t eFlags, CUtlVector<CUtlString> &vecMessages);

	protected:
		void LoadMetadataBase(CProfileSystem *pSystem, KeyValues3 *pData, CUtlVector<CUtlString> &vecMessages);
		bool LoadMetadataBases(CProfileSystem *pSystem, const char *pszName, KeyValues3 *pData, CUtlVector<CUtlString> &vecMessages);
		bool LoadMetadataBaseString(CProfileSystem *pSystem, const char *pszName, KeyValues3 *pData, CBufferString &sMessage);

		static Items_t *LoadAllocatedItems(KeyValues3 *pData, CUtlVector<CUtlString> &vecMessages);
		static IMenuProfile::MatrixOffset_t *LoadAllocatedMatrixOffset(KeyValues3 *pData, CUtlVector<CUtlString> &vecMessages);
		static CEntityKeyValues *LoadAllocatedEntityKeyValues(CProfileSystem *pSystem, KeyValues3 *pData, CUtlVector<CUtlString> &vecMessages);

	protected:
		static void RemoveStaticMembers(KeyValues3 *pData);
		static void RemoveStaticMetadataMembers(KeyValues3 *pData);

	public: // IMenuProfile
		const CUtlString &GetDisplayName() const override;
		const CUtlString &GetDescription() const override;
		const Items_t *GetItems() const override;
		const CUtlString &GetItemsVerificationClientConVarName() const override;
		const MatrixOffset_t *GetMatrixOffset() const override;
		const MatrixOffset_t *GetPreviosMatrixOffset() const override;
		const Color *GetBackgroundColor() const override;
		const Color *GetInactiveColor() const override;
		const Color *GetActiveColor() const override;
		const Color *GetTitleColor() const override;
		const Color *GetDisabledColor() const override;
		float GetBackgroundAwayUnits() const override;
		CUtlVector<const char *> GetResources() const override;
		CEntityKeyValues *GetAllocactedEntityKeyValues(CKeyValues3Context *pAllocator = nullptr, bool bIncludeBackground = true) const override;
	};
};

#endif // _INCLUDE_METAMOD_SOURCE_MENU_PROFILE_HPP_
