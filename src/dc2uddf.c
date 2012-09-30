/*
 ============================================================================
 Name        : dc2uddf.c
 Author      : Patrick Wagstrom
 Version     :
 Copyright   : Copyright (c) 2012 Patrick Wagstrom
 Description : A simple tool that utilizes libdivecomputer to download data
               from a dive comptuer and generate a UDDF file
 ============================================================================
 */

#define DC2UDDF_VERSION "1.0"
#define DC2UDDF_AUTHOR "Patrick Wagstrom"
#define DC2UDDF_EMAIL "patrick@wagstrom.net"

#include <stdio.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <libdivecomputer/context.h>
#include <libdivecomputer/common.h>
#include <libdivecomputer/parser.h>
#include <libdivecomputer/device.h>

#include "utils.h"

typedef struct dive_data_t {
    dc_device_t *device;
    FILE* fp;
    unsigned int number;
    dc_buffer_t *fingerprint;
} dive_data_t;

xmlNodePtr createGeneratorBlock () {
    xmlNodePtr generator = xmlNewNode(NULL, BAD_CAST "generator");
    // xmlAddChild(root, generator);

    xmlNodePtr name = xmlNewNode(NULL, BAD_CAST "name");
    xmlAddChild(name, xmlNewText(BAD_CAST "dc2uddf"));
    xmlAddChild(generator, name);

    xmlNodePtr type = xmlNewNode(NULL, BAD_CAST "type");
    xmlAddChild(type, xmlNewText(BAD_CAST "converter"));
    xmlAddChild(generator, type);

    xmlNodePtr version = xmlNewNode(NULL, BAD_CAST "version");
    xmlAddChild(version, xmlNewText(BAD_CAST DC2UDDF_VERSION));
    xmlAddChild(generator, version);

    xmlNodePtr manufacturer = xmlNewNode(NULL, BAD_CAST "manufacturer");
    xmlNodePtr manufacturerName = xmlNewNode(NULL, BAD_CAST "name");
    xmlAddChild(manufacturerName, xmlNewText(BAD_CAST DC2UDDF_AUTHOR));
    xmlAddChild(manufacturer, manufacturerName);
    xmlNodePtr contact = xmlNewNode(NULL, BAD_CAST "contact");
    xmlNodePtr email = xmlNewNode(NULL, BAD_CAST "email");
    xmlAddChild(email, xmlNewText(BAD_CAST DC2UDDF_EMAIL));
    xmlAddChild(contact, email);
    xmlAddChild(manufacturer, contact);

    xmlAddChild(generator, manufacturer);

    return generator;

/*    <generator>
        <!-- description of the program generating the UDDF file -->
        <name>DSL - Diver's Super Logbook</name>
        <type>logbook</type>
        <manufacturer>
            <name>Dive Heroes Company</name>
            <address>
                <!-- address data of manufacturer -->
            </address>
            <contact>
                <!-- contact data of manufacturer -->
            </contact>
        </manufacturer>
        <version>3.14159</version>
        <datetime>2004-09-30</datetime>
    </generator> */
}

xmlNodePtr createWaypoint(xmlChar *depth, xmlChar *time, xmlChar *temperature,
        xmlChar *pressure) {
    xmlNodePtr waypoint = xmlNewNode(NULL, BAD_CAST "waypoint");
    if (depth != NULL) {
        xmlNodePtr depthNode = xmlNewNode(NULL, BAD_CAST "depth");
        xmlAddChild(depthNode, xmlNewText(BAD_CAST depth));
        xmlAddChild(waypoint, depthNode);
    }

    if (time != NULL) {
        xmlNodePtr timeNode = xmlNewNode(NULL, BAD_CAST "divetime");
        xmlAddChild(timeNode, xmlNewText(BAD_CAST time));
        xmlAddChild(waypoint, timeNode);
    }

    if (temperature != NULL) {
        xmlNodePtr tempNode = xmlNewNode(NULL, BAD_CAST "temperature");
        xmlAddChild(tempNode, xmlNewText(BAD_CAST temperature));
        xmlAddChild(waypoint, tempNode);
    }

    if (pressure != NULL) {
        xmlNodePtr pressureNode = xmlNewNode(NULL, BAD_CAST "tankpressure");
        xmlAddChild(pressureNode, xmlNewText(BAD_CAST pressure));
        xmlAddChild(waypoint, pressureNode);
    }
    return waypoint;
}

xmlNodePtr createDive() {
    xmlNodePtr dive = xmlNewNode(NULL, BAD_CAST "dive");
    xmlNewProp(dive, BAD_CAST "id", BAD_CAST "test_dive1");
    xmlNodePtr dateTime = xmlNewNode(NULL, BAD_CAST "datetime");
    xmlAddChild(dateTime, xmlNewText(BAD_CAST "2012-01-01T00:00"));
    xmlAddChild(dive, dateTime);

    xmlNodePtr samples = xmlNewNode(NULL, BAD_CAST "samples");
    xmlAddChild(dive, samples);

    xmlAddChild(samples, createWaypoint("0.0", "0.0", "273", "20000000"));
    xmlAddChild(samples, createWaypoint("1.0", "30.0", "273", "19000000"));
    xmlAddChild(samples, createWaypoint("2.0", "60.0", "273", "18000000"));
    xmlAddChild(samples, createWaypoint("3.0", "90.0", "273", "16000000"));
    xmlAddChild(samples, createWaypoint("4.0", "120.0", "273", "15000000"));
    xmlAddChild(samples, createWaypoint("5.0", "150.0", "273", "14000000"));
    xmlAddChild(samples, createWaypoint("6.0", "180.0", "273", "13000000"));
    xmlAddChild(samples, createWaypoint("7.0", "210.0", "273", "12000000"));
    xmlAddChild(samples, createWaypoint("8.0", "240.0", "273", "10000000"));
    xmlAddChild(samples, createWaypoint("7.0", "270.0", "274", "9000000"));
    xmlAddChild(samples, createWaypoint("6.0", "300.0", "273", "8000000"));
    xmlAddChild(samples, createWaypoint("5.0", "330.0", "274", "7000000"));
    xmlAddChild(samples, createWaypoint("4.0", "360.0", "274", "6000000"));
    xmlAddChild(samples, createWaypoint("3.0", "390.0", "273", "5000000"));
    xmlAddChild(samples, createWaypoint("2.0", "420.0", "273", "4500000"));
    xmlAddChild(samples, createWaypoint("1.0", "450.0", "273", "4000000"));
    xmlAddChild(samples, createWaypoint("0.0", "480.0", "273", "3000000"));
    return dive;
}

xmlNodePtr createRepetitionGroup() {
    xmlNodePtr repetitionGroup = xmlNewNode(NULL, BAD_CAST "repetitiongroup");
    xmlNewProp(repetitionGroup, BAD_CAST "id", BAD_CAST "test1");
    xmlAddChild(repetitionGroup, createDive());
    return repetitionGroup;
}

xmlNodePtr createProfileData() {
    xmlNodePtr profile_data = xmlNewNode(NULL, BAD_CAST "profiledata");
    xmlAddChild(profile_data, createRepetitionGroup());
    return profile_data;
}

static int
dive_cb (const unsigned char *data, unsigned int size, const unsigned char *fingerprint, unsigned int fsize, void *userdata)
{
    dive_data_t *divedata = (dive_data_t *) userdata;

    divedata->number++;

    message ("Dive: number=%u, size=%u, fingerprint=", divedata->number, size);
    unsigned int i;
    for (i = 0; i < fsize; ++i) {
        message ("%02X", fingerprint[i]);
    }
    message ("\n");

    if (divedata->number == 1) {
        divedata->fingerprint = dc_buffer_new (fsize);
        dc_buffer_append (divedata->fingerprint, fingerprint, fsize);
    }

    if (divedata->fp) {
        fprintf (divedata->fp, "<dive>\n<number>%u</number>\n<size>%u</size>\n<fingerprint>", divedata->number, size);
        for (i = 0; i < fsize; ++i)
            fprintf (divedata->fp, "%02X", fingerprint[i]);
        fprintf (divedata->fp, "</fingerprint>\n");

        doparse (divedata->fp, divedata->device, data, size);

        fprintf (divedata->fp, "</dive>\n");
    }

    return 1;
}

/**
 * open an existing data dump and attempt to read it in...
 *
 */
dc_status_t parseFile(char *fn) {
    FILE *fp = fopen(fn, "rb");
    // Allocate a memory buffer.
    dc_buffer_t *buffer = dc_buffer_new (0);

    // Read the entire file into the buffer.
    size_t n = 0;
    unsigned char block[1024] = {0};
    while ((n = fread (block, 1, sizeof (block), fp)) > 0) {
        dc_buffer_append (buffer, block, n);
    }
    // Close the file.
    fclose (fp);

    dc_loglevel_t loglevel = DC_LOGLEVEL_WARNING;

    dc_context_t *context = NULL;
    dc_status_t rc = dc_context_new (&context);
    if (rc != DC_STATUS_SUCCESS) {
        message_set_logfile (NULL);
        return EXIT_FAILURE;
    }

    dc_context_set_loglevel (context, loglevel);
    dc_context_set_logfunc (context, logfunc, NULL);

    dc_device_t *device = NULL;

    rc = create_fake_uwatec_smart(device, context);

    dive_data_t divedata = {0};
    divedata.device = device;
    divedata.fingerprint = NULL;
    divedata.number = 0;
    rc = uwatec_smart_extract_dives(device, dc_buffer_get_data(buffer),
            dc_buffer_get_size(buffer), dive_cb, divedata);
    return rc;
}

int main(int argc, char **argv) {
    xmlDocPtr doc = NULL;
    xmlNodePtr root_node = NULL;

	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL, BAD_CAST "uddf");
	xmlNewProp(root_node, BAD_CAST "version", BAD_CAST "3.0.0");
	xmlDocSetRootElement(doc, root_node);
	xmlAddChild(root_node, createGeneratorBlock());
	xmlAddChild(root_node, createProfileData());
    xmlSaveFormatFileEnc(argc > 1 ? argv[1] : "-", doc, "UTF-8", 1);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    message("parsing file\n");
    parseFile("SMART.DMP");
    message("file parsed\n");

    return EXIT_SUCCESS;
}
