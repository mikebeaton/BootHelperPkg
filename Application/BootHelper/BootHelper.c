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
#include <Library/OcConfigurationLib.h>
#include <Library/OcDebugLogLib.h>
#include <Library/OcFileLib.h>
#include <Library/OcStorageLib.h>

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
BH_ON_EXIT mBhOnExit = BhOnExitExit;

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
  BOOLEAN isString
  )
{
  DisplayNvramValueWithoutGuid(Name, &gEfiAppleGuid, isString);
}

EFI_STATUS
ToggleAppleVar(
  IN CHAR16 *Name,
  IN CHAR8 *PreferredValue,
  UINTN PreferredSize
  )
{
  return ToggleOrSetVar(Name, &gEfiAppleGuid, PreferredValue, PreferredSize, TRUE);
}

EFI_STATUS
SetAppleVar(
  IN CHAR16 *Name,
  IN CHAR8 *PreferredValue,
  UINTN PreferredSize
  )
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
BhMain ()
{
  BOOLEAN showOCVersion = FALSE;

  while (TRUE) {
    // inter alia, we want to clear the other stuff on the hidden text screen, before switching to viewing the text...
    if (mClearScreen) gST->ConOut->ClearScreen(gST->ConOut);

    SetColour(EFI_LIGHTMAGENTA);
    Print(L"macOS NVRAM Boot Helper\n");
    Print(L"0.2.6 store\n");
    SetColour(EFI_WHITE);
    Print(L"\n");

#if 1
    DisplayAppleVar(L"boot-args", TRUE);
    DisplayAppleVar(L"csr-active-config", FALSE);
    DisplayAppleVar(L"StartupMute", TRUE);
#endif
    if (showOCVersion) {
      DisplayNvramValueWithoutGuid(L"opencore-version", &gEfiOpenCoreGuid, TRUE);
    }

    SetColour(EFI_LIGHTRED);
    Print(L"\nboot-[A]rgs; [B]ig Sur; [C]atalina; Startup[M]ute\n[R]eboot; [S]hutdown; [Q]uit; E[x]it; [L]ist\n");
    SetColour(EFI_WHITE);

    EFI_INPUT_KEY key;

    while (TRUE) {
      getkeystroke(&key);

      CHAR16 c = key.UnicodeChar;
      if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';

      if (c == 'a') {
        ToggleBootArgs();
        break;
#if 0
      } else if (c == 'z') {
        SetBootArgs();
        break;
#endif
	  } else if (c == 'c') {
        ToggleCsrActiveConfig(0x77);
        break;
      } else if (c == 'b') {
        ToggleCsrActiveConfig(0x7f);
        break;
      } else if (c == 'm') {
        ToggleStartupMute();
        break;
      } else if (c == 'o') {
        showOCVersion = !showOCVersion;
        break;
      } else if (c == 'r') {
        mBhOnExit = BhOnExitReboot;
        return EFI_SUCCESS;
      } else if (c == 's') {
        mBhOnExit = BhOnExitShutdown;
        return EFI_SUCCESS;
      } else if (c == 'l') {
        Print (L"Listing... (any key for next or [Q]uit; E[x]it; List [a]ll remaining)\n");
        EFI_STATUS Status;
        Status = ListVars();
        if (Status == EFI_NOT_FOUND) {
          Print( L"Listed.\n");
        } else if (Status == EFI_SUCCESS) {
          Print (L"Quit.\n");
        } else {
          Print (L"Error: %r!\n", Status);
        }
        Print (L"Any Key...\n");
        getkeystroke (&key);
        break;
      } else if (c == 'x' || c == 'q') {
        return EFI_SUCCESS;
      }
    }
  }
}

#if 0
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
#endif

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
    CpuDeadLoop();
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
    CpuDeadLoop();
    return EFI_NOT_FOUND;
  }

  OC_STORAGE_CONTEXT OpenCoreStorage;

  Status = OcStorageInitFromFs (
    &OpenCoreStorage,
    FileSystem,
    BOOT_HELPER_ROOT_PATH,
    NULL //mOpenCoreVaultKey
    );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "BH: Failed to open root FS - %r\n", Status));
    CpuDeadLoop();
    return Status;
  }

  CHAR8                     *ConfigData;
  UINT32                    ConfigDataSize;
  OC_GLOBAL_CONFIG          Config;

  ConfigData = OcStorageReadFileUnicode (
    &OpenCoreStorage,
    BOOT_HELPER_CONFIG_PATH,
    &ConfigDataSize
    );

  if (ConfigData != NULL) {
    DEBUG ((DEBUG_INFO, "BH: Loaded configuration of %u bytes\n", ConfigDataSize));

    Status = OcConfigurationInit (
      &Config,
      ConfigData,
      ConfigDataSize
      );

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "BH: Failed to parse configuration!\n"));
      CpuDeadLoop ();
      return EFI_UNSUPPORTED; ///< Should be unreachable.
    }

    FreePool (ConfigData);
  } else {
    DEBUG ((DEBUG_ERROR, "BH: Failed to load configuration!\n"));
    CpuDeadLoop ();
    return EFI_UNSUPPORTED; ///< Should be unreachable.
  }

  CONST CHAR8 *AsciiPicker;
  AsciiPicker = OC_BLOB_GET (&Config.Misc.Boot.PickerMode);
  Print (
    L"BH: I think we've loaded OpenCore config from %s/%s; Misc.Boot.PickerMode=\"%a\"\n",
    BOOT_HELPER_ROOT_PATH,
    BOOT_HELPER_CONFIG_PATH,
    AsciiPicker
    );

  Status = BhMain ();

  OcConfigurationFree (&Config);

  OcStorageFree (&OpenCoreStorage);

  if (mBhOnExit == BhOnExitReboot) {
    Print(L"\nRebooting...\n");
    Reboot();
    CpuDeadLoop();
  } else if (mBhOnExit == BhOnExitShutdown) {
    Print(L"\nShutting down...\n");
    Shutdown();
    CpuDeadLoop();
  }

  Print (L"\nExiting w/ %r...\n", Status);

  Print (L"\nAny key...\n");
  EFI_INPUT_KEY key;
  getkeystroke(&key);

  return Status;
}
