/** @file
  Declaration of NVRAM var list and display methods.

  Copyright (c) 2020, Mike Beaton. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-3-Clause

**/

#ifndef __BH__LIST_VARS__
#define __BH__LIST_VARS__

//
// Basic UEFI Libraries
//
#include <Uefi.h>
#include <Library/UefiLib.h>

// Return an NVRAM value, the allocated buffer for the data must be freed by the caller using FreePool
EFI_STATUS
GetNvramValue (EFI_GUID *Guid, CHAR16 *Name, UINTN *DataSize, UINT8 **Data);

// Display an NVRAM value, allocating and freeing the buffer needed for the data (always display GUID)
EFI_STATUS
DisplayNvramValue (EFI_GUID *Guid, CHAR16 *Name, BOOLEAN isString);

// Display an NVRAM value, allocating and freeing the buffer needed for the data (don't display GUID)
EFI_STATUS
DisplayNvramValueWithoutGuid (EFI_GUID *Guid, CHAR16 *Name, BOOLEAN isString);

// List all NVRAM vars to conout, with some keyboard control
EFI_STATUS
ListVars();

#endif
