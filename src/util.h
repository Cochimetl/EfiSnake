#include <efi.h>

EFI_STATUS util_print(EFI_SYSTEM_TABLE *SystemTable, CHAR16 *text);

BOOLEAN util_keysEqual(EFI_INPUT_KEY a, EFI_INPUT_KEY b);
