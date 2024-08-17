#include <efi.h>
#include "util.h"
#include "menu.h"

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
  menu_addOption(L"START");
  menu_addOption(L"QUIT");
  menu_draw(SystemTable);

  while(TRUE)
  {
    UINTN selection = 0;
    menu_passControl(SystemTable, &selection);

    if(selection == 0)
    {
      menu_clearOptions(SystemTable);
      menu_addOption(L"NONSENSE");
      menu_drawOptions(SystemTable);
    }
    else if(selection == 1)
    {
      return EFI_SUCCESS;
    }
    else
    {
      menu_clearOptions(SystemTable);
      menu_removeOptions();
      menu_addOption(L"START");
      menu_addOption(L"QUIT");
      menu_drawOptions(SystemTable);
    }
  }


  return EFI_SUCCESS;
}
