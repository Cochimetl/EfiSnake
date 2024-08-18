#include <efi.h>

EFI_STATUS util_print(EFI_SYSTEM_TABLE *SystemTable, CHAR16 *text);

BOOLEAN util_keysEqual(EFI_INPUT_KEY a, EFI_INPUT_KEY b);

EFI_STATUS util_rng(EFI_SYSTEM_TABLE *SystemTable, UINTN *value);

EFI_STATUS util_selectReasonableMode(EFI_SYSTEM_TABLE *SystemTable);
