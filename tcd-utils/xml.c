/* $Id: xml.c 4160 2012-01-15 21:49:13Z flaterco $ */

#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <tcd.h>

/* This should be done with stdbool.h, but VC doesn't have it. */
/* Using crappy old int, must be careful not to 'require' a 64-bit value. */
#ifndef require
#define require(expr) {       \
  int require_expr;           \
  require_expr = (int)(expr); \
  if (!require_expr) {        \
    format_barf ("failed assertion: " #expr, "xml.c", __LINE__);	\
    abort();                  \
  }                           \
}
#endif

void format_barf (char const * const barfmessage,
                  char const * const file,
                  int line);


/*****************************************************************************\

                            DISTRIBUTION STATEMENT

    This source file is unclassified, distribution unlimited, public
    domain.  It is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/



/*****************************************************************************\

    Function        parse_line - pull just the stuff between the " marks out
                    of the string.

    Synopsis        parse_line (in, search, result, rlen);

                    NV_CHAR *in             string to be parsed
                    NV_CHAR *search         identifier string 
                                            (ex. <timeadd value=)
                    NV_CHAR *result         string from between the " marks
                    size_t  rlen            result buffer length

    Returns         NV_BOOL                 NVfalse if the search string 
                                            wasn't in the input string or we
                                            didn't find the two " marks

    Author          Jan C. Depner
    Date            08/01/02


    Modified        David Flater
    2008-08-20
      Parameterized rlen (formerly hard-coded to 256).
      Return false if we run out of room before finding the matching quote
      (formerly it returned true, along with unterminated data).
      Changed i and j to unsigned.

\*****************************************************************************/

NV_BOOL parse_line (NV_CHAR *in, NV_CHAR *search, NV_CHAR *result, size_t rlen)
{
    NV_CHAR                      *ptr;
    NV_U_INT32                   i, j;
    NV_BOOL                      load;


    /*  Cant'f find the search string.  */

    if (!(ptr = strstr (in, search))) return (NVFalse);


    /*  Looking for the " marks.  */

    load = NVFalse;
    for (i = 0 , j = 0 ; i < rlen ; ++i)
    {
        /*  Second " mark, terminate the string.  */

        if ((ptr[i] == '"') && load) 
        {
            result[j] = '\0';
            return NVTrue;
        }


        /*  First " mark.  */

        if (ptr[i] == '"')
        {
            load = NVTrue;
        }
        else
        {
            /*  In between " marks.  */

            if (load) 
            {
                result[j++] = ptr[i];
            }
        }
    }

    /* Ran out of room */
    return (NVFalse);
}



/*****************************************************************************\

    Function        parse_xml_station - parse one </subordinatestation> record
                    from the input XTide xml file.  A "real" XML parser?  Hah!
                    Surely you jest!  No, and don't call me Shirley.

    Synopsis        parse_xml_station (fp, rec);

                    FILE *fp                file to read data from
                    TIDE_RECORD *rec        tide record structure to place the
                                            results in

    Returns         NV_BOOL                 NVfalse if we couldn't find the
                                            reference station in the existing
                                            harmonic constituent database

    Author          Jan C. Depner
    Date            08/01/02

    Modified        David Flater / August Hahn
    See README for details.

\*****************************************************************************/

NV_BOOL parse_xml_station (FILE *fp, TIDE_RECORD *rec)
{
    NV_CHAR                 tname[10000], string[10000];
    NV_BOOL                 simple, found_tz, found_level_units;
    TIDE_RECORD             ref;
    NV_INT32                i;

    simple = NVFalse;
    found_tz = NVFalse;
    found_level_units = NVFalse;

    /* DWF 2004-09-16 */
    require (rec != NULL);
    strcpy (tname, rec->header.name);
    memset (rec, 0, sizeof (TIDE_RECORD));
    strcpy (rec->header.name, tname);
    rec->header.record_type = 2;
    rec->restriction = (NV_U_BYTE) find_or_add_restriction ("Non-commercial use only", NULL);
    rec->min_direction = rec->max_direction = 361;
    {
      int r = find_dir_units ("degrees true");
      require (r > 0);
      rec->direction_units = (NV_U_BYTE)r;
    }
    rec->flood_begins = rec->ebb_begins = NULLSLACKOFFSET;

    /* Jan wrote: */
    /*  If you thought I was going to do a "real" XML parser you were sadly
        mistaken!  This is just enough to get through the damn file.  */

    require (fgets (string, sizeof (string), fp) != NULL);

    while (!(strstr (string, "</subordinatestation>")))
    {
        if (strstr (string, "<simpleoffsets>")) simple = NVTrue;

        if (parse_line (string, "name=", tname, sizeof(tname)))
          strcpy (rec->header.name, tname);
        else if (parse_line (string, "country=", tname, sizeof(tname)))
          rec->country = find_or_add_country (tname, NULL);
        else if (parse_line (string, "source=", tname, sizeof(tname)))
          strcpy (rec->source, tname);
        else if (parse_line (string, "pedigree=", tname, sizeof(tname))) {
          /* Old XML files used pedigree to mean source.  Assume that's
	     what we got if source was missing. */
          if (!(strlen (rec->source)))
            strcpy (rec->source, tname);
        } else if (parse_line (string, "restriction=", tname, sizeof(tname)))
	  rec->restriction = (NV_U_BYTE) find_or_add_restriction (tname, NULL);
        else if (parse_line (string, "latitude=", tname, sizeof(tname))) {
	  require (sscanf (tname, "%lf", &rec->header.latitude) == 1);
        } else if (parse_line (string, "longitude=", tname, sizeof(tname))) {
	  require (sscanf (tname, "%lf", &rec->header.longitude) == 1);
        } else if (parse_line (string, "timezone=", tname, sizeof(tname))) {
          rec->header.tzfile = find_or_add_tzfile (tname, NULL);
          found_tz = NVTrue;
	} else if (parse_line (string, "note=", tname, sizeof(tname)))
          strcpy (rec->notes, tname);
        else if (parse_line (string, "station_id_context=", tname, sizeof(tname)))
          strcpy (rec->station_id_context, tname);
        else if (parse_line (string, "station_id=", tname, sizeof(tname)))
          strcpy (rec->station_id, tname);
        else if (parse_line (string, "date_imported=", tname, sizeof(tname))) {
          require (sscanf (tname, "%u", &rec->date_imported) == 1);
        } else if (parse_line (string, "reference=", tname, sizeof(tname)))
        {
            if ((rec->header.reference_station = find_station (tname)) < 0) 
            {
                fprintf (stderr, "Can't find reference station %s for station %s\n", tname, rec->header.name);
                return (NVFalse);
            }

            /* DWF/AH: Get reference station for comparison of units
               and time zone */
	    memset (&ref, 0, sizeof (TIDE_RECORD));
	    read_tide_record(rec->header.reference_station, &ref);

            /* DWF: Initialize to reference station units */
	    rec->level_units = ref.level_units;

 	    /*  If no timezone name was found in this record, use the one from
                the reference station.  */
            if (!found_tz)
                rec->header.tzfile = ref.header.tzfile;
        }


        /*  Simple offset record.  */

        if (simple)
        {
	    if (parse_line (string, "<timeadd value=", tname, sizeof(tname))) {
                rec->max_time_add = rec->min_time_add = get_time (tname);
	    }

            if (parse_line (string, "<leveladd value=", tname, sizeof(tname))) 
            {
	        require (sscanf (tname, "%f", &rec->min_level_add) == 1);
                rec->max_level_add = rec->min_level_add;
	
                if (parse_line (string, "units=", tname, sizeof(tname)))
                {
                    i = find_level_units (tname);
                    if (i >= 0)
                    {
                        rec->level_units = (NV_U_BYTE)i;
  		        found_level_units = NVTrue;
                    } else {
			fprintf(stderr, "ERROR parsing units for leveladd on station %s\n", rec->header.name);
		    }
                } else {
		    fprintf(stderr, "Missing units for leveladd on station %s\n", rec->header.name);
		}
            }

            if (parse_line (string, "<levelmultiply value=", tname, sizeof(tname))) {
	        require (sscanf (tname, "%f", &rec->min_level_multiply) == 1);
                rec->max_level_multiply = rec->min_level_multiply;
            }
        }

        /*  Not-so-simple offset record.  */
	/*  This code assumes that the max offsets precede the min offsets in the offsets.xml file
	    if this is not always strictly true then this code will break!
	*/
        /* DWF: Fixed that, I think... 2003-03-18 */
        else
        {
            if (strstr (string, "<max>"))
            {
                while (!(strstr (string, "</max>")))
                {
		    require (fgets (string, sizeof (string), fp) != NULL);

                    if (parse_line (string, "<timeadd value=", tname, sizeof(tname)))
                        rec->max_time_add = get_time (tname);

                    if (parse_line (string, "<leveladd value=", tname, sizeof(tname))) 
                    {
		        require (sscanf (tname, "%f", &rec->max_level_add) == 1);

                        if (parse_line (string, "units=", tname, sizeof(tname)))
                        {
                            i = find_level_units (tname);
                            if (i >= 0)
                            {
			        if (found_level_units) { /* DWF */
                                  if (rec->level_units != i)
                                    fprintf (stderr, "ERROR!  Conflicting level units in station %s\n", rec->header.name);
			        }
                                rec->level_units = (NV_U_BYTE)i;
  		                found_level_units = NVTrue;
                            } else {
				fprintf(stderr, "ERROR parsing units for leveladd (max) on station %s\n", rec->header.name);
			    }
			} else {
			    fprintf(stderr, "Missing units for leveladd (max) on station %s\n", rec->header.name);
			}
                    }

                    if (parse_line (string, "<levelmultiply value=", tname, sizeof(tname))) 
		        require (sscanf (tname, "%f", &rec->max_level_multiply) == 1);

                    /* Ignore avglevel */

                    if (parse_line (string, "<direction value=", tname, sizeof(tname))) 
                    {
		        require (sscanf (tname, "%d", &rec->max_direction) == 1);
                        if (rec->max_direction < 0) rec->max_direction += 360;
                        rec->max_direction %= 360;

                        if (parse_line (string, "units=", tname, sizeof(tname))) {
                          /* FIXME, clashes are not detected. */
                          i = find_dir_units (tname);
                          if (i >= 0)
                            rec->direction_units = (NV_U_BYTE)i;
                          else
			    fprintf(stderr, "ERROR parsing direction (max) units on station %s\n", rec->header.name);
			} else {
			    fprintf(stderr, "Missing units for direction (max) on station %s\n", rec->header.name);
			}
                    }
                }
            }


            if (strstr (string, "<min>"))
            {
                while (!(strstr (string, "</min>")))
                {
		    require (fgets (string, sizeof (string), fp) != NULL);

                    if (parse_line (string, "<timeadd value=", tname, sizeof(tname)))
                        rec->min_time_add = get_time (tname);

                    if (parse_line (string, "<leveladd value=", tname, sizeof(tname))) 
                    {
		        require (sscanf (tname, "%f", &rec->min_level_add) == 1);

                        if (parse_line (string, "units=", tname, sizeof(tname)))
                        {
                            i = find_level_units (tname);
                            if (i >= 0)
                            {
			        if (found_level_units) { /* DWF */
                                  if (rec->level_units != i)
                                    fprintf (stderr, "ERROR!  Conflicting level units in station %s\n", rec->header.name);
			        }
                                rec->level_units = (NV_U_BYTE)i;
  		                found_level_units = NVTrue;
			    } else {
				fprintf(stderr, "ERROR parsing units for leveladd (min) on station %s\n", rec->header.name);
			    }
			} else {
			    fprintf(stderr, "Missing units for leveladd (min) on station %s\n", rec->header.name);
			}
                    }

                    if (parse_line (string, "<levelmultiply value=", tname, sizeof(tname))) 
		        require (sscanf (tname, "%f", &rec->min_level_multiply) == 1);

                    /* Ignore avglevel */

                    if (parse_line (string, "<direction value=", tname, sizeof(tname))) 
                    {
		        require (sscanf (tname, "%d", &rec->min_direction) == 1);
                        if (rec->min_direction < 0) rec->min_direction += 360;
                        rec->min_direction %= 360;

                        if (parse_line (string, "units=", tname, sizeof(tname))) {
                          /* FIXME, clashes are not detected. */
                          i = find_dir_units (tname);
                          if (i >= 0)
                            rec->direction_units = (NV_U_BYTE)i;
                          else
			    fprintf(stderr, "ERROR parsing direction (min) units on station %s\n", rec->header.name);
			}  else {
			    fprintf(stderr, "Missing units for direction (min) on station %s\n", rec->header.name);
			}
                    }
                }
            }

            if (parse_line (string, "<floodbegins value=", tname, sizeof(tname)))
                rec->flood_begins = get_time (tname);

            if (parse_line (string, "<ebbbegins value=", tname, sizeof(tname)))
                rec->ebb_begins = get_time (tname);
        }
        require (fgets (string, sizeof (string), fp) != NULL);
    }

    return (NVTrue);
}
