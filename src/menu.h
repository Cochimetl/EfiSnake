#include <efi.h>

EFI_STATUS menu_addOption(CHAR16 *name);

EFI_STATUS menu_removeOptions();

EFI_STATUS menu_drawOptions(EFI_SYSTEM_TABLE *SystemTable);

EFI_STATUS menu_clearOptions(EFI_SYSTEM_TABLE *SystemTable);

EFI_STATUS menu_draw(EFI_SYSTEM_TABLE *SystemTable);

EFI_STATUS menu_passControl(EFI_SYSTEM_TABLE *SystemTable, UINTN *selected);
