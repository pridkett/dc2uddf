#include <libxml/parser.h>
#include <libxml/tree.h>
#include <glib.h>
#include "dif.h"

#define DC2UDDF_VERSION "1.0"
#define DC2UDDF_AUTHOR "Patrick Wagstrom"
#define DC2UDDF_EMAIL "patrick@wagstrom.net"
#define UDDF_VERSION "3.1.0"

#define MAX_STRING_LENGTH 100
#define BAR_TO_PASCAL(a) (a*100000)
#define CELSIUS_TO_KELVIN(a) (a+273.15)

/**
 * Creates the simple generator XML block
 *    <generator>
 *       <!-- description of the program generating the UDDF file -->
 *       <name>DSL - Diver's Super Logbook</name>
 *       <type>logbook</type>
 *       <manufacturer>
 *           <name>Dive Heroes Company</name>
 *           <address>
 *               <!-- address data of manufacturer -->
 *           </address>
 *           <contact>
 *               <!-- contact data of manufacturer -->
 *           </contact>
 *       </manufacturer>
 *       <version>3.14159</version>
 *       <datetime>2004-09-30</datetime>
 *   </generator>
 */
xmlNodePtr _createGeneratorBlock(xml_options_t *options) {
    xmlNodePtr generator = xmlNewNode(NULL, BAD_CAST "generator");

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
}

/**
 * Creates a waypoint XML block
 *
 * <waypoint>
 *   <depth>10.0</depth>
 *   <divetime>120.0</divetime>
 *   <temperature>278.15</temperature>
 * </waypoint>
 */
xmlNodePtr _createWaypoint(dif_sample_t *sample, xml_options_t *options) {
    static const char *events[] = {
        "none", "deco", "rbt", "ascent", "ceiling", "workload", "transmitter",
        "violation", "bookmark", "surface", "safety stop", "gaschange",
        "safety stop (voluntary)", "safety stop (mandatory)", "deepstop",
        "ceiling (safety stop)", "unknown", "divetime", "maxdepth",
        "OLF", "PO2", "airtime", "rgbm", "heading", "tissue level warning",
        "gaschange2", "ndl"};

    xmlNodePtr xmlWaypoint = xmlNewNode(NULL, BAD_CAST "waypoint");

    gchar *nodeText = g_malloc(MAX_STRING_LENGTH);

    g_snprintf(nodeText, MAX_STRING_LENGTH, "%d", sample->timestamp);

    xmlNodePtr xmlDivetime = xmlNewNode(NULL, BAD_CAST "divetime");
    xmlAddChild(xmlDivetime, xmlNewText(BAD_CAST nodeText));
    xmlAddChild(xmlWaypoint, xmlDivetime);

    GList *subsample = g_list_first(sample->subsamples);
    while (subsample != NULL) {
        dif_subsample_t *ss = subsample->data;
        switch (ss->type) {
        case DIF_SAMPLE_TIME:
            printf("** Unable to process DIF_SAMPLE_TIME\n");
            break;
        case DIF_SAMPLE_DEPTH:
            g_snprintf(nodeText, MAX_STRING_LENGTH, "%0.2f", ss->value.depth);
            xmlNodePtr xmlDepth = xmlNewNode(NULL, BAD_CAST "depth");
            xmlAddChild(xmlDepth, xmlNewText(BAD_CAST nodeText));
            xmlAddChild(xmlWaypoint, xmlDepth);
            break;
        case DIF_SAMPLE_PRESSURE:
            g_snprintf(nodeText, MAX_STRING_LENGTH, "%0.2f", BAR_TO_PASCAL(ss->value.pressure.value));
            xmlNodePtr xmlTankpressure = xmlNewNode(NULL, BAD_CAST "tankpressure");
            xmlAddChild(xmlTankpressure, xmlNewText(BAD_CAST nodeText));
            xmlAddChild(xmlWaypoint, xmlTankpressure);
            break;
        case DIF_SAMPLE_TEMPERATURE:
            g_snprintf(nodeText, MAX_STRING_LENGTH, "%0.2f", CELSIUS_TO_KELVIN(ss->value.temperature));
            xmlNodePtr xmlTemperature = xmlNewNode(NULL, BAD_CAST "temperature");
            xmlAddChild(xmlTemperature, xmlNewText(BAD_CAST nodeText));
            xmlAddChild(xmlWaypoint, xmlTemperature);
            break;
        case DIF_SAMPLE_EVENT:
        {
            // WARNING: the event tag is NOT part of UDDF
            xmlNodePtr xmlEvent = xmlNewNode(NULL, BAD_CAST "event");
            g_snprintf(nodeText, MAX_STRING_LENGTH, "%u", ss->value.event.type);
            xmlNewProp(xmlEvent, BAD_CAST "type", BAD_CAST nodeText);
            g_snprintf(nodeText, MAX_STRING_LENGTH, "%u", ss->value.event.time);
            xmlNewProp(xmlEvent, BAD_CAST "time", BAD_CAST nodeText);
            g_snprintf(nodeText, MAX_STRING_LENGTH, "%u", ss->value.event.flags);
            xmlNewProp(xmlEvent, BAD_CAST "flags", BAD_CAST nodeText);
            g_snprintf(nodeText, MAX_STRING_LENGTH, "%u", ss->value.event.value);
            xmlNewProp(xmlEvent, BAD_CAST "value", BAD_CAST nodeText);
            xmlAddChild(xmlEvent, xmlNewText(BAD_CAST events[ss->value.event.type]));
            xmlAddChild(xmlWaypoint, xmlEvent);
            break;
        }
        case DIF_SAMPLE_RBT:
            g_snprintf(nodeText, MAX_STRING_LENGTH, "%d", ss->value.rbt);
            xmlNodePtr xmlRemainingbottomtime = xmlNewNode(NULL, BAD_CAST "remainingbottomtime");
            xmlAddChild(xmlRemainingbottomtime, xmlNewText(BAD_CAST nodeText));
            xmlAddChild(xmlWaypoint, xmlRemainingbottomtime);
            break;
        case DIF_SAMPLE_HEARTBEAT:
            g_snprintf(nodeText, MAX_STRING_LENGTH, "%d", ss->value.heartbeat);
            xmlNodePtr xmlHeartbeat = xmlNewNode(NULL, BAD_CAST "heartbeat");
            xmlAddChild(xmlHeartbeat, xmlNewText(BAD_CAST nodeText));
            xmlAddChild(xmlWaypoint, xmlHeartbeat);
            break;
        case DIF_SAMPLE_BEARING:
            g_snprintf(nodeText, MAX_STRING_LENGTH, "%d", ss->value.bearing);
            xmlNodePtr xmlHeading = xmlNewNode(NULL, BAD_CAST "heading");
            xmlAddChild(xmlHeading, xmlNewText(BAD_CAST nodeText));
            xmlAddChild(xmlWaypoint, xmlHeading);
            break;
        case DIF_SAMPLE_VENDOR:
        {
            // WARNING: the vendor tag is NOT part of UDDF
            xmlNodePtr xmlVendor = xmlNewNode(NULL, BAD_CAST "vendor");
            gchar *propText = g_malloc(MAX_STRING_LENGTH);
            g_snprintf(propText, MAX_STRING_LENGTH, "%u", ss->value.vendor.type);
            xmlNewProp(xmlVendor, BAD_CAST "type", BAD_CAST propText);
            g_snprintf(propText, MAX_STRING_LENGTH, "%u", ss->value.vendor.size);
            xmlNewProp(xmlVendor, BAD_CAST "size", BAD_CAST propText);
            guint vendorTextLength = ss->value.vendor.size*2+1;
            gchar *vendorText = g_malloc0(ss->value.vendor.size*2+1);
            unsigned int i;
            for (i=0; i < ss->value.vendor.size; i++) {
                g_snprintf((vendorText + i * 2), vendorTextLength - i * 2, "%02X", ((unsigned char *) ss->value.vendor.data)[i]);
            }
            xmlAddChild(xmlVendor, xmlNewText(BAD_CAST vendorText));
            xmlAddChild(xmlWaypoint, xmlVendor);
            g_free(propText);
            g_free(vendorText);
            break;
        }
        case DIF_SAMPLE_UNDEFINED:
            printf("** Unable to process DIF_SAMPLE_UNKNOWN\n");
            break;
        default:
            printf("** Unable to process unknown type: %d\n", ss->type);
            break;
        }
        subsample = g_list_next(subsample);
    }

    g_free(nodeText);
    return xmlWaypoint;
}

xmlNodePtr _createDive(dif_dive_t *dive, gchar *diveid, xml_options_t *options) {
    xmlNodePtr xmlDive = xmlNewNode(NULL, BAD_CAST "dive");
    xmlNewProp(xmlDive, BAD_CAST "id", BAD_CAST diveid);
    xmlNodePtr xmlDateTime = xmlNewNode(NULL, BAD_CAST "datetime");
    if (dive->datetime != NULL) {
        gchar *dt = g_date_time_format(dive->datetime, "%Y-%m-%dT%H:%M:%S");
        xmlAddChild(xmlDateTime, xmlNewText(BAD_CAST dt));
        g_free(dt);
        xmlAddChild(xmlDive, xmlDateTime);
    }

    xmlNodePtr xmlSamples = xmlNewNode(NULL, BAD_CAST "samples");
    xmlAddChild(xmlDive, xmlSamples);

    GList *samples = g_list_first(dive->samples);
    while (samples != NULL) {
        xmlNodePtr xmlWaypoint = _createWaypoint(samples->data, options);
        xmlAddChild(xmlSamples, xmlWaypoint);
        samples = g_list_next(samples);
    }
    return xmlDive;
}

xmlNodePtr _createRepetitionGroup(dif_dive_collection_t *dc, xml_options_t *options) {
    xmlNodePtr repetitionGroup = xmlNewNode(NULL, BAD_CAST "repetitiongroup");
    xmlNewProp(repetitionGroup, BAD_CAST "id", BAD_CAST "group1");

    guint ctr = 0;
    gchar *diveid = g_malloc(MAX_STRING_LENGTH);
    GList *dives = g_list_first(dc->dives);
    while (dives != NULL) {
        g_snprintf(diveid, MAX_STRING_LENGTH, "dive%d", ctr++);
        xmlAddChild(repetitionGroup, _createDive(dives->data, diveid, options));
        dives = g_list_next(dives);
    }
    g_free(diveid);
    return repetitionGroup;
}

xmlNodePtr _createProfileData(dif_dive_collection_t *dc, xml_options_t *options) {
    xmlNodePtr profile_data = xmlNewNode(NULL, BAD_CAST "profiledata");
    xmlAddChild(profile_data, _createRepetitionGroup(dc, options));
    return profile_data;
}

xml_options_t *dif_xml_options_alloc() {
    xml_options_t *options = g_malloc(sizeof(xml_options_t));
    options->filename = NULL;
    return options;
}

void dif_xml_options_free(xml_options_t *options) {
    /* for right now don't wipe out the filename */
//    if (options->filename != NULL) {
//        g_free(options->filename);
//    }
    g_free(options);
}

void dif_save_dive_collection_uddf(dif_dive_collection_t *dc, gchar* filename) {
    xml_options_t *options = dif_xml_options_alloc();
    options->filename = filename;
    dif_save_dive_collection_uddf_options(dc, options);
    dif_xml_options_free(options);
}

void dif_save_dive_collection_uddf_options(dif_dive_collection_t *dc, xml_options_t *options) {
    xmlDocPtr doc = NULL;
    xmlNodePtr root_node = NULL;
    printf("saving file to %s\n", options->filename);
    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "uddf");
    xmlNewProp(root_node, BAD_CAST "version", BAD_CAST UDDF_VERSION);
    xmlDocSetRootElement(doc, root_node);
    xmlAddChild(root_node, _createGeneratorBlock(options));
    printf("creating profile data\n");
    xmlAddChild(root_node, _createProfileData(dc, options));
    printf("saving data\n");
    xmlSaveFormatFileEnc(options->filename, doc, "UTF-8", 1);
    xmlFreeDoc(doc);
    xmlCleanupParser();
}
