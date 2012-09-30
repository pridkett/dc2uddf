/**
 * Create a fake uwatec smart device that we can use for reading back
 * a memory dump
 */
#include <stdlib.h> // malloc, free
#include <libdivecomputer/uwatec_smart.h>
#include <libdivecomputer/device.h>
#include <libdivecomputer/datetime.h>
#include "device-private.h"
#include "fake_uwatec_smart.h"

// copied from device.c
void
device_init (dc_device_t *device, dc_context_t *context, const device_backend_t *backend)
{
    device->backend = backend;

    device->context = context;

    device->event_mask = 0;
    device->event_callback = NULL;
    device->event_userdata = NULL;

    device->cancel_callback = NULL;
    device->cancel_userdata = NULL;

    memset (&device->devinfo, 0, sizeof (device->devinfo));
    memset (&device->clock, 0, sizeof (device->clock));
}

typedef struct uwatec_smart_device_t {
    dc_device_t base;
    unsigned int address;
    unsigned int timestamp;
    unsigned int devtime;
    dc_ticks_t systime;
} uwatec_smart_device_t;

static dc_status_t uwatec_smart_device_set_fingerprint (dc_device_t *device, const unsigned char data[], unsigned int size);
static dc_status_t uwatec_smart_device_version (dc_device_t *abstract, unsigned char data[], unsigned int size);
static dc_status_t uwatec_smart_device_dump (dc_device_t *abstract, dc_buffer_t *buffer);
static dc_status_t uwatec_smart_device_foreach (dc_device_t *abstract, dc_dive_callback_t callback, void *userdata);
static dc_status_t uwatec_smart_device_close (dc_device_t *abstract);

static const device_backend_t uwatec_smart_device_backend = {
    DC_FAMILY_UWATEC_SMART,
    uwatec_smart_device_set_fingerprint, /* set_fingerprint */
    uwatec_smart_device_version, /* version */
    NULL, /* read */
    NULL, /* write */
    uwatec_smart_device_dump, /* dump */
    uwatec_smart_device_foreach, /* foreach */
    uwatec_smart_device_close /* close */
};


dc_status_t *create_fake_uwatec_smart(dc_device_t **out, dc_context_t *context) {
    uwatec_smart_device_t *dev = (uwatec_smart_device_t *)malloc(sizeof(uwatec_smart_device_t));
    if (dev == NULL) {
        ERROR(context, "failed to allocate memory.");
        return DC_STATUS_NOMEMORY;
    }
    device_init(&dev->base, context, &uwatec_smart_device_backend);

    *out = (dc_device_t *) dev;
    return DC_STATUS_SUCCESS;
}
