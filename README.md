# macOS NVRAM Boot Helper

![screenshot](./images/screenshot.png?raw=true)

## Purpose

Can be used as a stand-alone bootable image (e.g. on a USB stick), or as a tool from UEFI Shell or OpenCore bootloader.

The idea is to let you quickly set some useful nvram values which would normally require booting all the way into macOS recovery.

Features:

 - You can very quickly do the equivalent of `csrutil enable/disable` (Big Sur or Catalina settings)

 - You can quickly set `nvram boot-args="-no_compat_check"`; which may be the only setting you need, after [installation](https://forums.macrumors.com/threads/macos-11-big-sur-on-unsupported-macs-thread.2242172/page-181?post=28960530#post-28960530), to run Big Sur on an only slightly incompatible Mac

 - You can quickly set/clear `StartupMute` - this doesn't require Recovery mode to set normally; I just found it useful to add it here

 - There is also a basic - but hopefully useful - ability to list and view the value of every variable stored in your Mac's nvram

## Usage

### Standard Usage

Download the current [release zip file](../../releases/download/0.1.0/BootHelper.zip).

Copy the entire EFI folder from the zip file into the root of any FAT32 USB drive. No other files on the drive need to be deleted. If you boot up your Mac while holding down Alt/Option then you should see your USB drive listed as 'Boot Helper', and you can boot from it to start the utilty.

Try pressing the labelled keys to see what they do. None of them modify anything which can do any harm. All the nvram keys toggle their settings, i.e. press once to set; press again to unset.

Changing the value of `csr-active-config` and rebooting is the same thing as using `csrutil enable/disable` from Recovery and rebooting.

### More Advanced

If you find the tool really useful, you could consider making a tiny (e.g. 20MB) FAT32 partition on your hard drive, and installing it there, so that it is permanently available. Again, the entire EFI directory can be copied to the root of any FAT32 (or FAT16, if it's small) partition, and should just work.

### Advanced

To use as an OpenCore tool, copy `EFI\BOOT\BootHelper.efi` into the `EFI\OC\Tools` of an existing OpenCore boot disk, and then configure this as a tool in the `Tools` section of `EFI\OC\config.plist` (following the pattern of any existing tools in there).

`BootHelper.efi` can also be run from OpenCore Open Shell or any other UEFI Shell, if you already have one configured.

## Future

These are just the settings I wanted to be able to change quickly. I am hoping to find the time to write additional code against OpenCore's plist library, in order to allow a `BootHelper.plist` file, which would let you configure the quick settings which you find most useful.

## Development/Contribution

The code is currently set up to compile using [VisualUefi](https://github.com/ionescu007/VisualUefi) - largely because getting started with EDK2 development is hard, and this seemed like a do-able way to get going. To use the code as is, you need to develop on Windows. On Windows only, please check in and out with `git config core.autocrlf` = `true` - the line endings in the repo are LF.

The code up to [this tag](../../tree/last-edk1) was built in EDK 1 and compiles fine on Linux and macOS (with [these prerequisites](https://forums.macrumors.com/threads/macos-11-big-sur-on-unsupported-macs-thread.2242172/page-202?post=29009038#post-29009038)).

As long as I can get Xcode QEMU support working (I'm currently very much enjoying the VisualUefi QEMU support) I'm planning to move the project back to a more standard EDK 2 set up, at which point I'll probably rename the repo to `BootHelperPkg` (the current url should get redirected by GitHub).

## Credits

An early version was inspired by and based on Barry K. Nathan's [setvars](https://github.com/barrykn/big-sur-micropatcher/tree/main/setvars) program from the big-sur-micropatcher.

The code to enter console mode is taken from [OpenCore](https://github.com/acidanthera/OpenCorePkg) and is used, and made available here, under the BSD license.
