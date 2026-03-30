# USBPark / Parktronic

Legacy USB parking-assist software + PCB design files for a real shipped product.

This project was originally a commercial product sold to **a few hundred customers**.  
It is now released publicly to support existing customer installations, maintenance, and long-term serviceability.

![USBPark demo (Audi)](audi.jpeg)

## At a glance

- Desktop parking-assist software (`parktronic` and `audi`) built with Qt/C++.
- Hardware design files in Eagle format (`.sch`, `.brd`) in [`pcb/`](pcb/).
- PIC18F-based controller design (PIC18F2550 appears in the v18 schematic/board design).
- Firmware update flow is integrated in the desktop app stack.

## Hardware

The latest schematic preview is included below:

![Parking Sensor USB v18 schematic](pcb/v18-schematic.webp)

Available PCB design assets:

- `pcb/Parking Sensor USB -v18.sch`
- `pcb/Parking Sensor USB -v18.brd`
- `pcb/Parking Sensor USB -v17b.sch`
- `pcb/Parking Sensor USB -v17b.brd`
- `pcb/v18-schematic.webp`

## Software components

### `parktronic/`

Main desktop application and diagnostics/configuration UI.

- Qt project: `parktronic/parktronic.pro`
- GUI tabs for diagnostics, firmware update, sounds, webcam, and settings
- Firmware resources referenced by the build (`hex/firmware.qrc`)
- Platform-specific entry points (`linux.cpp`, `win32.cpp`)

### `audi/`

Alternate/variant desktop UI package for the same product family.

- Qt project: `audi/audi.pro`
- Vehicle-oriented UI assets and sound notifications

## PIC18F firmware context

This project includes a PIC18F USB-controller hardware design and host-side firmware update integration:

- Schematic/board reference to PIC18F2550 in `pcb/Parking Sensor USB -v18.sch` and `.brd`
- Host software contains firmware version/update handling paths
- Project include paths indicate integration with firmware source/export infrastructure

## Build notes (legacy)

This is a legacy codebase (Qt4-era project files and platform-specific dependencies).

Typical flow:

```bash
cd parktronic
qmake parktronic.pro
make
```

and similarly for:

```bash
cd audi
qmake audi.pro
make
```

Notes:

- The original environment references external/internal Novorado libraries and SDK paths.
- You may need to adapt include/library paths before successful local builds.

## Why this repo is public

The code and hardware design are published to keep deployed units supportable for existing users:

- preserve serviceability
- enable repairs and compatibility work
- allow community maintenance of legacy installations

## Repository layout

```text
usbpark-code/
├── audi/         # Variant desktop app (Qt/C++)
├── parktronic/   # Main desktop app + diagnostics + firmware update UI
├── pcb/          # Eagle schematic/board files + schematic image
└── audi.jpeg     # Demo product image
```

## License and reuse

- `pcb/LICENSE` currently contains an MIT license text for PCB artifacts.
- Project history: originally commercial; now made public to support the installed base.
- If you need strict legal classification for reuse (public domain vs MIT-only subsets), document it explicitly for your distribution before shipping derivatives.

