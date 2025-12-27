/* $Id: restore_tide_db.h 1151 2006-11-21 01:37:00Z flaterco $ */

#include <stdio.h>
#include <tcd.h>



/*****************************************************************************\

                            DISTRIBUTION STATEMENT

    This source file is unclassified, distribution unlimited, public
    domain.  It is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/




/*  This file contains all of the extra verbiage for the XTide format
    harmonic constituent ASCII files and the XTide format subordinate station
    .xml files.  This is used by restore_tide_db when dumping a harmonic
    constituent database back to XTide format files.  */


/*  Legal BS.  */

NV_CHAR legal[] = {"\
# ** The following boilerplate text is a placeholder only.  Please       **\n\
# ** replace it with the boilerplate and changelog available separately. **\n\
#\n\
# Definitions and constants for XTide\n\
#\n\
# NOTE:  This file should not be used as-is to provide a constituent\n\
# set to harmgen because it may contain multiple constituents with the\n\
# same frequency.  A better constituent set is included in the harmgen\n\
# distribution (see the README).\n\
#\n\
#\n\
# ********* NOT FOR NAVIGATION ********\n\
#\n\
# *** DO NOT RELY ON THIS DATA FILE FOR DECISIONS THAT CAN RESULT IN ***\n\
# ***                   HARM TO ANYONE OR ANYTHING.                  ***\n\
#\n\
# This data file is distributed in the hope that it will be useful,\n\
# but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
#\n\
# Highly trustworthy tide predictions cannot be achieved on a zero\n\
# budget.  If you need guaranteed results, don't use this data!\n\
# Contact the tide authority for your region (NOAA in the U.S.).\n\
#\n\
# WHOSOEVER CHOOSES TO REDISTRIBUTE THIS DATA ASSUMES ALL LIABILITY\n\
# ASSOCIATED THEREWITH.  NEITHER DAVID FLATER NOR ANYONE ELSE INVOLVED\n\
# SHALL BE LIABLE PERSONALLY OR OTHERWISE FOR ANY LOSSES THAT THE\n\
# REDISTRIBUTOR MIGHT SUFFER AS A CONSEQUENCE OF ANY CLAIMS, BOGUS OR\n\
# OTHERWISE, THAT ARE MADE TO THIS DATA.\n\
#\n"};


NV_CHAR pre_num[] = {"\
#\n\
# ------------- Begin congen output -------------\n\
#\n\
# Number of constituents\n"};


NV_CHAR pre_speed[] = {"\
#\n\
# Constituent speeds\n\
# Format:  identifier [whitespace] speed [CR]\n\
# Speed is in degrees per solar hour.\n\
# Identifier is just a name for the constituent.  They are for\n\
# readability only; XTide assumes that the constituents will be listed\n\
# in the same order throughout this file.\n"};


NV_CHAR pre_year[] = {"\
#\n\
# Starting year for equilibrium arguments and node factors\n"};


NV_CHAR pre_equilibrium[] = {"\
#\n\
# The following table gives equilibrium arguments for each year that\n\
# we can predict tides for.  The equilibrium argument is in degrees for\n\
# the meridian of Greenwich, at the beginning of each year.\n\
#\n\
# First line:  how many years in this table [CR]\n\
# Remainder of table:  identifier [whitespace] arg [whitespace] arg...\n\
# Carriage returns inside the table will be ignored.\n\
#\n\
# The identifiers are for readability only; XTide assumes that they\n\
# are in the same order as defined above.\n\
#\n\
# DO NOT PUT COMMENT LINES INSIDE THE FOLLOWING TABLE.\n\
# DO NOT REMOVE THE \"*END*\" AT THE END.\n"};


NV_CHAR pre_node[] = {"\
#\n\
# Now come the node factors for the middle of each year that we can\n\
# predict tides for.\n\
#\n\
# First line:  how many years in this table [CR]\n\
# Remainder of table:  identifier [whitespace] factor [whitespace] factor...\n\
# Carriage returns inside the table will be ignored.\n\
#\n\
# The identifiers are for readability only; XTide assumes that they\n\
# are in the same order as defined above.\n\
#\n\
# DO NOT PUT COMMENT LINES INSIDE THE FOLLOWING TABLE.\n\
# DO NOT REMOVE THE \"*END*\" AT THE END.\n"};


NV_CHAR pre_data[] = {"\
#\n\
# ------------- End congen output -------------\n\
#\n\
# Harmonic constants.\n\
#\n\
# First line:  name of location\n\
# Second line:  time meridian [whitespace] tzfile\n\
# Third line:  DATUM [whitespace] units\n\
# Remaining lines:  identifier [whitespace] amplitude [whitespace] epoch\n\
#\n\
# The DATUM is the mean lower low water or equivalent constant for\n\
# calibrating the tide height.\n\
#\n\
# The time meridian takes the format [-]HH:MM and is hours east of\n\
# Greenwich.  For most data sets calibrated to Local Standard Time,\n\
# this is your time zone displacement in the _winter_.  Do not include\n\
# Daylight Savings Time!  If a data set is calibrated to GMT / UTC,\n\
# the meridian should be set to 0:00.\n\
#\n\
# The tzfile is a reference to a file in the zoneinfo directory as\n\
# described in the man page for tzset.\n\
#\n\
# Epoch is \"modified\" or \"adapted\" epoch in degrees, also known as\n\
# Kappa Prime.\n\
#\n\
# The constituent identifiers are for readability only.  XTide assumes\n\
# that they are in the same order as defined above.\n\
#\n\
# These data sets are distributed in the hope that they will be useful,\n\
# but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
#\n"};


NV_CHAR legal_xml[] = {"\
<!-- ** The following boilerplate text is a placeholder only.  Please ** -->\n\
<!-- ** replace it with the boilerplate and changelog available       ** -->\n\
<!-- ** separately.                                                   ** -->\n\
\n\
<!--\n\
\n\
Offset tide stations for use with XTide version 2.2.2 or later.\n\
\n\
All coordinates given in this file are approximate.  All tide\n\
predictions produced through the application of offsets are\n\
approximate.\n\
\n\
********* NOT FOR NAVIGATION ********\n\
\n\
*** DO NOT RELY ON THIS DATA FILE FOR DECISIONS THAT CAN RESULT IN ***\n\
***                   HARM TO ANYONE OR ANYTHING.                  ***\n\
\n\
This data file is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
\n\
Highly trustworthy tide predictions cannot be achieved on a zero\n\
budget.  If you need guaranteed results, don't use this data!\n\
Contact the tide authority for your region (NOAA in the U.S.).\n\
\n\
WHOSOEVER CHOOSES TO REDISTRIBUTE THIS DATA ASSUMES ALL LIABILITY\n\
ASSOCIATED THEREWITH.  NEITHER DAVID FLATER NOR ANYONE ELSE INVOLVED\n\
SHALL BE LIABLE PERSONALLY OR OTHERWISE FOR ANY LOSSES THAT THE\n\
REDISTRIBUTOR MIGHT SUFFER AS A CONSEQUENCE OF ANY CLAIMS, BOGUS OR\n\
OTHERWISE, THAT ARE MADE TO THIS DATA.\n\
\n"};


NV_CHAR pre_data_xml[] = {"\
-->\n\
\n\
<document>\n\
\n"};
