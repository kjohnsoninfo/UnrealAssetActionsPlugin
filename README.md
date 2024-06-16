# Quick Asset Actions - Unreal Engine 5 Plugin

The Quick Asset Actions plugin streamlines the process of finding and performing different actions on assets in the content browser. The plugin displays a list of assets under a user-selected folder and provides several options for the user to filter the displayed assets on search critera such as unused assets, assets with duplicate names, and more. The plugin also provides users the ability to perform common actions on one or more assets selected from the list such as delete, duplicate, rename, etc. In addition to the common actions included in Unreal Engine, the plugin features extended actions that allow users to replace an unwanted string in a list of asset names and automatically add prefixes according to standard naming conventions.

## Table of Contents
- [Overview](#overview)
  * [Features](#features)
  * [Installing the plugin](#installing-the-plugin)
  * [Opening the plugin](#opening-the-plugin)
  * [Plugin Defaults](#plugin-defaults)
- [Navigating Content Browser to Asset](#navigating-content-browser-to-asset)
- [Sorting the List View](#sorting-the-list-view)
- [Filtering the List View](#filtering-the-list-view)
  * [List All Assets](#list-all-assets)
  * [List Unused Assets](#list-unused-assets)
  * [List Duplicate Name Assets](#list-duplicate-name-assets)
  * [List Assets with No Prefix (or Incorrect Prefix)](#list-assets-with-no-prefix-or-incorrect-prefix)
- [Selecting Assets in the List View](#selecting-assets-in-the-list-view)
  * [Select / Deselect](#select--deselect)
  * [Select All](#select-all)
  * [Deselect All](#deselect-all)
- [Asset Actions](#asset-actions)
  * [Rename (Single Asset Action only)](#rename-single-asset-action-only)
  * [Add Prefixes for Selected](#add-prefixes-for-selected)
  * [Delete Selected](#delete-selected)
  * [Duplicate Selected](#duplicate-selected)
  * [Replace String for Selected](#replace-string-for-selected)

## Overview

Quick Asset Actions Demo

![demo.gif](Images/AssetActionsDemo.gif)

Overview of the UI

![UIoverview.png](Images/UIoverview.png)

### Features

1. List all assets under a selected folder with the following details:
   * Asset Type (Asset Class)
   * Asset Name
   * Asset Parent Folder (Path)
   * \# of Refs (Number of asset referencers)
2. Displays number of assets in the current list view (asset count)
3. Displays selected folder path
4. Doubleclick row to navigate to asset in content browser
5. Filter list of assets by: 
   * Unused Assets
   * Duplicated Name Assets (Assets with exactly the same name)
   * Assets with No Prefix (or incorrect prefixes)
6. Allows user to select assets by checking or unchecking checkboxes
   * Select all
   * Deselect all
7. Rename a single asset
8. Add Prefixes for selected assets
9. Delete selected assets
10. Duplicate selected assets
11. Replace a string or phrase for selected assets

### Installing the plugin

NOTE: This plugin is a editor only plugin and is intended to be used with the Unreal Editor. It was built in Unreal Engine 5.3.2 and is compatible with Unreal Engine 5.3.

The repo should contain the following files and folders:
1. Config/
2. Resources/
3. Source/
4. AssetActionsManager.uplugin

In your Unreal Engine Project directory, add a folder `Plugins` if there is not already one there. In the Plugins folder, add another folder for the Quick Asset Actions Plugin (mine is named AssetActionsManager but it can be whatever you choose). Clone or save the repo in the folder for the Quick Asset Actions Plugin. It should look something like this:


Next, open your games's `.sln` file and rebuild the game. This will generate the necessary Binaries, Config, and Content folders.


You should be able to open your game in Unreal Editor now. You can do this by opening your `.uproject` file. Check that the plugin is activated by going to the Edit menu -> Plugins. Under PROJECT, there should be a new category called “Asset Actions”. Select it and make sure that Quick Asset Actions is checked.

### Opening the plugin

The plugin adds a new menu entry to the right-click menu in the content browser. Simply right-click on a folder and click on Quick Asset Actions. This will open the plugin tab that lists all assets under the selected folder. 

You can click on this menu option when the tab is currently open and it will refresh the whole tab. If you click on a different folder, the tab will refresh with the new selected folder. Basically, you don’t have to close the tab to select a different folder. However, you can only have one tab open at a time. While technically you are able to run the plugin on the main “Content” folder which will include everything, this makes the plugin very slow. **It is recommended to use the plugin on smaller folders.** 

The plugin will search through all subfolders within a folder and display all assets found so it is not needed to open the plugin on every subfolder if you want those assets.

### Plugin Defaults
The plugin will open by default on:
* List All Assets
* Sort by Asset Name A → Z

These default settings will apply any time the Quick Asset Actions menu button is clicked. Clicking the manual refresh button on the right side of the window will **not** apply these default settings.

## Navigating Content Browser to Asset

Double click the row for an asset to navigate to the asset in the content browser:

## Sorting the List View

You are able to sort by:
1. Ascending - A → Z for alphanumeric, least → most for only numeric
2. Descending - Z → A for alphanumeric, most → least for only numeric

For the following columns:
1. CheckBox
   * Ascending - Checked assets first which are subsorted by Asset Name A → Z, then Unchecked assets which are also subsorted by Asset Name A → Z
   * Descending - Unchecked assets first which are subsorted by Asset Name A → Z, then Checked assets which are also subsorted by Asset Name A → Z
2. Asset Type
3. Asset Name
4. Asset Parent Folder
5. \# of Refs

## Filtering the List View

### List All Assets

This is the default option and will display all assets found by the tool in the selected folder and its subfolders.

### List Unused Assets

Unreal keeps track of asset references which you can see when you try to delete an asset that has a reference.

The tool checks the number of the references for each asset in the selected folder and only displays the assets that have 0 references. NOTE: The tool is checking asset referencers only.

### List Duplicate Name Assets
### List Assets with No Prefix (or Incorrect Prefix)
## Selecting Assets in the List View
### Select / Deselect
### Select All
### Deselect All
## Asset Actions
### Rename (Single Asset Action only)
### Add Prefixes for Selected
### Delete Selected
### Duplicate Selected
### Replace String for Selected

