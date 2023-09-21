# Map Release Tool for Digital Paint: Paintball2

This packs up a map file with all its dependencies so it can be released as a beta map on the forums. It also checks for common errors in the map and its dependencies.
You can download the most current version [here](https://github.com/richardebeling/PB2-Map-Release-Tool/releases/latest)

Simply drag-and-drop your mapfile from the maps/beta directory in the game onto the window.

![Screenshot of the main window](/screenshot.png?raw=true "Screenshot of the main window")

## Compiling
* Get the zlib dependency
	* Get [zlib](https://www.zlib.net/zlib13.zip) and extract the `zlib-1.3` folder to the source directory (or otherwise point VS to it's location)
	* Open `contrib\vstudio\vc14\zlibvc.sln` and acticate the zlibstat project for a statically linked build.
	* The provided VS-Project uses the "Multi-threaded (/MT)" runtime library. Make sure that it's the same for zlib (C/C++ -> Code Generation -> Runtime Library).
	* Compile. This should create `zlib-1.3\contrib\vstudio\vc14\x64\ZlibStatRelease\zlibstat.lib` that can be used as a dependency
* Compiling should now work