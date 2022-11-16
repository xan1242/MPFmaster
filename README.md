# EA Pathfinder v5 Tool

This tool is designed to decompile and compile the Pathfinder files used for interactive music in EA Games.

## Compatibility

This tool should work with any Pathfinder v5 file.

Some of the games are:

- Need for Speed Most Wanted (2005)

- Need for Speed Carbon

- Need for Speed Pro Street

- Need for Speed Undercover

- Need for Speed World

- Red Alert 3 (sample extraction is broken because of separate MUS files)

## Usage

Basic usage:

```
USAGE (decompile MPF to TXT): mpfmaster MPFfile [OutFile]
USAGE (compile TXT to MPF): mpfmaster -c source [MPFout]
USAGE (extract by sample num): mpfmaster -s MPFfile SampleNumber [OutSampleFile]
USAGE (extract all samples): mpfmaster -sa MPFfile [OutSampleFolder]
USAGE (update samples): mpfmaster -su MPFfile SampleFolder

For sample extraction, the MUS file needs to be present next to the MPF file with the same name!
For sample updating, the MUS file will be generated with the name of the MPF and placed next to it. You MUST have all files from the lowest to highest number!
If you omit the optional [out] name, it'll inherit the name of the input file.
The compiler is not very well written, so please follow the decompilation syntax closely!
Newly compiled files do not contain samples. Add them with the update samples command.
```

Example: you want to edit something in the existing track files...

- Make sure your MPF and MUS files are in the same folder!

- Decompile the MPF
  
  ```
  mpfmaster MW_Music.mpf
  ```

- Extract the samples
  
  ```
  mpfmaster -sa MW_Music.mpf mySampleFolder
  ```

- Edit whatever you need to edit

- Recompile the MPF
  
  ```
  mpfmaster -c MW_Music_decomp.txt mynewMPF.mpf
  ```

- Update the samples
  
  ```
  mpfmaster -su mynewMPF.mpf mySampleFolder
  ```

## TODO

This tool isn't perfect. It doesn't generate quite identical files to the originals, but they do work.

- Big endian support for consoles

- Try out compiling with gcc/clang -- currently works in macOS, still need make a CMakeLists

- Figure out why some variable types don't get updates for certain actions (mostly fixed)

- Multi track support? Currently it assumes each event action is track 1

- Improve the parser - it's currently a hackjob working on a line-per-line basis

- Clean up the code - everything is jammed in the main cpp file!

- Documentation about the syntax and available commands!

- Test with other games!

- Figure out sample extraction & repacking for Red Alert 3 -- maybe add a mode for separate files (or MPF updating only mode, no repacking)

# Credits

- Nicknine - the initial mpftotext script from which this tool was based on and specific code from VGMStream
- VGMStream project - header parsing for GSTR / EA Layer 3
