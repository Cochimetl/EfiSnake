#include <efi.h>
#include "util.h"
#include "menu.h"

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
  menu_draw(SystemTable);
  uefi_call_wrapper(SystemTable->BootServices->Stall, 1, 15 * 1000 * 1000);

  return EFI_SUCCESS;
}
