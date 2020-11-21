/** @file
  Declaration of NVRAM display functions.

  Copyright (c) 2020, Mike Beaton. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-3-Clause

**/

#ifndef __BH__DISPLAY_VAR__
#define __BH__DISPLAY_VAR__

//
// Basic UEFI Libraries
//
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>

// Display NVRAM var as a CHAR8 string
VOID
DisplayVarC8 (const CHAR8 *in, UINTN nChars, BOOLEAN isString);

// Display NVRAM var as a CHAR16 string
VOID
DisplayVarC16 (const CHAR16 *in, UINTN nChars, BOOLEAN isString);

// Display NVRAM var, automatically deciding (based on GUID) whether it is likely to be a CHAR8 or CHAR16 string
// (Only some QEMU vars are currently displayed as CHAR16, but it is nice to be able to read the relevant strings easily.)
VOID
DisplayVar (const EFI_GUID *Guid, const VOID *data, UINTN size, BOOLEAN isString);

#endif
