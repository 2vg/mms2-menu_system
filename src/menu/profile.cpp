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

#include <menu/profile.hpp>
#include <menu/profilesystem.hpp>

#include <imenuprofilesystem.hpp>

// Undefine Assert macro to avoid conflicts before including tier0 headers
#ifdef Assert
#undef Assert
#endif
#include <tier0/dbg.h>

Menu::CProfile::CProfile::~CProfile()
{
	if(m_pItems)
	{
		delete m_pItems;
	}

	if(m_pMatrixOffset)
	{
		delete m_pMatrixOffset;
	}

	if(m_pMatrixOffset_Previos)
	{
		delete m_pMatrixOffset_Previos;
	}

	if(m_pBackgroundColor)
	{
		delete m_pBackgroundColor;
	}

	if(m_pInactiveColor)
	{
		delete m_pInactiveColor;
	}

	if(m_pActiveColor)
	{
		delete m_pActiveColor;
	}

	if(m_pDisabledActiveColor)
	{
		delete m_pDisabledActiveColor;
	}

	if(m_pData)
	{
		delete m_pData;
	}
}

bool Menu::CProfile::Load(CProfileSystem *pSystem, KeyValues3 *pData, ProfileLoadFlags_t eFlags, CUtlVector<CUtlString> &vecMessages)
{
	// Metadata_t.
	if(eFlags & PROFILE_LOAD_FLAG_METADATA)
	{
		LoadMetadataBase(pSystem, pData, vecMessages);
	}

	KeyValues3 *pMember;

	// MenuProfile_t fields.
	m_sDisplayName = pData->GetMemberString("display_name");
	m_sDescription = pData->GetMemberString("description");
	m_pItems = LoadAllocatedItems(pData->FindMember("items"), vecMessages);
	m_sItemsVerificationClientConVarName = pData->GetMemberString("items_verification_client_convar_name");
	m_pMatrixOffset = LoadAllocatedMatrixOffset(pData->FindMember("matrix_offset"), vecMessages);
	m_pMatrixOffset_Previos = LoadAllocatedMatrixOffset(pData->FindMember("matrix_offset-previous"), vecMessages);
	m_pBackgroundColor = (pMember = pData->FindMember("background_color")) ? new Color(pMember->GetColor()) : nullptr;
	m_pInactiveColor = (pMember = pData->FindMember("inactive_color")) ? new Color(pMember->GetColor()) : nullptr;
	m_pActiveColor = (pMember = pData->FindMember("active_color")) ? new Color(pMember->GetColor()) : nullptr;
	m_pDisabledActiveColor = (pMember = pData->FindMember("disabled_active_color")) ? new Color(pMember->GetColor()) : nullptr;
	m_flBackgroundAwayUnits = pData->GetMemberFloat("background_away_units");
	m_vecResources.AddToTail(pData->GetMemberString("background_material_name"));

	if(!(eFlags & PROFILE_LOAD_FLAG_DONT_REMOVE_STATIC_MEMBERS))
	{
		RemoveStaticMembers(pData);
	}

	if(eFlags & PROFILE_LOAD_ENTITYKEYVALUES)
	{
		m_pData = LoadAllocatedEntityKeyValues(pSystem, pData, vecMessages);
	}

	return true;
}

void Menu::CProfile::LoadMetadataBase(CProfileSystem *pSystem, KeyValues3 *pData, CUtlVector<CUtlString> &vecMessages)
{
	// MetadataBase_t fields.
	m_aMetadata.m_bHidden = pData->GetMemberBool("hidden", false);
	LoadMetadataBases(pSystem, "inherits", pData->FindMember("inherits"), vecMessages);
}

bool Menu::CProfile::LoadMetadataBases(CProfileSystem *pSystem, const char *pszName, KeyValues3 *pData, CUtlVector<CUtlString> &vecMessages)
{
	if(!pData)
	{
		return false;
	}

	KV3Type_t eMemberType = pData->GetType();

	CBufferStringN<256> sMessage;

	if(eMemberType == KV3_TYPE_STRING)
	{
		if(!LoadMetadataBaseString(pSystem, pszName, pData, sMessage))
		{
			vecMessages.AddToTail(sMessage);
		}
	}
	else if(eMemberType == KV3_TYPE_ARRAY || eMemberType == KV3_TYPE_TABLE)
	{
		FOR_EACH_KV3(pData, it)
		{
			KeyValues3 *pArrayMember = it.Get();

			if(pArrayMember->GetType() == KV3_TYPE_STRING)
			{
				if(!LoadMetadataBaseString(pSystem, pszName, pArrayMember, sMessage))
				{
					vecMessages.AddToTail(sMessage);
				}
			}
			else
			{
				const char *pszMessageConcat[] = {"No handler ", "for \"", pArrayMember->GetTypeAsString(), "\" type", "of array by \"", pszName, "\" member"};

				sMessage.AppendConcat(ARRAYSIZE(pszMessageConcat), pszMessageConcat, NULL);
				vecMessages.AddToTail(sMessage);
				sMessage.Clear();
			}
		}
	}
	else
	{
		const char *pszMessageConcat[] = {"No handler ", "for \"", pData->GetTypeAsString(), "\" type", "of \"", pszName, "\" member"};

		sMessage.AppendConcat(ARRAYSIZE(pszMessageConcat), pszMessageConcat, NULL);
		vecMessages.AddToTail(sMessage);
		sMessage.Clear();
	}

	return true;
}

bool Menu::CProfile::LoadMetadataBaseString(CProfileSystem *pSystem, const char *pszName, KeyValues3 *pData, CBufferString &sMessage)
{
	const char *pszMemberValue = pData->GetString();

	auto *pProfile = pSystem->GetInternal(pszMemberValue);

	if(!pProfile)
	{
		const char *pszMessageConcat[] = {"Not found ", pszMemberValue, " profile ", "for \"", pszName, "\" member"};

		sMessage.AppendConcat(ARRAYSIZE(pszMessageConcat), pszMessageConcat, NULL);

		return false;
	}

	m_aMetadata.m_vecBases.AddToTail(pProfile);

	return true;
}

IMenuProfile::MatrixOffset_t *Menu::CProfile::LoadAllocatedMatrixOffset(KeyValues3 *pData, CUtlVector<CUtlString> &vecMessages)
{
	if(!pData)
	{
		return nullptr;
	}

	auto *pResult = new IMenuProfile::MatrixOffset_t();

	pResult->m_flForward = pData->GetMemberFloat("forward");
	pResult->m_flLeft = pData->GetMemberFloat("left");
	pResult->m_flRight = pData->GetMemberFloat("right");
	pResult->m_flUp = pData->GetMemberFloat("up");

	return pResult;
}

IMenuProfile::Items_t *Menu::CProfile::LoadAllocatedItems(KeyValues3 *pData, CUtlVector<CUtlString> &vecMessages)
{
	if(!pData)
	{
		return nullptr;
	}

	int nMemberCount = pData->GetMemberCount();

	if(!nMemberCount)
	{
		return nullptr;
	}

	auto *pResult = new Items_t();

	pResult->SetCount(nMemberCount);

	CBufferStringN<256> sMessage;

	KV3MemberId_t i = 0;

	do
	{
		const char *pszMemberName = pData->GetMemberName(i);

		KeyValues3 *pMember = pData->GetMember(i);

		int iItem = V_atoi(pszMemberName);

		if(iItem < nMemberCount)
		{
			pResult->Element(iItem) = pMember->GetString();
		}
		else
		{
			const char *pszMessageConcat[] = {"Item \"", pszMemberName, "\" overflow of "};

			sMessage.AppendConcat(ARRAYSIZE(pszMessageConcat), pszMessageConcat, NULL);

			CUtlString sItemMessage = sMessage;

			sItemMessage += iItem;
			sItemMessage += " limit";
			vecMessages.AddToTail(sItemMessage);
			sMessage.Clear();
		}

		i++;
	}
	while (i < nMemberCount);

	return pResult;
}

CEntityKeyValues *Menu::CProfile::LoadAllocatedEntityKeyValues(CProfileSystem *pSystem, KeyValues3 *pData, CUtlVector<CUtlString> &vecMessages)
{
	int nMemberCount = pData->GetMemberCount();

	if(!nMemberCount)
	{
		return nullptr;
	}

	CEntityKeyValues *pResult = new CEntityKeyValues(pSystem->GetEntityKeyValuesAllocator(), EKV_ALLOCATOR_EXTERNAL);

	KV3MemberId_t i = 0;

	do
	{
		const char *pszMemberName = pData->GetMemberName(i);

		KeyValues3 *pMember = pData->GetMember(i);

		KeyValues3 *pEKVMember = pResult->SetKeyValue({pData->GetMemberHash(i), pszMemberName});

		if(pEKVMember)
		{
			*pEKVMember = *pMember;
		}

		i++;
	}
	while (i < nMemberCount);

	return pResult;
}

void Menu::CProfile::RemoveStaticMembers(KeyValues3 *pData)
{
	RemoveStaticMetadataMembers(pData);

	pData->RemoveMember("display_name");
	pData->RemoveMember("description");

	pData->RemoveMember("items");
	pData->RemoveMember("items_verification_client_convar_name");
	pData->RemoveMember("matrix_offset");
	pData->RemoveMember("matrix_offset-previous");
	pData->RemoveMember("inactive_color");
	pData->RemoveMember("active_color");
	pData->RemoveMember("disabled_active_color");
	pData->RemoveMember("background_away_units");
}

void Menu::CProfile::RemoveStaticMetadataMembers(KeyValues3 *pData)
{
	pData->RemoveMember("hidden");
	pData->RemoveMember("inherits");
}

const CUtlString &Menu::CProfile::GetDisplayName() const
{
	const CUtlString *pResult = &m_sDisplayName;

	if(pResult->IsEmpty())
	{
		for(const auto &pInherited : m_aMetadata.GetBaseline())
		{
			if(!(pResult = &pInherited->GetDisplayName())->IsEmpty())
			{
				break;
			}
		}
	}

	return *pResult;
}

const CUtlString &Menu::CProfile::GetDescription() const
{
	const CUtlString *pResult = &m_sDescription;

	if(pResult->IsEmpty())
	{
		for(const auto &pInherited : m_aMetadata.GetBaseline())
		{
			if(!(pResult = &pInherited->GetDescription())->IsEmpty())
			{
				break;
			}
		}
	}

	return *pResult;
}

const IMenuProfile::Items_t *Menu::CProfile::GetItems() const
{
	const auto *pResult = m_pItems;

	if(!pResult)
	{
		for(const auto &pInherited : m_aMetadata.GetBaseline()) 
		{
			if(pResult = pInherited->GetItems()) 
			{
				break;
			}
		}
	}

	return pResult;
}

const CUtlString &Menu::CProfile::GetItemsVerificationClientConVarName() const
{
	const CUtlString *pResult = &m_sItemsVerificationClientConVarName;

	if(pResult->IsEmpty())
	{
		for(const auto &pInherited : m_aMetadata.GetBaseline())
		{
			if(!(pResult = &pInherited->GetItemsVerificationClientConVarName())->IsEmpty())
			{
				break;
			}
		}
	}

	return *pResult;
}

const IMenuProfile::MatrixOffset_t *Menu::CProfile::GetMatrixOffset() const
{
	const auto *pResult = m_pMatrixOffset;

	if(!pResult)
	{
		for(const auto &pInherited : m_aMetadata.GetBaseline()) 
		{
			if(pResult = pInherited->GetMatrixOffset()) 
			{
				break;
			}
		}
	}

	return pResult;
}

const IMenuProfile::MatrixOffset_t *Menu::CProfile::GetPreviosMatrixOffset() const
{
	const auto *pResult = m_pMatrixOffset_Previos;

	if(!pResult)
	{
		for(const auto &pInherited : m_aMetadata.GetBaseline()) 
		{
			if(pResult = pInherited->GetPreviosMatrixOffset()) 
			{
				break;
			}
		}
	}

	return pResult;
}

const Color *Menu::CProfile::GetBackgroundColor() const
{
	const auto *pResult = m_pBackgroundColor;

	if(!pResult)
	{
		for(const auto &pInherited : m_aMetadata.GetBaseline()) 
		{
			if(pResult = pInherited->GetBackgroundColor()) 
			{
				break;
			}
		}
	}

	return pResult;
}


const Color *Menu::CProfile::GetInactiveColor() const
{
	const auto *pResult = m_pInactiveColor;

	if(!pResult)
	{
		for(const auto &pInherited : m_aMetadata.GetBaseline()) 
		{
			if(pResult = pInherited->GetInactiveColor()) 
			{
				break;
			}
		}
	}

	return pResult;
}

const Color *Menu::CProfile::GetActiveColor() const
{
	const auto *pResult = m_pActiveColor;

	if(!pResult)
	{
		for(const auto &pInherited : m_aMetadata.GetBaseline()) 
		{
			if(pResult = pInherited->GetActiveColor()) 
			{
				break;
			}
		}
	}

	return pResult;
}

const Color *Menu::CProfile::GetDisabledActiveColor() const
{
	const auto *pResult = m_pDisabledActiveColor;

	if(!pResult)
	{
		for(const auto &pInherited : m_aMetadata.GetBaseline())
		{
			if(pResult = pInherited->GetDisabledActiveColor())
			{
				break;
			}
		}
	}

	return pResult;
}

float Menu::CProfile::GetBackgroundAwayUnits() const
{
	float flResult = m_flBackgroundAwayUnits;

	if(!flResult)
	{
		for(const auto &pInherited : m_aMetadata.GetBaseline()) 
		{
			if(flResult = pInherited->GetBackgroundAwayUnits()) 
			{
				break;
			}
		}
	}

	return flResult;
}


CUtlVector<const char *> Menu::CProfile::GetResources() const
{
	CUtlVector<const char *> vecResult = m_vecResources.GetExports();

	Metadata_t::Bases_t vecBaseline = m_aMetadata.GetBaseline();

	FOR_EACH_VEC_BACK(vecBaseline, i)
	{
		const Resources_t &vecBaseResources = vecBaseline[i]->m_vecResources;

		auto vecSubresult = vecBaseResources.GetExports();

		vecResult.AddMultipleToTail(vecSubresult.Count(), vecSubresult.Base());
	}

	return vecResult;
}

CEntityKeyValues *Menu::CProfile::GetAllocactedEntityKeyValues(CKeyValues3Context *pAllocator, bool bIncludeBackground) const
{
	CEntityKeyValues *pResult = new CEntityKeyValues(pAllocator, pAllocator ? EKV_ALLOCATOR_EXTERNAL : EKV_ALLOCATOR_NORMAL);

	Assert(pResult);

	Metadata_t::Bases_t vecBaseline = m_aMetadata.GetBaseline();

	FOR_EACH_VEC_BACK(vecBaseline, i)
	{
		const CEntityKeyValues *pCopyData = vecBaseline[i]->m_pData;

		if(pCopyData)
		{
			if(bIncludeBackground)
			{
				pResult->CopyFrom(pCopyData, false, false);
			}
			else
			{
				FOR_EACH_ENTITYKEY(pCopyData, it)
				{
					auto aKeyId = pCopyData->GetEntityKeyId(it);

					if(!V_strstr(aKeyId.GetString(), "background"))
					{
						auto *pKVToSet = pResult->SetKeyValue(aKeyId);

						if(pKVToSet)
							*pKVToSet = *pCopyData->GetKeyValue(it);
					}
				}
			}
		}
	}

	if(m_pData)
	{
		pResult->CopyFrom(m_pData);
	}

	return pResult;
}
