#include <efi.h>
#include <efilib.h>

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
  UINTN selected;
} gMenuEntries = {{}, 0, 0};

EFI_STATUS menu_addOption(CHAR16 *name)
{
  if(gMenuEntries.usedMenuEntries >= MAX_MENU_ENTRIES) return EFI_OUT_OF_RESOURCES;
  gMenuEntries.entries[gMenuEntries.usedMenuEntries].text = name;
  gMenuEntries.usedMenuEntries++;
  return EFI_SUCCESS;
}

EFI_STATUS menu_removeOptions()
{
  gMenuEntries.usedMenuEntries = 0;
  gMenuEntries.selected = 0;
  return EFI_SUCCESS;
}

EFI_STATUS menu_clearOptions(EFI_SYSTEM_TABLE *SystemTable)
{
  UINTN rows;
  UINTN columns;
  uefi_call_wrapper(SystemTable->ConOut->QueryMode, 4, SystemTable->ConOut, SystemTable->ConOut->Mode->Mode, &columns, &rows);

  for(UINTN i = 0; i < gMenuEntries.usedMenuEntries; i++)
  {
    UINTN rowDistance = (rows - gMenuEntries.usedMenuEntries) / (gMenuEntries.usedMenuEntries + 1);
    uefi_call_wrapper(SystemTable->ConOut->SetAttribute, 2, SystemTable->ConOut, gMenuEntries.selected == i ? gSelectedColour : gTextColour);
    UINTN entryLength = StrLen(gMenuEntries.entries[i].text);
    UINTN columnDistance = (columns - entryLength) / 2;
    uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, columnDistance, (rowDistance * (i + 1)) + i );
    for(UINTN j = 0; j < entryLength; j++)
    {
      uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, L" ");
    }
  }
  return EFI_SUCCESS;
}

EFI_STATUS menu_drawOptions(EFI_SYSTEM_TABLE *SystemTable)
{
  UINTN rows;
  UINTN columns;
  uefi_call_wrapper(SystemTable->ConOut->QueryMode, 4, SystemTable->ConOut, SystemTable->ConOut->Mode->Mode, &columns, &rows);

  for(UINTN i = 0; i < gMenuEntries.usedMenuEntries; i++)
  {
    UINTN rowDistance = (rows - gMenuEntries.usedMenuEntries) / (gMenuEntries.usedMenuEntries + 1);
    uefi_call_wrapper(SystemTable->ConOut->SetAttribute, 2, SystemTable->ConOut, gMenuEntries.selected == i ? gSelectedColour : gTextColour);
    UINTN entryLength = StrLen(gMenuEntries.entries[i].text);
    UINTN columnDistance = (columns - entryLength) / 2;
    uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, columnDistance, (rowDistance * (i + 1)) + i );
    uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, gMenuEntries.entries[i].text);
  }
  return EFI_SUCCESS;
}

EFI_STATUS menu_select(UINTN selection)
{
  if(selection >= gMenuEntries.usedMenuEntries) return EFI_INVALID_PARAMETER;
  gMenuEntries.selected = selection;
  return EFI_SUCCESS;
}

EFI_STATUS menu_draw(EFI_SYSTEM_TABLE *SystemTable)
{
  uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
  menu_drawFrame(SystemTable);
  menu_drawOptions(SystemTable);

  return EFI_SUCCESS;
}
BOOLEAN menu_keysEqual(EFI_INPUT_KEY a, EFI_INPUT_KEY b) { return a.ScanCode == b.ScanCode && a.UnicodeChar == b.UnicodeChar; }

EFI_STATUS menu_passControl(EFI_SYSTEM_TABLE *SystemTable, UINTN *selected)
{
  const EFI_INPUT_KEY KEY_UP = {1, 0};
  const EFI_INPUT_KEY KEY_W = {0, 119};

  const EFI_INPUT_KEY KEY_DOWN = {2, 0};
  const EFI_INPUT_KEY KEY_S = {0, 115};

  const EFI_INPUT_KEY KEY_ENTER = {0, 13};


  uefi_call_wrapper(SystemTable->ConIn->Reset, 2, SystemTable->ConIn, FALSE);
  while(uefi_call_wrapper(SystemTable->BootServices->Stall, 1, 1 * 10 * 1000) == EFI_SUCCESS)
  {
    EFI_INPUT_KEY key;
    while(uefi_call_wrapper(SystemTable->ConIn->ReadKeyStroke, 2, SystemTable->ConIn, &key) == EFI_SUCCESS)
    {
      BOOLEAN change = FALSE;
      if(menu_keysEqual(key, KEY_UP) || menu_keysEqual(key, KEY_W))
      {
        gMenuEntries.selected = gMenuEntries.selected ? gMenuEntries.selected - 1 : gMenuEntries.usedMenuEntries - 1;
        change = TRUE;
      }
      else if(menu_keysEqual(key, KEY_DOWN) || menu_keysEqual(key, KEY_S))
      {
        gMenuEntries.selected = (gMenuEntries.selected + 1) % gMenuEntries.usedMenuEntries;
        change = TRUE;
      }
      else if(menu_keysEqual(key, KEY_ENTER))
      {
        *selected = gMenuEntries.selected;
        return EFI_SUCCESS;
      }

      if(change)
      {
        menu_clearOptions(SystemTable);
        menu_drawOptions(SystemTable);
      }
    }
  }
  return EFI_DEVICE_ERROR;
}

