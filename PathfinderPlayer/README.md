# Pathfinder Player

This is an extremely basic player.

Currently it doesn't support any advanced actions (looping, event actions, variables, routers, etc). 

It only does basic branching with a single controller.

You can use this tool to preview what each node will sound like without having to test in the game itself.

## Usage

Before anything, install .NET Desktop Runtime. Release builds are built with version 6.0, so get that one.

1. Open the Pathfinder map file source (decompiled with MPFmaster)

2. Open a folder with samples in a supported format by BASS (WAV, MP3, OGG) - convert your samples if you have to

3. Type in your desired node in the "Next node" box and apply it (optional)

4. Press "Play"

5. Fiddle around with the "Intensity" controller as the player is running. You should be able to observe different behavior in real time.

## Status

It is currently very limited and buggy.

It currently has a basic node parser and nothing much more.

It does not exit out of branches and can get stuck if the branches lead nowhere.

It's also limited to 44.1kHz output as I did not write up a configuration dialog for setting up BASS and actually having a config file.

# Credits

[Un4seen Developments Ltd.](http://www.un4seen.com/) (Ian Luck) - BASS library

[ManagedBass wrapper for .NET](https://github.com/ManagedBass/ManagedBass)


