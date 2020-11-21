#include "EzKb.h"

EFI_STATUS
kbhit (EFI_INPUT_KEY *Key)
{
	return gST->ConIn->ReadKeyStroke (gST->ConIn, Key);
}

EFI_STATUS
getkeystroke (EFI_INPUT_KEY *Key)
{
	gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, 0);
	return gST->ConIn->ReadKeyStroke (gST->ConIn, Key);
}
