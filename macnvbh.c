#include <efi.h>
#include <efilib.h>
#include <ConsoleControl.h>

static EFI_RUNTIME_SERVICES* rt = NULL;
static EFI_BOOT_SERVICES* bt = NULL;

SIMPLE_TEXT_OUTPUT_INTERFACE *conOut = NULL;
SIMPLE_INPUT_INTERFACE *conIn = NULL;

#define PRINT_BUF_SIZE 1024

#if 0
void MyPrint(CONST CHAR16 *format, ...)
{
    CHAR16 buffer[PRINT_BUF_SIZE];
    va_list args;
    va_start (args, format);
    VSPrint(buffer, PRINT_BUF_SIZE, format, args);
    va_end(args);
    uefi_call_wrapper(conOut->OutputString, 2, conOut, buffer);
}
#endif

EFI_STATUS SetColour(UINTN Attribute)
{
    return uefi_call_wrapper(conOut->SetAttribute, 2, conOut, Attribute);
}

// ReadKeyStroke returns EFI_NOT_READY if no key available
// ReadKeyStroke returns EFI_SUCCESS if a key is available
// It will not wait for a key to be available.
EFI_STATUS kbhit(EFI_SYSTEM_TABLE *SystemTable, EFI_INPUT_KEY *Key)
{
    return conIn->ReadKeyStroke(conIn, Key);
}

// Wait for a key to be available, then read the key using ReadKeyStrokes
EFI_STATUS getkeystroke(EFI_SYSTEM_TABLE *SystemTable, EFI_INPUT_KEY *Key)
{
    uefi_call_wrapper(bt->WaitForEvent, 3, 1, &conIn->WaitForKey, 0);
    return uefi_call_wrapper(conIn->ReadKeyStroke, 2, conIn, Key);
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
        if (isString && c >= 32 && c < 127) out[o++] = in[i];
        else
        {
            out[o++] = '%';
            out[o++] = hexDigit(in[i] >> 4);
            out[o++] = hexDigit(in[i] & 0xF);
        }
    }
    out[o] = 0;
}

void string16to8(const CHAR16* in, CHAR8* out, UINTN size)
{
    if (size > BUF_SIZE) size = BUF_SIZE;

    for (UINTN i = 0; i < size; i++)
    {
        out[i] = in[i];
    }
    out[size] = 0;
}

void Shutdown()
{
    uefi_call_wrapper(rt->ResetSystem, 4, EfiResetShutdown, EFI_SUCCESS, 0, NULL);
}

void Reboot()
{
    uefi_call_wrapper(rt->ResetSystem, 4, EfiResetWarm, EFI_SUCCESS, 0, NULL);
}

static EFI_GUID appleGUID = {0x7c436110, 0xab2a, 0x4bbb, {0xa8, 0x80, 0xfe, 0x41, 0x99, 0x5c, 0x9f, 0x82}};

void DisplayNvramValue(CHAR16 *varName, BOOLEAN isString)
{
    // + 1 for \0 terminators
    CHAR8 buffer8[BUF_SIZE + 1];
    CHAR16 buffer16[3 * BUF_SIZE + 1];

    UINT32 attr;
    UINT32 data_size = BUF_SIZE;

    EFI_STATUS status;

    status = uefi_call_wrapper(rt->GetVariable, 5, varName, &appleGUID, &attr, &data_size, &buffer8);
    if (status == EFI_SUCCESS)
    {
        string8to16(buffer8, buffer16, data_size, isString);
        Print(L"%s=\"%s\"\n", varName, &buffer16);
        //Print(L"    size: %d\n", data_size);
        //Print(L"    attr: %d\n", attr);
        if (!isString) Print(L"     hex: 0x%08x\n", ((UINT32*)buffer8)[0]);
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

void SetBootArgs()
{
    UINT32 flags = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE;
    CHAR8 bootArgsVal[] = "-no_compat_check";
    uefi_call_wrapper(rt->SetVariable, 5, L"boot-args", &appleGUID, flags, sizeof(bootArgsVal), bootArgsVal);
}

#if 1
BOOLEAN TryProtocol(EFI_GUID proto_guid, void** out, const CHAR16* name,
    EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable) {

    *out = NULL;
    EFI_STATUS status;
    EFI_HANDLE interface = NULL;

    status = uefi_call_wrapper(systemTable->BootServices->LocateProtocol, 3,
        &proto_guid, NULL, &interface);

    if (EFI_ERROR(status)) {
        //Print(L"LocateProtocol error for %s: %r\n", name, status);
        return FALSE;
    }

    //Print(L"Locate protocol address: %s, %x\n", name, interface);
    *out = interface;

    return TRUE;
}
#endif

EFI_STATUS EFIAPI efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    //LegacySwitchToTextMode(SystemTable);

#if 1
    // Might need HandleProtocol to get this?
    conOut = SystemTable->ConOut;
    conIn = SystemTable->ConIn;
#endif

#if 0
    Print(L"conOut: %x\n", conOut);
    Print(L"    conOut->Mode: %x\n", conOut->Mode);
    Print(L"conIn: %x\n", conIn);

    SIMPLE_TEXT_OUTPUT_INTERFACE *xonOut = NULL;
    SIMPLE_INPUT_INTERFACE *xonIn = NULL;
#endif

    //uefi_call_wrapper(conOut->SetMode, 2, conOut, 0);
    // I think we can clear screen, then switch to viewing text...
    uefi_call_wrapper(conOut->ClearScreen, 1, conOut);

    EFI_GUID guid_conControl = EFI_CONSOLE_CONTROL_PROTOCOL_GUID;
    EFI_CONSOLE_CONTROL_PROTOCOL *conControl = NULL;

    if (TryProtocol(guid_conControl, (void**)&conControl,
        L"CONSOLE_CONTROL_PROTOCOL", ImageHandle, SystemTable) != TRUE) {
        SetBootArgs();
        return EFI_SUCCESS;
    }

#if 0
    EFI_GUID guid_conOut = EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID;

    if (TryProtocol(guid_conOut, (void**)&conOut,
        L"SIMPLE_TEXT_OUTPUT_PROTOCOL", ImageHandle, SystemTable) != TRUE) {
        Shutdown();
        return EFI_SUCCESS;
    }
    // Print(L"xonOut: %x\n", xonOut);
    // Print(L"    xonOut->Mode: %x\n", xonOut->Mode);

    EFI_GUID guid_conIn = EFI_SIMPLE_TEXT_INPUT_PROTOCOL_GUID;

    if (TryProtocol(guid_conIn, (void**)&conIn,
        L"SIMPLE_TEXT_INPUT_PROTOCOL", ImageHandle, SystemTable) != TRUE) {
        Shutdown();
        return EFI_SUCCESS;
    }
#endif

    SetColour(EFI_YELLOW);
    Print(L"NVRAM Boot Helper\n");
    Print(L"0.0.7\n");
    SetColour(EFI_WHITE);
    Print(L"\n");

    // globals
    rt = SystemTable->RuntimeServices;
    bt = SystemTable->BootServices;

    //uefi_call_wrapper(rt->SetVariable, 5, L"csr-active-config", &appleGUID, flags, 4, csrVal);
    //uefi_call_wrapper(rt->SetVariable, 5, L"EnableTRIM", &appleGUID, flags, 1, trimSetting);

    //uefi_call_wrapper(rt->ResetSystem, 4, EfiResetShutdown, EFI_SUCCESS, 0, NULL);

    //efi_guid_t guid = EFI_GLOBAL_VARIABLE_GUID;

    DisplayNvramValue(L"boot-args", TRUE);
    DisplayNvramValue(L"csr-active-config", FALSE);

#if 0
    UINTN Columns, Rows;
    EFI_STATUS status = uefi_call_wrapper(conOut->QueryMode, 4, conOut, 0, &Columns, &Rows);
    Print(L"Mode 0, status %d, columnsxrows = %dx%d\n", status, Columns, Rows);
#endif

    SetColour(EFI_LIGHTRED);
    Print(L"\n[R]eboot; Set [B]oot-args and reboot; [S]hutdown; e[X]it\n");
    SetColour(EFI_WHITE);

    uefi_call_wrapper(conControl->SetMode, 2, conControl, EfiConsoleControlScreenText);

    EFI_INPUT_KEY key;

    for (;;)
    {
        //Print(L"Wait for key...\n");
        getkeystroke(SystemTable, &key);
        //Print(L"Got key %c (%x)\n", key.UnicodeChar, key.UnicodeChar);

        CHAR16 c = key.UnicodeChar;
        if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';

        if (c == L'r')
        {
            Reboot();
            break;
        }
        else if (c == L'b')
        {
            SetBootArgs();
            Reboot();
            break;
        }
        else if (c == L's')
        {
            Shutdown();
            break;
        }
        else if (c == L'x')
        {
            break;
        }
    }

    return EFI_SUCCESS;
}
