/** @file
  BootHelper simple utility functions.

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

EFI_STATUS SetColour(UINTN Attribute)
{
	return gST->ConOut->SetAttribute(gST->ConOut, Attribute);
}

void Shutdown()
{
	gRT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
}

void Reboot()
{
	gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
}
