# 新しいテキストスタイルとカラーの追加方法

このドキュメントは、メニューシステムに新しいテキストスタイル（例：DisabledActiveText）と、それに対応するカラー（例：DisabledActiveColor）を追加する手順を説明します。

## 1. エンティティインデックスの追加

まず、新しいテキストレイヤーを描画するためのエンティティインデックスを定義します。

- **対象ファイル**: `public/imenuinstance.hpp`
- **修正箇所**: `MenuEntity_t` enum

`MENU_MAX_ENTITIES` の直前に、新しいインデックスを追加します。

```cpp
// public/imenuinstance.hpp

enum MenuEntity_t : uint8
{
	MENU_ENTITY_BACKGROUND_INDEX = 0,
	MENU_ENTITY_INACTIVE_INDEX = 1,
	MENU_ENTITY_ACTIVE_INDEX = 2,
	// --- ここから追加 ---
	MENU_ENTITY_NEW_STYLE_INDEX = 3, // 例：新しいスタイルのインデックス
	// --- ここまで追加 ---

	MENU_MAX_ENTITIES,
};
```

## 2. プロファイルへの色定義の追加

次に、新しい色をメニュープロファイルで管理できるようにします。

### 2.1. インターフェースの更新

- **対象ファイル**: `public/imenuprofile.hpp`
- **修正箇所**: `IMenuProfile` class

新しい色を取得するための純粋仮想関数を追加します。

```cpp
// public/imenuprofile.hpp

class IMenuProfile
{
public:
    // ... 既存のゲッター ...
	virtual const Color *GetActiveColor() const = 0;

	// --- ここから追加 ---
	/**
	 * @brief Gets the new style color.
	 * 
	 * @return Returns a pointer to new style color.
	 */
	virtual const Color *GetNewStyleColor() const = 0; // 例：新しい色のゲッター
	// --- ここまで追加 ---
    // ...
};
```

### 2.2. プロファイル実装の更新 (ヘッダー)

- **対象ファイル**: `include/menu/profile.hpp`
- **修正箇所**: `MenuProfile_t` struct, `CProfile` class

`MenuProfile_t` に色情報を保持するポインタを追加し、`CProfile` にゲッターのオーバーライド宣言を追加します。

```cpp
// include/menu/profile.hpp

struct MenuProfile_t
{
    // ...
    Color *m_pActiveColor = nullptr; // "active_color"
    // --- ここから追加 ---
    Color *m_pNewStyleColor = nullptr; // "new_style_color"
    // --- ここまで追加 ---
    // ...
};

class CProfile : public IMenuProfile, public MenuProfile_t
{
public:
    // ...
    const Color *GetActiveColor() const override;
    // --- ここから追加 ---
    const Color *GetNewStyleColor() const override;
    // --- ここまで追加 ---
    // ...
};
```

### 2.3. プロファイル実装の更新 (ソース)

- **対象ファイル**: `src/menu/profile.cpp`
- **修正箇所**: デストラクタ、`Load`, `RemoveStaticMembers`, 新しいゲッターの実装

```cpp
// src/menu/profile.cpp

// デストラクタにメモリ解放処理を追加
Menu::CProfile::~CProfile()
{
    // ...
	if(m_pActiveColor)
	{
		delete m_pActiveColor;
	}
    // --- ここから追加 ---
	if(m_pNewStyleColor)
	{
		delete m_pNewStyleColor;
	}
    // --- ここまで追加 ---
    // ...
}

// Load関数に設定ファイルからの読み込み処理を追加
bool Menu::CProfile::Load(...)
{
    // ...
	m_pActiveColor = (pMember = pData->FindMember("active_color")) ? new Color(pMember->GetColor()) : nullptr;
    // --- ここから追加 ---
	m_pNewStyleColor = (pMember = pData->FindMember("new_style_color")) ? new Color(pMember->GetColor()) : nullptr;
    // --- ここまで追加 ---
    // ...
}

// RemoveStaticMembers関数にメンバ削除処理を追加
void Menu::CProfile::RemoveStaticMembers(KeyValues3 *pData)
{
    // ...
	pData->RemoveMember("active_color");
    // --- ここから追加 ---
	pData->RemoveMember("new_style_color");
    // --- ここまで追加 ---
    // ...
}

// --- ファイルの末尾などにゲッターの実装を追加 ---
const Color *Menu::CProfile::GetNewStyleColor() const
{
	const auto *pResult = m_pNewStyleColor;

	if(!pResult)
	{
		for(const auto &pInherited : m_aMetadata.GetBaseline()) 
		{
			if(pResult = pInherited->GetNewStyleColor()) 
			{
				break;
			}
		}
	}

	return pResult;
}
```

### 2.4. 設定ファイルの更新

- **対象ファイル**: `configs/menu/system/profiles.json` (または任意のプロファイル設定ファイル)

プロファイル定義に新しい色の設定を追加します。

```json
{
    "hudmenu_annotation_style":
	{
        "inactive_color": "233 208 173 255",
		"active_color": "255 167 42 255",
        "disabled_active_color": "128 128 128 255",
        // --- ここから追加 ---
        "new_style_color": "100 100 255 255"
        // --- ここまで追加 ---
    }
}
```

## 3. 描画レイヤーとロジックの更新

メニューのページに新しいテキストレイヤーを追加し、描画ロジックを更新します。

### 3.1. ページクラスの更新

- **対象ファイル**: `include/menu.hpp`
- **修正箇所**: `IPage` interface, `CPage` class

`IPage` にゲッターの純粋仮想関数を追加し、`CPage` にテキストバッファとゲッターの実装、`Clear` 関数の更新を行います。

```cpp
// include/menu.hpp

class IPage
{
public:
    // ...
    virtual const char *GetActiveText() const = 0;
    // --- ここから追加 ---
    virtual const char *GetNewStyleText() const = 0;
    // --- ここまで追加 ---
    // ...
};

class CPage : public CPageBase
{
public:
    // ...
    virtual const char *GetActiveText() const override { /* ... */ }
    // --- ここから追加 ---
    const char *GetNewStyleText() const override
    {
        return m_sNewStyleText.Get();
    }
    // --- ここまで追加 ---

    void Clear() override
    {
        Base::Clear();
        m_sInactiveText.Clear();
        m_sActiveText.Clear();
        // --- ここから追加 ---
        m_sNewStyleText.Clear();
        // --- ここまで追加 ---
    }
    // ...
private:
    CBufferStringText m_sInactiveText;
    CBufferStringText m_sActiveText;
    // --- ここから追加 ---
    CBufferStringText m_sNewStyleText;
    // --- ここまで追加 ---
};
```

### 3.2. 描画ロジックの更新

- **対象ファイル**: `src/menu.cpp`
- **修正箇所**: `CPage::Render`

アイテムのスタイルに応じて、新しく追加したテキストバッファに文字列を書き込むロジックを追加します。

```cpp
// src/menu.cpp

void CMenu::CPage::Render(...)
{
    // ...
    // タイトル部分の処理
    if(!aTitleText.IsEmpty())
    {
        // ...
        aConcat.AppendEndsAndStartsToBuffer(m_sActiveText);
        // --- ここから追加 ---
        aConcat.AppendEndsAndStartsToBuffer(m_sNewStyleText);
        // --- ここまで追加 ---
    }

    // アイテム部分のループ
    for(ItemPosition_t i = iStartPosition; i < nItemsOnPage; i++)
    {
        // ...
        if(eItemStyle & MENU_ITEM_ACTIVE)
        {
            // ...
        }
        else
        {
            // ...
            aConcat.AppendToBuffer(m_sDisabledActiveText, pszItemContent);
            // --- ここで新しいスタイルに対する分岐を追加 ---
            // aConcat.AppendToBuffer(m_sNewStyleText, pszItemContent);
        }
    }
    // ...
}
```

## 4. エンティティ生成処理の更新

最後に、新しいテキストレイヤーを実際にエンティティとして生成するための処理を追加します。

### 4.1. キーバリュー生成関数の追加

- **対象ファイル**: `include/menu.hpp`
- **修正箇所**: `CMenu` class

新しいキーバリューを生成する関数の宣言を追加します。

```cpp
// include/menu.hpp

class CMenu : public IMenu, public CMenuBase
{
    // ...
	CEntityKeyValues *GetAllocatedActiveKeyValues(...);
	CEntityKeyValues *GetAllocatedDisabledActiveKeyValues(...);
    // --- ここから追加 ---
	CEntityKeyValues *GetAllocatedNewStyleKeyValues(CPlayerSlot aSlot, CKeyValues3Context *pAllocator = nullptr, bool bDrawBackground = true);
    // --- ここまで追加 ---
    // ...
};
```

### 4.2. キーバリュー生成ロジックの更新

- **対象ファイル**: `src/menu.cpp`
- **修正箇所**: `GenerateKeyValues`, 新しい `GetAllocated...` 関数の実装

`GenerateKeyValues` で新しい関数を呼び出し、その関数の実装を追加します。

```cpp
// src/menu.cpp

// GenerateKeyValuesに関数の呼び出しを追加
CUtlVector<CEntityKeyValues *> CMenu::GenerateKeyValues(...)
{
    // ...
	vecResult[MENU_ENTITY_ACTIVE_INDEX] = GetAllocatedActiveKeyValues(aSlot, pAllocator, bIncludeBackground);
	vecResult[MENU_ENTITY_DISABLED_ACTIVE_INDEX] = GetAllocatedDisabledActiveKeyValues(aSlot, pAllocator, bIncludeBackground);
    // --- ここから追加 ---
	vecResult[MENU_ENTITY_NEW_STYLE_INDEX] = GetAllocatedNewStyleKeyValues(aSlot, pAllocator, bIncludeBackground);
    // --- ここまで追加 ---
	return vecResult;
}

// --- ファイルの末尾などに新しい関数の実装を追加 ---
CEntityKeyValues *CMenu::GetAllocatedNewStyleKeyValues(CPlayerSlot aSlot, CKeyValues3Context *pAllocator, bool bDrawBackground)
{
	const IMenuProfile *pProfile = m_pProfile;
	Assert(pProfile);
	CEntityKeyValues *pMenuKV = pProfile->GetAllocactedEntityKeyValues(pAllocator);
	if(pMenuKV)
	{
		const Color *pColor = pProfile->GetNewStyleColor(); // 新しいゲッターを呼び出す
		if(pColor)
		{
			pMenuKV->SetColor("color", *pColor);
		}
		if(bDrawBackground)
		{
			pMenuKV->SetString("background_material_name", MENU_EMPTY_BACKGROUND_MATERIAL_NAME);
		}
		pMenuKV->SetString("message", GetCurrentPage(aSlot)->GetNewStyleText()); // 新しいテキストを取得
	}
	return pMenuKV;
}
```

### 4.3. エンティティスポーン処理の更新

- **対象ファイル**: `src/menusystem_plugin.cpp`
- **修正箇所**: `SpawnMenu`

新しいエンティティのキーバリューに、位置や角度などの必須情報を設定します。

```cpp
// src/menusystem_plugin.cpp

void MenuSystem_Plugin::SpawnMenu(...)
{
    // ...
	{
		SetMenuKeyValues(vecMenuKVs[MENU_ENTITY_BACKGROUND_INDEX], vecBackgroundOrigin, angRotation);
		SetMenuKeyValues(vecMenuKVs[MENU_ENTITY_INACTIVE_INDEX], vecOrigin, angRotation);
		SetMenuKeyValues(vecMenuKVs[MENU_ENTITY_ACTIVE_INDEX], vecOrigin, angRotation);
		SetMenuKeyValues(vecMenuKVs[MENU_ENTITY_DISABLED_ACTIVE_INDEX], vecOrigin, angRotation);
        // --- ここから追加 ---
		SetMenuKeyValues(vecMenuKVs[MENU_ENTITY_NEW_STYLE_INDEX], vecOrigin, angRotation);
        // --- ここまで追加 ---
	}
    // ...
}
```

### 4.4. 表示更新処理の更新

- **対象ファイル**: `src/menu.cpp`
- **修正箇所**: `CMenu::InternalDisplayAt`

新しいテキストレイヤーをエンティティにセットする処理を追加します。

```cpp
// src/menu.cpp
bool CMenu::InternalDisplayAt(...)
{
    // ...
	if(eFlags & MENU_DISPLAY_UPDATE_TEXT_NOW)
	{
		InternalSetMessage(MENU_ENTITY_BACKGROUND_INDEX, pPage->GetText());
		InternalSetMessage(MENU_ENTITY_INACTIVE_INDEX, pPage->GetInactiveText());
		InternalSetMessage(MENU_ENTITY_ACTIVE_INDEX, pPage->GetActiveText());
		InternalSetMessage(MENU_ENTITY_DISABLED_ACTIVE_INDEX, pPage->GetDisabledActiveText());
        // --- ここから追加 ---
		InternalSetMessage(MENU_ENTITY_NEW_STYLE_INDEX, pPage->GetNewStyleText());
        // --- ここまで追加 ---
	}
    // ...
}
```

以上の手順で、新しいテキストスタイルをシステムに追加できます。