# MQ2 Plugin Development 

MQ2 Plugins by design inherit any and all functionality used in the MQ2 main DLL (MQ2Main.dll).  This means developing plugins and developing the MQ2Main API are 99% equal, and if one can develop for one they can definitely develop for the other.

MQ2 Plugins provide added functionality to MQ2.  The default set of functionality is more or less the core "macro" commands and other general-use commands, parameters, and required API to make the entire system work.  Everything else belongs in plugins.  Because we don't want a new plugin for every command, most general-use commands will be implemented into the main DLL rather than plugins.  Commands specific to plugins with other functionality will of course go in the plugin itself.

## Creating a new plugin

**Before selecting an option, read the paragraph under "The functionses, my precious" that says TAKE OUT THE ONES YOUR PLUGIN DOES NOT USE at the end.**

> OPTION 1: MQ2Template is an empty plugin, and will always contain the newest possible plugin template.  The absolute first thing to do is to copy the contents of the MQ2Template directory, except for the readme.txt, to a new directory.  For example purposes, we will use MQ2MyPlugin, but you would of course insert your own plugin name.  Next, open each file in notepad or another text editor and replace every instance of MQ2Template with MQ2MyPlugin.  Also, rename each file from MQ2Template.* to MQ2MyPlugin.*.  After doing so, your plugin will be fully functional as a MQ2 plugin, and will compile to the Release directory. 

> OPTION 2: "mkplugin" has been created to automate the creation of one or more plugins at once, automating the process given in option one.  From the command line, "mkplugin <name>" will create a plugin. The plugin will automatically be prefixed with "MQ2" and the next letter capitalized (though the rest is untouched). For example, "**mkplugin myPlugin**" and the name given to the plugin will be "MQ2MyPlugin".  If you do "**mkplugin mq2myplugin**" the name given will be "MQ2Myplugin".  To create multiple plugins at once, just add a parameter to the command line for every plugin.  Example: "**mkplugin one two three four**" will create "MQ2One", "MQ2Two", "MQ2Three" and "MQ2Four".  mkplugin.exe is found in the Macroquest2 workspace/solution root directory.  If you aren't familiar with DOS, simply do this: Start->Run "**c:\mq2\mkplugin.exe one two three four**",  but replace c:\mq2 with the actual path to your MQ2.

Now you can open your project in visual studio or open the main file with whatever you happen to code with.  Immediately you will see several very concise functions, most of which simply DebugSpew.  Make sure to not muck with `PreSetup("MQ2MyPlugin");`.  It is where it is supposed to be, and sets up your plugin's DllMain and sets the INI filename and path.

### The functionses, my precious 

The MQ2 Plugin system is based on **callback** functions.  This means that MQ2 is going to call certain functions in your plugin, if they exist.  None of the callbacks are absolutely mandatory, but you will at least want InitializePlugin and ShutdownPlugin. ***TAKE OUT THE ONES YOUR PLUGIN DOES NOT USE.***

```c++
PLUGIN_API VOID InitializePlugin(VOID)
```

This function is called immediately after your plugin is loaded by the system.  Add any commands, aliases, detours, macro params here.  It is not safe to assume that EQ is "in-game" when this is called.

```c++
PLUGIN_API VOID ShutdownPlugin(VOID) 
```
This function is called immediately before your plugin is unloaded by the system.  Remove any commands, aliases, detours, macro params here.  It is not safe to assume that EQ is "in-game" when this is called. `OnCleanUI` is explicitly called immediately before `ShutdownPlugin()`, so it is not necessary to do anything found in `OnCleanUI` here.

```c++
PLUGIN_API VOID OnZoned(VOID)
```
This function is called immediately after the EQ client finishes entering a new zone.  It is safe to assume that EQ is "in-game" when this is called.

```c++
PLUGIN_API VOID OnCleanUI(VOID)
```
This function is called when EQ cleans its game ui.  This could be from in game or at character select, and I believe one other point.  If you need to clean up, make sure to check your data for existence first.

```c++
PLUGIN_API VOID SetGameState(DWORD GameState)
```
This function is called when EQ changes game state.  If you do not need to know when these changes occur, you do not need this function.  You can access MQ2's game state value with the `gGameState` global variable.

```c++
PLUGIN_API VOID OnPulse(VOID)
```
This function is called immediately after the MQ2Main Pulse function, many many times per second.  If you need continuous processing in your plugin, this is what you're looking for.

```c++
PLUGIN_API DWORD OnWriteChatColor(PCHAR Line, DWORD Color, DWORD Filter)
```
This function is called every time `WriteChatColor` is called by MQ2Main or any plugin, ignoring the filters built into MQ2Main.  If you need to filter this data, you need to implement the filters yourself.  Note the Filter parameter.  This is for future use, it will currently always be 0.  It's intended to allow chat plugins to redirect MQ output, etc, regardless of EQ's filters.  Eventually, MQ2 calls to `WriteChatColor` will use a dynamic filter system.  For now, ignore it.

```c++
PLUGIN_API DWORD OnIncomingChat(PCHAR Line, DWORD Color)
```
This function is called every time EQ shows a line of chat with `CEverQuest::dsp_chat`, but after MQ filters and chat events are taken care of. 

### Using your plugin's INI file

Plugin INI files are not created until used, so even though the INI file name is always defined for your plugin, it will not clutter up the directory if you do not use it.  The `INIFileName` variable is to be used as your plugin's INI filename.  Don't modify it, it's set for you to **MQ2MyPlugin.ini** in the correct directory.

### Adding Commands

Adding commands is incredibly simple with MQ2.  Make your command function with the standard prototype

```c++
VOID MyCommand(PSPAWNINFO pChar, PCHAR szLine)
```
Then, in `InitializePlugin`:
```c++
AddCommand("/mycommand" ,MyCommand);
```

and in `ShutdownPlugin`:
```c++
RemoveCommand("/mycommand");
```
Follow the pre-existing rules for coding your command function.

### Adding Macro Parameters 
*outdated!*

Adding macro parameters is also incredibly simple.  Make your param function with the standard prototype
```c++
DWORD parmMyParm(PCHAR szVar, PCHAR szOutput, PSPAWNINFO pChar)
```
Then, in `InitializePlugin`:
```c++
AddParm("$myparm",parmMyParm);
```

and in `ShutdownPlugin`:
```c++
RemoveParm("$myparm");
```
Follow the pre-existing rules for coding your parameter function.

### Adding Detours

Adding detours is almost equally simple.. but I'm going back to sleep very soon and don't want to show how to do this at the moment.  Take a look at the existing plugins for examples.
