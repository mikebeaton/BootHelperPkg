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
// Shell Library
//
//#include <Library/ShellLib.h>

//
// Local includes
//
#include "EzKb.h"
#include "DisplayVars.h"

//
// We run on any UEFI Specification
//
//extern CONST UINT32 _gUefiDriverRevision = 0;

//
// Our name
//
//CHAR8 *gEfiCallerBaseName = "BootHelper";

#if false
EFI_STATUS
EFIAPI
UefiUnload (
    IN EFI_HANDLE ImageHandle
    )
{
    // 
    // This code should be compiled out and never called 
    // 
    ASSERT(FALSE);
    return EFI_SUCCESS;
}
#endif

#define EFI_OPEN_CORE_GUID \
  { 0x4d1fda02, 0x38c7, 0x4a6a, {0x9c, 0xc6, 0x4b, 0xcc, 0xa8, 0xb3, 0x01, 0x02} }
STATIC EFI_GUID gEfiOpenCoreGuid = EFI_OPEN_CORE_GUID;

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

static EFI_GUID appleGUID = { 0x7c436110, 0xab2a, 0x4bbb, {0xa8, 0x80, 0xfe, 0x41, 0x99, 0x5c, 0x9f, 0x82} };

void DisplayAppleNvramValue(CHAR16 *varName, BOOLEAN isString)
{
	DisplayNvramValueWithoutGuid(&appleGUID, varName, isString);
}

// with zero terminator
STATIC CHAR8 gBootArgsVal[] = "-no_compat_check";

// one char, no zero terminator
STATIC CHAR8 gStartupMuteVal[] = { '1' };

// equal to max size of toggled var
STATIC CHAR8 gGetVarBuffer[] = "-no_compat_check";

STATIC UINT32 gFlags = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE;

void ToggleOrSetVar(IN CHAR16 *varName, IN CHAR8 *preferredValue, UINTN actualSize, BOOLEAN toggle)
{
	UINT32 attr;

	UINTN data_size = actualSize;
	if (!EFI_ERROR(gRT->GetVariable(varName, &appleGUID, &attr, &data_size, gGetVarBuffer)) &&
		data_size == actualSize &&
		CompareMem(preferredValue, gGetVarBuffer, actualSize) == 0)
	{
		if (toggle)
		{
			gRT->SetVariable(varName, &appleGUID, gFlags, 0, NULL);
			//Print(L"Deleting %s\n", varName);
		}
		else
		{
			//Print(L"Not setting %s, already set\n", varName);
		}
	}
	else
	{
		gRT->SetVariable(varName, &appleGUID, gFlags, actualSize, preferredValue);
		//Print(L"Setting %s\n", varName);
	}
}

void ToggleVar(IN CHAR16 *varName, IN CHAR8 *preferredValue, UINTN actualSize)
{
	ToggleOrSetVar(varName, preferredValue, actualSize, TRUE);
}

void SetVar(IN CHAR16 *varName, IN CHAR8 *preferredValue, UINTN actualSize)
{
	ToggleOrSetVar(varName, preferredValue, actualSize, FALSE);
}

void ToggleBootArgs()
{
	ToggleVar(L"boot-args", gBootArgsVal, sizeof(gBootArgsVal));
}

void SetBootArgs()
{
	SetVar(L"boot-args", gBootArgsVal, sizeof(gBootArgsVal));
}

void ToggleCsrActiveConfig(UINT32 value)
{
	ToggleVar(L"csr-active-config", (CHAR8 *)&value, sizeof(value));
}

void ToggleStartupMute()
{
	ToggleVar(L"StartupMute", gStartupMuteVal, sizeof(gStartupMuteVal));
}

EFI_STATUS
EFIAPI
UefiMain(
	IN EFI_HANDLE ImageHandle,
	IN EFI_SYSTEM_TABLE* SystemTable
)
{
	BOOLEAN showOCVersion = FALSE;

	for (;;)
	{
		// inter alia, we want to clear the other stuff on the hidden text screen, before switching to viewing the text...
		gST->ConOut->ClearScreen(gST->ConOut);

		SetColour(EFI_LIGHTMAGENTA);
		Print(L"macOS NVRAM Boot Helper\n");
		Print(L"0.2.0\n");
		SetColour(EFI_WHITE);
		Print(L"\n");

#if 1
		DisplayAppleNvramValue(L"boot-args", TRUE);
		DisplayAppleNvramValue(L"csr-active-config", FALSE);
		DisplayAppleNvramValue(L"StartupMute", TRUE);
#endif
		if (showOCVersion)
		{
			DisplayNvramValue(&gEfiOpenCoreGuid, L"opencore-version", TRUE);
		}

		SetColour(EFI_LIGHTRED);
		Print(L"\nboot-[A]rgs; [B]ig Sur; [C]atalina; Startup[M]ute\n[R]eboot; [S]hutdown; [Q]uit; E[x]it; [L]ist\n");
		SetColour(EFI_WHITE);

		EFI_INPUT_KEY key;

		for (;;)
		{
			getkeystroke(&key);

			CHAR16 c = key.UnicodeChar;
			if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';

			if (c == 'a')
			{
				ToggleBootArgs();
				break;
			}
			else if (c == 'c')
			{
				ToggleCsrActiveConfig(0x77);
				break;
			}
			else if (c == 'b')
			{
				ToggleCsrActiveConfig(0x7f);
				break;
			}
			else if (c == 'm')
			{
				ToggleStartupMute();
				break;
			}
			else if (c == 'o')
			{
				showOCVersion = !showOCVersion;
				break;
			}
			else if (c == 'r')
			{
				Print(L"\nRebooting...");
				Reboot();
				break;
			}
			else if (c == 's')
			{
				Print(L"\nShutting down...");
				Shutdown();
				break;
			}
			else if (c == 'l')
			{
				Print(L"Listing... (any key for next or [Q]uit; E[x]it; List [a]ll remaining)\n");
				EFI_STATUS Status;
				Status = ListVars();
				if (Status == EFI_NOT_FOUND) {
					Print(L"Listed.\n");
				} else if (Status == EFI_SUCCESS) {
					Print(L"Quit.\n");
				} else {
					Print(L"Error!\n");
				}
				Print(L"Any Key...\n");
				getkeystroke(&key);
				break;
			}
			else if (c == 'x' || c == 'q')
			{
				Print(L"\nExiting...\n");
				return EFI_SUCCESS;
			}
		}
	}
}
