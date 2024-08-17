#include <efi.h>

UINTN gBackgroundColour = EFI_BLACK;
UINTN gTextColour = EFI_WHITE;
UINTN gSelectedColour = EFI_YELLOW;

struct Frame
{
  CHAR16 symbol;
  CHAR16 zeroTerminator;
  UINTN attribute;
  double distance;
  double linesize;
} gFrame = {'@', '\0', EFI_TEXT_ATTR(EFI_GREEN,EFI_LIGHTGRAY), 0.02, 0.02};

EFI_STATUS menu_drawFrame(EFI_SYSTEM_TABLE *SystemTable)
{
  UINTN rows;
  UINTN columns;
  uefi_call_wrapper(SystemTable->ConOut->QueryMode, 4, SystemTable->ConOut, SystemTable->ConOut->Mode->Mode, &columns, &rows);
  UINTN rowDistance = gFrame.distance * rows;
  UINTN columnDistance = gFrame.distance * columns;
  UINTN rowLinesize = gFrame.linesize * rows;
  UINTN columnLinesize = gFrame.linesize * columns;

  rowLinesize = rowLinesize ? rowLinesize : 1;
  columnLinesize = columnLinesize ? columnLinesize : 1;
  rowDistance = rowDistance ? rowDistance : 1;
  columnDistance = columnDistance ? columnDistance : 1;

  uefi_call_wrapper(SystemTable->ConOut->SetAttribute, 2, SystemTable->ConOut, gFrame.attribute);
  for(UINTN row = rowDistance; row < rows - rowDistance; row++)
  {
    for(UINTN i = 0; i < columnLinesize; i++)
    {
      uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, columnDistance + i, row);
      uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, &gFrame.symbol);
      uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, columns - columnDistance - 1 - i, row);
      uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, &gFrame.symbol);
    }
  }
  for(UINTN column = columnDistance; column < columns - columnDistance; column++)
  {
    for(UINTN i = 0; i < rowLinesize; i++)
    {
      uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, column, rowDistance + i);
      uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, &gFrame.symbol);
      uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, column, rows - rowDistance - 1 - i);
      uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, &gFrame.symbol);
    }
  }
  return EFI_SUCCESS;
}

UINTN gSelected = 0;

struct MenuEntry
{
  UINTN row;
  UINTN column;
  CHAR16 *text;
};

#define MAX_MENU_ENTRIES 5

struct MenuEntries
{
  struct MenuEntry entries[MAX_MENU_ENTRIES];
  UINTN usedMenuEntries;
} gMenuEntries = {{}, 0};

EFI_STATUS menu_addOption(CHAR16 *name)
{
  if(gMenuEntries.usedMenuEntries >= MAX_MENU_ENTRIES) return EFI_OUT_OF_RESOURCES;
  gMenuEntries.entries[gMenuEntries.usedMenuEntries].text = name;
  gMenuEntries.usedMenuEntries++;
  return EFI_SUCCESS;
}

EFI_STATUS menu_clearOptions()
{
  gMenuEntries.usedMenuEntries = 0;
  return EFI_SUCCESS;
}

EFI_STATUS menu_select(UINTN selection)
{
  if(selection >= gMenuEntries.usedMenuEntries) return EFI_INVALID_PARAMETER;
  gSelected = selection;
  return EFI_SUCCESS;
}

EFI_STATUS menu_draw(EFI_SYSTEM_TABLE *SystemTable)
{
  uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
  menu_drawFrame(SystemTable);

  return EFI_SUCCESS;
}

