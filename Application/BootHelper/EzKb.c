/** @file
  Simple keyboard functions.

  Copyright (c) 2020, Mike Beaton. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-3-Clause

**/

//
// Basic UEFI Libraries
//
#include <Uefi.h>
#include <Library/UefiLib.h>

//
// Boot and Runtime Services
//
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

EFI_STATUS
kbhit (EFI_INPUT_KEY *Key)
{
	return gST->ConIn->ReadKeyStroke (gST->ConIn, Key);
}

EFI_STATUS
getkeystroke (EFI_INPUT_KEY *Key)
{
	gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, 0);
	return gST->ConIn->ReadKeyStroke (gST->ConIn, Key);
}
