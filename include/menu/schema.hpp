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

#ifndef _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_HPP_
#	define _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_HPP_

#	pragma once

#	include <array>
#	include <cstddef>
#	include <type_traits>

#	include <tier0/dbg.h>
#	include <tier0/platform.h>
#	include <tier0/utlstring.h>
#	include <tier1/utlsymbollarge.h>
#	include <tier1/utlvector.h>
#	include <tier1/utlmap.h>
#	include <entity2/entityinstance.h>

#	include <concat.hpp>
#	include <gamedata.hpp>

#	define INVALID_SCHEMA_FIELD_OFFSET -1
#	define INVALID_SCHEMA_FIELD_ARRAY_SIZE -1

#	define MAX_SCHEMA_TYPE_NAME_SIZE 256

#	ifdef _DEBUG
#		define SCHEMA_FORCEINLINE
#	else
#		define SCHEMA_FORCEINLINE FORCEINLINE
#	endif

#	define SCHEMA_METHOD_ACCESSOR(methodName, classType, fieldAccessor, fieldType, fieldOffsetVar, additionalOffset) \
	SCHEMA_FORCEINLINE fieldAccessor<fieldType> methodName(classType *pInstance) const \
	{ \
		Assert(pInstance); \
		Assert(fieldOffsetVar != INVALID_SCHEMA_FIELD_OFFSET); \
	\
		return {pInstance, static_cast<uintp>(fieldOffsetVar) + additionalOffset}; \
	}

#	define SCHEMA_METHOD_ACCESSOR2(methodName, classType, fieldAccessor, fieldType, fieldOffsetVar, additionalOffset) \
	SCHEMA_FORCEINLINE fieldAccessor<classType, fieldType> methodName(classType *pComponent) const \
	{ \
		Assert(pComponent); \
		Assert(fieldOffsetVar != INVALID_SCHEMA_FIELD_OFFSET); \
	\
		return {pComponent, static_cast<uintp>(fieldOffsetVar) + additionalOffset}; \
	}

#	define SCHEMA_METHOD_ARRAY_ACCESSOR(methodName, classType, fieldAccessor, fieldType, fieldOffsetVar, additionalOffset, fieldSizeVar) \
	SCHEMA_FORCEINLINE fieldAccessor<fieldType> methodName(classType *pInstance) const \
	{ \
		Assert(pInstance); \
		Assert(fieldOffsetVar != INVALID_SCHEMA_FIELD_OFFSET); \
		Assert(fieldSizeVar != INVALID_SCHEMA_FIELD_ARRAY_SIZE); \
	\
		return {{pInstance, static_cast<uintp>(fieldOffsetVar)}, static_cast<uintp>(fieldSizeVar) + additionalOffset}; \
	}

#	define SCHEMA_METHOD_ARRAY_ACCESSOR2(methodName, classType, fieldAccessor, fieldType, fieldOffsetVar, additionalOffset, fieldSizeVar) \
	SCHEMA_FORCEINLINE fieldAccessor<classType, fieldType> methodName(classType *pComponent) const \
	{ \
		Assert(pComponent); \
		Assert(fieldOffsetVar != INVALID_SCHEMA_FIELD_OFFSET); \
		Assert(fieldSizeVar != INVALID_SCHEMA_FIELD_ARRAY_SIZE); \
	\
		return {{pComponent, static_cast<uintp>(fieldOffsetVar) + additionalOffset}, static_cast<uintp>(fieldSizeVar)}; \
	}

#	define SCHEMA_COMPONENT_ACCESSOR_METHOD2(methodName, classType, fieldType, fieldOffsetVar, additionalOffset) SCHEMA_METHOD_ACCESSOR2(methodName, classType, Accessor::CField, fieldType, fieldOffsetVar, additionalOffset)
#	define SCHEMA_COMPONENT_ARRAY_ACCESSOR_METHOD2(methodName, classType, fieldType, fieldOffsetVar, additionalOffset, fieldSizeVar) SCHEMA_METHOD_ARRAY_ACCESSOR2(methodName, classType, Accessor::CArrayField, fieldType, fieldOffsetVar, additionalOffset, fieldSizeVar)
#	define SCHEMA_COMPONENT_ACCESSOR_METHOD(methodName, classType, fieldType, fieldOffsetVar) SCHEMA_COMPONENT_ACCESSOR_METHOD2(methodName, classType, fieldType, fieldOffsetVar, 0)
#	define SCHEMA_COMPONENT_ARRAY_ACCESSOR_METHOD(methodName, classType, fieldType, fieldOffsetVar, fieldSizeVar) SCHEMA_COMPONENT_ARRAY_ACCESSOR_METHOD2(methodName, classType, fieldType, fieldOffsetVar, 0, fieldSizeVar)

#	define SCHEMA_INSTANCE_ACCESSOR_METHOD2(methodName, classType, fieldType, fieldOffsetVar, additionalOffset) SCHEMA_METHOD_ACCESSOR(methodName, classType, Accessor::CInstanceField, fieldType, fieldOffsetVar, additionalOffset)
#	define SCHEMA_INSTANCE_ARRAY_ACCESSOR_METHOD2(methodName, classType, fieldType, fieldOffsetVar, additionalOffset, fieldSizeVar) SCHEMA_METHOD_ARRAY_ACCESSOR(methodName, classType, Accessor::CInstanceArrayField, fieldType, fieldOffsetVar, additionalOffset, fieldSizeVar)
#	define SCHEMA_INSTANCE_ACCESSOR_METHOD(methodName, classType, fieldType, fieldOffsetVar) SCHEMA_INSTANCE_ACCESSOR_METHOD2(methodName, classType, fieldType, fieldOffsetVar, 0)
#	define SCHEMA_INSTANCE_ARRAY_ACCESSOR_METHOD(methodName, classType, fieldType, fieldOffsetVar, fieldSizeVar) SCHEMA_INSTANCE_ARRAY_ACCESSOR_METHOD2(methodName, classType, fieldType, fieldOffsetVar, 0, fieldSizeVar)


#	define SCHEMA_CLASS_FIELD_LAMBDA_CAPTURE(fieldOffsetVar) \
	[&_offset = fieldOffsetVar](const CUtlSymbolLarge &, SchemaClassFieldData_t *pField) \
	{ \
		_offset = pField->m_nSingleInheritanceOffset; \
	}
#	define SCHEMA_CLASS_ARRAY_FIELD_LAMBDA_CAPTURE(fieldType, fieldOffsetVar, fieldSizeVar) \
	[&_offset = fieldOffsetVar, &_arraySize = fieldSizeVar](const CUtlSymbolLarge &, SchemaClassFieldData_t *pField) \
	{ \
		_offset = pField->m_nSingleInheritanceOffset; \
	\
		Assert(pField->m_pType); \
	\
		{ \
			int nSize {}; \
			uint8 nAlignment {}; \
		\
			pField->m_pType->GetSizeAndAlignment(nSize, nAlignment); \
			_arraySize = nSize / sizeof(fieldType); \
		} \
	}

#	define SCHEMA_CLASS_FIELD_SHARED_LAMBDA_CAPTURE(fieldOffsetVar) {SCHEMA_CLASS_FIELD_LAMBDA_CAPTURE(fieldOffsetVar)}
#	define SCHEMA_CLASS_ARRAY_FIELD_SHARED_LAMBDA_CAPTURE(fieldType, fieldOffsetVar, fieldSizeVar) {SCHEMA_CLASS_ARRAY_FIELD_LAMBDA_CAPTURE(fieldType, fieldOffsetVar, fieldSizeVar)}

class ISchemaSystem;
class CSchemaClassInfo;
class CSchemaSystemTypeScope;
class CSchemaType_DeclaredClass;
struct SchemaClassFieldData_t;

class CConcatLineString;

namespace Menu
{
	namespace Schema
	{
		class CSystem;
	}; // Menu::Schema
}; // Menu

template<class CLASS>
inline Menu::Schema::CSystem *schema_system_cast(CLASS *pTarget)
{
	return static_cast<Menu::Schema::CSystem *>(pTarget);
}

template<class T, class CLASS>
inline T component_upper_cast(CLASS aComponent)
{
	return static_cast<T>(aComponent);
}

template<class T, class CLASS>
inline T instance_upper_cast(CLASS aInstance)
{
	if constexpr (std::is_pointer_v<std::remove_reference_t<T>>)
	{
		return reinterpret_cast<T>(aInstance); // Replace to dynamic_cast<> when exact hierarchy classes are declared.
	}

	return static_cast<T>(aInstance);
}

namespace Menu
{
	namespace Schema
	{
		class CSystem
		{
		public:
			CSystem();

			using CStringVector = GameData::CStringVector;

			struct DetailsBase_t
			{
				DetailsBase_t() = delete;
				DetailsBase_t(CStringVector *pMessages)
				 :  m_pMessages(pMessages)
				{
				}

				CStringVector *m_pMessages;
			}; // Menu::Schema::CSystem::DetailsBase_t

			template<std::size_t N>
			struct DetailsConcatBase_t : DetailsBase_t
			{
				using Base_t = DetailsBase_t;
				using ConcatArr_t = std::array<const CConcatLineString *, N>;

				DetailsConcatBase_t() = delete;
				DetailsConcatBase_t(CStringVector *pMessages, ConcatArr_t &&arrConcats)
				 :  Base_t(pMessages), 
				    m_arrConcats(std::move(arrConcats))
				{
				}

				static_assert(N != 0, "Template parameter N (number of nests) must not be 0. Use \"DetailsBase_t\" instead");
				static_assert(N <= g_arrEmbedsConcat.size(), "Template parameter N (number of nests) over the limit");
				static constexpr std::size_t sm_nEmbeds = N;

				std::array<const CConcatLineString *, N> m_arrConcats; // From more nested to less.
			}; // Menu::Schema::CSystem::DetailsConcatBase_t<N>

			template<std::size_t N>
			using Details_t = DetailsConcatBase_t<N>;

			static constexpr std::size_t sm_nMaxDetailsNesting = 8;
			static_assert(sm_nMaxDetailsNesting <= g_arrEmbedsConcat.size(), "Number of max details nestings over the limit");

			template<std::size_t N>
			using NestingDetails_t = DetailsConcatBase_t<sm_nMaxDetailsNesting - N>;

			using FullDetails_t = NestingDetails_t<1>;
			using SingleDetails_t = Details_t<1>;

			using TypeScopeDetails_t =      NestingDetails_t<1>;
			using ClassDetails_t =          NestingDetails_t<3>;
			using ClassTypeDetails_t =      NestingDetails_t<4>;
			using BaseClassTypeDetails_t =  NestingDetails_t<4>;
			using FieldDetails_t =          NestingDetails_t<5>;
			using FieldTypeDetails_t =      NestingDetails_t<6>;
			using MetadataDetails_t =       NestingDetails_t<6>;
			using MetadataEntryDetails_t =  NestingDetails_t<7>;

		protected:
			using CDetailsConcatImpl = SingleDetails_t;

			abstract_class IDetailsConcat
			{
			public:
				virtual void AppendHeader() = 0;
				virtual void AppendMembers() = 0;
				virtual void AppendEmpty() = 0;
			}; // Menu::Schema::CSystem::IDetailsConcat

			class CDetailsConcatBase : private CDetailsConcatImpl, public IDetailsConcat
			{
			public:
				using Impl = CDetailsConcatImpl;

				template<class T, uintp N = T::sm_nEmbeds, typename std::enable_if_t<std::is_base_of_v<DetailsConcatBase_t<N>, T>, int> = 0>
				CDetailsConcatBase(const T *pDetails)
				 :  Impl({{pDetails->m_pMessages}, {pDetails->m_arrConcats[N - 1]}}) // Go down to a single concat.
				{
					Assert(pDetails);
				}

			protected:
				CStringVector *GetMessages() { return m_pMessages; }
				const CConcatLineString *GetConcatLine() const { return m_arrConcats[0]; }

			public:
				void AppendHeader() override { AssertMsg(0, "Not implemented"); }
				void AppendMembers() override { AssertMsg(0, "Not implemented"); }
				void AppendEmpty() override;
			}; // Menu::Schema::CSystem::CDetailsConcatBase

			class CDetailsConcatTypeScope : public CDetailsConcatBase
			{
			public:
				using Base = CDetailsConcatBase;
				using Base::Base;

				CDetailsConcatTypeScope(const Base &aBase, const CSchemaSystemTypeScope *pData)
				 :  Base(aBase), 
				    m_pData(pData)
				{
				}

			public:
				void AppendHeader() override;
				void AppendMembers() override;
				virtual void AppendClasses();

			private:
				const CSchemaSystemTypeScope *m_pData;
			}; // Menu::Schema::CSystem::CDetailsConcatTypeScope

			class CDetailsConcatType : public CDetailsConcatBase
			{
			public:
				using Base = CDetailsConcatBase;

				CDetailsConcatType(const Base &aBase, const CSchemaType *pData)
				 :  Base(aBase), 
				    m_pData(pData)
				{
				}

			public:
				void AppendHeader() override;
				void AppendMembers() override;

			private:
				const CSchemaType *m_pData;
			}; // Menu::Schema::CSystem::CDetailsConcatType

			class CDetailsConcatClass : public CDetailsConcatBase
			{
			public:
				using Base = CDetailsConcatBase;

				CDetailsConcatClass(const Base &aBase, const SchemaClassInfoData_t *pData)
				 :  Base(aBase), 
				    m_pData(pData)
				{
				}

			public:
				void AppendHeader() override;
				void AppendMembers() override;
				virtual void AppendBaseClasses();
				virtual void AppendFields();

			private:
				const SchemaClassInfoData_t *m_pData;
			}; // Menu::Schema::CSystem::CDetailsConcatClass

			class CDetailsConcatField : public CDetailsConcatBase
			{
			public:
				using Base = CDetailsConcatBase;

				CDetailsConcatField(const Base &aBase, const SchemaClassFieldData_t *pData)
				 :  Base(aBase), 
				    m_pData(pData)
				{
				}

			public:
				void AppendHeader() override;
				void AppendMembers() override;
				virtual void AppendMetadataMember();

			private:
				const SchemaClassFieldData_t *m_pData;
			}; // Menu::Schema::CSystem::CDetailsConcatField

			class CDetailsConcatMetadataEntry : public CDetailsConcatBase
			{
			public:
				using Base = CDetailsConcatBase;

				CDetailsConcatMetadataEntry(const Base &aBase, const SchemaMetadataEntryData_t *pData)
				 :  Base(aBase), 
				    m_pData(pData)
				{
				}

			public:
				void AppendHeader() override;
				void AppendMembers() override;

			private:
				const SchemaMetadataEntryData_t *m_pData;
			}; // Menu::Schema::CSystem::CDetailsConcatMetadataEntry

		public:
			bool Init(ISchemaSystem *pSchemaSystem, const CUtlVector<const char *> &vecLoadedLibraries, CStringVector *pMessages = nullptr);
			bool Load(FullDetails_t *pDetails = nullptr); // Calls the classes -> fields callbacks.
			void Clear();

		public:
			class CClass
			{
				friend class CSystem;

			public:
				using Fields = GameData::Config::Storage<CUtlSymbolLarge, SchemaClassFieldData_t *>;

				Fields &GetFields()
				{
					return m_aFieldStorage;
				}

				SchemaClassFieldData_t *GetField(const CUtlSymbolLarge &sName) const;
				void SetField(const CUtlSymbolLarge &sName, SchemaClassFieldData_t *pData);

				SchemaClassFieldData_t *FindField(const char *pszName) const;
				int FindFieldOffset(const char *pszName) const; // Returns -1 if not found.

			protected:
				void LoadFields(CSchemaClassInfo *pInfo, FieldDetails_t *pDetails = nullptr);

			public: // Fields symbols.
				CUtlSymbolLarge GetFieldSymbol(const char *pszName);
				CUtlSymbolLarge FindFieldSymbol(const char *pszName) const;

			private:
				Fields m_aFieldStorage;
				CUtlSymbolTableLarge m_tableFileds;
			}; // Menu::Schema::CSystem::CClass

			CClass *GetClass(const char *pszName);
			CClass *FindClass(const char *pszName);
			int FindClassFieldOffset(const char *pszClassName, const char *pszFiledName); // Returns -1 if not found.

		protected:
			void LoadClasses(CSchemaSystemTypeScope *pScope, ClassDetails_t *pDetails = nullptr);
			void ClearClasses();

		public: // Class symbols.
			CUtlSymbolLarge GetClassSymbol(const char *pszName);
			CUtlSymbolLarge FindClassSymbol(const char *pszName) const;

		private:
			CUtlVector<CSchemaSystemTypeScope *> m_vecTypeScopes;

			CUtlSymbolTableLarge m_tableClasses;
			CUtlMap<CUtlSymbolLarge, CClass> m_mapClasses;
		}; // Menu::Schema::CSystem

		namespace Accessor
		{
			template<class T, typename F>
			class CField
			{
			public:
				CField() = delete;
				FORCEINLINE CField(T *pInitTarget, uintp nInitOffset)
				:  m_pTarget(pInitTarget), 
				   m_nOffset(nInitOffset)
				{
				}

				template<class FC>
				const auto &Cast() const
				{
					return *reinterpret_cast<const CField<T, FC> *>(this);
				}

			protected:
				FORCEINLINE T *GetTarget() const
				{
					return m_pTarget;
				}

				FORCEINLINE uintp GetOffset() const
				{
					return m_nOffset;
				}

				FORCEINLINE F *GetPointer(uintp nExtraOffset = 0) const
				{
					return reinterpret_cast<F *>(reinterpret_cast<uintp>(GetTarget()) + GetOffset() + nExtraOffset);
				}

				FORCEINLINE F &GetRef(uintp nExtraOffset = 0) const
				{
					return *GetPointer(nExtraOffset);
				}

			public:
				FORCEINLINE operator F() const
				{
					return GetRef();
				}

				FORCEINLINE F &operator=(const F &aData) const
				{
					return GetRef() = aData;
				}

				FORCEINLINE F *operator->() const
				{
					return GetPointer();
				}

			private:
				T *m_pTarget;
				uintp m_nOffset;
			}; // Menu::Schema::Accessor::CField<T, F>

			template<class T, typename F>
			class CArrayField : public CField<T, F>
			{
			public:
				using Base = CField<T, F>;

				CArrayField() = delete;
				FORCEINLINE CArrayField(const Base &aInitField, uintp nInitSize)
				 :  Base(aInitField), 
				    m_nSize(nInitSize)
				{
				}

				template<class FM>
				FORCEINLINE auto &Cast() const
				{
					return *reinterpret_cast<const CArrayField<T, FM> *>(this);
				}

			public:
				FORCEINLINE operator F*() const
				{
					return Base::GetPointer();
				}

				FORCEINLINE F &operator[](uintp nCell) const
				{
					Assert(nCell < m_nSize);

					return Base::GetRef(nCell * sizeof(F));
				}

				FORCEINLINE uintp GetSize() const
				{
					return m_nSize;
				}

			private:
				uintp m_nSize;
			}; // Menu::Schema::Accessor::CArrayField<T, F>

			template<typename F>
			using CInstanceFieldBase = CField<CEntityInstance, F>;

			template<typename F>
			class CInstanceField : public CInstanceFieldBase<F>
			{
			public:
				using Base = CInstanceFieldBase<F>;
				using Base::Base;
				using Base::operator F;
				using Base::operator=;
				using Base::operator->;

				template<class FC>
				FORCEINLINE auto &Cast() const
				{
					return *reinterpret_cast<const CInstanceField<FC> *>(this);
				}

			public:
				FORCEINLINE void MarkNetworkChanged()
				{
					Base::GetTarget()->NetworkStateChanged(Base::GetOffset());
				}
			}; // Menu::Schema::Accessor::CInstanceField<F>

			template<typename F>
			using CInstanceArrayFieldBase = CArrayField<CEntityInstance, F>;

			template<typename F>
			class CInstanceArrayField : public CInstanceArrayFieldBase<F>
			{
			public:
				using Base = CInstanceArrayFieldBase<F>;
				using Base::Base;
				using Base::operator F;
				using Base::operator=;
				using Base::operator->;
				using Base::operator[];

				template<class FC>
				FORCEINLINE auto &Cast() const
				{
					return *reinterpret_cast<const CInstanceArrayField<FC> *>(this);
				}

			public:
				FORCEINLINE void MarkNetworkChanged()
				{
					Base::GetTarget()->NetworkStateChanged(Base::GetOffset());
				}
			}; // Menu::Schema::Accessor::CInstanceArrayField<F>
		}; // Menu::Schema::Accessor
	}; // Menu::Schema
}; // Menu

#endif // _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_HPP_
