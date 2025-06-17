## Menu System
> This is a unifying menu system for interacting with the player displayed control panel for your plugin

### Basic menu 
* Normal menu to interactive press on items
-----
<p align="center">
  <img height="650px" src="https://github.com/Wend4r/mms2-menu_system/blob/main/.github/resources/preview-profile-hudmenu_annotation_style.png">
  <img height="420px" src="https://github.com/Wend4r/mms2-menu_system/blob/main/.github/resources/preview-profile-hudmenu_annotation_style2.png">
</p>

### Custom profiles
* You can customize the menu to any style you like
-----
<p align="center">
  <img height="650px"src="https://github.com/Wend4r/mms2-menu_system/blob/main/.github/resources/preview-profile-hudmenu_annotation_style-custom.png">
  <img height="650px"src="https://github.com/Wend4r/mms2-menu_system/blob/main/.github/resources/preview-profile-hudmenu_annotation_style-custom2.png">
</p>

* You can see the API system in `public` folder

## Requirements (included)

* [Source SDK](https://github.com/Wend4r/sourcesdk) - Valve policy with edits from the community. See your game license
* * [Game Protobufs](https://github.com/SteamDatabase/Protobufs) - public domain
* * [Protocol Buffers](https://github.com/protocolbuffers/protobuf) - Google Inc.
* [Metamod:Source](https://github.com/alliedmodders/metamod-source) (runtime) - zLib/libpng, provided "as-is"

* [Entity Manager](https://github.com/Wend4r/mms2-entity_manager) (runtime, not lower v1.0.3) - GPL-3.0
* [Any Config](https://github.com/Wend4r/s2u-any_config) - GPL-3.0
* [GameData](https://github.com/Wend4r/s2u-gamedata) - GPL-3.0
* [Logger](https://github.com/Wend4r/s2u-logger) - GPL-3.0
* [DynLibUtils](https://github.com/Wend4r/cpp-memory_utils) - MIT
* [Translations](https://github.com/Wend4r/s2u-translations) - GPL-3.0

## How to build

### 1. Install dependencies

#### Windows
> [!NOTE]
> Setting up [CMake tools with Visual Studio Installer](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio#installation)

#### Linux (Ubuntu/Debian)
```sh
sudo apt-get install cmake ninja-build
```

### 2. Clone the repository
```
git clone https://github.com/Wend4r/mms2-menu_system.git "Menu System"
cd "Menu System"
git submodule update --init --depth 1
git submodule update --init --recursive external/sourcesdk
```

### 3. Intialize the environment

#### Windows
> [!IMPORTANT]
> To configure your system for development, you need to add the following path to the `Path` environment variable: `%PROGRAMFILES%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build`

```bat
REM C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat
vcvarsall x64
```

### 4. Configure
```
cmake --preset Debug
```

### 4.1. Build (hot)
```
cmake --build --preset Debug --parallel
```

* Once the plugin is compiled the files would be packaged and placed in ``build/{OS}/{PRESET}`` folder.
* Be aware that plugins get loaded either by corresponding ``.vdf`` files in the metamod folder, or by listing them in ``addons/metamod/metaplugins.ini`` file.
* Copy the following folders into the plugin folder: `configs`, `gamedata` and `translations`.

### 5. Extra

#### To prevent console spam

Add these to `cleanercs2`

```cfg
.*OnMenuStart.*
.*OnMenuSelect.*
.*OnMenuEnd.*
.*OnMenuDestroy.*
.*OnDispatchConCommandHook.*
.*OnMenuDrawTitle.*
.*OnMenuDisplayItem.*
.*point_worldtext.*
.*Menu entities position.*
.*Origin:.*
.*Rotation.*
.*predicted_viewmodel.*
.*Handle a chat command:.*
.*Player slot:.*
.*Is silent: .*
.*Arguments:.*
.*CNETMsg_SetConVar.*
.*sv_disable_radar.*
.*select item.*
```
