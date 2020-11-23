/** @file
  Declaration of BootHelper global variables.

  Copyright (c) 2020, Mike Beaton. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-3-Clause

**/

#ifndef __BH__BH__
#define __BH__BH__

//
// Basic UEFI Libraries
//
#include <Uefi.h>
#include <Library/UefiLib.h>

#define BOOT_HELPER_ROOT_PATH       L"EFI\\BootHelper"
#define BOOT_HELPER_CONFIG_PATH     L"BootHelper.plist"

typedef enum BH_ON_EXIT_ {
  BhOnExitExit,
  BhOnExitShutdown,
  BhOnExitReboot,
} BH_ON_EXIT;

extern BOOLEAN mInteractive;
extern BOOLEAN mClearScreen;
extern BH_ON_EXIT mBhOnExit;

#endif
