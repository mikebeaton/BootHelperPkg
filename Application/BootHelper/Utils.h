/** @file
  Declaration of BootHelper simple utility functions.

  Copyright (c) 2020, Mike Beaton. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-3-Clause

**/

#ifndef __BH__UTILS__
#define __BH__UTILS__

//
// Basic UEFI Libraries
//
#include <Uefi.h>
#include <Library/UefiLib.h>

EFI_STATUS SetColour(
  UINTN Attribute
  );

void Shutdown();

void Reboot();

#endif
