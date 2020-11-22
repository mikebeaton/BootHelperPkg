/** @file
  NVRAM var list and display methods.

  Copyright (c) 2020, Mike Beaton. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-3-Clause

**/

//
// Basic UEFI Libraries
//
#include <Uefi.h>
#include <Library/UefiLib.h>
//#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
////#include <Library/BaseLib.h>

//
// Boot and Runtime Services
//
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

//
// Local includes
//
#include "EzKb.h"

#define EFI_QEMU_C16_GUID_1 \
  { 0x158DEF5A, 0xF656, 0x419C, {0xB0, 0x27, 0x7A, 0x31, 0x92, 0xC0, 0x79, 0xD2} }
#define EFI_QEMU_C16_GUID_2 \
  { 0x0053D9D6, 0x2659, 0x4599, {0xA2, 0x6B, 0xEF, 0x45, 0x36, 0xE6, 0x31, 0xA9} }

STATIC EFI_GUID gEfiQemuC16lGuid1 = EFI_QEMU_C16_GUID_1;
STATIC EFI_GUID gEfiQemuC16lGuid2 = EFI_QEMU_C16_GUID_2;

CHAR16 HexChar(CHAR16 c)
{
	if (c < 10) return c + L'0';
	else return c - 10 + L'a';
}

// Display NVRAM var as a CHAR8 string
VOID
DisplayVarC8 (const CHAR8* in, UINTN nChars, BOOLEAN isString)
{
	Print (L"\"");
	for (UINTN i = 0; i < nChars; i++) {
		CHAR8 c = in[i];
		if (isString && c >= 32 && c < 127) {
			Print (L"%c", (CHAR16)c);
			if (c == '%') Print(L"%%"); // escape % so that representation is unambiguous & reversible
		} else {
			Print (L"%%");
			Print (L"%c", HexChar((in[i] >> 4) & 0xF));
			Print (L"%c", HexChar(in[i] & 0xF));
		}
	}
	Print (L"\"");

	if (nChars == 8) {
		Print (L" 0x%016lx", ((UINT64*)in)[0]);
	} else if (nChars == 4) {
		Print (L" 0x%08x", ((UINT32*)in)[0]);
	} else if (nChars == 2) {
		Print (L" 0x%04x", ((UINT16*)in)[0]);
	} else if (nChars == 1) {
		Print (L" 0x%02x", ((UINT8*)in)[0]);
	}
}

// Display NVRAM var as a CHAR16 string
VOID
DisplayVarC16 (const CHAR16* in, UINTN nChars, BOOLEAN isString)
{
	Print (L"L\"");
	for (UINTN i = 0; i < nChars; i++) {
		CHAR16 c = in[i];
		if (isString && c >= 32) {
			Print (L"%c", c);
			if (c == L'%') Print (L"%%"); // escape % so that representation is unambiguous & reversible
		} else {
			Print (L"%%");
			Print (L"%c", HexChar((in[i] >> 12) & 0xF));
			Print (L"%c", HexChar((in[i] >> 8) & 0xF));
			Print (L"%c", HexChar((in[i] >> 4) & 0xF));
			Print (L"%c", HexChar(in[i] & 0xF));
		}
	}
	Print (L"\"");
}

// Display NVRAM var, automatically deciding (based on GUID) whether it is likely to be a CHAR8 or CHAR16 string
// (Only some QEMU vars are currently displayed as CHAR16, but it is nice to be able to read the relevant strings easily.)
VOID
DisplayVar (
  IN EFI_GUID *Guid,
  IN VOID *Data,
  UINTN DataSize,
  BOOLEAN isString)
{
	// some known guid's which seem to have only CHAR16 strings in them
	// don't even try to display it as CHAR16 string if byte size is odd
	if ((DataSize & 1) == 0 && (
		CompareMem (Guid, &gEfiQemuC16lGuid1, sizeof(EFI_GUID)) == 0 ||
		CompareMem (Guid, &gEfiQemuC16lGuid2, sizeof(EFI_GUID)) == 0
	)) {
		DisplayVarC16 ((CHAR16 *)Data, DataSize >> 1, isString);
	} else {
		DisplayVarC8 ((CHAR8 *)Data, DataSize, isString);
	}
}

EFI_STATUS
GetNvramValue (
  IN CHAR16 *Name,
  IN EFI_GUID *Guid,
  OUT UINT32 *Attr,
  OUT UINTN *DataSize,
  OUT VOID **Data)
{
	EFI_STATUS Status;

    //
    // Initialize variable data buffer as an empty buffer
    //
    *DataSize = 0;
    *Data = NULL;

    //
    // Loop until a large enough variable data buffer is allocated
    //
    do {
        Status = gRT->GetVariable(Name, Guid, Attr, DataSize, *Data);
        if (Status == EFI_BUFFER_TOO_SMALL) {
            //
            // Allocate new buffer for the variable data
            //
            *Data = AllocatePool(*DataSize);
            if (*Data == NULL) {
                return EFI_OUT_OF_RESOURCES;
            }
        }
    } while (Status == EFI_BUFFER_TOO_SMALL);

    if (EFI_ERROR(Status)) {
        FreePool(*Data);
        return Status;
    }

	return EFI_SUCCESS;
}

EFI_STATUS
DisplayNvramValueOptionalGuid (
  IN EFI_GUID *Guid,
  IN CHAR16 *Name,
  BOOLEAN isString,
  BOOLEAN displayGuid)
{
	EFI_STATUS Status;
	UINT32 Attributes;
	UINTN DataSize;
	VOID *Data;

	if (displayGuid) {
    	Print(L"%g:", Name);
	}

    Print(L"%s", Name);

    Status = GetNvramValue(Name, Guid, &Attributes, &DataSize, &Data);

    if (EFI_ERROR(Status)) {
		if (Status == EFI_NOT_FOUND) {
			Print(L": EFI_NOT_FOUND\n");
		} else {
			Print(L": EFI_UNKOWN_STATUS=%0x\n", Status);
		}

        return Status;
    }

    Print(L" = ");
    DisplayVar(Guid, Data, DataSize, TRUE);
	if ((Attributes & EFI_VARIABLE_NON_VOLATILE) == 0) {
		Print(L" (non-persistent)");
	}
    Print(L"\n");

    FreePool(Data);

	return EFI_SUCCESS;
}

EFI_STATUS
DisplayNvramValue (
  IN EFI_GUID *Guid,
  IN CHAR16 *Name,
  BOOLEAN isString)
{
	return DisplayNvramValueOptionalGuid (Guid, Name, isString, TRUE);
}

EFI_STATUS
DisplayNvramValueWithoutGuid (
  IN EFI_GUID *Guid,
  IN CHAR16 *Name,
  BOOLEAN isString)
{
	return DisplayNvramValueOptionalGuid (Guid, Name, isString, FALSE);
}

EFI_STATUS
ListVars ()
{
	EFI_STATUS Status;
	EFI_GUID Guid;
	UINTN NameBufferSize;
	UINTN NameSize;
	CHAR16 *Name;

	BOOLEAN showAll = FALSE;

	//
	// Initialize the variable name and data buffer variables
	// to retrieve the first variable name in the variable store
	//
	NameBufferSize = sizeof(CHAR16);
	Name = AllocateZeroPool(NameBufferSize);

	//
	// Loop through all variables in the variable store
	//
	while (TRUE)
	{
		do {
			//
			// Loop until a large enough variable name buffer is allocated
			// do {
			NameSize = NameBufferSize;
			Status = gRT->GetNextVariableName(&NameSize, Name, &Guid);
			if (Status == EFI_BUFFER_TOO_SMALL) {
				//
				// Grow the buffer Name to NameSize bytes
				//
				Name = ReallocatePool(NameBufferSize, NameSize, Name);
				if (Name == NULL) {
					return EFI_OUT_OF_RESOURCES;
				}
				NameBufferSize = NameSize;
			}
		} while (Status == EFI_BUFFER_TOO_SMALL);

		//
		// Exit main loop after last variable name is retrieved
		//
		if (EFI_ERROR (Status)) {
			FreePool (Name);
			return Status;
		}

		//
		// Display var
		//
        Status = DisplayNvramValue (&Guid, Name, TRUE);

		//
		// Not expecting error here but exit if there is one
		//
		if (EFI_ERROR (Status)) {
			FreePool (Name);
			return Status;
		}

		//
		// Keyboard control
		//
		if (!showAll) {
			EFI_INPUT_KEY key;
			getkeystroke (&key);

			CHAR16 c = key.UnicodeChar;
			if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
			if (c == 'q' || c == 'x') {
				FreePool (Name);
				return EFI_SUCCESS;
			} else if (c == 'a') {
				showAll = TRUE;
			}
		}
	}
}
