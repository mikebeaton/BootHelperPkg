#include <efi.h>
#include <efilib.h>

// ReadKeyStroke returns EFI_NOT_READY if no key available
// ReadKeyStroke returns EFI_SUCCESS if a key is available
// It will not wait for a key to be available.
EFI_STATUS kbhit(EFI_SYSTEM_TABLE *SystemTable, EFI_INPUT_KEY *Key)
{
    return SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, Key);
}

// Wait for a key to be available, then read the key using ReadKeyStroke
EFI_STATUS getkeystroke(EFI_SYSTEM_TABLE *SystemTable, EFI_INPUT_KEY *Key)
{
    uefi_call_wrapper(SystemTable->BootServices->WaitForEvent, 3, 1, &SystemTable->ConIn->WaitForKey, 0);
    return uefi_call_wrapper(SystemTable->ConIn->ReadKeyStroke, 2, SystemTable->ConIn, Key);
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

static EFI_GUID appleGUID = {0x7c436110, 0xab2a, 0x4bbb, {0xa8, 0x80, 0xfe, 0x41, 0x99, 0x5c, 0x9f, 0x82}};

static EFI_RUNTIME_SERVICES* rt;
static EFI_BOOT_SERVICES* bt;

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
        Print(L"    size: %d\n", data_size);
        Print(L"    attr: %d\n", attr);
        Print(L"     hex: 0x%08x\n", ((UINT32*)buffer8)[0]);
    }
    else if (status == EFI_BUFFER_TOO_SMALL)
    {
        Print(L"%s: EFI_BUFFER_TOO_SMALL(%d > %d)\n", varName, data_size, BUF_SIZE);
        Print(L"    attr: %x\n", attr);
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


EFI_STATUS EFIAPI efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    // Might need HandleProtocol to get this?
    SIMPLE_TEXT_OUTPUT_INTERFACE *conOut = SystemTable->ConOut;

    EFI_STATUS status;
    status = uefi_call_wrapper(conOut->SetMode, 2, conOut, 0);

    Print(L" == Big Sur Boot Helper ==\n");

    // globals
    rt = SystemTable->RuntimeServices;
    bt = SystemTable->BootServices;

    //uefi_call_wrapper(rt->SetVariable, 5, L"csr-active-config", &appleGUID, flags, 4, csrVal);
    //uefi_call_wrapper(rt->SetVariable, 5, L"EnableTRIM", &appleGUID, flags, 1, trimSetting);

    //uefi_call_wrapper(rt->ResetSystem, 4, EfiResetShutdown, EFI_SUCCESS, 0, NULL);

    //efi_guid_t guid = EFI_GLOBAL_VARIABLE_GUID;

    DisplayNvramValue(L"boot-args", TRUE);
    DisplayNvramValue(L"csr-active-config", FALSE);

    uefi_call_wrapper(conOut->OutputString, 2, conOut, L"Example text\n\n");

    UINTN Columns, Rows;
    status = uefi_call_wrapper(conOut->QueryMode, 4, conOut, 0, &Columns, &Rows);
    Print(L"Mode 0, status %d, columnsxrows = %dx%d\n", status, Columns, Rows);
    Print(L"\n[R]eboot; Set [B]oot-args and reboot; [S]hutdown; e[X]it\n", status, Columns, Rows);

    EFI_INPUT_KEY key;

    for (;;)
    {
        Print(L"Wait for key...\n");
        getkeystroke(SystemTable, &key);
        Print(L"Got key %c (%x)\n", key.UnicodeChar, key.UnicodeChar);

        CHAR16 c = key.UnicodeChar;
        if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';

        switch (c)
        {
            case L'r':
                uefi_call_wrapper(rt->ResetSystem, 4, EfiResetWarm, EFI_SUCCESS, 0, NULL);
                break;

            case L'b':
                SetBootArgs();
                uefi_call_wrapper(rt->ResetSystem, 4, EfiResetWarm, EFI_SUCCESS, 0, NULL);
                break;

            case L's':
                uefi_call_wrapper(rt->ResetSystem, 4, EfiResetShutdown, EFI_SUCCESS, 0, NULL);
                break;

            case L'x':
                return EFI_SUCCESS;
        }
    }
}
