# CPCPhoenix

CPCPhoenix is an Amstrad CPC 464/664/6128 emulator for Windows, written in C++.

## Features

- Z80 CPU core
- CPC system emulation: Gate Array, CRTC, PPI, PSG (AY-3-8912), FDC
- Disk (`.dsk`) and tape (`.cdt`) support
- Integrated debugger and sound analyzer
- Snapshot save/load (`.sna`)
- Copy text on the host (e.g. a BASIC listing) and paste it into the emulated CPC

## Limitations

- The emulator supports custom ROMs and RAM expansions, but there's no UI for
  adding them yet — they can only be configured in code.
- Disk and tape emulation only supports read operations. Write operations are not
  supported for now.

## Building

Open `Src\CPCPhoenix.sln` in Visual Studio 2022, pick your platform and
configuration, and build.

Output executables are named `CPCPhoenix_{Platform}_{Configuration}.exe`
(e.g. `CPCPhoenix_x64_Release.exe`).

### Runtime requirements

The system ROM files (`OS`, `BASIC` and `AMSDOS`) must be present under
`Bin\CPCPhoenix\Roms\`, relative to the executable's working directory.

Make sure your working directory is set to `Bin\CPCPhoenix\` when you run it —
otherwise CPCPhoenix won't find the ROMs.

## Command-line options

| Option | Description |
| --- | --- |
| `--diskA <path>` | Insert a disk image into drive A |
| `--diskA_archive <path>` | Load a disk image for drive A from an archive |
| `--diskB <path>` | Insert a disk image into drive B |
| `--diskB_archive <path>` | Load a disk image for drive B from an archive |
| `--tape <path>` | Load a tape image |
| `--autotype <text>` | Inject text into the emulated CPC |

## Thanks

Special thanks to everyone who has contributed in any form, including:

- [Daniel Varela](https://github.com/danielvarelagil2) — fixed bugs and
  improved the audio emulation

## Legal

The CPC system ROMs are redistributed with permission from Amstrad. See
[LEGAL.md](LEGAL.md) for details.
