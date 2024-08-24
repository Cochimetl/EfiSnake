#include <efi.h>
#include <efilib.h>
#include "util.h"
#include "menu.h"
#include "highscore.h"
#include "snake.h"

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
  InitializeLib(ImageHandle, SystemTable);
  uefi_call_wrapper(SystemTable->BootServices->SetWatchdogTimer, 4, 0, 0, 0, NULL);

  util_selectReasonableMode(SystemTable);

  menu_addOption(L"SINGLEPLAYER");
  menu_addOption(L"MULTIPLAYER");
  menu_addOption(L"HIGHSCORE");
  menu_addOption(L"QUIT");
  
  highscore_loadScores(SystemTable, ImageHandle);

  while(TRUE)
  {
    UINTN selection = 0;
    menu_draw(SystemTable);
    menu_passControl(SystemTable, &selection);

    if(selection == 0)
    {
      UINTN score = 0;
      snake_singleplayer(SystemTable, &score);
      highscore_passControl(SystemTable, ImageHandle, score);
    }
    else if(selection == 1)
    {
      UINTN score = 0;
      snake_multiplayer(SystemTable, &score);
      highscore_passControl(SystemTable, ImageHandle, score);
    }
    else if(selection == 2)
    {
      highscore_passControl(SystemTable, ImageHandle, 0);
    }
    else if(selection == 3)
    {
      return EFI_SUCCESS;
    }
  }


  return EFI_SUCCESS;
}
