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


EFI_STATUS EFIAPI efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    /* Making bootArgs[] static keeps it contiguous within the compiled binary.
     * This might be good in case there's a need later to do binary patching
     * of some kind later. It certainly does no harm that I can see.
     */
    //static char bootArgs[] = "-no_compat_check";

    /* 'w' (0x77) disables SIP; 0x08 disables authenticated root */
    /* As of Big Sur beta 9 (or maybe beta 7), 0x7f not 0x77     */
    //static const char csrVal[4] = {0x7f, 0x08, 0x00, 0x00};

    /* 1-char array rather than just a char variable, so that I can
     * treat it the same way as the others when calling
     * rt->SetVariable().
     *
     * 0x01 enables TRIM even on non-Apple SSDs, like `trimforce enable`.
     */
    //static const char trimSetting[1] = {0x01};

    EFI_GUID appleGUID = {0x7c436110, 0xab2a, 0x4bbb, {0xa8, 0x80, 0xfe, 0x41, 0x99, 0x5c, 0x9f, 0x82}};
    //const UINT32 flags = EFI_VARIABLE_BOOTSERVICE_ACCESS|EFI_VARIABLE_RUNTIME_ACCESS|EFI_VARIABLE_NON_VOLATILE;

    EFI_RUNTIME_SERVICES* rt;
    //EFI_BOOT_SERVICES* bt;

    InitializeLib(ImageHandle, SystemTable);

    SIMPLE_TEXT_OUTPUT_INTERFACE *conOut = SystemTable->ConOut;

    EFI_STATUS status;
    status = uefi_call_wrapper(conOut->SetMode, 2, conOut, 0);

    Print(L" == Big Sur Boot Helper ==\n");
    rt = SystemTable->RuntimeServices;
    //bt = SystemTable->BootServices;

    //uefi_call_wrapper(rt->SetVariable, 5, L"csr-active-config", &appleGUID, flags, 4, csrVal);
    //uefi_call_wrapper(rt->SetVariable, 5, L"boot-args", &appleGUID, flags, sizeof(bootArgs), bootArgs);
    //uefi_call_wrapper(rt->SetVariable, 5, L"EnableTRIM", &appleGUID, flags, 1, trimSetting);

    //uefi_call_wrapper(rt->ResetSystem, 4, EfiResetShutdown, EFI_SUCCESS, 0, NULL);

    CHAR16 name[] = L"boot-args";
    //efi_guid_t guid = EFI_GLOBAL_VARIABLE_GUID;
    UINT32 attr;
    UINT32 data_size = 512;
    CHAR16 data[512];

    status = uefi_call_wrapper(rt->GetVariable, 5, name, &appleGUID, &attr, &data_size, &data);
    Print(L"Status: %d\n", status);
    Print(L"data_size: %d\n", data_size);
    Print(L"attr: %x\n", attr);
    Print(L"char[0]: %d\n", (UINT32)data[0]);

    uefi_call_wrapper(conOut->OutputString, 2, conOut, L"Example text\n");

    UINTN Columns, Rows;
    status = uefi_call_wrapper(conOut->QueryMode, 4, conOut, 0, &Columns, &Rows);
    Print(L"Mode 0, status %d, columnsxrows = %dx%d", status, Columns, Rows);

    EFI_INPUT_KEY key;

    for (;;)
    {
        Print(L"Wait for key...\n");
        getkeystroke(SystemTable, &key);
        Print(L"Got key %c\n", key.UnicodeChar);
	switch (key.UnicodeChar)
	{
            case L'r':
                Print(L"Reboot?");
                break;

            case L's':
                Print(L"Shutdown?");
                break;

            case L'q':
                uefi_call_wrapper(rt->ResetSystem, 4, EfiResetWarm, EFI_SUCCESS, 0, NULL);
                break;
	}
    }

    return EFI_SUCCESS;
}
