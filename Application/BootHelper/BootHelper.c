//
// Basic UEFI Libraries
//
#include <Uefi.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>

//
// Boot and Runtime Services
//
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

//
// OC Libraries
//
#include <Library/OcDebugLogLib.h>
#include <Library/OcFileLib.h>

//
// Shell Library
//
//#include <Library/ShellLib.h>

#include <Protocol/LoadedImage.h>

//
// Local includes
//
#include "BootHelper.h"
#include "EzKb.h"
#include "DisplayVars.h"
#include "Utils.h"

BOOLEAN mInteractive = TRUE;
BOOLEAN mClearScreen = FALSE;

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

#define EFI_APPLE_GUID \
  { 0x7c436110, 0xab2a, 0x4bbb, {0xa8, 0x80, 0xfe, 0x41, 0x99, 0x5c, 0x9f, 0x82} }
STATIC EFI_GUID gEfiAppleGuid = EFI_APPLE_GUID;

// with zero terminator
STATIC CHAR8 gBootArgsVal[] = "-no_compat_check";

// one char, no zero terminator
STATIC CHAR8 gStartupMuteVal[] = { '1' };

VOID
DisplayAppleVar(
  IN CHAR16 *Name,
  BOOLEAN isString)
{
	DisplayNvramValueWithoutGuid(Name, &gEfiAppleGuid, isString);
}

EFI_STATUS
ToggleAppleVar(
  IN CHAR16 *Name,
  IN CHAR8 *PreferredValue,
  UINTN PreferredSize)
{
	return ToggleOrSetVar(Name, &gEfiAppleGuid, PreferredValue, PreferredSize, TRUE);
}

EFI_STATUS
SetAppleVar(
  IN CHAR16 *Name,
  IN CHAR8 *PreferredValue,
  UINTN PreferredSize)
{
	return ToggleOrSetVar(Name, &gEfiAppleGuid, PreferredValue, PreferredSize, FALSE);
}

void ToggleBootArgs()
{
	ToggleAppleVar(L"boot-args", gBootArgsVal, sizeof(gBootArgsVal));
}

void SetBootArgs()
{
	SetAppleVar(L"boot-args", gBootArgsVal, sizeof(gBootArgsVal));
}

void ToggleCsrActiveConfig(UINT32 value)
{
	ToggleAppleVar(L"csr-active-config", (CHAR8 *)&value, sizeof(value));
}

void ToggleStartupMute()
{
	ToggleAppleVar(L"StartupMute", gStartupMuteVal, sizeof(gStartupMuteVal));
}

EFI_STATUS
EFIAPI
BhMain (
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *FileSystem
  )
{
	BOOLEAN showOCVersion = FALSE;

	for (;;)
	{
		// inter alia, we want to clear the other stuff on the hidden text screen, before switching to viewing the text...
		if (mClearScreen) gST->ConOut->ClearScreen(gST->ConOut);

		SetColour(EFI_LIGHTMAGENTA);
		Print(L"macOS NVRAM Boot Helper\n");
		Print(L"0.2.6\n");
		SetColour(EFI_WHITE);
		Print(L"\n");

#if 1
		DisplayAppleVar(L"boot-args", TRUE);
		DisplayAppleVar(L"csr-active-config", FALSE);
		DisplayAppleVar(L"StartupMute", TRUE);
#endif
		if (showOCVersion)
		{
			DisplayNvramValueWithoutGuid(L"opencore-version", &gEfiOpenCoreGuid, TRUE);
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
#if 0
			else if (c == 'z')
			{
				SetBootArgs();
				break;
			}
#endif
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

VOID
DebugDebug (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;

  OC_LOG_PROTOCOL *mInternalOcLog = NULL;
  Status = gBS->LocateProtocol (
    &gOcLogProtocolGuid,
    NULL,
    (VOID **) &mInternalOcLog
  );
  Print (L"Located OcLogProtocol=%d (EFI_NOT_FOUND=%d)\n", Status, EFI_NOT_FOUND);

  Print (L"DebugPrintEnabled=%d\n", DebugPrintEnabled());
  Print (L"DebugPrintLevelEnabled(DEBUG_INFO)=%d\n", DebugPrintLevelEnabled(DEBUG_INFO));

#if !defined(MDEPKG_NDEBUG)
  Print (L"Debug seems enabled\n");
#else
  Print (L"Debug not enabled!\n");
#endif
}

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  //DebugDebug (ImageHandle, SystemTable);

  EFI_STATUS                        Status;
  EFI_LOADED_IMAGE_PROTOCOL         *LoadedImage;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *FileSystem;

  DEBUG ((DEBUG_INFO, "BH: Starting BootHelper...\n"));

  LoadedImage = NULL;
  Status = gBS->HandleProtocol (
    ImageHandle,
    &gEfiLoadedImageProtocolGuid,
    (VOID **) &LoadedImage
    );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "BH: Failed to locate loaded image - %r\n", Status));
    return EFI_NOT_FOUND;
  }

  DebugPrintDevicePath (DEBUG_INFO, "BH: Booter path", LoadedImage->FilePath);

  //
  // Obtain the file system device path
  //
  FileSystem = LocateFileSystem (
    LoadedImage->DeviceHandle,
    LoadedImage->FilePath
    );

  if (FileSystem == NULL) {
    DEBUG ((DEBUG_ERROR, "BH: Failed to obtain own file system\n"));
    return EFI_NOT_FOUND;
  }

#if 1
  Status = BhMain (FileSystem);

  return Status;
#else
  return EFI_SUCCESS;
#endif
}
