# CPCPhoenix

CPCPhoenix is an Amstrad CPC 464/664/6128 emulator for Windows, written in C++.

## Features

- Z80 CPU core
- CPC system emulation: Gate Array, CRTC, PPI, PSG (AY-3-8912), FDC
- Disk (`.dsk`) and tape (`.cdt`) support
- Integrated debugger and sound analyzer
- Snapshot save/load (`.sna`)
- Copy text from the host machine (e.g. a BASIC listing) and paste it into the emulated CPC

## Building

Open `Src\CPCPhoenix.sln` in Visual Studio 2022, select the desired platform and configuration and build.

Output executables are named `CPCPhoenix_{Platform}_{Configuration}.exe`
(e.g. `CPCPhoenix_x64_Release.exe`).

### Runtime requirements

The system ROM files (`OS`, `BASIC` and `AMSDOS`) must be present relative to
the executable's working directory, under `Bin\CPCPhoenix\Roms\`.

Make sure that you change the working directory to `Bin\CPCPhoenix\`; otherwise, CPCPhoenix won't be able to find the ROMs.

## Command-line options

| Option | Description |
| --- | --- |
| `--diskA <path>` | Insert a disk image into drive A |
| `--diskA_archive <path>` | Load a disk image for drive A from an archive |
| `--diskB <path>` | Insert a disk image into drive B |
| `--diskB_archive <path>` | Load a disk image for drive B from an archive |
| `--tape <path>` | Load a tape image |
| `--autotype <text>` | Inject text into the emulated CPC |

## Legal

The CPC system ROMs are redistributed with permission from Amstrad. See
[LEGAL.md](LEGAL.md) for details.
