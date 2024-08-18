#include <efi.h>

EFI_STATUS util_print(EFI_SYSTEM_TABLE *SystemTable, CHAR16 *text)
{
  return uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, text);
}

BOOLEAN util_keysEqual(EFI_INPUT_KEY a, EFI_INPUT_KEY b) { return a.ScanCode == b.ScanCode && a.UnicodeChar == b.UnicodeChar; }

EFI_STATUS util_rng(EFI_SYSTEM_TABLE *SystemTable, UINTN *value)
{
  EFI_GUID rngProtocolGuid = (EFI_GUID) EFI_RNG_PROTOCOL_GUID;
  EFI_RNG_PROTOCOL *rngProtocol;

  EFI_STATUS status = uefi_call_wrapper(SystemTable->BootServices->LocateProtocol, 3, &rngProtocolGuid, NULL, &rngProtocol);

  if(status == EFI_NOT_FOUND) // Virtualbox EFI emulation doesn't offer RNG
  {
    static UINTN counter = 1;
    *value = 0x12345 * counter++;
    return EFI_SUCCESS;
  }

  uefi_call_wrapper(rngProtocol->GetRNG, 4, rngProtocol, NULL, sizeof(UINTN), value);
  return EFI_SUCCESS;
}

EFI_STATUS util_selectReasonableMode(EFI_SYSTEM_TABLE *SystemTable)
{
  UINTN rows;
  UINTN columns;
  UINTN bestRows = 0xFFFF;
  UINTN bestColumns = 0xFFFF;
  UINTN bestMode = 0;
  for(UINTN i = 0; i <= SystemTable->ConOut->Mode->MaxMode; i++)
  {
    uefi_call_wrapper(SystemTable->ConOut->QueryMode, 4, SystemTable->ConOut, i, &columns, &rows);
    if(rows <= bestRows && rows >= 40 && columns <= bestColumns && columns >= 100)
    {
      bestMode = i;
      bestRows = rows;
      bestColumns = columns;
    }
  }

  uefi_call_wrapper(SystemTable->ConOut->SetMode, 2, SystemTable->ConOut, bestMode);
  return EFI_SUCCESS;
}
