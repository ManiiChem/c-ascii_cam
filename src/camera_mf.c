#define COBJMACROS

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <stdio.h>
#include <string.h>
#include "camera.h"

static IMFSourceReader *reader = NULL;
static int cam_width = 0;
static int cam_height = 0;

void camera_close (void) {
    if (reader != NULL) {
        IMFSourceReader_Release(reader);
        reader = NULL;
    }
    
    MFShutdown();
    CoUninitialize();
}

bool camera_open (int *width, int *height) {
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (FAILED(hr)) {
        return false;
    }  

    hr = MFStartup(MF_VERSION, MFSTARTUP_FULL);
    if (FAILED(hr)) {
        CoUninitialize();
        return false;
    }

    //printf("Media Foundation started.\n");

    IMFMediaSource *source = NULL;
    IMFAttributes *reader_settings = NULL;
    IMFMediaType *type = NULL;
    UINT64 size = 0;

    IMFAttributes *settings = NULL;
    IMFActivate **devices = NULL;
    UINT32 count = 0;
    bool ok = false;

    // setting up the settings box lol
    hr = MFCreateAttributes(&settings, 1);
    if (FAILED(hr)) goto cleanup;

    // put the camera request setting in it
    hr = IMFAttributes_SetGUID(settings, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(hr)) goto cleanup;

    // ask windows for the list of cameras
    hr = MFEnumDeviceSources(settings, &devices, &count);
    if (FAILED(hr)) goto cleanup;

    // if no camera found 
    if (count == 0) goto cleanup;

    // switch on the first camera
    hr = IMFActivate_ActivateObject(devices[0], &IID_IMFMediaSource, (void **) &source);
    if (FAILED(hr)) goto cleanup;

    // make a reader for it, allowed to convert pixel formats for us
    hr = MFCreateAttributes(&reader_settings, 1);
    if (FAILED(hr)) goto cleanup;

    hr = IMFAttributes_SetUINT32(reader_settings, &MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING, TRUE);
    if (FAILED(hr)) goto cleanup;

    hr = MFCreateSourceReaderFromMediaSource(source, reader_settings, &reader);
    if (FAILED(hr)) goto cleanup;

    // ask for pictures in the NV12 format
    hr = MFCreateMediaType(&type);
    if (FAILED(hr)) goto cleanup;

    hr = IMFMediaType_SetGUID(type, &MF_MT_MAJOR_TYPE, &MFMediaType_Video);
    if (FAILED(hr)) goto cleanup;

    hr = IMFMediaType_SetGUID(type, &MF_MT_SUBTYPE, &MFVideoFormat_NV12); // in NV12 the first width * height bytes are the brightness of each pixel which are the ones i need
    if (FAILED(hr)) goto cleanup;

    hr = IMFSourceReader_SetCurrentMediaType(reader, (DWORD) MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, type);
    if (FAILED(hr)) goto cleanup;

    IMFMediaType_Release(type);
    type = NULL;

    // find out how big the pictures are
    hr = IMFSourceReader_GetCurrentMediaType(reader, (DWORD) MF_SOURCE_READER_FIRST_VIDEO_STREAM, &type);
    if (FAILED(hr)) goto cleanup;

    hr = IMFMediaType_GetUINT64(type, &MF_MT_FRAME_SIZE, &size);
    if (FAILED(hr)) goto cleanup;

    // width is top 32 bit, height is bottom 32 bit so i use sum bitwise ops
    cam_width = (int) (size >> 32);
    cam_height = (int) (size & 0xFFFFFFFF);
    *width = cam_width;
    *height = cam_height;

    ok = true;

cleanup:
    if (type != NULL) {
        IMFMediaType_Release(type);
    }

    if (reader_settings != NULL) {
        IMFAttributes_Release(reader_settings);
    }

    if (source != NULL) {
        IMFMediaSource_Release(source);
    }

    // give back each camera in the list
    for (UINT32 i = 0; i < count; i++) {
        IMFActivate_Release(devices[i]);
    }

    // give back the list itself
    CoTaskMemFree(devices);

    // give back the settings box if we ever got one
    if (settings != NULL) {
        IMFAttributes_Release(settings);
    }

    if (!ok) {
        camera_close();
    }
    
    return ok;
}

bool camera_read_frame (Frame *f) {
    IMFSample *sample = NULL;
    IMFMediaBuffer *buffer = NULL;
    BYTE *data = NULL;
    DWORD length = 0;
    DWORD flags = 0;
    HRESULT hr;
    bool ok = false;

    // the frame must be the same size as the cameras pictures
    if (f->width != cam_width || f->height != cam_height) {
        printf("Error: Frame size does not match the camera.\n");
        return false;
    }

    // sometimes the camera doesnt send stuff instantly so now keep asking for a pic till it arrives
    while (sample == NULL) {
        hr = IMFSourceReader_ReadSample(reader, (DWORD) MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, NULL, &flags, NULL, &sample);

        if (FAILED(hr) || (flags & MF_SOURCE_READERF_ENDOFSTREAM)) {
            return false;
        }
    }

    // get the pictures byte as one block
    hr = IMFSample_ConvertToContiguousBuffer(sample, &buffer);
    if (FAILED(hr)) goto cleanup;

    hr = IMFMediaBuffer_Lock(buffer, &data, NULL, &length);
    if (FAILED(hr)) goto cleanup;

    if (length >= (DWORD) (cam_width * cam_height)) {
        for (int y = 0; y < f->height; y++) {
            uint8_t *frame_row = f->pixels + y * f->stride; // start of row y in our frame
            BYTE *camera_row = data + y * cam_width;        // start of row y in the cameras memory

            memcpy(frame_row, camera_row, f->width);
        }

        ok = true;
    }

    IMFMediaBuffer_Unlock(buffer);

cleanup:
    if (buffer != NULL) {
        IMFMediaBuffer_Release(buffer);
    }

    IMFSample_Release(sample);
    return ok;
}