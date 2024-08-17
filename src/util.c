#include <efi.h>

EFI_STATUS util_print(EFI_SYSTEM_TABLE *SystemTable, CHAR16 *text)
{
  return uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, text);
}

BOOLEAN util_keysEqual(EFI_INPUT_KEY a, EFI_INPUT_KEY b) { return a.ScanCode == b.ScanCode && a.UnicodeChar == b.UnicodeChar; }
