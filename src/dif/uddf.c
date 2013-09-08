#include <libxml/parser.h>
#include <libxml/tree.h>
#include <glib.h>
#include <stdio.h>
#include "dif.h"

#define DC2UDDF_VERSION "1.0"
#define DC2UDDF_AUTHOR "Patrick Wagstrom"
#define DC2UDDF_EMAIL "patrick@wagstrom.net"
#define UDDF_VERSION "3.2.0"
#define UDDF_NAMESPACE "http://www.streit.cc/uddf/3.2/"

#define MAX_STRING_LENGTH 100
#define BAR_TO_PASCAL(a) (a*100000)
#define CELSIUS_TO_KELVIN(a) (a+273.15)

/**
 * helper function for comparing two nodes and sorting them
 *
 * This is mainly used for the <waypoint> tag which requires that each of the
 * subsamples be in alphabetical order
 */
gint _g_list_sort_xmlNodePtrs(gconstpointer a, gconstpointer b) {
    xmlNodePtr node1 = (xmlNodePtr) a;
    xmlNodePtr node2 = (xmlNodePtr) b;
    return xmlStrcmp(node1->name, node2->name);
}

/**
 * helper function for creating datetime nodes
 */
xmlNodePtr _createDateTime(GDateTime *dt, xml_options_t *options) {
    xmlNodePtr xmlDateTime = xmlNewNode(NULL, BAD_CAST "datetime");
    gchar *tzstr = g_date_time_format(dt, "%z");
    gchar *dtstr = g_date_time_format(dt, "%Y-%m-%dT%H:%M:%S");
    gchar *xmlstr = g_malloc(MAX_STRING_LENGTH);
    g_snprintf(xmlstr, MAX_STRING_LENGTH, "%s%c%c%c:%c%c",
            dtstr, tzstr[0], tzstr[1], tzstr[2], tzstr[3], tzstr[4]);
    xmlAddChild(xmlDateTime, xmlNewText(BAD_CAST xmlstr));
    g_free(dtstr);
    g_free(tzstr);
    g_free(xmlstr);
    return xmlDateTime;
}

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
 *
 * The schema is VERY picky about the formatting of this element, everything
 * must appear in the exact order.
 */
xmlNodePtr _createGeneratorBlock(xml_options_t *options) {
    xmlNodePtr generator = xmlNewNode(NULL, BAD_CAST "generator");

    xmlNodePtr name = xmlNewNode(NULL, BAD_CAST "name");
    xmlAddChild(name, xmlNewText(BAD_CAST "dc2uddf"));
    xmlAddChild(generator, name);

    xmlNodePtr type = xmlNewNode(NULL, BAD_CAST "type");
    xmlAddChild(type, xmlNewText(BAD_CAST "converter"));
    xmlAddChild(generator, type);

    xmlNodePtr manufacturer = xmlNewNode(NULL, BAD_CAST "manufacturer");
    xmlNewProp(manufacturer, BAD_CAST "id", "dc2uddf");
    xmlNodePtr manufacturerName = xmlNewNode(NULL, BAD_CAST "name");
    xmlAddChild(manufacturerName, xmlNewText(BAD_CAST DC2UDDF_AUTHOR));
    xmlAddChild(manufacturer, manufacturerName);
    xmlNodePtr contact = xmlNewNode(NULL, BAD_CAST "contact");
    xmlNodePtr email = xmlNewNode(NULL, BAD_CAST "email");
    xmlAddChild(email, xmlNewText(BAD_CAST DC2UDDF_EMAIL));
    xmlAddChild(contact, email);
    xmlAddChild(manufacturer, contact);
    xmlAddChild(generator, manufacturer);

    xmlNodePtr version = xmlNewNode(NULL, BAD_CAST "version");
    xmlAddChild(version, xmlNewText(BAD_CAST DC2UDDF_VERSION));
    xmlAddChild(generator, version);

    GDateTime *dt = g_date_time_new_now_utc();
    xmlAddChild(generator, _createDateTime(dt, options));
    g_date_time_unref(dt);

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

    GList *xmlSubsamples= NULL;

    xmlNodePtr xmlWaypoint = xmlNewNode(NULL, BAD_CAST "waypoint");

    gchar *nodeText = g_malloc(MAX_STRING_LENGTH);

    g_snprintf(nodeText, MAX_STRING_LENGTH, "%d", sample->timestamp);

    xmlNodePtr xmlDivetime = xmlNewNode(NULL, BAD_CAST "divetime");
    xmlAddChild(xmlDivetime, xmlNewText(BAD_CAST nodeText));
    xmlSubsamples = g_list_append(xmlSubsamples, xmlDivetime);


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
            xmlSubsamples = g_list_append(xmlSubsamples, xmlDepth);
            break;
        case DIF_SAMPLE_PRESSURE:
            g_snprintf(nodeText, MAX_STRING_LENGTH, "%0.2f", BAR_TO_PASCAL(ss->value.pressure.value));
            xmlNodePtr xmlTankpressure = xmlNewNode(NULL, BAD_CAST "tankpressure");
            xmlAddChild(xmlTankpressure, xmlNewText(BAD_CAST nodeText));
            xmlSubsamples = g_list_append(xmlSubsamples, xmlTankpressure);
            break;
        case DIF_SAMPLE_TEMPERATURE:
            g_snprintf(nodeText, MAX_STRING_LENGTH, "%0.2f", CELSIUS_TO_KELVIN(ss->value.temperature));
            xmlNodePtr xmlTemperature = xmlNewNode(NULL, BAD_CAST "temperature");
            xmlAddChild(xmlTemperature, xmlNewText(BAD_CAST nodeText));
            xmlSubsamples = g_list_append(xmlSubsamples, xmlTemperature);
            break;
        case DIF_SAMPLE_EVENT:
            if (options->useInvalidElements) {
                // WARNING: the event tag is NOT part of UDDF, however there are cases when
                // it might be desirable to save these events for debugging and validation.
                //
                // This is partially because there are various events in the UDDF schema
                // that don't easily map, so I haven't implemented them yet. This will at
                // at least tell you if the event happened.
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
                xmlSubsamples = g_list_append(xmlSubsamples, xmlEvent);
            } else {
                printf("** Received an EVENT that isn't part of the schema. To dump this event please set xml_options_t->useInvalidElements to TRUE\n");
            }
            break;
        case DIF_SAMPLE_RBT:
            g_snprintf(nodeText, MAX_STRING_LENGTH, "%d", ss->value.rbt);
            xmlNodePtr xmlRemainingbottomtime = xmlNewNode(NULL, BAD_CAST "remainingbottomtime");
            xmlAddChild(xmlRemainingbottomtime, xmlNewText(BAD_CAST nodeText));
            xmlSubsamples = g_list_append(xmlSubsamples, xmlRemainingbottomtime);
            break;
        case DIF_SAMPLE_HEARTBEAT:
            g_snprintf(nodeText, MAX_STRING_LENGTH, "%d", ss->value.heartbeat);
            xmlNodePtr xmlHeartbeat = xmlNewNode(NULL, BAD_CAST "heartbeat");
            xmlAddChild(xmlHeartbeat, xmlNewText(BAD_CAST nodeText));
            xmlSubsamples = g_list_append(xmlSubsamples, xmlHeartbeat);
            break;
        case DIF_SAMPLE_BEARING:
            g_snprintf(nodeText, MAX_STRING_LENGTH, "%d", ss->value.bearing);
            xmlNodePtr xmlHeading = xmlNewNode(NULL, BAD_CAST "heading");
            xmlAddChild(xmlHeading, xmlNewText(BAD_CAST nodeText));
            xmlSubsamples = g_list_append(xmlSubsamples, xmlHeading);
            break;
        case DIF_SAMPLE_VENDOR:
            if (options->useInvalidElements) {
                // WARNING: similar to the event tag, the vendor tag is not a
                // part of UDDF but can be recorded by dc2uddf.  I haven't had
                // any of these tags fire of on my computer, so I'm not certain
                // when they come up or what they mean.
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
                xmlSubsamples = g_list_append(xmlSubsamples, xmlVendor);
                g_free(propText);
                g_free(vendorText);
            } else {
                printf("** Received a VENDOR event that I don't understand and isn't part of the schema. To dump this event please set xml_options_t->useInvalidElements to TRUE\n");
            }
            break;
        case DIF_SAMPLE_UNDEFINED:
            printf("** Unable to process DIF_SAMPLE_UNKNOWN\n");
            break;
        default:
            printf("** Unable to process unknown type: %d\n", ss->type);
            break;
        }
        subsample = g_list_next(subsample);
    }

    // sort the different subsamples and save them as children of the waypoint
    xmlSubsamples = g_list_sort(xmlSubsamples, _g_list_sort_xmlNodePtrs);
    xmlSubsamples = g_list_first(xmlSubsamples);
    while (xmlSubsamples != NULL) {
        xmlAddChild(xmlWaypoint, xmlSubsamples->data);
        xmlSubsamples = g_list_next(xmlSubsamples);
    }
    g_free(nodeText);
    g_list_free(xmlSubsamples);
    return xmlWaypoint;
}

xmlNodePtr _createDive(dif_dive_t *dive, gchar *diveid, xml_options_t *options) {
    gchar *tempStr = g_malloc(MAX_STRING_LENGTH);

    xmlNodePtr xmlDive = xmlNewNode(NULL, BAD_CAST "dive");
    xmlNewProp(xmlDive, BAD_CAST "id", BAD_CAST diveid);
    xmlNodePtr xmlInformationBeforeDive = xmlNewNode(NULL, BAD_CAST "informationbeforedive");
    if (dive->datetime != NULL) {
        GDateTime *dt = dive->datetime;
        xmlAddChild(xmlInformationBeforeDive, _createDateTime(dt, options));
    }

    xmlNodePtr xmlSurfaceIntervalBeforeDive = xmlNewNode(NULL, BAD_CAST "surfaceintervalbeforedive");
    if (dive->surfaceInterval < 0) {
        xmlNodePtr xmlInfinity = xmlNewNode(NULL, BAD_CAST "infinity");
        xmlAddChild(xmlSurfaceIntervalBeforeDive, xmlInfinity);
    } else {
        xmlNodePtr xmlPassedTime = xmlNewNode(NULL, BAD_CAST "passedtime");
        g_snprintf(tempStr, MAX_STRING_LENGTH, "%d", dive->surfaceInterval);
        xmlAddChild(xmlPassedTime, xmlNewText(BAD_CAST tempStr));
        xmlAddChild(xmlSurfaceIntervalBeforeDive, xmlPassedTime);
    }
    xmlAddChild(xmlInformationBeforeDive, xmlSurfaceIntervalBeforeDive);
    xmlAddChild(xmlDive, xmlInformationBeforeDive);

    /* iterate over all of the samples */
    xmlNodePtr xmlSamples = xmlNewNode(NULL, BAD_CAST "samples");
    xmlAddChild(xmlDive, xmlSamples);
    dive = dif_dive_sort_samples(dive);
    GList *samples = g_list_first(dive->samples);
    while (samples != NULL) {
        xmlNodePtr xmlWaypoint = _createWaypoint(samples->data, options);
        xmlAddChild(xmlSamples, xmlWaypoint);
        samples = g_list_next(samples);
    }


    /* if gasmixes are specified, then we'll link to them */
    if (dive->gasmixes != NULL) {
        guint ctr = 0;
        GList *gasmixes = g_list_first(dive->gasmixes);
        while (gasmixes != NULL) {
            xmlNodePtr xmlTankdata = xmlNewNode(NULL, BAD_CAST "tankdata");
            // id is NOT a valid parameter for tankdata
            // g_snprintf(tempStr, MAX_STRING_LENGTH, "%s_tank%d", diveid, ctr++);
            // xmlNewProp(xmlTankdata, BAD_CAST "id", BAD_CAST tempStr);
            dif_gasmix_t *gasmix = gasmixes->data;
            xmlNodePtr xmlLink = xmlNewNode(NULL, BAD_CAST "link");
            xmlNewProp(xmlLink, BAD_CAST "ref", BAD_CAST dif_gasmix_name(gasmix));
            xmlAddChild(xmlTankdata, xmlLink);
            xmlAddChild(xmlDive, xmlTankdata);

            // FIXME: this gets the initial pressure of any tank and isn't bound
            // to the specific tank. Need to see how this actually works in more
            // detail and understand the numbering that libdivecomputer uses
            // when creating tanks
            gdouble initialPressure = dif_dive_get_initial_pressure(dive, -1);
            if (initialPressure > GAS_EPSILON) {
                g_snprintf(tempStr, MAX_STRING_LENGTH, "%0.1f", BAR_TO_PASCAL(initialPressure));
                xmlNodePtr xmlTankPressureBegin = xmlNewNode(NULL, BAD_CAST "tankpressurebegin");
                xmlAddChild(xmlTankPressureBegin, xmlNewText(BAD_CAST tempStr));
                xmlAddChild(xmlTankdata, xmlTankPressureBegin);
            }
            gasmixes = g_list_next(gasmixes);
        }
    }

    /* create the informationafterdive field */
    xmlNodePtr xmlInformationAfterDive = xmlNewNode(NULL, BAD_CAST "informationafterdive");

    /* create the lowest temperature field */
    gdouble lowestTemperature = 9999;
    samples = g_list_first(dive->samples);
    while (samples != NULL){
        dif_sample_t *sample = samples->data;
        dif_subsample_t *subsample = dif_sample_get_subsample(sample, DIF_SAMPLE_TEMPERATURE);
        if (subsample != NULL && subsample->value.temperature > 0.1 && subsample->value.temperature < lowestTemperature) {
            lowestTemperature = subsample->value.temperature;
        }
        samples = g_list_next(samples);
    }

    if (lowestTemperature < 9998) {
        xmlNodePtr xmlLowestTemperature = xmlNewNode(NULL, BAD_CAST "lowesttemperature");
        g_snprintf(tempStr, MAX_STRING_LENGTH, "%0.1f", (double)lowestTemperature);
        xmlAddChild(xmlLowestTemperature, xmlNewText(BAD_CAST tempStr));
        xmlAddChild(xmlInformationAfterDive, xmlLowestTemperature);
    }
    xmlAddChild(xmlDive, xmlInformationAfterDive);

    /* calculate the greatest depth */
    xmlNodePtr xmlGreatestDepth = xmlNewNode(NULL, BAD_CAST "greatestdepth");
    gdouble greatestDepth = 0.0;
    samples = g_list_first(dive->samples);
    while (samples != NULL) {
        dif_sample_t *sample = samples->data;
        dif_subsample_t *subsample = dif_sample_get_subsample(sample, DIF_SAMPLE_DEPTH);
        if (subsample != NULL && subsample->value.depth > greatestDepth) {
            greatestDepth = subsample->value.depth;
        }
        samples = g_list_next(samples);
    }
    g_snprintf(tempStr, MAX_STRING_LENGTH, "%0.1f", greatestDepth);
    xmlAddChild(xmlGreatestDepth, xmlNewText(BAD_CAST tempStr));
    xmlAddChild(xmlInformationAfterDive, xmlGreatestDepth);

    /* create the dive duration field */
    xmlNodePtr xmlDiveDuration = xmlNewNode(NULL, BAD_CAST "diveduration");
    guint maxtimestamp = 0;
    samples = g_list_first(dive->samples);
    while (samples != NULL) {
        dif_sample_t *sample = samples->data;
        if (sample->timestamp > maxtimestamp) {
            maxtimestamp = sample->timestamp;
        }
        samples = g_list_next(samples);
    }
    g_snprintf(tempStr, MAX_STRING_LENGTH, "%0.1f", (gdouble)maxtimestamp);
    xmlAddChild(xmlDiveDuration, xmlNewText(BAD_CAST tempStr));
    xmlAddChild(xmlInformationAfterDive, xmlDiveDuration);

    g_free(tempStr);

    return xmlDive;
}

xmlNodePtr _createRepetitionGroup(GList *dives, gchar *groupid, xml_options_t *options) {
    xmlNodePtr repetitionGroup = xmlNewNode(NULL, BAD_CAST "repetitiongroup");
    xmlNewProp(repetitionGroup, BAD_CAST "id", BAD_CAST groupid);

    guint ctr = 0;
    gchar *diveid = g_malloc(MAX_STRING_LENGTH);
    dives = g_list_first(dives);
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

    gchar *groupid = g_malloc(MAX_STRING_LENGTH);
    dc = dif_dive_collection_sort_dives(dc);
    dc = dif_dive_collection_calculate_surface_interval(dc);

    GList *dives = g_list_first(dc->dives);
    int year1 = 0, month1 = 0, day1 = 0;
    int year2 = 0, month2 = 0, day2 = 0;
    GList *repetitionDives = NULL;
    guint groupCtr = 0;
    while (dives != NULL) {
        dif_dive_t *dive = dives->data;
        GDateTime *dt = dive->datetime;
        /* if a date isn't present, always give it a new repetition group */
        if (dt != NULL) {
            g_date_time_get_ymd(dt, &year2, &month2, &year2);
        } else {
            year2 = year1 ++;
        }
        /* if this next dive doesn't belong to this group, output the current
         * repetition group and clear the list
         */
        if (year1 != year2 || month1 != month2 || day1 != day2) {
            if (repetitionDives != NULL) {
                g_snprintf(groupid, MAX_STRING_LENGTH, "group%d", groupCtr++);
                xmlAddChild(profile_data, _createRepetitionGroup(repetitionDives, groupid, options));
                g_list_free (repetitionDives);
            }
            repetitionDives = NULL;
        }
        repetitionDives = g_list_append(repetitionDives, dive);
        year1 = year2; month1 = month2; day1 = day2;
        dives = g_list_next(dives);
    }
    if (repetitionDives != NULL) {
        g_snprintf(groupid, MAX_STRING_LENGTH, "group%d", groupCtr++);
        xmlAddChild(profile_data, _createRepetitionGroup(repetitionDives, groupid, options));
        g_list_free (repetitionDives);
    }
    g_free(groupid);

    return profile_data;
}

xmlNodePtr _createGasDefinitions(dif_dive_collection_t *dc, xml_options_t *options) {
    xmlNodePtr xmlGasDefinitions = xmlNewNode(NULL, BAD_CAST "gasdefinitions");

    /* iterate over all of the dives and iterate over their gasmixes
     * and store the different gas mixes in a hash table
     */
    GList *dives = g_list_first(dc->dives);
    GHashTable *gasMixes = g_hash_table_new(g_str_hash, g_str_equal);
    while(dives != NULL) {
        dif_dive_t *dive = dives->data;
        GList *diveMixes = g_list_first(dive->gasmixes);
        while (diveMixes != NULL) {
            dif_gasmix_t *gasmix = diveMixes->data;
            gasmix->type = dif_gasmix_type(gasmix);
            gchar *gasmixName = dif_gasmix_name(gasmix);
            if (!g_hash_table_contains(gasMixes, gasmixName)) {
                g_hash_table_insert(gasMixes, gasmixName, gasmix);
            }
            diveMixes = g_list_next(diveMixes);
        }
        dives = g_list_next(dives);
    }

    /* iterate over the hash table to output the mixes */
    gchar *tempStr = g_malloc(MAX_STRING_LENGTH);
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init (&iter, gasMixes);
    while (g_hash_table_iter_next (&iter, &key, &value))
    {
        gchar *mixname = (gchar *)key;
        dif_gasmix_t *gasmix = (dif_gasmix_t *)value;
        xmlNodePtr xmlMix = xmlNewNode(NULL, BAD_CAST "mix");
        xmlNewProp(xmlMix, BAD_CAST "id", BAD_CAST mixname);

        xmlNodePtr xmlName = xmlNewNode(NULL, BAD_CAST "name");
        xmlAddChild(xmlName, xmlNewText(BAD_CAST mixname));
        xmlAddChild(xmlMix, xmlName);

        xmlNodePtr xmlO2 = xmlNewNode(NULL, BAD_CAST "o2");
        g_snprintf(tempStr, MAX_STRING_LENGTH, "%0.4f", gasmix->oxygen/100);
        xmlAddChild(xmlO2, xmlNewText(BAD_CAST tempStr));
        xmlAddChild(xmlMix, xmlO2);

        xmlNodePtr xmlN2 = xmlNewNode(NULL, BAD_CAST "n2");
        g_snprintf(tempStr, MAX_STRING_LENGTH, "%0.4f", gasmix->nitrogen/100);
        xmlAddChild(xmlN2, xmlNewText(BAD_CAST tempStr));
        xmlAddChild(xmlMix, xmlN2);

        xmlNodePtr xmlHe = xmlNewNode(NULL, BAD_CAST "he");
        g_snprintf(tempStr, MAX_STRING_LENGTH, "%0.4f", gasmix->helium/100);
        xmlAddChild(xmlHe, xmlNewText(BAD_CAST tempStr));
        xmlAddChild(xmlMix, xmlHe);

        xmlNodePtr xmlAr = xmlNewNode(NULL, BAD_CAST "ar");
        g_snprintf(tempStr, MAX_STRING_LENGTH, "%0.4f", gasmix->argon/100);
        xmlAddChild(xmlAr, xmlNewText(BAD_CAST tempStr));
        xmlAddChild(xmlMix, xmlAr);

        xmlNodePtr xmlH2 = xmlNewNode(NULL, BAD_CAST "h2");
        g_snprintf(tempStr, MAX_STRING_LENGTH, "%0.4f", gasmix->hydrogen/100);
        xmlAddChild(xmlH2, xmlNewText(BAD_CAST tempStr));
        xmlAddChild(xmlMix, xmlH2);

        xmlAddChild(xmlGasDefinitions, xmlMix);
    }
    g_free(tempStr);

    return xmlGasDefinitions;
}

/**
 * allocates space for the XML serialization options
 *
 * by default this is set with filename=NULL and useInvalidElements=FALSE
 */
xml_options_t *dif_xml_options_alloc() {
    xml_options_t *options = g_malloc(sizeof(xml_options_t));
    options->filename = NULL;
    options->useInvalidElements = FALSE;
    return options;
}

void dif_xml_options_free(xml_options_t *options) {
    /* for right now don't wipe out the filename */
//    if (options->filename != NULL) {
//        g_free(options->filename);
//    }
    g_free(options);
}

/**
 * simple helper function for saving a collection of dives
 *
 * for the most part this is a pass through function to dif_save_dive_collection_uddf_options
 * However, it will create an xml_options_t with the filename and then set the filename.
 * After completion it will free the xml_options_t
 *
 * @param dc: the collection of dives
 * @param filename: the file to save the data to
 */
void dif_save_dive_collection_uddf(dif_dive_collection_t *dc, gchar* filename) {
    xml_options_t *options = dif_xml_options_alloc();
    options->filename = filename;
    dif_save_dive_collection_uddf_options(dc, options);
    dif_xml_options_free(options);
}

/**
 * saves a collection of dives to a file
 *
 * @param dc: collection of dives to save
 * @param options: the set of serialization options
 */
void dif_save_dive_collection_uddf_options(dif_dive_collection_t *dc, xml_options_t *options) {
    xmlDocPtr doc = NULL;
    xmlNodePtr root_node = NULL;
    printf("saving file to %s\n", options->filename);
    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "uddf");
    xmlNsPtr nsptr = xmlNewNs(root_node, UDDF_NAMESPACE, NULL);
    xmlNewProp(root_node, BAD_CAST "version", BAD_CAST UDDF_VERSION);
    xmlDocSetRootElement(doc, root_node);
    xmlAddChild(root_node, _createGeneratorBlock(options));
    xmlAddChild(root_node, _createGasDefinitions(dc, options));
    printf("creating profile data\n");
    xmlAddChild(root_node, _createProfileData(dc, options));
    printf("saving data\n");
    xmlSaveFormatFileEnc(options->filename, doc, "UTF-8", 1);
    xmlFreeDoc(doc);
    xmlCleanupParser();
}
