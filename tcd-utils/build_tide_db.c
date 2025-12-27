/* $Id: build_tide_db.c 8293 2024-02-22 21:03:18Z flaterco $ */

#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <tcd.h>

#include "build_tide_db.h"

/* This should be done with stdbool.h, but VC doesn't have it. */
/* Using crappy old int, must be careful not to 'require' a 64-bit value. */
#ifndef require
#define require(expr) {       \
  int require_expr;           \
  require_expr = (int)(expr); \
  if (!require_expr) {        \
    format_barf ("failed assertion: " #expr, "build_tide_db.c", __LINE__); \
    abort();                  \
  }                           \
}
#endif


/*****************************************************************************\

                               DISTRIBUTION STATEMENT

     This source file is unclassified, distribution unlimited, public
     domain.  It is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/



/*****************************************************************************\

     Program         build_tide_db - builds a harmonic constituent database
                     from XTide format harmonics/offset files and/or NAVO format
                     harmonics files.

     Synopsis        build_tide_db OUTPUT_DB_NAME MASTER_HARMONIC_NAME [more harmonics or xml or navo files]

                     OUTPUT_DB_NAME          database name, the extension .tcd
                                             will be appended to the name if
                                             not already there
                     MASTER_HARMONIC_NAME    ASCII harmonics file that the database will use to create
                     the parameters for the stations
                     [...]                   additional ASCII harmonics or offsets files (in XML format), or
                     NAVO files.

                     Returns         NV_INT32                0 on successful completion, -1 on
                     failure

     Author          Jan C. Depner
     Date            08/01/02

     Modified        David Flater / August Hahn
     See README for details.

\*****************************************************************************/

NV_BOOL parse_xml_station (FILE *fp, TIDE_RECORD *rec);
NV_BOOL parse_line (NV_CHAR *in, NV_CHAR *search, NV_CHAR *result, size_t rlen);

void format_barf (char const * const barfmessage,
                  char const * const file,
                  int line) {
  fprintf (stderr, "%s line %d: %s\n\
\n\
This program (build_tide_db) was intended to be used as a throwaway migration\n\
utility.  As such, it was never made robust for arbitrary input.  To maximize\n\
your chances of success, copy the formatting of a legacy harmonics file as\n\
closely as possible.  Even blank lines and comments matter.\n", file, line, barfmessage);
}

NV_INT32 ProcessHarmonicsFile(FILE* fp, NV_U_INT32 num_constituents)
{
    NV_CHAR                    string[256], tname[100];
    NV_INT32                   i, j, num, comment_pos = 0, cnt = 0;
    NV_BOOL                    got_units;
    TIDE_RECORD                rec;
    int startover = 1;

    /*  XTide format ASCII constituent file.  */

    /*  Skip past the speeds and factors since we already have these.  */
    i = 0;
    while (i < 2)
    {
        require (fgets (string, sizeof (string), fp) != NULL);
	if (!strncmp (string, "*END*", 5)) ++i;
    }

    /*  Right now this is hard-wired to get past the verbiage at the
	beginning of the constituents.  This is not part of the
	comments for the first record and I need to grab those
	comments.  */
    /* DWF 2004-08-09 changed from counting 27 lines to looking for
       MERCHANTABILITY. */
    /* DWF 2004-08-15 if not found, it's a warning, not an error. */
    do {
      if (!fgets (string, sizeof (string), fp)) {
        format_barf ("format warning: no tide records found.  Creating valid harmonics file containing constituent definitions only.", "build_tide_db.c", __LINE__);
        return 0;
      }
      require (string[0] == '#');
    } while (!strstr (string, "MERCHANTABILITY"));
    /* Lose extra blank */
    require (fgets (string, sizeof (string), fp) != NULL);
    require (!strcmp (string, "#\n"));

    while (fgets (string, sizeof (string), fp))
    {
        if (startover) {
          startover = comment_pos = 0;
          got_units = NVFalse;
          memset (&rec, 0, sizeof (TIDE_RECORD));
          rec.header.record_type = 1;
          rec.header.reference_station = -1;
          rec.confidence = 10;
	  rec.restriction = (NV_U_BYTE) find_or_add_restriction ("Non-commercial use only", NULL);
  	  rec.datum = find_datum ("Mean Lower Low Water");
          rec.max_direction = rec.min_direction = 361;
	  {
	    int r = find_dir_units ("degrees true");
	    require (r > 0);
	    rec.direction_units = (NV_U_BYTE)r;
	  }
        }

	/*  Comments.  */
	if (string[0] == '#' || strlen (string) < 2)
	{
	    /* FIXME fixup hot comments protocol after X-fields added */
	    if (strstr (string, "BEGIN HOT COMMENTS"))
	      ;
            else if (!strncmp (string, "# country:", 10))
	      rec.country = find_or_add_country (string+10, NULL);
	    else if (!strncmp (string, "# Source:", 9) ||
                     !strncmp (string, "# source:", 9))
	    {
		if ((strlen (&string[9])) < ONELINER_LENGTH)
		{
                  if (string[9] == ' ')
		    strcpy (rec.source, &string[10]);
                  else
		    strcpy (rec.source, &string[9]);
		} else {
		    fprintf (stderr, "libtcd fatal error:  Source '%s' exceeded ONELINER_LENGTH compiled-in limit %d.  Change limit and recompile.\n", string+9, ONELINER_LENGTH);
		    exit (-1);
		}
	    }
            else if (!strncmp (string, "# restriction:", 14))
              rec.restriction = find_or_add_restriction (string+14, NULL);
            /* Space intentionally included */
            else if (!strncmp (string, "# note: ", 8))
              strcat (rec.notes, string+8);
            else if (!strncmp (string, "# station_id_context:", 21)) {
              if ((strlen (&string[21])) < ONELINER_LENGTH)
                strcpy (rec.station_id_context, string+21);
              else {
		    fprintf (stderr, "libtcd fatal error:  Station ID context '%s' exceeded ONELINER_LENGTH compiled-in limit %d.  Change limit and recompile.\n", string+21, ONELINER_LENGTH);
		    exit (-1);
	      }
            } else if (!strncmp (string, "# station_id:", 13)) {
              if ((strlen (&string[13])) < ONELINER_LENGTH)
                strcpy (rec.station_id, string+13);
              else {
		    fprintf (stderr, "libtcd fatal error:  Station ID '%s' exceeded ONELINER_LENGTH compiled-in limit %d.  Change limit and recompile.\n", string+13, ONELINER_LENGTH);
		    exit (-1);
	      }
	    } else if (!strncmp (string, "# date_imported:", 16)) {
              require (sscanf (string+16, "%u", &(rec.date_imported)) == 1);
            } else if (!strncmp (string, "# max_direction:", 16)) {
              require (sscanf (string+16, "%d", &(rec.max_direction)) == 1);
            } else if (!strncmp (string, "# min_direction:", 16)) {
              require (sscanf (string+16, "%d", &(rec.min_direction)) == 1);
            } else if (!strncmp (string, "# Datum:", 8) ||
                       !strncmp (string, "# datum:", 8)) {
	      rec.datum = find_or_add_datum (&string[8], NULL);
            } else if (!strncmp (string, "# Datumkind:", 12) ||
                       !strncmp (string, "# datumkind:", 12)) {
	      rec.datum = find_or_add_datum (&string[12], NULL);
            } else if (!strncmp (string, "# expiration:", 13)) {
              require (sscanf (string+13, "%u", &(rec.expiration_date)) == 1);
            } else if (!strncmp (string, "# used_until:", 13)) {
              unsigned y, m, d;
              require (sscanf (string+13, "%u-%u-%u", &y, &m, &d) == 3);
              rec.expiration_date = y*10000 + m*100 + d;
            } else if (!strncmp (string, "# months_on_station:", 20)) {
              unsigned m;
              require (sscanf (string+20, "%u", &m) == 1);
              rec.months_on_station = (NV_U_INT16)m;
            } else if (!strncmp (string, "# last_date_on_station:", 23)) {
              require (sscanf (string+23, "%u", &(rec.last_date_on_station)) == 1);
            } else if (!strncmp (string, "# confidence:", 13)) {
              unsigned i;
              require (sscanf (string+13, "%u", &i) == 1);
              rec.confidence = (NV_U_BYTE)i;
            } else if (!strncmp (string, "# !units:", 9) ||
                     !strncmp (string, "#!units:", 8))
	    {
		/*  Do nothing.  This is superceded by the datum units.  */
	    }
	    else if (!strncmp (string, "# !longitude:", 13) ||
	      !strncmp (string, "#!longitude:", 12))
	    {
                /* Not required */
		sscanf (&string[13], "%lf", &rec.header.longitude);
	    }
	    else if (!strncmp (string, "# !latitude:", 12) ||
	      !strncmp (string, "#!latitude:", 11))
	    {
                /* Not required */
		sscanf (&string[12], "%lf", &rec.header.latitude);
	    }
	    else
	    {
              /* FIXME unknown hot comments will become X-fields */
		if (strlen(string) < 3) {
		    /* Blank line */
		    if (comment_pos + 1 >= MONOLOGUE_LENGTH) {
			fprintf (stderr, "libtcd fatal error:  Comments exceeded MONOLOGUE_LENGTH compiled-in limit %d.  Change limit and recompile.\n", MONOLOGUE_LENGTH);
                        exit (-1);
		    }
		    rec.comments[comment_pos++] = 10;
		} else {
		    num = strlen (string+1);
		    require (num > 0);
		    if (comment_pos + num >= MONOLOGUE_LENGTH) {
			fprintf (stderr, "libtcd fatal error:  Comments containing '%s' exceeded MONOLOGUE_LENGTH compiled-in limit %d.  Change limit and recompile.\n", string+1, MONOLOGUE_LENGTH);
                        exit (-1);
		    } else {
                        /* Compress out leading blank, if present */
                        if (string[1] == ' ') {
			    strncpy (&rec.comments[comment_pos], string+2, num-1);
			    comment_pos += num-1;
                        } else {
			    /* stringop-truncation warning here is irrelevant.  The
                               whole record was zero-initialized. */
			    strncpy (&rec.comments[comment_pos], string+1, num);
			    comment_pos += num;
                        }
		    }
		}
	    }
	}


	/*  Not comments.  */

	else
	{
	    if ((strlen (string)) < ONELINER_LENGTH)
	    {
		if (string[strlen(string) - 1] == '\n')
		    string[strlen(string) - 1] = '\0';
		strcpy (rec.header.name, string);
	    }
	    else
	    {
		fprintf (stderr, "libtcd fatal error:  Name '%s' exceeded ONELINER_LENGTH compiled-in limit %d.  Change limit and recompile.\n", string, ONELINER_LENGTH);
		exit (-1);
	    }

	    require (fgets (string, sizeof (string), fp) != NULL);
	    require (sscanf (string, "%d:%d %s", &i, &j, tname) == 3);
	    rec.zone_offset = get_time (string);

	    rec.header.tzfile = find_or_add_tzfile (tname, NULL);

	    require (fgets (string, sizeof (string), fp) != NULL);
	    require (sscanf (string, "%f %s", &rec.datum_offset, tname) == 2);
	    i = find_level_units (tname);

	    if (i < 0)
	      got_units = NVFalse;
	    else {
 	      got_units = NVTrue;
	      rec.level_units = i;
	    }

	    for (i = 0 ; i < (int)num_constituents ; i++)
	    {
  	        require (fgets (string, sizeof (string), fp) != NULL);

		require (sscanf (string, "%s %f %f", tname, &rec.amplitude[i], &rec.epoch[i]) == 3);

		/*  We don't need no stinkin' negatives!  */

		if (rec.amplitude[i] < 0.0)
		{
		    rec.amplitude[i] = -rec.amplitude[i];
		    rec.epoch[i] += 180.0;
		}

		while (rec.epoch[i] < 0.0) rec.epoch[i] += 360.0;
		rec.epoch[i] = fmod ((NV_FLOAT64) rec.epoch[i], 360.0);
	    }

	    /*  Add the final 0 to the comments record since we stripped
		it on input.  */

	    rec.comments[comment_pos - 1] = (char)0;

	    if (got_units)
	    {
		add_tide_record (&rec, NULL);
		cnt++;
	    }
	    else
	    {
		fprintf (stderr, "Invalid level units for record - %s\n", rec.header.name);
	    }

	    startover = 1;
	}
    }

    return cnt;
}

NV_INT32 ProcessNavoFile(FILE* fp)
{
    NV_CHAR                    string[256], dum1[3], dum2[5], name[4][12];
    NV_FLOAT32                 amp[4], epo[4], dummy;
    NV_INT32                   i, j, k, num, cnt = 0, year, ilat, ilon, deg, 
                               min;
    TIDE_RECORD                rec;

    memset (&rec, 0, sizeof (TIDE_RECORD));
    while (fgets (string, sizeof (string), fp))
    {
	strcpy (rec.header.name, &string[1]);

	require (fgets (string, sizeof (string), fp) != NULL);

	sscanf (string, "%d %d %d %f %f %s %s %d %d", &i, &ilat, &ilon,
	 &dummy, &rec.datum_offset, dum1, dum2, &year, &num);

	deg = abs (ilat) / 100;
	min = abs (ilat) % 100;
	rec.header.latitude = (NV_FLOAT64) deg + (NV_FLOAT64) min / 60.0;
	if (ilat < 0) rec.header.latitude = -rec.header.latitude;

	deg = abs (ilon) / 100;
	min = abs (ilon) % 100;
	rec.header.longitude = (NV_FLOAT64) deg + (NV_FLOAT64) min / 60.0;
	if (ilon < 0) rec.header.longitude = -rec.header.longitude;

	sprintf (rec.comments, "Year of data : %d\n", year);

	rec.last_date_on_station = year * 10000 + 601;


        /*  DOH!  Fixed a really dumb SOD here (I can't reference name[i]).
            JCD */

	for (i = 0 ; i < num ; i++)
	{
	    if (!(k = i % 4))
	    {
	        require (fgets (string, sizeof (string), fp) != NULL);

		sscanf (string, " %5s %f %f %5s %f %f %5s %f %f %5s %f %f",
                        name[k], &amp[k], &epo[k], name[k + 1], &amp[k + 1],
                        &epo[k + 1], name[k + 2], &amp[k + 2], &epo[k + 2],
                        name[k + 3], &amp[k + 3], &epo[k + 3]);
	    }

	    j = find_constituent (name[k]);

	    if (j == -1)
		fprintf (stderr, "Ignoring unknown constituent : %s\n", name[k]);


	    /*  It appears that NAVO uses 999.9 as a NULL epoch.  */

	    if (epo[k] != 999.9)
	    {
		rec.amplitude[j] = amp[k];
		rec.epoch[j] = epo[k];


		/*  We don't need no stinkin' negatives!  */

		if (rec.amplitude[j] < 0.0)
		{
		    rec.amplitude[j] = -rec.amplitude[j];
		    rec.epoch[j] += 180.0;
		}

		while (rec.epoch[j] < 0.0) rec.epoch[j] += 360.0;
		rec.epoch[j] = fmod ((NV_FLOAT64) rec.epoch[j], 360.0);
	    }
	}

	/*  Defaults (for now).  */

	rec.header.record_type = 1;
	rec.header.reference_station = -1;
	rec.restriction = 1;
	rec.expiration_date = 0;
	rec.months_on_station = 0;
	rec.confidence = 10;
	rec.zone_offset = 0;
        rec.country = 0;
	rec.level_units = find_level_units ("meters");
	rec.header.tzfile = find_or_add_tzfile ("GMT0", NULL);
	rec.datum = find_datum ("Mean Lower Low Water");
	strcpy (rec.source, "IHO/NAVO ihbtides file");

	add_tide_record (&rec, NULL);
	cnt++;
	memset (&rec, 0, sizeof (TIDE_RECORD));
    }

    return cnt;
}

NV_INT32 ProcessXMLFile(FILE* fp)
{
    NV_CHAR                    string[256], comments[MONOLOGUE_LENGTH];
    NV_BOOL                    done;
    TIDE_RECORD                rec;
    NV_INT32	cnt = 0; /* DWF added initialization */
    int incomment=0;

    done = NVFalse;

    while (!done)
    {
        require (fgets (string, sizeof (string), fp) != NULL);

	if (!strncmp (string, "<document>", 10)) done = NVTrue;
    }

    comments[0] = '\0';
    while (fgets (string, sizeof (string), fp))
    {
      if (!strncmp (string, "<!-- Comments on ", 17))
        incomment=1;
      else if (incomment) {
        if (!strncmp (string, "-->", 3))
          incomment=0;
        else {
          require (strlen(string) + strlen(comments) + 1 < MONOLOGUE_LENGTH);
          strcat (comments, string);
        }
      } else if (parse_line (string, "<subordinatestation name=", rec.header.name, sizeof(rec.header.name)))
	{
	  if (parse_xml_station (fp, &rec))
	    {
	        /* Evil */
  	        /* rec.country = locate_country(rec.header.name); */
                /* Stupid notes workaround */
                if (strlen (rec.comments)) {
                  strcat (comments, "\n");
                  strcat (comments, rec.comments);
                }
                strcpy (rec.comments, comments);
		add_tide_record (&rec, NULL);
		cnt++;
                comments[0] = '\0';
	    }
	}
    }

    return cnt;
}

void CreateTCDFromHarmonics(NV_CHAR* dbname, FILE* fp)
{
    NV_CHAR	string[256], tname[100], **constituent;
    NV_FLOAT64	*speed, temp[10];
    NV_FLOAT32	**equilibrium, **node_factor;
    NV_INT32	i, constituents, start_year, num_years, num, j, end, k;
    NV_BOOL	done;

    /*  Get the number of constituents.  */

    done = NVFalse;
    while (!done)
    {
	require (fgets (string, sizeof (string), fp) != NULL);

	if (string[0] != '#')
        {
	    require (sscanf (string, "%d", &constituents) == 1);
	    done = NVTrue;
	}
    }

    require (constituents > 0);
    if (constituents > MAX_CONSTITUENTS) {
	fprintf (stderr, "libtcd fatal error:  exceeded MAX_CONSTITUENTS compiled-in limit %d.  Change limit and recompile.\n", MAX_CONSTITUENTS);
	exit (-1);
    }

    if ((speed = (NV_FLOAT64 *) malloc (constituents * sizeof (NV_FLOAT64))) == NULL)
    {
	perror ("Allocating speed memory");
	exit (-1);
    }

    if ((constituent = (NV_CHAR **) malloc (constituents * sizeof (NV_CHAR *))) == NULL)
    {
	perror ("Allocating name memory");
	exit (-1);
    }

    /*  Get the constituent names and speeds.  */

    done = NVFalse;
    while (!done)
    {
        require (fgets (string, sizeof (string), fp) != NULL);

	if (string[0] != '#')
	{
	    for (i = 0 ; i < constituents ; i++)
	    {
		require (sscanf (string, "%s %lf", tname, &speed[i]) == 2);

		constituent[i] = (NV_CHAR *) malloc (strlen (tname) + 1);
		strcpy (constituent[i], tname);

		require (fgets (string, sizeof (string), fp) != NULL);
	    }
	    done = NVTrue;
	}
    }

    /*  Get the start year.  */

    done = NVFalse;
    while (!done)
    {
        require (fgets (string, sizeof (string), fp) != NULL);

	if (string[0] != '#')
	{
	    require (sscanf (string, "%d", &start_year) == 1);
	    done = NVTrue;
	}
    }

    /*  Get the number of years.  */

    done = NVFalse;
    while (!done)
    {
        require (fgets (string, sizeof (string), fp) != NULL);

	if (string[0] != '#')
	{
	    require (sscanf (string, "%d", &num_years) == 1);
	    done = NVTrue;
	}
    }

    /*  Get the equilibrium arguments.  */

    equilibrium = (NV_FLOAT32 **) malloc (constituents * sizeof (NV_FLOAT32 *));

    done = NVFalse;
    while (!done)
    {
        require (fgets (string, sizeof (string), fp) != NULL);

	if (string[0] != '#')
	{
	    for (i = 0 ; i < constituents ; i++)
	    {
		/*  Check the constituent name just for grins.  */

		if (strncmp (string, constituent[i], strlen ( constituent[i])))
		{
		    fprintf (stderr, "Constituent names don't match!\n");
		    fprintf (stderr, "%s %s\n", string, constituent[i]);
		    exit (-1);
		}

		equilibrium[i] = (NV_FLOAT32 *) malloc (num_years * sizeof (NV_FLOAT32));

		num = num_years / 10;
		if (num_years % 10) num++;

		for (j = 0 ; j < num ; j++)
		{
		    require (fgets (string, sizeof (string), fp) != NULL);
		    require ((sscanf (string, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
	      &temp[0], &temp[1], &temp[2], &temp[3],
	      &temp[4], &temp[5], &temp[6], &temp[7],
	      &temp[8], &temp[9]) == 10) || (j+1 == num));

		    end = j * 10 + 10;
		    if (end > num_years) end = num_years;
		    for (k = j * 10 ; k < end ; k++)
		    {
			equilibrium[i][k] = temp[k - j * 10];
		    }
		}
		require (fgets (string, sizeof (string), fp) != NULL);
	    }
	    done = NVTrue;
	}
    }

    /*  Get the middle of year node factors.  */

    node_factor = (NV_FLOAT32 **) malloc (constituents * sizeof (NV_FLOAT32 *));

    done = NVFalse;
    while (!done)
    {
        require (fgets (string, sizeof (string), fp) != NULL);

	if (string[0] != '#' && string[0] != '*')
	{
	    /*  Check the number of years, I don't know what to do if
	    they don't match.  */

	    require (sscanf (string, "%d", &num) == 1);
	    if (num != num_years)
	    {
		fprintf (stderr, "Number of years don't match!\n");
		fprintf (stderr, "%d %d\n", num, num_years);
		exit (-1);
	    }

	    /*  Read the first constituent name.  */
	    require (fgets (string, sizeof (string), fp) != NULL);

	    for (i = 0 ; i < constituents ; i++)
	    {
		/*  Check the constituent name just for grins.  */

		if (strncmp (string, constituent[i], strlen ( constituent[i])))
		{
		    fprintf (stderr, "Constituent names don't match!\n");
		    fprintf (stderr, "%s %s\n", string, constituent[i]);
		    exit (-1);
		}

		node_factor[i] = (NV_FLOAT32 *) malloc (num_years * sizeof (NV_FLOAT32));

		num = num_years / 10;
		if (num_years % 10) num++;
		for (j = 0 ; j < num ; j++)
		{
		    require (fgets (string, sizeof (string), fp) != NULL);
		    require ((sscanf (string, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
                            &temp[0], &temp[1], &temp[2], &temp[3],
                            &temp[4], &temp[5], &temp[6], &temp[7],
                            &temp[8], &temp[9]) == 10) || (j+1 == num));

		    end = j * 10 + 10;
		    if (end > num_years) end = num_years;
		    for (k = j * 10 ; k < end ; k++)
		    {
			node_factor[i][k] = temp[k - j * 10];
		    }
		}
		require (fgets (string, sizeof (string), fp) != NULL);
	    }
	    done = NVTrue;
	}
    }

    /*  Create the database.  */

    if (!create_tide_db (dbname, constituents,
			 (const NV_CHAR **)constituent,
			 speed, start_year, num_years,
			 (const NV_FLOAT32 **)equilibrium,
			 (const NV_FLOAT32 **)node_factor))
    {
	fprintf (stderr, "Error creating data base file.\n");
	exit (-1);
    }

    /* Input file fp left in open but otherwise unspecified state */

    for (i = 0 ; i < constituents ; i++)
    {
	if (constituent[i] != NULL) free (constituent[i]);
    }
    if (constituent != NULL) free (constituent);

    if (speed != NULL) free (speed);

    for (i = 0 ; i < constituents ; i++)
    {
	if (equilibrium[i] != NULL) free (equilibrium[i]);
    }
    if (equilibrium != NULL) free (equilibrium);

    for (i = 0 ; i < constituents ; i++)
    {
	if (node_factor[i] != NULL) free (node_factor[i]);
    }
    if (node_factor != NULL) free (node_factor);
}

void CreateTCDFromNavo(NV_CHAR* dbname)
{
    NV_FLOAT32	**equilibrium, **node_factor;
    NV_INT32	i, j, constituents, start_year, num_years;

    /*  NAVO format.  */
    /*  Use defaults for speeds, etc.  */

    constituents = DEFAULT_CONSTITUENTS;
    start_year = DEFAULT_START_YEAR;
    num_years = DEFAULT_NUM_YEARS;

    equilibrium = (NV_FLOAT32 **) malloc (constituents * sizeof (NV_FLOAT32 *));
    node_factor = (NV_FLOAT32 **) malloc (constituents * sizeof (NV_FLOAT32 *));

    for (i = 0 ; i < constituents ; i++)
    {
	equilibrium[i] = (NV_FLOAT32 *) malloc (num_years * sizeof (NV_FLOAT32));
	node_factor[i] = (NV_FLOAT32 *) malloc (num_years * sizeof (NV_FLOAT32));
	for (j = 0 ; j < num_years ; j++)
	{
	    equilibrium[i][j] = default_equilibrium[i][j];
	    node_factor[i][j] = default_node_factor[i][j];
	}
    }

    /*  Create the database.  */

    if (!create_tide_db (dbname, constituents,
			 (const NV_CHAR **)default_constituent,
			 default_speed, start_year, num_years,
			 (const NV_FLOAT32 **)equilibrium,
			 (const NV_FLOAT32 **)node_factor))
    {
	fprintf (stderr, "Error creating data base file.\n");
	exit (-1);
    }

    for (i = 0 ; i < constituents ; i++)
    {
	if (equilibrium[i] != NULL) free (equilibrium[i]);
    }
    if (equilibrium != NULL) free (equilibrium);


    for (i = 0 ; i < constituents ; i++)
    {
	if (node_factor[i] != NULL) free (node_factor[i]);
    }
    if (node_factor != NULL) free (node_factor);
}

NV_INT32 main (NV_INT32 argc, char *argv[])
{
    FILE                       *fp;
    NV_CHAR                    string[256], dbname[512];
    NV_INT32                   i, cnt = 0;
    NV_BOOL                    xml, navo;
    DB_HEADER_PUBLIC           db;

    if (argc < 3)
    {
        fprintf (stderr, "%s\n", PACKAGE_STRING);
	fprintf (stderr, "Usage: %s output_tcd_filename input_master_harm [input_child_harm...] [input_xml...] [input_nvo...]\n", argv[0]);
	exit(-1);

    } else {
	/*  Add the .tcd extension if it's not there.  */
	strcpy (dbname, argv[1]);
	if (!strstr (dbname, ".tcd")) strcat (dbname, ".tcd");
    }

    xml = NVFalse;
    navo = NVFalse;

    /* check first for an XML file at argv[2], exit if it's there. */
    if ((fp = fopen (argv[2], "r")) == NULL)
    {
	perror (argv[2]);
	exit (-1);
    } else {
	/*  Check for XML input file.  */
        require (fgets (string, sizeof (string), fp) != NULL);

	if (string[0] == '<')
	{
	    fprintf (stderr, "Cannot use an XML file as the first input file!\n");
	    exit (-1);
	}
	fclose(fp);
    }

    /*  Check to see if the data base file already exists.  If not, we'll create it.  */
    if ((fp = fopen (dbname, "rb+")) == NULL)
    {
	fprintf(stderr, "\nCreating TCD file from %s\n", argv[2]);

	/*  Check for old NAVO format file.  */
        if (strstr (argv[2], ".nvo")) navo = NVTrue;

        /*  NAVO format.  */
        if (navo)
        {
	  CreateTCDFromNavo(dbname);  /* CreateTCDFromNavo doesn't need a file! */
        } else {
	    /*  XTide format.  */
	    /*  Open the input file.  */
	    if ((fp = fopen (argv[2], "r")) == NULL)
	    {
		perror (argv[2]);
		exit (-1);
	    }
	    CreateTCDFromHarmonics(dbname, fp);
	    fclose(fp);
	}
    }
    else
    {
	/*  Database already existed so we're going to append data.
	This can be dangerous, as there is no checking to see if the stations are
	already in the .tcd file... */
        fclose (fp);

        if (!open_tide_db (dbname))
        {
            fprintf (stderr, "Unable to read harmonics database: %s\n", dbname);
            exit (-1);
        }
    }

    db = get_tide_db_header();

    for (i = 2; i < argc; i++)
    {
	fprintf(stderr, "\nProcessing file %s\n", argv[i]);

	navo = NVFalse;
	xml =  NVFalse;

	if ((fp = fopen (argv[i], "r")) == NULL)
	{
	    perror (argv[i]);
	    exit (-1);
	}

        /*  Check for old NAVO format file.  */
        if (strstr (argv[i], ".nvo")) {
	    navo = NVTrue;
	} else {
	    /*  Check for XML input file.  */
	    require (fgets (string, sizeof (string), fp) != NULL);

	    if (string[0] == '<') xml = NVTrue;

	    fseek (fp, 0, SEEK_SET);
	}

	/*  XTide format XML offset file (subordinate stations).  */
	if (xml)
	{
	    cnt += ProcessXMLFile(fp);
	}

	/*  NAVO format constituent file.  */
	else if (navo)
	{
	    cnt += ProcessNavoFile(fp);
	}

	/*  XTide constituent file.  */
	else
	{
	    cnt += ProcessHarmonicsFile(fp, db.constituents);
	}
	fclose(fp);
    }

    close_tide_db ();

    fprintf (stderr, "%d records written\n", cnt);

    return (0);
    
}
