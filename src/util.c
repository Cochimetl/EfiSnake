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

EFI_STATUS util_openFile(EFI_SYSTEM_TABLE *SystemTable, EFI_HANDLE ImageHandle, CHAR16 *filename, EFI_FILE_HANDLE *file, BOOLEAN write)
{
  EFI_GUID imageProtGuid = (EFI_GUID) EFI_LOADED_IMAGE_PROTOCOL_GUID;
  EFI_LOADED_IMAGE_PROTOCOL *imageProt;
  uefi_call_wrapper(SystemTable->BootServices->OpenProtocol, 6, ImageHandle, &imageProtGuid, &imageProt, ImageHandle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

  EFI_GUID fsGuid = (EFI_GUID) EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fsProt;
  uefi_call_wrapper(SystemTable->BootServices->OpenProtocol, 6, imageProt->DeviceHandle, &fsGuid, &fsProt, ImageHandle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

  EFI_FILE_PROTOCOL *root;
  uefi_call_wrapper(fsProt->OpenVolume, 2, fsProt, &root);

  UINT64 mode = write ? (EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE) : EFI_FILE_MODE_READ;
  EFI_STATUS status = uefi_call_wrapper(root->Open, 5, root, file, filename, mode, 0);

  return status;
}

EFI_STATUS util_readFile(EFI_SYSTEM_TABLE *SystemTable, EFI_HANDLE ImageHandle, CHAR16 *filename, UINT8 *buffer, UINTN *bufferSize)
{
  EFI_FILE_PROTOCOL *file;
  EFI_STATUS status = util_openFile(SystemTable, ImageHandle, filename, &file, FALSE);
  if(status != EFI_SUCCESS) return status;

  uefi_call_wrapper(file->Read, 3, file, bufferSize, buffer);
  uefi_call_wrapper(file->Close, 1, file);
  return EFI_SUCCESS;
}

EFI_STATUS util_writeFile(EFI_SYSTEM_TABLE *SystemTable, EFI_HANDLE ImageHandle, CHAR16 *filename, UINT8 *buffer, UINTN *bufferSize)
{
  EFI_FILE_PROTOCOL *file;
  EFI_STATUS status = util_openFile(SystemTable, ImageHandle, filename, &file, TRUE);
  if(status != EFI_SUCCESS) return status;

  uefi_call_wrapper(file->Write, 3, file, bufferSize, buffer);
  uefi_call_wrapper(file->Close, 1, file);
  return EFI_SUCCESS;
}

