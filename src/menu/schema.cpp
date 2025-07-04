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

#include <menu/schema.hpp>

#include <stddef.h>

#include <array>

#ifdef __cpp_lib_ranges
#	include <ranges>
#else
#	include <algorithm>
#endif // __cpp_lib_ranges

#include <tier0/bufferstring.h>
#include <tier0/commonmacros.h>
#include <tier0/utlstringtoken.h>
#include <tier1/utlmap.h>
#include <tier1/utlvector.h>
#include <schemasystem/schemasystem.h>

#define SCHEMA_HASH_TYPE CUtlStringToken
#define SCHEMA_HASH(x) SCHEMA_HASH_TYPE(x)
#define SCHEMA_MAKE_HASH MakeStringToken

#define SCHEMA_DETAILS_CONCAT_DECLARE_VARIABLES(messagesVar, concatVar) \
	auto *messagesVar = GetMessages(); \
	\
	Assert(messagesVar); \
	\
	const auto *concatVar = GetConcatLine(); \
	\
	Assert(concatVar);

static const std::array s_arrSchemaMetadataHash_Integer 
{
	SCHEMA_HASH("MNetworkVarEmbeddedFieldOffsetDelta"),
	SCHEMA_HASH("MNetworkBitCount"),
	SCHEMA_HASH("MNetworkPriority"),

	SCHEMA_HASH("MPropertySortPriority"),

	SCHEMA_HASH("MParticleMinVersion"),
	SCHEMA_HASH("MParticleMaxVersion"),

	SCHEMA_HASH("MNetworkEncodeFlags"),
};

static const std::array s_arrSchemaMetadataHash_Float 
{
	SCHEMA_HASH("MNetworkMinValue"),
	SCHEMA_HASH("MNetworkMaxValue"),
};

static const std::array s_arrSchemaMetadataHash_String 
{
	SCHEMA_HASH("MNetworkChangeCallback"),

	SCHEMA_HASH("MNetworkUserGroup"),
	SCHEMA_HASH("MNetworkAlias"),
	SCHEMA_HASH("MNetworkEncoder"),
	SCHEMA_HASH("MNetworkTypeAlias"),
	SCHEMA_HASH("MNetworkSerializer"),

	SCHEMA_HASH("MNetworkExcludeByName"),
	SCHEMA_HASH("MNetworkExcludeByUserGroup"),

	SCHEMA_HASH("MNetworkIncludeByName"),
	SCHEMA_HASH("MNetworkIncludeByUserGroup"),

	SCHEMA_HASH("MNetworkUserGroupProxy"),
	SCHEMA_HASH("MNetworkReplayCompatField"),

	SCHEMA_HASH("MPropertyFriendlyName"),
	SCHEMA_HASH("MPropertyDescription"),
	SCHEMA_HASH("MPropertyAttributeRange"),
	SCHEMA_HASH("MPropertyStartGroup"),
	SCHEMA_HASH("MPropertyAttributeChoiceName"),
	SCHEMA_HASH("MPropertyGroupName"),

	SCHEMA_HASH("MPropertyArrayElementNameKey"),
	SCHEMA_HASH("MPropertyFriendlyName"),
	SCHEMA_HASH("MPropertyDescription"),

	SCHEMA_HASH("MPropertyAttributeEditor"),
	SCHEMA_HASH("MPropertySuppressExpr"),

	SCHEMA_HASH("MPropertyCustomFGDType"),

	SCHEMA_HASH("MPropertyArrayElementNameKey"),
	SCHEMA_HASH("MPropertyFriendlyName"),
	SCHEMA_HASH("MPropertyDescription"),

	SCHEMA_HASH("MKV3TransferName"),
	SCHEMA_HASH("MFieldVerificationName"),
	SCHEMA_HASH("MVectorIsSometimesCoordinate"),
	SCHEMA_HASH("MVDataUniqueMonotonicInt"),
	SCHEMA_HASH("MScriptDescription"),
};

static const std::array s_arrSchemaMetadataHash_BufferString 
{
	SCHEMA_HASH("MResourceTypeForInfoType"),
};

static const std::array s_arrSchemaMetadataHash_VarName 
{
	SCHEMA_HASH("MNetworkVarNames"),
	SCHEMA_HASH("MNetworkOverride"),
	SCHEMA_HASH("MNetworkVarTypeOverride"),
};

void Menu::Schema::CSystem::CDetailsConcatBase::AppendEmpty()
{
	SCHEMA_DETAILS_CONCAT_DECLARE_VARIABLES(pMessages, pConcat);

	pMessages->AddToTail(CBufferStringN<8>(pConcat->GetEnds()));
}

void Menu::Schema::CSystem::CDetailsConcatTypeScope::AppendHeader()
{
	SCHEMA_DETAILS_CONCAT_DECLARE_VARIABLES(pMessages, pConcat);

	CHugeBufferString sBuffer;

	pConcat->AppendStringHeadToBuffer(sBuffer, m_pData->m_szScopeName);
	pMessages->AddToTail(sBuffer);
}

void Menu::Schema::CSystem::CDetailsConcatTypeScope::AppendMembers()
{
	SCHEMA_DETAILS_CONCAT_DECLARE_VARIABLES(pMessages, pConcat);

	CHugeBufferString sBuffer;

	auto *pTypeScope = m_pData;

	pConcat->AppendToBuffer(sBuffer, "Count of classes", pTypeScope->m_DeclaredClasses.m_Map.Count());
	pMessages->AddToTail(sBuffer);
	sBuffer.Clear();

	pConcat->AppendToBuffer(sBuffer, "Count of enums", pTypeScope->m_DeclaredEnums.m_Map.Count());
	pMessages->AddToTail(sBuffer);
	sBuffer.Clear();

	pConcat->AppendToBuffer(sBuffer, "Count of atomic infos", pTypeScope->m_AtomicInfos.m_Map.Count());
	pMessages->AddToTail(sBuffer);
	sBuffer.Clear();

	pConcat->AppendToBuffer(sBuffer, "Count of fixed arrays", pTypeScope->m_FixedArrays.m_Map.Count());
	pMessages->AddToTail(sBuffer);
	sBuffer.Clear();

	pConcat->AppendToBuffer(sBuffer, "Count of bitfields", pTypeScope->m_Bitfields.m_Map.Count());
	pMessages->AddToTail(sBuffer);
}

void Menu::Schema::CSystem::CDetailsConcatTypeScope::AppendClasses()
{
	SCHEMA_DETAILS_CONCAT_DECLARE_VARIABLES(pMessages, pConcat);

	CTinyBufferString sBuffer;

	pConcat->AppendToBuffer(sBuffer, "Classes");
	pMessages->AddToTail(sBuffer);
}

void Menu::Schema::CSystem::CDetailsConcatType::AppendHeader()
{
	SCHEMA_DETAILS_CONCAT_DECLARE_VARIABLES(pMessages, pConcat);

	CSmallBufferString sBuffer;

	pConcat->AppendHeadToBuffer(sBuffer, "Type");
	pMessages->AddToTail(sBuffer);
}

void Menu::Schema::CSystem::CDetailsConcatType::AppendMembers()
{
	SCHEMA_DETAILS_CONCAT_DECLARE_VARIABLES(pMessages, pConcat);

	CHugeBufferString sBuffer;

	auto *pType = m_pData;

	{
		CBufferStringN<MAX_SCHEMA_TYPE_NAME_SIZE> sName;

		pType->ToString(sName, true);
		pConcat->AppendStringToBuffer(sBuffer, "Name", sName.Get());
		pMessages->AddToTail(sBuffer);
		sBuffer.Clear();
	}

	{
		int nSize;
		uint8 nAlignment;

		pType->GetSizeAndAlignment(nSize, nAlignment);

		pConcat->AppendToBuffer(sBuffer, "Size", nSize);
		pMessages->AddToTail(sBuffer);
		sBuffer.Clear();

		pConcat->AppendToBuffer(sBuffer, "Alignment", nAlignment);
		pMessages->AddToTail(sBuffer);
	}
}

void Menu::Schema::CSystem::CDetailsConcatClass::AppendHeader()
{
	SCHEMA_DETAILS_CONCAT_DECLARE_VARIABLES(pMessages, pConcat);

	CSmallBufferString sBuffer;

	pConcat->AppendStringHeadToBuffer(sBuffer, m_pData->m_pszName);
	pMessages->AddToTail(sBuffer);
}

void Menu::Schema::CSystem::CDetailsConcatClass::AppendMembers()
{
	SCHEMA_DETAILS_CONCAT_DECLARE_VARIABLES(pMessages, pConcat);

	CHugeBufferString sBuffer;

	auto *pInfo = m_pData;

	pConcat->AppendStringToBuffer(sBuffer, "Project name", pInfo->m_pszProjectName);
	pMessages->AddToTail(sBuffer);
	sBuffer.Clear();

	pConcat->AppendToBuffer(sBuffer, "Size", pInfo->m_nSize);
	pMessages->AddToTail(sBuffer);
	sBuffer.Clear();

	pConcat->AppendToBuffer(sBuffer, "Count of fields", pInfo->m_nFieldCount);
	pMessages->AddToTail(sBuffer);
	sBuffer.Clear();

	pConcat->AppendToBuffer(sBuffer, "Count of static fields", pInfo->m_nStaticFieldCount);
	pMessages->AddToTail(sBuffer);
	sBuffer.Clear();

	pConcat->AppendToBuffer(sBuffer, "Alignment", pInfo->m_nAlignment);
	pMessages->AddToTail(sBuffer);
}

void Menu::Schema::CSystem::CDetailsConcatClass::AppendBaseClasses()
{
	SCHEMA_DETAILS_CONCAT_DECLARE_VARIABLES(pMessages, pConcat);

	CSmallBufferString sBuffer;

	pConcat->AppendToBuffer(sBuffer, "Base classes");
	pMessages->AddToTail(sBuffer);
}

void Menu::Schema::CSystem::CDetailsConcatClass::AppendFields()
{
	SCHEMA_DETAILS_CONCAT_DECLARE_VARIABLES(pMessages, pConcat);

	CSmallBufferString sBuffer;

	pConcat->AppendToBuffer(sBuffer, "Fields");
	pMessages->AddToTail(sBuffer);
}

void Menu::Schema::CSystem::CDetailsConcatField::AppendHeader()
{
	SCHEMA_DETAILS_CONCAT_DECLARE_VARIABLES(pMessages, pConcat);

	CXLargeBufferString sBuffer;

	pConcat->AppendStringHeadToBuffer(sBuffer, m_pData->m_pszName);
	pMessages->AddToTail(sBuffer);
}

void Menu::Schema::CSystem::CDetailsConcatField::AppendMembers()
{
	SCHEMA_DETAILS_CONCAT_DECLARE_VARIABLES(pMessages, pConcat);

	CTinyBufferString sBuffer;

	pConcat->AppendToBuffer(sBuffer, "Offset", m_pData->m_nSingleInheritanceOffset);
	pMessages->AddToTail(sBuffer);
}

void Menu::Schema::CSystem::CDetailsConcatField::AppendMetadataMember()
{
	SCHEMA_DETAILS_CONCAT_DECLARE_VARIABLES(pMessages, pConcat);

	CHugeBufferString sBuffer;

	pConcat->AppendToBuffer(sBuffer, "Metadata");
	pMessages->AddToTail(sBuffer);
}

void Menu::Schema::CSystem::CDetailsConcatMetadataEntry::AppendHeader()
{
	SCHEMA_DETAILS_CONCAT_DECLARE_VARIABLES(pMessages, pConcat);

	CHugeBufferString sBuffer;

	if(m_pData->m_pData)
	{
		pConcat->AppendStringHeadToBuffer(sBuffer, m_pData->m_pszName);
	}
	else
	{
		pConcat->AppendStringHeadWithoutBeforeToBuffer(sBuffer, m_pData->m_pszName);
	}

	pMessages->AddToTail(sBuffer);
}

void Menu::Schema::CSystem::CDetailsConcatMetadataEntry::AppendMembers()
{
	SCHEMA_DETAILS_CONCAT_DECLARE_VARIABLES(pMessages, pConcat);

	CHugeBufferString sBuffer;

	void *pData = m_pData->m_pData;

	if(pData)
	{
		const char *pszName = m_pData->m_pszName;

		SCHEMA_HASH_TYPE nNameHash = SCHEMA_MAKE_HASH(pszName);

		const char szConcatKey[] = "Storage";

		union Data_t
		{
			~Data_t() = delete;

			int nValue;
			float flValue;
			const char *pszValue;
			CBufferStringN<32> sValue;

			struct VarName_t
			{
				const char *pszVar;
				const char *pszName;
			} m_aKeyValue;

		} *pDataT = reinterpret_cast<Data_t *>(pData);

#ifdef __cpp_lib_ranges
		if(std::ranges::find(s_arrSchemaMetadataHash_Integer, nNameHash) != s_arrSchemaMetadataHash_Integer.cend())
		{
			pConcat->AppendToBuffer(sBuffer, szConcatKey, pDataT->nValue);
			pMessages->AddToTail(sBuffer);
		}
		else if(std::ranges::find(s_arrSchemaMetadataHash_Float, nNameHash) != s_arrSchemaMetadataHash_Float.cend())
		{
			pConcat->AppendToBuffer(sBuffer, szConcatKey, pDataT->nValue);
			pMessages->AddToTail(sBuffer);
		}
		else if(std::ranges::find(s_arrSchemaMetadataHash_String, nNameHash) != s_arrSchemaMetadataHash_String.cend())
		{
			pConcat->AppendStringToBuffer(sBuffer, szConcatKey, pDataT->pszValue);
			pMessages->AddToTail(sBuffer);
		}
		else if(std::ranges::find(s_arrSchemaMetadataHash_BufferString, nNameHash) != s_arrSchemaMetadataHash_BufferString.cend())
		{
			pConcat->AppendStringToBuffer(sBuffer, szConcatKey, pDataT->sValue.Get());
			pMessages->AddToTail(sBuffer);
		}
		else if(std::ranges::find(s_arrSchemaMetadataHash_VarName, nNameHash) != s_arrSchemaMetadataHash_VarName.cend())
		{
			pConcat->AppendStringToBuffer(sBuffer, pDataT->m_aKeyValue.pszVar, pDataT->m_aKeyValue.pszName);
			pMessages->AddToTail(sBuffer);
		}
#else
		if(std::find(s_arrSchemaMetadataHash_Integer.cbegin(), s_arrSchemaMetadataHash_Integer.cend(), nNameHash) != s_arrSchemaMetadataHash_Integer.cend())
		{
			pConcat->AppendToBuffer(sBuffer, szConcatKey, pDataT->nValue);
			pMessages->AddToTail(sBuffer);
		}
		else if(std::find(s_arrSchemaMetadataHash_Float.cbegin(), s_arrSchemaMetadataHash_Float.cend(), nNameHash) != s_arrSchemaMetadataHash_Float.cend())
		{
			pConcat->AppendToBuffer(sBuffer, szConcatKey, pDataT->nValue);
			pMessages->AddToTail(sBuffer);
		}
		else if(std::find(s_arrSchemaMetadataHash_String.cbegin(), s_arrSchemaMetadataHash_String.cend(), nNameHash) != s_arrSchemaMetadataHash_String.cend())
		{
			pConcat->AppendStringToBuffer(sBuffer, szConcatKey, pDataT->pszValue);
			pMessages->AddToTail(sBuffer);
		}
		else if(std::find(s_arrSchemaMetadataHash_BufferString.cbegin(), s_arrSchemaMetadataHash_BufferString.cend(), nNameHash) != s_arrSchemaMetadataHash_BufferString.cend())
		{
			pConcat->AppendStringToBuffer(sBuffer, szConcatKey, pDataT->sValue.Get());
			pMessages->AddToTail(sBuffer);
		}
		else if(std::find(s_arrSchemaMetadataHash_VarName.cbegin(), s_arrSchemaMetadataHash_VarName.cend(), nNameHash) != s_arrSchemaMetadataHash_VarName.cend())
		{
			pConcat->AppendKeyStringValueStringToBuffer(sBuffer, pDataT->m_aKeyValue.pszVar, pDataT->m_aKeyValue.pszName);
			pMessages->AddToTail(sBuffer);
		}
#endif
		else
		{
			pConcat->AppendPointerToBuffer(sBuffer, szConcatKey, pData);
			pMessages->AddToTail(sBuffer);
		}
	}
}

Menu::Schema::CSystem::CSystem()
 :  m_mapClasses(DefLessFunc(CUtlSymbolLarge))
{
}

bool Menu::Schema::CSystem::Init(ISchemaSystem *pSchemaSystem, const CUtlVector<const char *> &vecLoadedLibraries, CStringVector *pMessages)
{
	CBufferStringN<1024> sBuffer;

	int nPreviousCount = m_vecTypeScopes.Count();

	for(const auto &pszName : vecLoadedLibraries)
	{
		auto *pTypeScope = pSchemaSystem->FindTypeScopeForModule(pszName);

		if(pTypeScope)
		{
			m_vecTypeScopes.AddToTail(pTypeScope);
		}
		else if(pMessages)
		{
			pMessages->AddToTail(CLargeBufferString({"Failed to ", "find \"", pszName, "\" library"}));
		}
	}

	return nPreviousCount < m_vecTypeScopes.Count();
}

bool Menu::Schema::CSystem::Load(FullDetails_t *pDetails)
{
	for(auto *pTypeScope : m_vecTypeScopes)
	{
		if(pDetails)
		{
			CDetailsConcatTypeScope aDetailsTypeScope(pDetails, pTypeScope);

			aDetailsTypeScope.AppendHeader();
			aDetailsTypeScope.AppendMembers();
			aDetailsTypeScope.AppendClasses();
		}

		LoadClasses(pTypeScope, reinterpret_cast<ClassDetails_t *>(pDetails));
	}

	return true;
}

void Menu::Schema::CSystem::Clear()
{
	ClearClasses();
	m_vecTypeScopes.Purge();
}

SchemaClassFieldData_t *Menu::Schema::CSystem::CClass::GetField(const CUtlSymbolLarge &sName) const
{
	return m_aFieldStorage.Get(sName);
}

void Menu::Schema::CSystem::CClass::SetField(const CUtlSymbolLarge &sName, SchemaClassFieldData_t *pData)
{
	m_aFieldStorage.Set(sName, Move(pData));
}

SchemaClassFieldData_t *Menu::Schema::CSystem::CClass::FindField(const char *pszName) const
{
	auto sFoundSymbol = FindFieldSymbol(pszName);

	if(!sFoundSymbol.IsValid())
	{
		return nullptr;
	}

	return GetField(sFoundSymbol);
}

int Menu::Schema::CSystem::CClass::FindFieldOffset(const char *pszName) const
{
	auto *pField = FindField(pszName);

	if(!pField)
	{
		return INVALID_SCHEMA_FIELD_OFFSET;
	}

	return pField->m_nSingleInheritanceOffset;
}

void Menu::Schema::CSystem::CClass::LoadFields(CSchemaClassInfo *pInfo, FieldDetails_t *pDetails)
{
	const uint16 nFieldCount = pInfo->m_nFieldCount;

	for(uint16 n = 0; n < nFieldCount; n++)
	{
		auto &aField = pInfo->m_pFields[n];

		const auto &sField = FindFieldSymbol(aField.m_pszName);

		if(!sField.IsValid())
		{
			continue;
		}

		auto *pField = &aField;

		if(pDetails)
		{
			CDetailsConcatField aDetailsField(pDetails, pField);

			aDetailsField.AppendHeader();
			aDetailsField.AppendMembers();

			auto *pFieldType = aField.m_pType;

			if(pFieldType)
			{
				CDetailsConcatType aDetailsType(reinterpret_cast<FieldTypeDetails_t *>(pDetails), pFieldType);

				aDetailsType.AppendHeader();
				aDetailsType.AppendMembers();
			}

			int nMetadataEntryCount = aField.m_nStaticMetadataCount;

			if(nMetadataEntryCount)
			{
				aDetailsField.AppendMetadataMember();

				{
					int i = 0;

					do
					{
						auto *pMetadata = &aField.m_pStaticMetadata[i];

						CDetailsConcatMetadataEntry aDetailsMetadataEntry(reinterpret_cast<MetadataEntryDetails_t *>(pDetails), pMetadata);

						aDetailsMetadataEntry.AppendHeader();
						aDetailsMetadataEntry.AppendMembers();

						i++;
					}
					while(i < nMetadataEntryCount);
				}
			}
		}

		SetField(sField, pField);
	}
}

CUtlSymbolLarge Menu::Schema::CSystem::CClass::GetFieldSymbol(const char *pszName)
{
	return m_tableFileds.AddString(pszName);
}

CUtlSymbolLarge Menu::Schema::CSystem::CClass::FindFieldSymbol(const char *pszName) const
{
	return m_tableFileds.Find(pszName);
}

Menu::Schema::CSystem::CClass *Menu::Schema::CSystem::GetClass(const char *pszName)
{
	auto sSymbol = GetClassSymbol(pszName);

	if(!sSymbol.IsValid())
	{
		return nullptr;
	}

	auto iFound = m_mapClasses.Find(sSymbol);

	if(!m_mapClasses.IsValidIndex(iFound))
	{
		iFound = m_mapClasses.Insert(sSymbol, {});
	}

	return &m_mapClasses[iFound];
}

Menu::Schema::CSystem::CClass *Menu::Schema::CSystem::FindClass(const char *pszName)
{
	auto sFoundSymbol = FindClassSymbol(pszName);

	if(!sFoundSymbol.IsValid())
	{
		return nullptr;
	}

	auto iFound = m_mapClasses.Find(sFoundSymbol);

	if(!m_mapClasses.IsValidIndex(iFound))
	{
		return nullptr;
	}

	return &m_mapClasses[iFound];
}

int Menu::Schema::CSystem::FindClassFieldOffset(const char *pszClassName, const char *pszFiledName)
{
	auto *pClass = FindClass(pszClassName);

	if(!pClass)
	{
		return INVALID_SCHEMA_FIELD_OFFSET;
	}

	return pClass->FindFieldOffset(pszFiledName);
}

void Menu::Schema::CSystem::LoadClasses(CSchemaSystemTypeScope *pScope, ClassDetails_t *pDetails)
{
	auto mapDeclaredClasses = pScope->m_DeclaredClasses.m_Map;

	FOR_EACH_MAP_FAST(mapDeclaredClasses, i)
	{
		auto *pDeclared = mapDeclaredClasses[i];

		auto *pClassInfo = pDeclared->m_pClassInfo;

		const char *pszClassName = pClassInfo->m_pszName;

		auto *pClass = FindClass(pszClassName);

		if(pClass)
		{
			if(pDetails)
			{
				CDetailsConcatClass aDetailsClass(pDetails, pClassInfo);

				aDetailsClass.AppendHeader();
				aDetailsClass.AppendMembers();

#if false
				{
					auto *pClassType = pClassInfo->m_pDeclaredClass;

					if(pClassType)
					{
						CDetailsConcatType aDetailsType(reinterpret_cast<ClassTypeDetails_t *>(pDetails), pClassType);

						aDetailsType.AppendHeader();
						aDetailsType.AppendMembers();
					}
				}
#endif // false

				aDetailsClass.AppendBaseClasses();

				int nBaseClasses = pClassInfo->m_nBaseClassCount;

				for(int i = 0; i < nBaseClasses; i++)
				{
					auto *pBaseClass = pClassInfo->m_pBaseClasses[i].m_pClass;

					auto *pBaseClassType = pBaseClass->m_pDeclaredClass;

					if(pBaseClassType)
					{
						CDetailsConcatType aDetailsType(reinterpret_cast<BaseClassTypeDetails_t *>(pDetails), pBaseClassType);

						aDetailsType.AppendHeader();
						aDetailsType.AppendMembers();
					}
				}

				aDetailsClass.AppendFields();
			}

			pClass->LoadFields(pClassInfo, reinterpret_cast<FieldDetails_t *>(pDetails));
		}
	}
}

void Menu::Schema::CSystem::ClearClasses()
{
	m_tableClasses.Purge();
	m_mapClasses.Purge();
}

CUtlSymbolLarge Menu::Schema::CSystem::GetClassSymbol(const char *pszName)
{
	return m_tableClasses.AddString(pszName);
}

CUtlSymbolLarge Menu::Schema::CSystem::FindClassSymbol(const char *pszName) const
{
	return m_tableClasses.Find(pszName);
}
