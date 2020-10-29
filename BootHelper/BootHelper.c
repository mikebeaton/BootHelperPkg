//
// Basic UEFI Libraries
//
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Protocol/ConsoleControl/ConsoleControl.h>

//
// Boot and Runtime Services
//
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

//
// Shell Library
//
#include <Library/ShellLib.h>

//
// We run on any UEFI Specification
//
extern CONST UINT32 _gUefiDriverRevision = 0;

//
// Our name
//
CHAR8 *gEfiCallerBaseName = "BootHelper";

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
}

EFI_STATUS
EFIAPI
OcConsoleControlEntryMode(
	IN EFI_HANDLE        ImageHandle,
	IN EFI_SYSTEM_TABLE  *SystemTable
)
{
	EFI_STATUS                   Status;
	EFI_CONSOLE_CONTROL_PROTOCOL *ConsoleControl;

	EFI_GUID efiConsoleControlProtocolGuid = EFI_CONSOLE_CONTROL_PROTOCOL_GUID;

	//
	// On several types of firmware, we need to use legacy console control protocol to
	// switch to text mode, otherwise a black screen will be shown.
	//
	Status = gBS->HandleProtocol(
		gST->ConsoleOutHandle,
		&efiConsoleControlProtocolGuid,
		(VOID **)&ConsoleControl
	);
	if (EFI_ERROR(Status)) {
		Status = gBS->LocateProtocol(
			&efiConsoleControlProtocolGuid,
			NULL,
			(VOID **)&ConsoleControl
		);
	}

	if (!EFI_ERROR(Status)) {
		ConsoleControl->SetMode(
			ConsoleControl,
			0
		);
	}

	return EFI_SUCCESS;
}

// ReadKeyStroke returns EFI_NOT_READY if no key available
// ReadKeyStroke returns EFI_SUCCESS if a key is available
// It will not wait for a key to be available.
EFI_STATUS kbhit(EFI_INPUT_KEY *Key)
{
	return gST->ConIn->ReadKeyStroke(gST->ConIn, Key);
}

// Wait for a key to be available, then read the key using ReadKeyStrokes
EFI_STATUS getkeystroke(EFI_INPUT_KEY *Key)
{
	gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, 0);
	return gST->ConIn->ReadKeyStroke(gST->ConIn, Key);
}

int mymcmp(CONST IN CHAR8 *buf1, CONST IN CHAR8 *buf2, UINTN size)
{
	for (UINTN i = 0; i < size; i++)
	{
		if (buf1[i] - buf2[i] != 0) return buf1[i] - buf2[i];
	}
	return 0;
}

CHAR16 HexChar(CHAR16 c)
{
	if (c < 10) return c + L'0';
	else return c - 10 + L'a';
}

#define EFI_OPEN_CORE_GUID \
  { 0x4d1fda02, 0x38c7, 0x4a6a, {0x9c, 0xc6, 0x4b, 0xcc, 0xa8, 0xb3, 0x01, 0x02} }
STATIC EFI_GUID gEfiOpenCoreGuid = EFI_OPEN_CORE_GUID;

#define EFI_QEMU_C16_GUID_1 \
  { 0x158DEF5A, 0xF656, 0x419C, {0xB0, 0x27, 0x7A, 0x31, 0x92, 0xC0, 0x79, 0xD2} }
#define EFI_QEMU_C16_GUID_2 \
  { 0x0053D9D6, 0x2659, 0x4599, {0xA2, 0x6B, 0xEF, 0x45, 0x36, 0xE6, 0x31, 0xA9} }

STATIC EFI_GUID gEfiQemuC16lGuid1 = EFI_QEMU_C16_GUID_1;
STATIC EFI_GUID gEfiQemuC16lGuid2 = EFI_QEMU_C16_GUID_2;

void DisplayVarC8(const CHAR8* in, UINTN nChars, BOOLEAN isString)
{
	Print(L"\"");
	for (UINTN i = 0; i < nChars; i++)
	{
		CHAR8 c = in[i];
		if (isString && c >= 32 && c < 127)
		{
			Print(L"%c", (CHAR16)c);
			if (c == '%') Print(L"%%"); // escape % so that representation is unambiguous & reversible
		}
		else
		{
			Print(L"%%");
			Print(L"%c", HexChar((in[i] >> 4) & 0xF));
			Print(L"%c", HexChar(in[i] & 0xF));
		}
	}
	Print(L"\"");

	if (nChars == 4)
	{
		Print(L" (0x%08x)", ((UINT32*)in)[0]);
	}
	else if (nChars == 2)
	{
		Print(L" (0x%04x)", ((UINT16*)in)[0]);
	}
	else if (nChars == 1)
	{
		Print(L" (0x%02x)", ((UINT8*)in)[0]);
	}
}

void DisplayVarC16(const CHAR16* in, UINTN nChars, BOOLEAN isString)
{
	Print(L"L\"");
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
			Print(L"%%");
			Print(L"%c", HexChar((in[i] >> 12) & 0xF));
			Print(L"%c", HexChar((in[i] >> 8) & 0xF));
			Print(L"%c", HexChar((in[i] >> 4) & 0xF));
			Print(L"%c", HexChar(in[i] & 0xF));
		}
	}
	Print(L"\"");
}

void DisplayVar(const EFI_GUID *Guid, const CHAR8* in, UINTN size, BOOLEAN isString)
{
	// some known guid's which seem to have only CHAR16 strings in them
	if ((size & 1) == 0 &&
		(mymcmp((CHAR8 *)Guid, (CHAR8 *)&gEfiQemuC16lGuid1, sizeof(Guid)) == 0 ||
		 mymcmp((CHAR8 *)Guid, (CHAR8 *)&gEfiQemuC16lGuid2, sizeof(Guid)) == 0))
	{
		DisplayVarC16((CHAR16 *)in, size >> 1, isString);
	}
	else
	{
		DisplayVarC8(in, size, isString);
	}
}

EFI_STATUS ListVars()
{

	EFI_STATUS Status;
	EFI_GUID Guid;
	UINTN NameBufferSize;
	UINTN NameSize;
	CHAR16 *Name;
	UINTN DataSize;
	UINT8 *Data;

	//Print(L"-a");
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
			//Print(L"-b");
			  //
			  // Loop until a large a large enough variable name buffer is allocated
			  // do {
			NameSize = NameBufferSize;
			Status = gRT->GetNextVariableName(&NameSize, Name, &Guid);
			if (Status == EFI_BUFFER_TOO_SMALL) {
				//Print(L"-c");
					//
					// Grow the buffer Name to NameSize bytes
					//
				Name = ReallocatePool(NameBufferSize, NameSize, Name);
				if (Name == NULL) {
					return EFI_OUT_OF_RESOURCES;
				}
				NameBufferSize = NameSize;
			}
			//Print(L"-d");
		} while (Status == EFI_BUFFER_TOO_SMALL);

		//
		// Exit main loop after last variable name is retrieved
		//
		if (EFI_ERROR(Status)) {
			//Print(L"-e");
			FreePool(Name);
			return Status;
		}

		//
		// Print variable guid and name
		//
		Print(L"%g:%s", &Guid, Name);

		//
		// Initialize variable data buffer as an empty buffer
		//
		DataSize = 0;
		Data = NULL;

		//
		// Loop until a large enough variable data buffer is allocated
		//
		do {
			Status = gRT->GetVariable(Name, &Guid, NULL, &DataSize, Data);
			if (Status == EFI_BUFFER_TOO_SMALL) {
				//Print(L"-f");
					//
					// Allocate new buffer for the variable data
					//
				Data = AllocatePool(DataSize);
				if (Data == NULL) {
					FreePool(Name);
					return EFI_OUT_OF_RESOURCES;
				}
			}
			//Print(L"-g");
		} while (Status == EFI_BUFFER_TOO_SMALL);
		if (EFI_ERROR(Status)) {
			//Print(L"-h");
			FreePool(Data);
			FreePool(Name);
			return Status;
		}

		//Print(L"-i");
		//
		// Print variable data
		//
#if 1
		Print(L" = ");
		DisplayVar(&Guid, Data, DataSize, TRUE);
#else
		UINTN Index;
		for (Index = 0; Index < DataSize; Index++) {
			if ((Index & 0x0f) == 0) {
				Print(L"\n ");
			}
			Print(L"%02x ", Data[Index]);
		}
#endif
		Print(L"\n");
		//Print(L"-j");
		FreePool(Data);

		EFI_INPUT_KEY key;
		getkeystroke(&key);

		CHAR16 c = key.UnicodeChar;
		if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
		if (c == 'q')
		{
			FreePool(Name);
			return EFI_SUCCESS;
		}

	}
}

#define PRINT_BUF_SIZE 1024

#if 0
void MyPrint(CONST CHAR16 *format, ...)
{
	CHAR16 buffer[PRINT_BUF_SIZE];
	va_list args;
	va_start(args, format);
	VSPrint(buffer, PRINT_BUF_SIZE, format, args);
	va_end(args);
	conOut->OutputString(conOut, buffer);
}
#endif

EFI_STATUS SetColour(UINTN Attribute)
{
	return gST->ConOut->SetAttribute(gST->ConOut, Attribute);
}

#define BUF_SIZE 255

CHAR16 hexDigit(CHAR8 val)
{
	val &= 0xF;

	if (val < 10) return val + '0';
	else return val - 10 + 'a';
}

void string8to16(const CHAR8* in, CHAR16* out, UINTN size, BOOLEAN isString)
{
	if (size > BUF_SIZE) size = BUF_SIZE;

	UINTN o = 0;
	for (UINTN i = 0; i < size; i++)
	{
		CHAR8 c = in[i];
		if (isString && c == 0) break;
		if (isString && c >= 32 && c < 127)
		{
			out[o++] = c;
			if (c == '%') out[o++] = c; // escape % itself
		}
		else
		{
			out[o++] = '%';
			out[o++] = hexDigit((c >> 4) & 0xF);
			out[o++] = hexDigit(c & 0xF);
		}
	}
	out[o] = 0;
}

void string16to8(const CHAR16* in, CHAR8* out, UINTN size)
{
	if (size > BUF_SIZE) size = BUF_SIZE;

	for (UINTN i = 0; i < size; i++)
	{
		out[i] = (CHAR8)in[i];
	}
	out[size] = 0;
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

void DisplayNvramValue(CHAR16 *varName, EFI_GUID *guid, BOOLEAN isString)
{
	// + 1 for \0 terminators
	CHAR8 buffer8[BUF_SIZE + 1];
	CHAR16 buffer16[3 * BUF_SIZE + 1];

	UINT32 attr;
	UINTN data_size = BUF_SIZE;

	EFI_STATUS status;

	status = gRT->GetVariable(varName, &appleGUID, &attr, &data_size, &buffer8);
	if (status == EFI_SUCCESS)
	{
		Print(L"%s=", varName);

		if (!isString && data_size == 4)
		{
			Print(L"0x%08x", ((UINT32*)buffer8)[0]);
		}
		else if (!isString && data_size == 2)
		{
			Print(L"0x%04x", ((UINT16*)buffer8)[0]);
		}
		else if (!isString && data_size == 1)
		{
			Print(L"0x%02x", ((UINT8*)buffer8)[0]);
		}
		else
		{
#if 0
			// use ' for non-null-terminated strings (including, but not only, single chars)
			CHAR16 quote = (CHAR16)(buffer8[data_size - 1] == '\0' ? '"' : '`');
#else
			// quote them both the same, to avoid unecessary confusion
			CHAR16 quote = (CHAR16)'"';
#endif
			string8to16(buffer8, buffer16, data_size, isString);
			Print(L"%c%s%c", quote, buffer16, quote);
		}

		Print(L" (%spersistent)", (attr & EFI_VARIABLE_NON_VOLATILE) == 0 ? L"non-" : L"");
		Print(L"\n");
	}
	else if (status == EFI_BUFFER_TOO_SMALL)
	{
		Print(L"%s: EFI_BUFFER_TOO_SMALL(%d > %d)\n", varName, data_size, BUF_SIZE);
		//Print(L"    attr: %x\n", attr);
	}
	else if (status == EFI_NOT_FOUND)
	{
		Print(L"%s: EFI_NOT_FOUND\n", varName);
	}
	else
	{
		Print(L"%s: EFI_UNKOWN_STATUS=%d\n", varName, status);
	}
}

void DisplayAppleNvramValue(CHAR16 *varName, BOOLEAN isString)
{
	DisplayNvramValue(varName, &appleGUID, isString);
}

// with zero terminator
STATIC CHAR8 gBootArgsVal[] = "-no_compat_check";

// one char, no zero terminator
STATIC CHAR8 gStartupMuteVal[] = { '1' };

// equal to max size of toggled var
STATIC CHAR8 gGetVarBuffer[] = "-no_compat_check";

STATIC UINT32 gFlags = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE;

void ToggleVar(IN CHAR16 *varName, IN CHAR8 *preferredValue, UINTN actualSize)
{
	UINT32 attr;

	UINTN data_size = actualSize;
	if (!EFI_ERROR(gRT->GetVariable(varName, &appleGUID, &attr, &data_size, gGetVarBuffer)) &&
		data_size == actualSize &&
		mymcmp(preferredValue, gGetVarBuffer, actualSize) == 0)
	{
		gRT->SetVariable(varName, &appleGUID, gFlags, 0, NULL);
	}
	else
	{
		gRT->SetVariable(varName, &appleGUID, gFlags, actualSize, preferredValue);
	}
}

void ToggleBootArgs()
{
	ToggleVar(L"boot-args", gBootArgsVal, sizeof(gBootArgsVal));
}

void ToggleCsrActiveConfig(UINT32 value)
{
	ToggleVar(L"csr-active-config", (CHAR8 *)&value, sizeof(value));
}

void ToggleStartupMute()
{
	ToggleVar(L"StartupMute", gStartupMuteVal, sizeof(gStartupMuteVal));
}

#if 0
BOOLEAN TryProtocol(EFI_GUID proto_guid, void** out, const CHAR16* name,
	EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable) {

	*out = NULL;
	EFI_STATUS status;
	EFI_HANDLE interface = NULL;

	status = systemTable->BootServices->LocateProtocol(&proto_guid, NULL, &interface);

	if (EFI_ERROR(status)) {
		//Print(L"LocateProtocol error for %s: %r\n", name, status);
		return FALSE;
	}

	//Print(L"Locate protocol address: %s, %x\n", name, interface);
	*out = interface;

	return TRUE;
}
#endif

EFI_STATUS
EFIAPI
UefiMain(
	IN EFI_HANDLE ImageHandle,
	IN EFI_SYSTEM_TABLE* SystemTable
)
{
	OcConsoleControlEntryMode(ImageHandle, SystemTable);

	for (;;)
	{
		// inter alia, we want to clear the other stuff on the hidden text screen, before switching to viewing the text...
		gST->ConOut->ClearScreen(gST->ConOut);

		SetColour(EFI_LIGHTMAGENTA);
		Print(L"macOS NVRAM Boot Helper\n");
		Print(L"0.0.21\n");
		SetColour(EFI_WHITE);
		Print(L"\n");

		//gRT->SetVariable(L"csr-active-config", &appleGUID, gFlags, 4, csrVal);
		//gRT->SetVariable(L"EnableTRIM", &appleGUID, gFlags, 1, trimSetting);

		//gRT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);

		//efi_guid_t guid = EFI_GLOBAL_VARIABLE_GUID;

		DisplayAppleNvramValue(L"boot-args", TRUE);
		DisplayAppleNvramValue(L"csr-active-config", FALSE);
		DisplayAppleNvramValue(L"StartupMute", TRUE);

		SetColour(EFI_LIGHTRED);
		Print(L"\nboot-[A]rgs; [B]ig Sur; [C]atalina; Startup[M]ute\n[R]eboot; [S]hutdown; E[x]it; [L]ist\n");
		SetColour(EFI_WHITE);

		EFI_INPUT_KEY key;

		for (;;)
		{
			//Print(L"Wait for key...\n");
			getkeystroke(&key);
			//Print(L"Got key %c (%x)\n", key.UnicodeChar, key.UnicodeChar);

			CHAR16 c = key.UnicodeChar;
			if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';

			if (c == L'a')
			{
				ToggleBootArgs();
				break;
			}
			else if (c == L'c')
			{
				ToggleCsrActiveConfig(0x77);
				break;
			}
			else if (c == L'b')
			{
				ToggleCsrActiveConfig(0x7f);
				break;
			}
			else if (c == L'm')
			{
				ToggleStartupMute();
				break;
			}
			else if (c == L'o')
			{
				DisplayNvramValue(L"opencore-version", &gEfiOpenCoreGuid, TRUE);
			}
			else if (c == L'r')
			{
				Print(L"\nRebooting...");
				Reboot();
				break;
			}
			else if (c == L's')
			{
				Print(L"\nShutting down...");
				Shutdown();
				break;
			}
			else if (c == L'l')
			{
				Print(L"Listing...\n");
				ListVars();
				Print(L"Listed.\nAny Key...\n");
				getkeystroke(&key);
				break;
			}
			else if (c == L'x')
			{
				Print(L"\nExiting...\n");
				return EFI_SUCCESS;
			}
		}
	}
}
