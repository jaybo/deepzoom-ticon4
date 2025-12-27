/* $Id: restore_tide_db.c 3176 2008-08-20 23:36:01Z flaterco $ */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "restore_tide_db.h"



/*****************************************************************************\

                            DISTRIBUTION STATEMENT

    This source file is unclassified, distribution unlimited, public
    domain.  It is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/



/*****************************************************************************\

    Program         restore_tide_db - builds XTide format harmonics/offset 
                    files from a harmonic constituent database (.tcd) file.

    Synopsis        build_tide_db INPUT_DB_NAME OUTPUT_BASE_FILENAME

                    INPUT_DB_NAME           database name, the extension .tcd
                                            will be appended to the name if
                                            not already there
                    OUTPUT_BASE_FILENAME    ASCII harmonics/offset base file 
                                            name, .txt and .xml will appended 
                                            to this filename for reference and
                                            subordinate files respectively

    Returns         NV_INT32                0 on successful completion, -1 on
                                            failure

    Author          Jan C. Depner
    Date            08/01/02

\*****************************************************************************/

NV_INT32 main (NV_INT32 argc, char *argv[])
{
    FILE                       *fp1, *fp2;
    NV_CHAR                    string[512], tname[256], xml_file[512],
      timestamp[80], tcdname[512];
    NV_U_INT32                 i, j, k, end, num;
    NV_BOOL                    xml = NVFalse;
    TIDE_RECORD                rec;
    DB_HEADER_PUBLIC           db;


    if (argc < 3)
    {
        fprintf (stderr, "%s\n", PACKAGE_STRING);
        fprintf (stderr, "Usage: %s <INPUT TCD FILE> <OUTPUT FILE>\n", 
            argv[0]);
        exit (-1);
    }


    /*  Try to open the file name as is, then add .tcd and try it again if it
        didn't open.  */

    strcpy (tcdname, argv[1]);
    if (!open_tide_db (tcdname)) 
    {
        if (!strstr (tcdname, ".tcd"))
        {
            strcat (tcdname, ".tcd");
            if (!open_tide_db (tcdname)) 
            {
                fprintf (stderr, "\nUnable to open %s or\n%s\n", argv[1], 
                    tcdname);
                perror (argv[1]);
                exit (-1);
            }
        }
        else
        {
            perror (argv[1]);
            exit (-1);
        }
    }


    /*  Reference station file.  */

    sprintf (string, "%s.txt", argv[2]);
    if ((fp1 = fopen (string, "w")) == NULL)
    {
        perror (string);
        exit (-1);
    }
    {
      time_t timep;
      struct tm *temptm;
      timep = time(NULL);
      temptm = localtime (&timep);
      strftime (timestamp, 80, "%Y-%m-%d %H:%M %Z", temptm);
      fprintf (fp1, "# Output of restore_tide_db.\n");
      fprintf (fp1, "# This file was converted from %s\n# at %s\n#\n", tcdname,
        timestamp);
    }

    /*  Subordinate station file.  */

    sprintf (xml_file, "%s.xml", argv[2]);
    if ((fp2 = fopen (xml_file, "w")) == NULL)
    {
        perror (xml_file);
        exit (-1);
    }
    fprintf (fp2, "<?xml version=\"1.0\" encoding='ISO-8859-1'?>\n\n");
    fprintf (fp2, "<!-- Output of restore_tide_db. -->\n");
    fprintf (fp2, "<!-- This file was converted from %s -->\n<!-- at %s -->\n\n",
             tcdname, timestamp);

    db = get_tide_db_header ();



    /*  Legal BS.  */


    fprintf (fp1, "%s", legal);
    fprintf (fp2, "%s", legal_xml);
    fprintf (fp2, "%s", pre_data_xml);


    /*  Constituent names and speeds.  */

    fprintf (fp1, "%s", pre_num);
    fprintf (fp1, "%d\n", db.constituents);

    fprintf (fp1, "%s", pre_speed);

    for (i = 0 ; i < db.constituents ; i++)
    {
        fprintf (fp1, "%-10s                  %11.7f\n", get_constituent (i), 
            get_speed (i));
    }


    /*  Equilibrium arguments.  */

    fprintf (fp1, "%s", pre_year);

    fprintf (fp1, "%d\n", db.start_year);

    fprintf (fp1, "%s", pre_equilibrium);

    fprintf (fp1, "%u\n", db.number_of_years);

    for (i = 0 ; i < db.constituents ; i++)
    {
        fprintf (fp1, "%s\n", get_constituent (i));

        num = db.number_of_years / 10;
        if (db.number_of_years % 10) num++;
        for (j = 0 ; j < num ; j++)
        {
            end = j * 10 + 10;
            if (end > db.number_of_years) end = db.number_of_years;
            for (k = j * 10 ; k < end ; k++)
            {
                if (k > j*10)
                  fprintf (fp1, " ");
                fprintf (fp1, "%6.2f", get_equilibrium (i, k));
            }
            fprintf (fp1, "\n");
        }
    }

    fprintf (fp1, "*END*\n");




    /*  Mid-year node factors.  */

    fprintf (fp1, "%s", pre_node);

    fprintf (fp1, "%d\n", db.number_of_years);


    for (i = 0 ; i < db.constituents ; i++)
    {
        fprintf (fp1, "%s\n", get_constituent (i));

        num = db.number_of_years / 10;
        if (db.number_of_years % 10) num++;
        for (j = 0 ; j < num ; j++)
        {
            end = j * 10 + 10;
            if (end > db.number_of_years) end = db.number_of_years;
            for (k = j * 10 ; k < end ; k++)
            {
                if (k > j*10)
                  fprintf (fp1, " ");
                fprintf (fp1, "%06.4f", get_node_factor (i, k));
            }
            fprintf (fp1, "\n");
        }
    }

    fprintf (fp1, "*END*\n");


    fprintf (fp1, "%s", pre_data);




    /*  Finally, the constituents or correctors.  */

    for (i = 0 ; i < db.number_of_records ; i++)
    {
        memset (&rec, 0, sizeof (TIDE_RECORD));

        read_tide_record (i, &rec);


        /*  Reference stations.  */

        if (rec.header.record_type == 1)
        {
            fprintf (fp1, "# ");
            for (j = 0 ; j < strlen (rec.comments) ; j++)
            {
                fprintf (fp1, "%c", rec.comments[j]);
                if (rec.comments[j] == 10) fprintf (fp1, "# ");
            }
            fprintf (fp1, "\n");

            fprintf (fp1, "# BEGIN HOT COMMENTS\n");
            fprintf (fp1, "# country: %s\n", get_country (rec.country));
            if (strlen (rec.source))
              fprintf (fp1, "# source: %s\n", rec.source);
            fprintf (fp1, "# restriction: %s\n", get_restriction (rec.restriction));
	    if (strlen (rec.notes)) {
              unsigned i, l;
              /* Wish I had my Dstr... */
              fprintf (fp1, "# note: ");
              l = strlen (rec.notes);
              for (i=0; i<l; i++) {
                if (rec.notes[i] == '\n')
                  fprintf (fp1, "\n# note: ");
                else
                  fprintf (fp1, "%c", rec.notes[i]);
              }
              fprintf (fp1, "\n");
            }

            if (strlen (rec.station_id_context))
              fprintf (fp1, "# station_id_context: %s\n", rec.station_id_context);
            if (strlen (rec.station_id))
              fprintf (fp1, "# station_id: %s\n", rec.station_id);

            if (rec.date_imported)
              fprintf (fp1, "# date_imported: %d\n", rec.date_imported);

            /* FIXME worry about xfields later. */

            /* Direction units assumed degrees true */

            if (rec.max_direction != 361)
              fprintf (fp1, "# max_direction: %d\n", rec.max_direction);

            if (rec.min_direction != 361)
              fprintf (fp1, "# min_direction: %d\n", rec.min_direction);

            fprintf (fp1, "# datum: %s\n", get_datum (rec.datum));

            if (rec.expiration_date)
              fprintf (fp1, "# expiration: %d\n", rec.expiration_date);

            if (rec.months_on_station)
              fprintf (fp1, "# months_on_station: %d\n", rec.months_on_station);
            if (rec.last_date_on_station)
              fprintf (fp1, "# last_date_on_station: %d\n", rec.last_date_on_station);

            fprintf (fp1, "# confidence: %d\n", rec.confidence);

            fprintf (fp1, "# !units: %s\n", get_level_units (rec.level_units));
            fprintf (fp1, "# !longitude: %0.4f\n", rec.header.longitude);
            fprintf (fp1, "# !latitude: %0.4f\n", rec.header.latitude);

            if (rec.legalese)
              fprintf (fp1, "%s - %s\n", rec.header.name, get_legalese(rec.legalese));
            else
              fprintf (fp1, "%s\n", rec.header.name);

            strcpy (tname, ret_time (rec.zone_offset));
            fprintf (fp1, "%s %s\n", tname, get_tzfile (rec.header.tzfile));
            fprintf (fp1, "%.4f %s\n", rec.datum_offset, 
                get_level_units (rec.level_units));



            for (j = 0 ; j < db.constituents ; j++)
            {
                if (rec.amplitude[j] == 0.0 && rec.epoch[j] == 0.0)
                {
                    fprintf (fp1, "x 0 0\n");
                }
                else
                {
                    fprintf (fp1, "%-10s     %7.4f  %6.2f\n", 
                        get_constituent (j), rec.amplitude[j], rec.epoch[j]);
                }
            }
        }


        /*  Subordinate stations.  */

        else if (rec.header.record_type == 2)
        {
            xml = NVTrue;

            {
              int i = strlen(rec.comments);
              if (i) {
                if (rec.comments[i-1] == '\n')
                  rec.comments[i-1] = '\0';
                fprintf (fp2, "<!-- Comments on %s:\n", rec.header.name);
                fprintf (fp2, "%s\n-->\n\n", rec.comments);
              }
            }

            /* Lose double quotes */
            {
              char *i;
              while ((i = strchr (rec.header.name, '"')))
                *i = '\'';
            }

            if (rec.legalese)
              fprintf (fp2, "<subordinatestation name=\"%s - %s\"\n", 
		       rec.header.name, get_legalese(rec.legalese));
            else
              fprintf (fp2, "<subordinatestation name=\"%s\"\n", 
                  rec.header.name);

            fprintf (fp2, "   latitude=\"%.4f\"\n", rec.header.latitude);
            fprintf (fp2, "   longitude=\"%.4f\"\n", rec.header.longitude);
            fprintf (fp2, "   timezone=\"%s\"\n",  get_tzfile (rec.header.tzfile));
            fprintf (fp2, "   country=\"%s\"\n", get_country(rec.country));
            fprintf (fp2, "   source=\"%s\"\n", rec.source);
            fprintf (fp2, "   restriction=\"%s\"\n", get_restriction(rec.restriction));
            if (strlen (rec.notes)) {
              /* No multi-line literals permitted */
              /* (And long literals will break build_tide_db!) */
              unsigned i, l;
              l = strlen(rec.notes);
              for (i=0; i<l; i++)
                if (rec.notes[i] == '\n')
                  rec.notes[i] = ' ';
              fprintf (fp2, "   note=\"%s\"\n", rec.notes);
            }
            if (strlen (rec.station_id_context))
              fprintf (fp2, "   station_id_context=\"%s\"\n", rec.station_id_context);
            if (strlen (rec.station_id))
              fprintf (fp2, "   station_id=\"%s\"\n", rec.station_id);
            if (rec.date_imported)
              fprintf (fp2, "   date_imported=\"%d\"\n", rec.date_imported);
            /* FIXME xfields someday */
            fprintf (fp2, "   reference=\"%s\"", get_station (rec.header.reference_station));
            fprintf (fp2, ">\n");

            if (rec.max_time_add       == rec.min_time_add        &&
                rec.max_level_add      == rec.min_level_add       &&
                rec.max_level_multiply == rec.min_level_multiply  &&
                rec.max_direction == 361 &&
                rec.min_direction == 361 &&
                rec.flood_begins  == NULLSLACKOFFSET &&
                rec.ebb_begins    == NULLSLACKOFFSET) {
                fprintf (fp2, "<simpleoffsets>\n");

                if (rec.min_time_add) fprintf (fp2, 
                    "   <timeadd value=\"%s\"/>\n", 
                    ret_time (rec.min_time_add));

                if (rec.min_level_add != 0.0) 
                    fprintf (fp2, 
                    "   <leveladd value=\"%.3f\" units=\"%s\"/>\n", 
                    rec.min_level_add, get_level_units (rec.level_units));

                if (rec.min_level_multiply != 0.0) fprintf (fp2, 
                    "   <levelmultiply value=\"%.3f\"/>\n", 
                    rec.min_level_multiply);

                fprintf (fp2, "</simpleoffsets>\n");
            }
            else
            {
                fprintf (fp2, "<offsets>\n");
                fprintf (fp2, "   <max>\n");

                if (rec.max_time_add) 
                    fprintf (fp2, "      <timeadd value=\"%s\"/>\n", 
                    ret_time (rec.max_time_add));

                if (rec.max_level_add != 0.0) 
                    fprintf (fp2, 
                    "      <leveladd value=\"%.3f\" units=\"%s\"/>\n", 
                    rec.max_level_add, get_level_units (rec.level_units));

                if (rec.max_level_multiply != 0.0) 
                    fprintf (fp2, "      <levelmultiply value=\"%.3f\"/>\n", 
                    rec.max_level_multiply);

                if (rec.max_direction != 361) 
                    fprintf (fp2, 
                    "      <direction value=\"%03d\" units=\"%s\"/>\n",
                    rec.max_direction, get_dir_units (rec.direction_units));


                fprintf (fp2, "   </max><min>\n");


                if (rec.min_time_add) 
                    fprintf (fp2, "      <timeadd value=\"%s\"/>\n", 
                    ret_time (rec.min_time_add));

                if (rec.min_level_add != 0.0) 
                    fprintf (fp2, 
                    "      <leveladd value=\"%.3f\" units=\"%s\"/>\n", 
                    rec.min_level_add, get_level_units (rec.level_units));

                if (rec.min_level_multiply != 0.0) 
                    fprintf (fp2, "      <levelmultiply value=\"%.3f\"/>\n", 
                    rec.min_level_multiply);

                if (rec.min_direction != 361)
                    fprintf (fp2, 
                    "      <direction value=\"%03d\" units=\"%s\"/>\n",
                    rec.min_direction, get_dir_units (rec.direction_units));


                fprintf (fp2, "   </min>\n");


                /* DWF updated per slack offsets fix 2003-03-18 */
                if (rec.flood_begins != NULLSLACKOFFSET || rec.ebb_begins != NULLSLACKOFFSET)
                {
                    fprintf (fp2, 
                        "   <!-- Slack offsets are only of the timeadd variety. -->\n");

                    if (rec.flood_begins != NULLSLACKOFFSET)
                      fprintf (fp2, "   <floodbegins value=\"%s\"/>\n",
                        ret_time (rec.flood_begins));

                    if (rec.ebb_begins != NULLSLACKOFFSET)
                      fprintf (fp2, "   <ebbbegins value=\"%s\"/>\n",
                        ret_time (rec.ebb_begins));
                }

                fprintf (fp2, "</offsets>\n");
            }

            fprintf (fp2, "</subordinatestation>\n\n");
        }
    }

    fprintf (fp2, "</document>\n");

    fclose (fp1);
    fclose (fp2);


    /*  If we didn't actually have any subordinate stations remove the .xml
        file.  */

    if (!xml) remove (xml_file);


    return (0);
}
