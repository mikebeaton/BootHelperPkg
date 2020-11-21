
#include <Library/BaseMemoryLib.h>

#include "DisplayVar.h"

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

VOID
DisplayVarC8 (const CHAR8* in, UINTN nChars, BOOLEAN isString)
{
	Print (L"\"");
	for (UINTN i = 0; i < nChars; i++)
	{
		CHAR8 c = in[i];
		if (isString && c >= 32 && c < 127)
		{
			Print (L"%c", (CHAR16)c);
			if (c == '%') Print(L"%%"); // escape % so that representation is unambiguous & reversible
		}
		else
		{
			Print (L"%%");
			Print (L"%c", HexChar((in[i] >> 4) & 0xF));
			Print (L"%c", HexChar(in[i] & 0xF));
		}
	}
	Print (L"\"");

	if (nChars == 8)
	{
		Print (L" (0x%016lx)", ((UINT64*)in)[0]);
	}
	else if (nChars == 4)
	{
		Print (L" (0x%08x)", ((UINT32*)in)[0]);
	}
	else if (nChars == 2)
	{
		Print (L" (0x%04x)", ((UINT16*)in)[0]);
	}
	else if (nChars == 1)
	{
		Print (L" (0x%02x)", ((UINT8*)in)[0]);
	}
}

VOID
DisplayVarC16 (const CHAR16* in, UINTN nChars, BOOLEAN isString)
{
	Print (L"L\"");
	for (UINTN i = 0; i < nChars; i++)
	{
		CHAR16 c = in[i];
		if (isString && c >= 32)
		{
			Print(L"%c", c);
			if (c == L'%') Print(L"%%"); // escape % so that representation is unambiguous & reversible
		}
		else
		{
			Print (L"%%");
			Print (L"%c", HexChar((in[i] >> 12) & 0xF));
			Print (L"%c", HexChar((in[i] >> 8) & 0xF));
			Print (L"%c", HexChar((in[i] >> 4) & 0xF));
			Print (L"%c", HexChar(in[i] & 0xF));
		}
	}
	Print (L"\"");
}

VOID
DisplayVar (const EFI_GUID *Guid, const VOID *data, UINTN size, BOOLEAN isString)
{
	// some known guid's which seem to have only CHAR16 strings in them
	if ((size & 1) == 0 &&
		(CompareMem (Guid, &gEfiQemuC16lGuid1, sizeof(EFI_GUID)) == 0 ||
		 CompareMem (Guid, &gEfiQemuC16lGuid2, sizeof(EFI_GUID)) == 0))
	{
		DisplayVarC16 ((CHAR16 *)data, size >> 1, isString);
	}
	else
	{
		DisplayVarC8 ((CHAR8 *)data, size, isString);
	}
}
