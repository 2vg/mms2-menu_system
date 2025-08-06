このC++メニュープラグインを、C#からコールできるようにしてください
以下C++をC#プラグインからコールする例です

```
// C++ header
#pragma once

#define MULTIADDONMANAGER_INTERFACE "MultiAddonManager003"
class IMultiAddonManager
{
public:
	// These add/remove to the internal list without reloading anything
	// pszWorkshopID is the workshop ID in string form (e.g. "3157463861")
	virtual bool AddAddon(const char *pszWorkshopID, bool bRefresh = false) = 0;
	virtual bool RemoveAddon(const char *pszWorkshopID,  bool bRefresh = false) = 0;
	
	// Returns true if the given addon is mounted in the filesystem. 
	// Pass 'true' to bCheckWorkshopMap to check from the server mounted workshop map as well.
	virtual bool IsAddonMounted(const char *pszWorkshopID, bool bCheckWorkshopMap = false) = 0;

	// Start an addon download of the given workshop ID
	// Returns true if the download successfully started or the addon already exists, and false otherwise
	// bImportant: If set, the map will be reloaded once the download finishes 
	// bForce: If set, will start the download even if the addon already exists
	virtual bool DownloadAddon(const char *pszWorkshopID, bool bImportant = false, bool bForce = true) = 0;

	// Refresh addons, applying any changes from add/remove
	// This will trigger a map reload once all addons are updated and mounted
	virtual void RefreshAddons(bool bReloadMap = false) = 0;

	// Clear the internal list and unmount all addons excluding the current workshop map
	virtual void ClearAddons() = 0;
	
	// Check whether the server is connected to the game coordinator, and therefore is capable of downloading addons.
	// Should be called before calling DownloadAddon.
	virtual bool HasUGCConnection() = 0;
	
	// Functions to manage addons to be loaded only by a client. 
	// Pass a steamID value of 0 to perform the operation on a global list instead, and bRefresh to 'true' to trigger a reconnect if necessary.
	virtual void AddClientAddon(const char *pszAddon, uint64 steamID64 = 0, bool bRefresh = false) = 0;
	virtual void RemoveClientAddon(const char *pszAddon, uint64 steamID64 = 0) = 0;
	virtual void ClearClientAddons(uint64 steamID64 = 0) = 0;
};

// C#
RegisterListener<Listeners.OnMetamodAllPluginsLoaded>(() => {
    nint? pMultiAddonManager = Utilities.MetaFactory("MultiAddonManager002");
    if (!pMultiAddonManager.HasValue) {
        Console.WriteLine("Multiaddonmanager not installed!");
        return;
    }
    var RefreshAddons = VirtualFunction.CreateVoid<IntPtr>(pMultiAddonManager.Value, 4);
    RefreshAddons.Invoke(pMultiAddonManager.Value);
});
```

ひな形はこれを使ってください

```
using CounterStrikeSharp.API;
using CounterStrikeSharp.API.Core;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using CounterStrikeSharp.API.Modules.Memory;
using Microsoft.Extensions.Logging;
using MenuSystemSharp.API;

namespace MenuSystemSharp;

public interface IWrapperWithInstancePtr { IntPtr InstancePtr { get; } }

public class MenuSystemCSharp : BasePlugin
{
    private const string MENU_SYSTEM_VERSION = "Menu System v1.0.0";
    private const string MENU_LIBRARY_PATH = "/csgo/addons/menu_system/bin/menu";
    internal static string StaticModuleName => "MenuSystemCSharp";
    public override string ModuleName => StaticModuleName;
    public override string ModuleVersion => "1.0.0";
    public override string ModuleAuthor => "uru";
    public override string ModuleDescription => "C# implementation for Wend4r's MetaMod Menu System";

    // when plugin load
    public override void Load(bool hotReload)
    {
        Instance = this;
        
        RegisterListener<Listeners.OnMetamodAllPluginsLoaded>(() =>
        {
            // ...
        });
    }

    // when plugin unload
    public override void Unload(bool hotReload)
    {
    }
}

// if call native export
//string menuPath = $"{Server.GameDirectory}{MENU_LIBRARY_PATH}";
//if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows)) menuPath += ".dll";
//else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux)) menuPath += ".so";
```

また、外部プラグインが利用するためにAPIインターフェースも実装してください。
以下例

```
using CounterStrikeSharp.API.Core;

namespace MenuSystemSharp.API;

// ...

/// <summary>
/// Menu item style flags
/// </summary>
[Flags]
public enum MenuItemStyleFlags : byte
{
    // ...
}
```

最低限必要なのは、

- メニューの作成(タイトルの編集含む)、表示、クローズ
- メニューアイテムの作成、セット
- 現在アクティブなメニューのインスタンス？オブジェクト？を取得する

です、後は無くても構いません

以下注意点:
- なるべく最初に示したMetaFactory経由での関数利用をしたいが、Exportを呼んでもよい
  - `menusystem_exports.cpp` にExport用の情報が入ってます
  - もしexportのコール時で難しい状況があれば、C++側に簡単なExportを増やしたりして構いません
    - これは、現在のExport(menusystem_exports.cpp)やMetaFactory経由で難しい場合にC++側に変更をしたり、それをC#経由で利用する、ということです
      - 読み込むときは`NativeLibrary`などを活用できるでしょう
- APIインターフェースは、外部からはそれしか参照しません。MenuSystemSharp自体はそれ単体で動作する見込みです。
  - 外部プラグインはインターフェースのみを依存関係に持つ
  - ```
    External Plugin
        ↓ (references)
    MenuSystemSharp.API (NuGet Package)
        ↑ (implementation registration)
    MenuSystemSharp (Plugin)
        ↓ (native library calls)
    Wend4r's MetaMod Menu System
    ```
- セグフォが起きないように注意して実装すること
- 不完全な実装や未実装などがないようにしてください

それぞれ、menusystem.csとmenusystem_api.csとして出力してほしい

他に必要な情報があればきいてください
