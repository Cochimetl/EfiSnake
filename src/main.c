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

  menu_addOption(L"SINGLEPLAYER");
  menu_addOption(L"MULTIPLAYER");
  menu_addOption(L"QUIT");
  menu_draw(SystemTable);

  while(TRUE)
  {
    UINTN selection = 0;
    menu_passControl(SystemTable, &selection);

    if(selection == 0)
    {
      UINTN score = 0;
      snake_singleplayer(SystemTable, &score);
      menu_draw(SystemTable);
    }
    else if(selection == 1)
    {
      UINTN score = 0;
      snake_multiplayer(SystemTable, &score);
      menu_draw(SystemTable);
    }
    else if(selection == 2)
    {
      return EFI_SUCCESS;
    }
  }


  return EFI_SUCCESS;
}
