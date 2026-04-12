# Random Items
This repo is a fork of [RandomItems](https://github.com/bennett-sh/hm-random-items) by bennett-sh. All credits go to him.

A Hitman 3 SDK mod that allows you to spawn a random item every 15 seconds by default, or whatever delay you choose.




## Installation Instructions

1. Download the latest version of [ZHMModSDK](https://github.com/OrfeasZ/ZHMModSDK) and install it.
2. Download the latest release of this repo from the right-side panel on this page.
3. Copy `randomitems.dll` to the ZHMModSDK `mods` folder (e.g. `C:\Games\HITMAN 3\Retail\mods`).
4. Run the game and once in the main menu, press the `~` key (`^` on QWERTZ layouts) and enable `RandomItems` from the menu at the top of the screen (you may need to restart your game afterwards).
5. If you change the item categories while the mod is running, click `Rebuild Item Pool` or stop and start the item spawner.
6. Enjoy!



### Experimental Configuration
Some repository items do not have a `Title` property. By default, the mod excludes them because they usually are not spawnable and can cause longer gaps between spawned items. To include them, tick the checkbox and, if the mod is already running, click `Rebuild Item Pool`.

## Building

### 1. Clone this repository locally with all submodules.

You can either use `git clone --recurse-submodules` or run `git submodule update --init --recursive` after cloning.

### 2. Install Visual Studio (any edition).

Make sure you install the C++ and game development workloads.

### 3. Open the project in your IDE of choice.

See instructions for [Visual Studio](https://github.com/OrfeasZ/ZHMModSDK/wiki/Setting-up-Visual-Studio-for-development) or [CLion](https://github.com/OrfeasZ/ZHMModSDK/wiki/Setting-up-CLion-for-development).
