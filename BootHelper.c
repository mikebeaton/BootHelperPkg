#include <efi.h>
#include <efilib.h>
#include <ConsoleControl.h>

static EFI_RUNTIME_SERVICES* rt = NULL;
static EFI_BOOT_SERVICES* bt = NULL;

SIMPLE_TEXT_OUTPUT_INTERFACE *conOut = NULL;
SIMPLE_INPUT_INTERFACE *conIn = NULL;

CHAR16 HexChar(UINT16 c)
{
    if (c < 10) return c + (CHAR16)'0'; 
    else return c - 10 + (CHAR16)'a';
}

void DisplayVar(const CHAR8* in, UINTN size, BOOLEAN isString)
{
    for (UINTN i = 0; i < size; i++)
    {
        CHAR8 c = in[i];
        if (isString && c >= 32 && c < 127) Print(L"%c", (CHAR16)c);
        else
        {
            Print(L"%%");
            Print(L"%c", HexChar(in[i] >> (UINT16)4));
            Print(L"%c", HexChar(in[i] & (UINT16)0xF));
        }
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
NameBufferSize = sizeof (CHAR16);
Name = AllocateZeroPool (NameBufferSize);

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
  Status = uefi_call_wrapper(rt->GetNextVariableName, 3, &NameSize, Name, &Guid);
  if (Status == EFI_BUFFER_TOO_SMALL) {
//Print(L"-c");
    //
    // Grow the buffer Name to NameSize bytes
    //
    Name = ReallocatePool(Name, NameBufferSize, NameSize);
    if (Name == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    NameBufferSize = NameSize;
  }
//Print(L"-d");
}
while (Status == EFI_BUFFER_TOO_SMALL);

//
// Exit main loop after last variable name is retrieved
//
if (EFI_ERROR (Status)) {
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
  Status = uefi_call_wrapper(rt->GetVariable, 5, Name, &Guid, NULL, &DataSize, Data);
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
if (EFI_ERROR (Status)) {
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
Print(L" = \"");
DisplayVar(Data, DataSize, TRUE);
Print(L"\"");
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

}
}

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

    conOut = SystemTable->ConOut;
    conIn = SystemTable->ConIn;

    EFI_GUID guid_conControl = EFI_CONSOLE_CONTROL_PROTOCOL_GUID;
    EFI_CONSOLE_CONTROL_PROTOCOL *conControl = NULL;

    if (TryProtocol(guid_conControl, (void**)&conControl,
        L"CONSOLE_CONTROL_PROTOCOL", ImageHandle, SystemTable) != TRUE) {
        SetBootArgs();
        return EFI_SUCCESS;
    }

    uefi_call_wrapper(conControl->SetMode, 2, conControl, EfiConsoleControlScreenText);

    for (;;)
    {
        // inter alia, we want to clear the other stuff on the hidden text screen, before switching to viewing the text...
        uefi_call_wrapper(conOut->ClearScreen, 1, conOut);

        SetColour(EFI_YELLOW);
        Print(L"macOS Boot Helper\n");
        Print(L"0.0.8\n");
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

        SetColour(EFI_LIGHTRED);
        Print(L"\n[B]oot-args; [C]sr-active-config; [R]eboot; [S]hutdown; E[x]it; [L]ist\n");
        SetColour(EFI_WHITE);

        EFI_INPUT_KEY key;

        for (;;)
        {
            //Print(L"Wait for key...\n");
            getkeystroke(SystemTable, &key);
            //Print(L"Got key %c (%x)\n", key.UnicodeChar, key.UnicodeChar);

            CHAR16 c = key.UnicodeChar;
            if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';

            if (c == L'b')
            {
                SetBootArgs();
                break;
            }
            else if (c == L'c')
            {
                //SetBootArgs();
                //Reboot();
                break;
            }
            else if (c == L'r')
            {
                Reboot();
                break;
            }
            else if (c == L's')
            {
                Shutdown();
                break;
            }
            else if (c == L'l')
            {
                Print(L"Listing...\n");
                ListVars();
                Print(L"Listed.\nAny Key...\n");
                getkeystroke(SystemTable, &key);
                break;
            }
            else if (c == L'x')
            {
                Print(L"Changing mode...");
                uefi_call_wrapper(conControl->SetMode, 2, conControl, EfiConsoleControlScreenGraphics);
                Print(L"Exiting...");
                return 0;
            }
        }
    }
}
