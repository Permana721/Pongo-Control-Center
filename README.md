# Pongo Control Center

Interactive terminal-based RGB keyboard backlight controller for Axioo Pongo 725 laptops running Arch Linux and its derivatives.

## Features

- Dynamic interactive CLI menu
- ANSI 24-bit True Color gradient slider
- Mouse and keyboard navigation support
- Standalone C binary with no external GUI dependencies
- Automatic kernel driver management

## Requirements

- Arch Linux or derivatives (CachyOS, EndeavourOS, Manjaro)
- `base-devel` package group
- `git`
- AUR helper (`yay` or `paru`)

## Installation

Clone the repository and build the package using `yay`:

```bash
git clone https://github.com/Permana721/Pongo-Control-Center.git
cd Pongo-Control-Center
yay -Bi .

```

The installer automatically resolves all necessary dependencies (including `clevo-drivers-dkms-git`), compiles the binary, and sets the required hardware execution permissions.

## Usage

Run the following command in any terminal:

```bash
pongo-control-center

```

### Navigation Controls

* Up / Down Arrows: Navigate menu items
* Enter: Confirm selection
* Left / Right Arrows (or `<` / `>`): Adjust sliders
* Mouse Click: Drag or click directly on sliders to change values

## Uninstallation

To remove the application from your system:

```bash
sudo pacman -Rns pongo-control-center

```

## License

This project is licensed under the GPL-3.0 License.
