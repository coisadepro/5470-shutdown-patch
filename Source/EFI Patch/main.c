#include <efi.h>
#include <efilib.h>

EFI_STATUS Setup(EFI_HANDLE ImageHandle, EFI_LOADED_IMAGE** loaded_image, EFI_HANDLE** device, EFI_FILE** root_dir);
static UINTN file_read(EFI_FILE_HANDLE dir, const CHAR16 *name, CHAR8 **content);
EFI_STATUS file_delete(EFI_FILE* dir, const CHAR16 *name);
void Shutdown();
BOOLEAN ShouldShutdown(EFI_FILE* root_dir);
static EFI_STATUS image_start(EFI_HANDLE ImageHandle, EFI_HANDLE device, CHAR16* image_path);
void Stall();


EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    EFI_LOADED_IMAGE* loaded_image;
    EFI_HANDLE* device;
    EFI_FILE* root_dir;
    EFI_STATUS err;

    err = Setup(ImageHandle, &loaded_image, &device, &root_dir);
    if (EFI_ERROR(err))
        return err;

    if(ShouldShutdown(root_dir))
    {
        Print(L"Shuting down\n");
        Shutdown();
    }
    else
    {
        //Start Clover EFI
        err = image_start(ImageHandle, device, L"\\EFI\\BOOT\\_BOOTX64.efi");
        if(EFI_ERROR(err))
        {
            if (err == EFI_ACCESS_DENIED || err == EFI_SECURITY_VIOLATION)
                Print(L"ERROR: Secure boot is enabled in BIOS. Please disable it.\n");
            else
                Print(L"ERROR LOADING CLOVER EFI\n");
            
            return err;
        }
    }

    uefi_call_wrapper(root_dir->Close, 1, root_dir);
    uefi_call_wrapper(BS->CloseProtocol, 4, ImageHandle, &LoadedImageProtocol, ImageHandle, NULL);
    return EFI_SUCCESS;
}

BOOLEAN ShouldShutdown(EFI_FILE* root_dir){
    CHAR8 *content = NULL;
    UINTN len;
    
    len = file_read(root_dir, L"\\EFI\\.PENDING_SHUTDOWN", &content);
    if(len > 0)
    {
        FreePool(content);

        EFI_STATUS err = file_delete(root_dir, L"\\EFI\\.PENDING_SHUTDOWN");
        if(EFI_ERROR(err) || err == EFI_WARN_DELETE_FAILURE)
            Print(L"ERROR: %r\n", err);
        return TRUE;
    }
    else
    {
        FreePool(content);
        return FALSE;
    }
}

void Shutdown(){
    uefi_call_wrapper(RT -> ResetSystem, 4, EfiResetShutdown, EFI_SUCCESS, 0, NULL);
}

static UINTN file_read(EFI_FILE_HANDLE dir, const CHAR16 *name, CHAR8 **content) {
    EFI_FILE_HANDLE handle;
    EFI_FILE_INFO *info;
    CHAR8 *buf;
    UINTN buflen;
    EFI_STATUS err;
    UINTN len = 0;

    err = uefi_call_wrapper(dir->Open, 5, dir, &handle, name, EFI_FILE_MODE_READ, 0);

    if (EFI_ERROR(err))
        goto out;

    info = LibFileInfo(handle);

    buflen = info->FileSize+1;
    buf = AllocatePool(buflen);

    err = uefi_call_wrapper(handle->Read, 3, handle, &buflen, buf);

    if (EFI_ERROR(err) == EFI_SUCCESS) {
        buf[buflen] = '\0';
        *content = buf;
        len = buflen;
    }
    else
        FreePool(buf);

    FreePool(info);
    uefi_call_wrapper(handle->Close, 1, handle);
out:
    return len;
}

EFI_STATUS file_delete(EFI_FILE* dir, const CHAR16 *name)
{
    EFI_FILE* file;
    EFI_STATUS err = uefi_call_wrapper(dir->Open, 5, dir, &file, name, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
    
    if (EFI_ERROR(err))
        return err;

    return uefi_call_wrapper(file->Delete, 1, file);
}

EFI_STATUS Setup(EFI_HANDLE ImageHandle, EFI_LOADED_IMAGE** loaded_image, EFI_HANDLE** device, EFI_FILE** root_dir){
    EFI_STATUS err;
    
    err = uefi_call_wrapper(BS->OpenProtocol, 6, ImageHandle, &LoadedImageProtocol, loaded_image,
    ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);

    if (EFI_ERROR(err)) {
        Print(L"Error getting a LoadedImageProtocol handle: %r \n", err);
        Stall();
        return err;
    }

    device[0] = loaded_image[0]->DeviceHandle;
    root_dir[0] = LibOpenRoot(device[0]);
    if (!root_dir) {
        Print(L"Unable to open root directory: %r \n", err);
        Stall();
        return EFI_LOAD_ERROR;
    }
    return EFI_SUCCESS;
}

static EFI_STATUS image_start(EFI_HANDLE ImageHandle, EFI_HANDLE device, CHAR16* image_path)
{
    EFI_STATUS err;
    EFI_HANDLE image;
    EFI_DEVICE_PATH* path;

    path = FileDevicePath(device, image_path);
    if (!path) {
            Print(L"Error getting device path.\n");
            Stall();
            return EFI_INVALID_PARAMETER;
    }

    err = uefi_call_wrapper(BS->LoadImage, 6, FALSE, ImageHandle, path, NULL, 0, &image);
    if (EFI_ERROR(err)) {
            Print(L"Error loading %s: %r\n", image_path, err);
            Stall();
            goto out;
    }

    err = uefi_call_wrapper(BS->StartImage, 3, image, NULL, NULL);
    uefi_call_wrapper(BS->UnloadImage, 1, image);

out:
    FreePool(path);
    return err;
}

void Stall(){
    uefi_call_wrapper(BS->Stall, 1, 3 * 1000 * 1000);
}