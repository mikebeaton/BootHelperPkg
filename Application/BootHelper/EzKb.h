/** @file
  Declaration of simple keyboard functions.

  Copyright (c) 2020, Mike Beaton. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-3-Clause

**/

#ifndef __BH__EZ_KB__
#define __BH__EZ_KB__

//
// Basic UEFI Libraries
//
#include <Uefi.h>
#include <Library/UefiLib.h>

// ReadKeyStroke returns EFI_NOT_READY if no key available
// ReadKeyStroke returns EFI_SUCCESS if a key is available
// It will not wait for a key to be available.
EFI_STATUS
kbhit (EFI_INPUT_KEY *Key);

// Wait for a key to be available, then read the key using ReadKeyStrokes
EFI_STATUS
getkeystroke (EFI_INPUT_KEY *Key);

#endif
