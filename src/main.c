#include <efi.h>
#include <efilib.h>

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
   InitializeLib(ImageHandle, SystemTable);
   Print(L"Hello, world!\n");
   uefi_call_wrapper(SystemTable->BootServices->Stall, 1, 15 * 1000 * 1000);

   return EFI_SUCCESS;
}
