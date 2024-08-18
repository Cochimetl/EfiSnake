#include <efi.h>
#include <efilib.h>
#include "util.h"
#include "menu.h"
#include "snake.h"

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
  InitializeLib(ImageHandle, SystemTable);
  uefi_call_wrapper(SystemTable->BootServices->SetWatchdogTimer, 4, 0, 0, 0, NULL);

  util_selectReasonableMode(SystemTable);

  menu_addOption(L"START");
  menu_addOption(L"QUIT");
  menu_draw(SystemTable);

  while(TRUE)
  {
    UINTN selection = 0;
    menu_passControl(SystemTable, &selection);

    if(selection == 0)
    {
      UINTN score = 0;
      snake_passControl(SystemTable, &score);
      menu_clearOptions(SystemTable);
      menu_addOption(L"NONSENSE");
      menu_draw(SystemTable);
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
