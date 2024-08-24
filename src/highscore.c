#include <efi.h>
#include <efilib.h>

#include "util.h"

typedef struct
{
  UINTN score;
  CHAR16 name[16];
} Highscore;

Highscore gHighScore[10] = {{1000, L"First"}
                           ,{900, L"Second"}
                           ,{800, L"Third"}
                           ,{700, L"Fourth"}
                           ,{600, L"Fifth"}
                           ,{500, L"Sixth"}
                           ,{400, L"Seventh"}
                           ,{300, L"Eight"}
                           ,{200, L"Ninth"}
                           ,{100, L"Tenth"}};

EFI_STATUS highscore_loadScores(EFI_SYSTEM_TABLE *SystemTable, EFI_HANDLE ImageHandle)
{
  UINTN size = sizeof(gHighScore);
  return util_readFile(SystemTable, ImageHandle, L"highscore", (UINT8*)&gHighScore, &size);
}

EFI_STATUS highscore_saveScores(EFI_SYSTEM_TABLE *SystemTable, EFI_HANDLE ImageHandle)
{
  UINTN size = sizeof(gHighScore);
  return util_writeFile(SystemTable, ImageHandle, L"highscore", (UINT8*)&gHighScore, &size);
}

EFI_STATUS highscore_drawReturnButton(EFI_SYSTEM_TABLE *SystemTable, BOOLEAN active)
{
  uefi_call_wrapper(SystemTable->ConOut->SetAttribute, 2, SystemTable->ConOut, active ? EFI_YELLOW : EFI_WHITE);

  CHAR16 * buttonString = L"RETURN";

  UINTN rows;
  UINTN columns;
  uefi_call_wrapper(SystemTable->ConOut->QueryMode, 4, SystemTable->ConOut, SystemTable->ConOut->Mode->Mode, &columns, &rows);

  UINTN offsetX = (columns - StrLen(buttonString)) / 2;
  UINTN offsetY = rows * 0.85;

  uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, offsetX,  offsetY);
  uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, buttonString);
  return EFI_SUCCESS;
}

EFI_STATUS highscore_drawScores(EFI_SYSTEM_TABLE *SystemTable)
{

  uefi_call_wrapper(SystemTable->ConOut->SetAttribute, 2, SystemTable->ConOut, EFI_BLACK);
  uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
  uefi_call_wrapper(SystemTable->ConOut->SetAttribute, 2, SystemTable->ConOut, EFI_WHITE);

  UINTN rows;
  UINTN columns;
  uefi_call_wrapper(SystemTable->ConOut->QueryMode, 4, SystemTable->ConOut, SystemTable->ConOut->Mode->Mode, &columns, &rows);

  CHAR16 * titleString = L"HIGHSCORE";

  UINTN titleOffsetX = (columns - StrLen(titleString)) / 2;
  UINTN titleOffsetY = rows * 0.15;

  uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, titleOffsetX,  titleOffsetY );
  uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, titleString);

  UINTN scoreOffsetX = columns * 0.4;
  UINTN scoreDistance = columns * 0.2;
  UINTN scoreOffsetY = rows * 0.3;

  for(UINTN i = 0; i < 10; i++)
  {
    uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, scoreOffsetX,  scoreOffsetY + i*2 );
    uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, gHighScore[i].name);
    uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, scoreOffsetX + scoreDistance,  scoreOffsetY + i*2 );
    Print(L"%d", gHighScore[i].score);
  }
  return EFI_SUCCESS;
}

EFI_STATUS highscore_insertScore(EFI_SYSTEM_TABLE *SystemTable, EFI_HANDLE ImageHandle, UINTN score)
{
  if(score < gHighScore[9].score) return EFI_SUCCESS;

  UINTN newScore;
  for(newScore = 0; newScore < 10; newScore++)
  {
    if(gHighScore[newScore].score < score) break;
  }

  for(UINTN i = 8; i >= newScore; i--)
  {
    gHighScore[i + 1].score = gHighScore[i].score;
    StrCpy(gHighScore[i + 1].name, gHighScore[i].name);
  }

  highscore_drawScores(SystemTable);

  UINTN rows;
  UINTN columns;
  uefi_call_wrapper(SystemTable->ConOut->QueryMode, 4, SystemTable->ConOut, SystemTable->ConOut->Mode->Mode, &columns, &rows);

  UINTN scoreOffsetX = columns * 0.4;
  UINTN scoreDistance = columns * 0.2;
  UINTN scoreOffsetY = rows * 0.3 + newScore * 2;

  uefi_call_wrapper(SystemTable->ConOut->SetAttribute, 2, SystemTable->ConOut, EFI_YELLOW);
  uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, scoreOffsetX,  scoreOffsetY );
  uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, L"               ");

  uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, scoreOffsetX + scoreDistance,  scoreOffsetY);
  Print(L"%d", score);

  gHighScore[newScore].score = score;



  const EFI_INPUT_KEY KEY_ENTER = {0, 13};
  const EFI_INPUT_KEY KEY_BACKSPACE = {0, 8};

  UINTN currentNameLength = 0;
  uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, scoreOffsetX,  scoreOffsetY);
  uefi_call_wrapper(SystemTable->ConOut->EnableCursor, 2, SystemTable->ConOut, TRUE);

  uefi_call_wrapper(SystemTable->ConIn->Reset, 2, SystemTable->ConIn, FALSE);
  while(uefi_call_wrapper(SystemTable->BootServices->Stall, 1, 1 * 10 * 1000) == EFI_SUCCESS)
  {
    EFI_INPUT_KEY key;
    while(uefi_call_wrapper(SystemTable->ConIn->ReadKeyStroke, 2, SystemTable->ConIn, &key) == EFI_SUCCESS)
    {
      if(util_keysEqual(key, KEY_ENTER))
      {
        gHighScore[newScore].name[currentNameLength] = '\0';
        uefi_call_wrapper(SystemTable->ConOut->EnableCursor, 2, SystemTable->ConOut, FALSE);
        highscore_saveScores(SystemTable, ImageHandle);
        return EFI_SUCCESS;
      }
      else if(util_keysEqual(key, KEY_BACKSPACE))
      {
        if(currentNameLength > 0)
        {
          currentNameLength--;
          uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, scoreOffsetX + currentNameLength,  scoreOffsetY);
          Print(L" ");
          uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, scoreOffsetX + currentNameLength,  scoreOffsetY);
        }
      }
      else if(key.UnicodeChar && currentNameLength < 15)
      {
        gHighScore[newScore].name[currentNameLength] = key.UnicodeChar;
        Print(L"%c", key.UnicodeChar);
        currentNameLength++;
      }
    }
  }
  return EFI_DEVICE_ERROR;
}

EFI_STATUS highscore_passControl(EFI_SYSTEM_TABLE *SystemTable, EFI_HANDLE ImageHandle, UINTN score)
{
  const EFI_INPUT_KEY KEY_ENTER = {0, 13};

  if(score > gHighScore[9].score) highscore_insertScore(SystemTable, ImageHandle, score);

  highscore_drawScores(SystemTable);
  highscore_drawReturnButton(SystemTable, TRUE);

  uefi_call_wrapper(SystemTable->ConIn->Reset, 2, SystemTable->ConIn, FALSE);
  while(uefi_call_wrapper(SystemTable->BootServices->Stall, 1, 1 * 10 * 1000) == EFI_SUCCESS)
  {
    EFI_INPUT_KEY key;
    while(uefi_call_wrapper(SystemTable->ConIn->ReadKeyStroke, 2, SystemTable->ConIn, &key) == EFI_SUCCESS)
    {
      if(util_keysEqual(key, KEY_ENTER))
      {
        return EFI_SUCCESS;
      }
    }
  }
  return EFI_DEVICE_ERROR;
}
