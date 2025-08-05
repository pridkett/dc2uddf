/*
 ============================================================================
 Name        : dc2uddf.c
 Author      : Patrick Wagstrom
 Version     :
 Copyright   : Copyright (c) 2012-2013 Patrick Wagstrom
               Based on code originally Copyright (C) 2009 Jef Driesen
 Description : A simple tool that utilizes libdivecomputer to download data
               from a dive computer and generate a UDDF file
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <libdivecomputer/context.h>
#include <libdivecomputer/common.h>
#include <libdivecomputer/parser.h>
#include <libdivecomputer/device.h>
#include <libdivecomputer/descriptor.h>
#include <libdivecomputer/iostream.h>
#include <libdivecomputer/serial.h>
#include <libdivecomputer/irda.h>
#include <libdivecomputer/usbhid.h>
#include <libdivecomputer/bluetooth.h>

#include "utils.h"
#include "dif/dif.h"

static const char *g_cachedir = NULL;
static int g_cachedir_read = 1;

typedef struct program_options_t {
    gchar *backend;
    gchar *devname;
    gchar *xmlfile;
    gchar *logfile;
    guchar truncateDives;
    guchar initialPressureFix;
    guchar dumpDives;
    guchar dumpMemory;
    guchar useInvalidElements;
} program_options_t;

typedef struct dive_data_t {
    dc_device_t *device;
    unsigned int number;
    dc_buffer_t *fingerprint;
    dif_dive_collection_t *dc;
} dive_data_t;

typedef struct device_data_t {
    dc_event_devinfo_t devinfo;
    dc_event_clock_t clock;
} device_data_t;

typedef struct sample_cb_data_t {
    dif_dive_t *dive;
    dif_sample_t *sample;
} sample_cb_data_t;

#ifdef _WIN32
#define DC_TICKS_FORMAT "%I64d"
#else
#define DC_TICKS_FORMAT "%lld"
#endif

typedef struct backend_table_t {
    const char *name;
    dc_family_t type;
} backend_table_t;

static const backend_table_t g_backends[] = {
        {"solution",    DC_FAMILY_SUUNTO_SOLUTION},
        {"eon",         DC_FAMILY_SUUNTO_EON},
        {"vyper",       DC_FAMILY_SUUNTO_VYPER},
        {"vyper2",      DC_FAMILY_SUUNTO_VYPER2},
        {"d9",          DC_FAMILY_SUUNTO_D9},
        {"aladin",      DC_FAMILY_UWATEC_ALADIN},
        {"memomouse",   DC_FAMILY_UWATEC_MEMOMOUSE},
        {"smart",       DC_FAMILY_UWATEC_SMART},
        {"sensus",      DC_FAMILY_REEFNET_SENSUS},
        {"sensuspro",   DC_FAMILY_REEFNET_SENSUSPRO},
        {"sensusultra", DC_FAMILY_REEFNET_SENSUSULTRA},
        {"vtpro",       DC_FAMILY_OCEANIC_VTPRO},
        {"veo250",      DC_FAMILY_OCEANIC_VEO250},
        {"atom2",       DC_FAMILY_OCEANIC_ATOM2},
        {"nemo",        DC_FAMILY_MARES_NEMO},
        {"puck",        DC_FAMILY_MARES_PUCK},
        {"darwin",      DC_FAMILY_MARES_DARWIN},
        {"iconhd",      DC_FAMILY_MARES_ICONHD},
        {"ostc",        DC_FAMILY_HW_OSTC},
        {"frog",        DC_FAMILY_HW_FROG},
        {"edy",         DC_FAMILY_CRESSI_EDY},
        {"n2ition3",    DC_FAMILY_ZEAGLE_N2ITION3},
        {"cobalt",      DC_FAMILY_ATOMICS_COBALT}
};

static dc_family_t
lookup_type (const char *name) {
    unsigned int i = 0;
    unsigned int nbackends = sizeof(g_backends)/sizeof(g_backends[0]);
    for (i = 0; i < nbackends; ++i) {
        if (strcmp(name, g_backends[i].name) == 0) {
            return g_backends[i].type;
        }
    }

    return DC_FAMILY_NULL;
}

static unsigned char
hex2dec (unsigned char value) {
    if (value >= '0' && value <= '9') {
        return value - '0';
    } else if (value >= 'A' && value <= 'F') {
        return value - 'A' + 10;
    } else if (value >= 'a' && value <= 'f') {
        return value - 'a' + 10;
    } else {
        return 0;
    }
}

static dc_buffer_t *
fpconvert (const char *fingerprint) {
    unsigned int i = 0;

    /* get the length of the fingerprint data */
    size_t nbytes = (fingerprint ? strlen(fingerprint) / 2 : 0);
    if (nbytes == 0) {
        return NULL;
    }

    /* allocate a memory buffer */
    dc_buffer_t *buffer = dc_buffer_new(nbytes);

    /* convert the hexadecimal string */
    for (i = 0; i < nbytes; ++i) {
        unsigned char msn = hex2dec(fingerprint[i*2+0]);
        unsigned char lsn = hex2dec(fingerprint[i*2+1]);
        unsigned char byte = (msn << 4) + lsn;

        dc_buffer_append(buffer, &byte, 1);
    }

    return buffer;
}



dif_sample_event_t
dc_to_dif_event(parser_sample_event_t ev) {
    switch(ev) {
    case SAMPLE_EVENT_NONE:
        return DIF_SAMPLE_EVENT_NONE;
    case SAMPLE_EVENT_DECOSTOP:
        return DIF_SAMPLE_EVENT_DECOSTOP;
    case SAMPLE_EVENT_RBT:
        return DIF_SAMPLE_EVENT_RBT;
    case SAMPLE_EVENT_ASCENT:
        return DIF_SAMPLE_EVENT_ASCENT;
    case SAMPLE_EVENT_CEILING:
        return DIF_SAMPLE_EVENT_CEILING;
    case SAMPLE_EVENT_WORKLOAD:
        return DIF_SAMPLE_EVENT_WORKLOAD;
    case SAMPLE_EVENT_TRANSMITTER:
        return DIF_SAMPLE_EVENT_TRANSMITTER;
    case SAMPLE_EVENT_VIOLATION:
        return DIF_SAMPLE_EVENT_VIOLATION;
    case SAMPLE_EVENT_BOOKMARK:
        return DIF_SAMPLE_EVENT_BOOKMARK;
    case SAMPLE_EVENT_SURFACE:
        return DIF_SAMPLE_EVENT_SURFACE;
    case SAMPLE_EVENT_SAFETYSTOP:
        return DIF_SAMPLE_EVENT_SAFETYSTOP;
    case SAMPLE_EVENT_GASCHANGE:
        return DIF_SAMPLE_EVENT_GASCHANGE;
    case SAMPLE_EVENT_SAFETYSTOP_VOLUNTARY:
        return DIF_SAMPLE_EVENT_SAFETYSTOP_VOLUNTARY;
    case SAMPLE_EVENT_SAFETYSTOP_MANDATORY:
        return DIF_SAMPLE_EVENT_SAFETYSTOP_MANDATORY;
    case SAMPLE_EVENT_DEEPSTOP:
        return DIF_SAMPLE_EVENT_DEEPSTOP;
    case SAMPLE_EVENT_CEILING_SAFETYSTOP:
        return DIF_SAMPLE_EVENT_CEILING_SAFETYSTOP;
    case SAMPLE_EVENT_UNKNOWN:
        return DIF_SAMPLE_EVENT_UNKNOWN;
    case SAMPLE_EVENT_DIVETIME:
        return DIF_SAMPLE_EVENT_DIVETIME;
    case SAMPLE_EVENT_MAXDEPTH:
        return DIF_SAMPLE_EVENT_MAXDEPTH;
    case SAMPLE_EVENT_OLF:
        return DIF_SAMPLE_EVENT_OLF;
    case SAMPLE_EVENT_PO2:
        return DIF_SAMPLE_EVENT_PO2;
    case SAMPLE_EVENT_AIRTIME:
        return DIF_SAMPLE_EVENT_AIRTIME;
    case SAMPLE_EVENT_RGBM:
        return DIF_SAMPLE_EVENT_RGBM;
    case SAMPLE_EVENT_HEADING:
        return DIF_SAMPLE_EVENT_HEADING;
    case SAMPLE_EVENT_TISSUELEVEL:
        return DIF_SAMPLE_EVENT_TISSUELEVEL;
    case SAMPLE_EVENT_GASCHANGE2:
        return DIF_SAMPLE_EVENT_GASCHANGE2;
    // case SAMPLE_EVENT_NDL:
    //    return DIF_SAMPLE_EVENT_NDL;
    }
    WARNING("Unknown libdivecomputer sample event");
    return DIF_SAMPLE_EVENT_UNKNOWN;
}

void
sample_cb (dc_sample_type_t type, const dc_sample_value_t *value, void *userdata) {
    sample_cb_data_t *udata = (sample_cb_data_t *) userdata;
    dif_dive_t *dive = udata->dive;
    dif_subsample_t *subsample = NULL;
    dif_sample_t *sample = udata->sample;

    switch (type) {
    case DC_SAMPLE_TIME:
        sample = dif_sample_alloc();
        udata->sample = sample;
        dive = dif_dive_add_sample(dive, sample);
        sample->timestamp = value->time;
        break;
    case DC_SAMPLE_DEPTH:
        subsample = dif_subsample_alloc();
        subsample->type = DIF_SAMPLE_DEPTH;
        subsample->value.depth = value->depth;
        sample = dif_sample_add_subsample(sample, subsample);
        break;
    case DC_SAMPLE_PRESSURE:
        subsample = dif_subsample_alloc();
        subsample->type = DIF_SAMPLE_PRESSURE;
        subsample->value.pressure.tank = value->pressure.tank;
        subsample->value.pressure.value = value->pressure.value;
        sample = dif_sample_add_subsample(sample, subsample);
        break;
    case DC_SAMPLE_TEMPERATURE:
        subsample = dif_subsample_alloc();
        subsample->type = DIF_SAMPLE_TEMPERATURE;
        subsample->value.temperature = value->temperature;
        sample = dif_sample_add_subsample(sample, subsample);
        break;
    case DC_SAMPLE_EVENT:
        subsample = dif_subsample_alloc();
        subsample->type = DIF_SAMPLE_EVENT;
        subsample->value.event.type = dc_to_dif_event(value->event.type);
        subsample->value.event.time = value->event.time;
        subsample->value.event.flags = value->event.flags;
        subsample->value.event.value = value->event.value;
        sample = dif_sample_add_subsample(sample, subsample);
        break;
    case DC_SAMPLE_RBT:
        subsample = dif_subsample_alloc();
        subsample->type = DIF_SAMPLE_RBT;
        // TODO: my computer reports RBT in minutes, so we multiply here
        subsample->value.rbt = value->rbt*60;
        sample = dif_sample_add_subsample(sample, subsample);
        break;
    case DC_SAMPLE_HEARTBEAT:
        subsample = dif_subsample_alloc();
        subsample->type = DIF_SAMPLE_HEARTBEAT;
        subsample->value.heartbeat = value->heartbeat;
        sample = dif_sample_add_subsample(sample, subsample);
        break;
    case DC_SAMPLE_BEARING:
        subsample = dif_subsample_alloc();
        subsample->type = DIF_SAMPLE_BEARING;
        subsample->value.bearing = value->bearing;
        sample = dif_sample_add_subsample(sample, subsample);
        break;
    case DC_SAMPLE_VENDOR:
        subsample = dif_subsample_alloc();
        subsample->type = DIF_SAMPLE_VENDOR;
        subsample->value.vendor.type = value->vendor.type;
        subsample->value.vendor.size = value->vendor.size;
        subsample->value.vendor.data = value->vendor.data;
        sample = dif_sample_add_subsample(sample, subsample);
        break;
    default:
        WARNING("Unknown sample type");
        break;
    }
}

static dc_status_t
doparse(dif_dive_collection_t *dc, dc_device_t *device, const unsigned char data[], unsigned int size) {
    dif_dive_t *dive = NULL;
    unsigned int i = 0;

    /* allocate the dive */
    message("allocating the dive\n");
    dive = dif_dive_alloc();
    if (dive == NULL || dc == NULL) {
        WARNING("Error creating the dive object");
        return DC_STATUS_NOMEMORY;
    }
    dc = dif_dive_collection_add_dive(dc, dive);

    /* create the parser */
    message("Creating the parser.\n");
    dc_parser_t *parser = NULL;
    dc_status_t rc = dc_parser_new(&parser, device, data, size);
    if (rc != DC_STATUS_SUCCESS) {
        WARNING("Error creating the parser.");
        return rc;
    }

    /* parse the datetime of the dive*/
    message("Parsing the datetime.\n");
    dc_datetime_t dt = {0};
    rc = dc_parser_get_datetime(parser, &dt);
    if (rc != DC_STATUS_SUCCESS && rc != DC_STATUS_UNSUPPORTED) {
        WARNING("Error parsing the datetime.");
        dc_parser_destroy(parser);
        return rc;
    }

    dive = dif_dive_set_datetime(dive, dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);

    /* parse the divetime - in seconds */
    message("Parsing the divetime.\n");
    unsigned int divetime = 0;
    rc = dc_parser_get_field(parser, DC_FIELD_DIVETIME, 0, &divetime);
    if (rc != DC_STATUS_SUCCESS && rc != DC_STATUS_UNSUPPORTED) {
        WARNING("Error parsing the divetime.");
        dc_parser_destroy(parser);
        return rc;
    }
    dive = dif_dive_set_duration(dive, divetime);

    /* parse the maximum depth */
    message("Parsing the maximum depth.\n");
    double maxdepth = 0.0;
    rc = dc_parser_get_field(parser, DC_FIELD_MAXDEPTH, 0, &maxdepth);
    if (rc != DC_STATUS_SUCCESS && rc != DC_STATUS_UNSUPPORTED) {
        WARNING("Error parsing the maximum depth.");
        dc_parser_destroy(parser);
        return rc;
    }
    dive = dif_dive_set_maxdepth(dive, maxdepth);

    /* parse the gas mixes */
    message("Parsing the gas mixes.\n");
    unsigned int ngases = 0;
    rc = dc_parser_get_field(parser, DC_FIELD_GASMIX_COUNT, 0, &ngases);
    if (rc != DC_STATUS_SUCCESS && rc != DC_STATUS_UNSUPPORTED) {
        WARNING("Error parsing the gas mix count.");
        dc_parser_destroy(parser);
        return rc;
    }

    for (i=0; i < ngases; ++i) {
        dc_gasmix_t gasmix = {0};
        rc = dc_parser_get_field(parser, DC_FIELD_GASMIX, i, &gasmix);
        if (rc != DC_STATUS_SUCCESS && rc != DC_STATUS_UNSUPPORTED) {
            WARNING("Error parsing the gas mix.");
            dc_parser_destroy(parser);
            return rc;
        }
        dif_gasmix_t *mix = dif_gasmix_alloc();
        if (mix == NULL) {
            WARNING("Error allocating gas mix object.");
            return DC_STATUS_NOMEMORY;
        }
        mix->id = i;
        mix->helium = gasmix.helium * 100;
        mix->oxygen = gasmix.oxygen * 100;
        mix->nitrogen = gasmix.nitrogen * 100;
        if (dif_gasmix_is_valid(mix)) {
            dive = dif_dive_add_gasmix(dive, mix);
        } else {
            dif_gasmix_free(mix);
        }
    }

    /* parse the sample data */
    message("Parsing the sample data.\n");
    sample_cb_data_t *cbdata = g_malloc(sizeof(sample_cb_data_t));
    cbdata->dive = dive;
    cbdata->sample = NULL;

    rc = dc_parser_samples_foreach(parser, sample_cb, cbdata);
    if (rc != DC_STATUS_SUCCESS) {
        WARNING("Error parsing the sample data.");
        dc_parser_destroy(parser);
        return rc;
    }

    g_free(cbdata);

    /* destroy the parser */
    message("Destroying the parser.\n");
    rc = dc_parser_destroy(parser);
    if (rc != DC_STATUS_SUCCESS) {
        WARNING("Error destroying the parser.");
        return rc;
    }

    return DC_STATUS_SUCCESS;
}

static int
dive_cb (const unsigned char *data, unsigned int size, const unsigned char *fingerprint, unsigned int fsize, void *userdata) {
    unsigned int i;
    dive_data_t *divedata = (dive_data_t *) userdata;
    dif_dive_collection_t *dc = divedata->dc;
    divedata->number++;

    message("Dive: number=%u, size=%u, fingerprint=", divedata->number, size);
    for (i = 0; i < fsize; ++i) {
        message("%02X", fingerprint[i]);
    }
    message("\n");

    doparse(dc, divedata->device, data, size);
    return 1;
}

static void
event_cb (dc_device_t *device, dc_event_type_t event, const void *data, void *userdata)
{
    const dc_event_progress_t *progress = (dc_event_progress_t *) data;
    const dc_event_devinfo_t *devinfo = (dc_event_devinfo_t *) data;
    const dc_event_clock_t *clock = (dc_event_clock_t *) data;

    device_data_t *devdata = (device_data_t *) userdata;

    switch (event) {
    case DC_EVENT_WAITING:
        message("Event: waiting for user action\n");
        break;
    case DC_EVENT_PROGRESS:
        message("Event: progress %3.2f%% (%u/%u)\n",
                100.0 * (double) progress->current / (double) progress->maximum,
                progress->current, progress->maximum);
        break;
    case DC_EVENT_DEVINFO:
        devdata->devinfo = *devinfo;
        message("Event: model=%u (0x%08x), firmware=%u (0x%08x), serial=%u (0x%08x)\n",
                devinfo->model, devinfo->model,
                devinfo->firmware, devinfo->firmware,
                devinfo->serial, devinfo->serial);
//        if (g_cachedir && g_cachedir_read) {
//            dc_buffer_t *fingerprint = fpread(g_cachedir, dc_device_get_type (device), devinfo->serial);
//            dc_device_set_fingerprint(device,
//                    dc_buffer_get_data(fingerprint),
//                    dc_buffer_get_size(fingerprint));
//            dc_buffer_free(fingerprint);
//        }
        break;
    case DC_EVENT_CLOCK:
        devdata->clock = *clock;
        message ("Event: systime=" DC_TICKS_FORMAT ", devtime=%u\n",
            clock->systime, clock->devtime);
        break;
    default:
        break;
    }
}

volatile sig_atomic_t g_cancel = 0;
void
sighandler (int signum)
{
#ifndef _WIN32
    // Restore the default signal handler.
    signal (signum, SIG_DFL);
#endif

    g_cancel = 1;
}

static int
cancel_cb (void *userdata) {
    return g_cancel;
}

static dc_status_t
dowork(dc_context_t *context, dc_descriptor_t *descriptor, program_options_t *options, dc_buffer_t *fingerprint) {
    dc_status_t rc = DC_STATUS_SUCCESS;

    /* initialize the device data */
    device_data_t devdata = {{0}};

    /* open the device */
    message("Opening the device (%s, %s, %s.\n",
            dc_descriptor_get_vendor(descriptor),
            dc_descriptor_get_product(descriptor),
            options->devname ? options->devname : "null");
    dc_device_t *device = NULL;
    /* Determine the transport type */
    dc_transport_t transports = dc_descriptor_get_transports(descriptor);
    dc_iostream_t *iostream = NULL;
    
    if (transports & DC_TRANSPORT_SERIAL) {
        rc = dc_serial_open(&iostream, context, options->devname);
    } else if (transports & DC_TRANSPORT_IRDA) {
        /* For IrDA, we need to parse the address from the device name */
        unsigned int address = 0;
        if (options->devname) {
            address = strtoul(options->devname, NULL, 0);
        }
        rc = dc_irda_open(&iostream, context, address, 1);
    } else if (transports & DC_TRANSPORT_USBHID) {
        /* For USB HID, we would need to enumerate devices first */
        WARNING("USB HID transport not yet implemented.");
        return DC_STATUS_UNSUPPORTED;
    } else if (transports & DC_TRANSPORT_BLUETOOTH) {
        /* For Bluetooth, we would need to parse the address */
        WARNING("Bluetooth transport not yet implemented.");
        return DC_STATUS_UNSUPPORTED;
    } else {
        WARNING("No supported transport found.");
        return DC_STATUS_UNSUPPORTED;
    }
    
    if (rc != DC_STATUS_SUCCESS) {
        WARNING("Error opening the I/O stream.");
        return rc;
    }
    
    rc = dc_device_open(&device, context, descriptor, iostream);
    if (rc != DC_STATUS_SUCCESS) {
        WARNING("Error opening device.");
        dc_iostream_close(iostream);
        return rc;
    }

    /* register the event handler */
    message("Registering the event handler.\n");
    int events = DC_EVENT_WAITING | DC_EVENT_PROGRESS | DC_EVENT_DEVINFO | DC_EVENT_CLOCK;
    rc = dc_device_set_events(device, events, event_cb, &devdata);
    if (rc != DC_STATUS_SUCCESS) {
        WARNING("Error registering the event handler.");
        dc_device_close(device);
        dc_iostream_close(iostream);
        return rc;
    }

    /* register the cancellation handler */
    message("Registering the cancellation handler.\n");
    rc = dc_device_set_cancel(device, cancel_cb, NULL);
    if (rc != DC_STATUS_SUCCESS) {
        WARNING("Error registering the cancellation handler.");
        dc_device_close(device);
        dc_iostream_close(iostream);
        return rc;
    }

    /* register the fingerprint data */
    if (fingerprint) {
        message("Registering the fingerprint data.\n");
        rc = dc_device_set_fingerprint(device, dc_buffer_get_data(fingerprint), dc_buffer_get_size(fingerprint));
        if (rc != DC_STATUS_SUCCESS) {
            WARNING("Error registerting the fingerprint data");
            dc_device_close(device);
            dc_iostream_close(iostream);
            return rc;
        }
    }

    /* dump the memory if requested */
    if (options->dumpMemory) {
        WARNING("Memory dump not enabled.");
    }

    /* dump the dives if requested */
    if (options->dumpDives) {
        /* initialize the dive data */
        dive_data_t divedata = {0};
        dif_dive_collection_t *dc = dif_dive_collection_alloc();

        divedata.device = device;
        divedata.fingerprint = NULL;
        divedata.number = 0;
        divedata.dc = dc;
        /* download the dives */
        message("Downloading the dives.\n");
        rc = dc_device_foreach(device, dive_cb, &divedata);
        if (rc != DC_STATUS_SUCCESS) {
            WARNING("Error downloading the dives.");
            dc_buffer_free(divedata.fingerprint);
            dc_device_close (device);
            dc_iostream_close(iostream);
            return rc;
        }

        xml_options_t *xmlOptions = dif_xml_options_alloc();
        xmlOptions->filename = options->xmlfile;
        xmlOptions->useInvalidElements = options->useInvalidElements;

        if (options->truncateDives) {
            divedata.dc = dif_alg_dc_truncate_dives(divedata.dc);
        }

        if (options->initialPressureFix) {
            divedata.dc = dif_alg_dc_initial_pressure_fix(divedata.dc);
        }
        dif_save_dive_collection_uddf_options(divedata.dc, xmlOptions);

        /* free the fingerprint buffer */
        dc_buffer_free(divedata.fingerprint);
        dif_dive_collection_free(divedata.dc);
        dif_xml_options_free(xmlOptions);
    }

    /* close the device */
    message("Closing the device.\n");
    rc = dc_device_close(device);
    if (rc != DC_STATUS_SUCCESS) {
        WARNING("Error closing the device.");
        dc_iostream_close(iostream);
        return rc;
    }

    /* close the I/O stream */
    dc_iostream_close(iostream);

    return DC_STATUS_SUCCESS;
}

static dc_status_t
search(dc_descriptor_t **out, const char *name, dc_family_t backend, unsigned int model) {
    dc_status_t rc = DC_STATUS_SUCCESS;

    dc_iterator_t *iterator = NULL;
    rc = dc_descriptor_iterator(&iterator);
    if (rc != DC_STATUS_SUCCESS) {
        WARNING("Error creating the device descriptor iterator.");
        return rc;
    }

    dc_descriptor_t *descriptor = NULL, *current = NULL;
    while ((rc = dc_iterator_next(iterator, &descriptor)) == DC_STATUS_SUCCESS) {
        if (name) {
            const char *vendor = dc_descriptor_get_vendor(descriptor);
            const char *product = dc_descriptor_get_product(descriptor);

            size_t n = strlen(vendor);
            if (strncasecmp(name, vendor, n) == 0 && name[n] == ' ' &&
                    strcasecmp(name + n + 1, product) == 0) {
                current = descriptor;
                break;
            } else if (strcasecmp(name, product) == 0) {
                current = descriptor;
                break;
            }
        } else {
            if (backend == dc_descriptor_get_type(descriptor)) {
                if (model == dc_descriptor_get_model(descriptor)) {
                    /* Exact match found. Return immediately */
                    dc_descriptor_free(current);
                    current = descriptor;
                    break;
                } else {
                    /* Possible match found. Keep searching for an exact match.
                     * If no exact match is found, return the first match found.
                     */
                    if (current == NULL) {
                        current = descriptor;
                        descriptor = NULL;
                    }
                }
            }
        }

        dc_descriptor_free(descriptor);
    }

    if (rc != DC_STATUS_SUCCESS && rc != DC_STATUS_DONE) {
        dc_descriptor_free(current);
        dc_iterator_free(iterator);
        WARNING("Error iterating the device descriptor.");
        return rc;
    }

    dc_iterator_free(iterator);

    *out = current;

    return DC_STATUS_SUCCESS;
}

int dump_dives(program_options_t *options) {
    dc_family_t backend = DC_FAMILY_NULL;
    dc_loglevel_t loglevel = DC_LOGLEVEL_WARNING;
    const char *logfile = "output.log";
    const char *name = NULL;
    const char *fingerprint = NULL;
    unsigned int model = 0;

    if (options->backend != NULL) {
        backend = lookup_type(options->backend);
    }
    signal (SIGINT, sighandler);

    message_set_logfile(logfile);

    dc_context_t *context = NULL;

    /* create a new context */
    dc_status_t rc = dc_context_new(&context);
    if (rc != DC_STATUS_SUCCESS) {
        message_set_logfile(NULL);
        return EXIT_FAILURE;
    }

    dc_context_set_loglevel(context, loglevel);
    dc_context_set_logfunc(context, logfunc, NULL);

    dc_descriptor_t *descriptor = NULL;
    rc = search(&descriptor, name, backend, model);
    if (rc != DC_STATUS_SUCCESS) {
        message_set_logfile(NULL);
        return EXIT_FAILURE;
    }

    /* fail if no device descriptor found */
    if (descriptor == NULL) {
        WARNING("No matching device found");
        /* FIXME: bail out to usage information */
        message_set_logfile(NULL);
        return EXIT_FAILURE;
    }

    dc_buffer_t *fp = fpconvert(fingerprint);
    rc = dowork(context, descriptor, options, fp);
    dc_buffer_free(fp);
    /* FIXME: why aren't calls to errmsg working? */
    // message("Result: %s\n", errmsg(rc));

    dc_descriptor_free(descriptor);
    dc_context_free(context);

    message_set_logfile(NULL);

    return rc != DC_STATUS_SUCCESS ? EXIT_FAILURE : EXIT_SUCCESS;
}

void usage() {
    fprintf(stderr, "usage: dc2uddf -b BACKEND -d DEVNAME [options]\n");
    fprintf(stderr, "example: dc2uddf -b smart -d \"Uwatec Galileo\"\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "arguments:\n");
    fprintf(stderr, "  -b,--backend BACKEND: use backend called BACKEND\n");
    fprintf(stderr, "  -d,--device DEVICE: use device called DEVICE\n");
    fprintf(stderr, "  -o,--output UDDFFILE: save UDDF to file called UDDFFILE\n");
    fprintf(stderr, "  -i,--ipf: calculate initial pressure fix\n");
    fprintf(stderr, "  -t,--truncate: truncate dives after surfacing\n");
    fprintf(stderr, "  --invalid: add invalid <event> and <vendor> tags to assist debugging\n");
    fprintf(stderr, "  --listbackends: print all the backends\n");
    fprintf(stderr, "  --listdevices: print all the devices\n");
    fprintf(stderr, "  -h,--help: print this help screen\n");
    exit(EXIT_FAILURE);
}

void print_backends()
{
    unsigned int i;
    fprintf(stderr, "Supported backends:\n\n");
    unsigned int nbackends = sizeof (g_backends) / sizeof (g_backends[0]);
    for (i = 0; i < nbackends; ++i) {
        fprintf(stderr, "  %s\n", g_backends[i].name);
    }
    exit(EXIT_FAILURE);
}

void
print_devices()
{
    fprintf(stderr, "Supported devices:\n\n");
    dc_iterator_t *iterator = NULL;
    dc_descriptor_t *descriptor = NULL;
    dc_descriptor_iterator (&iterator);
    while (dc_iterator_next (iterator, &descriptor) == DC_STATUS_SUCCESS) {
        fprintf (stderr, "   %s %s\n",
            dc_descriptor_get_vendor (descriptor),
            dc_descriptor_get_product (descriptor));
        dc_descriptor_free (descriptor);
    }
    dc_iterator_free (iterator);
    exit(EXIT_FAILURE);
}

int
main(int argc, char **argv)
{
    int opt;

    program_options_t options;
    options.backend = NULL;
    options.devname = NULL;
    options.xmlfile = "output.uddf";
    options.truncateDives = 0;
    options.initialPressureFix = 0;
    options.logfile = "output.log";
    options.dumpDives = 1;
    options.dumpMemory = 0;
    options.useInvalidElements = 0;

    static struct option long_options[] = {
            {"backend",      required_argument, NULL, 'b'},
            {"device",       required_argument, NULL, 'd'},
            {"output",       required_argument, NULL, 'o'},
            {"help",         no_argument,       NULL, 'h'},
            {"ipf",          no_argument,       NULL, 'i'},
            {"truncate",     no_argument,       NULL, 't'},
            {"invalid",      no_argument,       NULL, 0},
            {"listdevices",  no_argument,       NULL, 0},
            {"listbackends", no_argument,       NULL, 0},
            {NULL,           no_argument,       NULL, 0}
    };
    char *getopt_short = "b:d:o:hit";
    /* getopt_long stores the option index here. */
    int option_index = 0;

    opt = getopt_long (argc, argv, getopt_short,
            long_options, &option_index);
    while (opt != -1)
    {
        switch (opt)
        {
        case 0:
            /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0)
                break;
            if (g_strcmp0("listdevices", long_options[option_index].name) == 0) {
                print_devices();
            }
            if (g_strcmp0("listbackends", long_options[option_index].name) == 0) {
                print_backends();
            }
            if (g_strcmp0("invalid", long_options[option_index].name) == 0) {
                options.useInvalidElements = 1;
            }
            break;

        case 'b':
            options.backend = optarg;
            break;

        case 'd':
            options.devname = optarg;
            break;

        case 'o':
            options.xmlfile = optarg;
            break;

        case 'i':
            options.initialPressureFix = 1;
            break;

        case 't':
            options.truncateDives = 1;
            break;

        case '?':
        case 'h':
            usage();
            break;

        default:
            printf("Unknown argument: %c", (char)opt);
            return EXIT_FAILURE;
        }
        opt = getopt_long (argc, argv, getopt_short,
                long_options, &option_index);
    }

    if (options.backend == NULL || options.devname == NULL) {
        usage();
    }

    return dump_dives(&options);
}
