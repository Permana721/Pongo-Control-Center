# Pongo Control Center

A lightweight, terminal-based RGB keyboard backlight controller for Axioo Pongo 725 laptops running Arch Linux and its derivatives (CachyOS, EndeavourOS, Manjaro).

## Requirements

* Arch Linux or an Arch-based distribution
* `base-devel` package group
* `git`

## Installation

Clone the repository and build the package using `makepkg`:

```bash
git clone [https://github.com/Permana721/Pongo-Control-Center.git](https://github.com/Permana721/Pongo-Control-Center.git)
cd Pongo-Control-Center
makepkg -si
The installer will automatically handle dependencies (including tuxedo-keyboard-dkms), compile the source code, set the required binary permissions, and load the kernel module.

Usage
Run the following command in your terminal:

Bash
pongo-control-center
Controls
Up / Down Arrows: Navigate menus

Enter: Confirm selection

Left / Right Arrows or < / >: Adjust slider values

Mouse Click / Drag: Adjust slider values directly

Uninstallation
To remove the package from your system:

Bash
sudo pacman -R pongo-control-center
License
This project is licensed under the GPL-3.0 License.