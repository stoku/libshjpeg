/*
 * libshjpeg: A library for controlling SH-Mobile JPEG hardware codec
 *
 * Copyright (C) 2010 IGEL Co.,Ltd.
 * Copyright (C) 2008,2009 Renesas Technology Corp.
 * Copyright (C) 2008 Denis Oliver Kropp
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA  02110-1301 USA
 */

/*Additional error messages using the libjpeg ERREXITn macros.
  Since only one add one table is allowed we add these error
  messages to the end of the standard message table used by
  libjpeg itself.
  There should be suffient space between the last libjpeg message
  code and any user defined ones (the documentation suggests applications
  start their indices at 1000).*/

#include <jversion.h>
#ifndef JERROR_H
#include <jerror.h>
#endif

#ifndef JMESSAGE
#ifndef LIBMESSAGE_H
/* First time through, define the enum list */
#define JMAKE_ENUM_LIST
#else
/* Repeated inclusions of this file are no-ops unless JMESSAGE is defined */
#define JMESSAGE(code,string)
#endif /* JERROR_H */
#endif /* JMESSAGE */

#ifdef JMAKE_ENUM_LIST

typedef enum {

#define JMESSAGE(code,string)   code ,

#endif /* JMAKE_ENUM_LIST */
 /* Must be first entry! */
JMESSAGE(SHJMSG_NOMESSAGE=JMSG_LASTMSGCODE, "Bogus message code %d")
JMESSAGE(SHJMSG_BAD_VERSION,
        "Wrong JPEG wrapper version: wrapper compiled using %d,\n"
        "but caller expects %d.\n"
        "Application, wrapper, and library versions must all match.")
JMESSAGE(SHJMSG_INVALID_CONTEXT,
        "Parallel processing of multiple images not supported.")
JMESSAGE(SHJMSG_COMPRESS_ERR, "JPEG compression error in JPU")
JMESSAGE(SHJMSG_JPU_MODE, "Processing in JPU mode")
JMESSAGE(SHJMSG_LIBJPEG_MODE, "Processing in libjpeg mode")
#ifdef JMAKE_ENUM_LIST

  SHJMSG_LASTMSGCODE
} SHJ_MESSAGE_CODE;

#undef JMAKE_ENUM_LIST
#endif /* JMAKE_ENUM_LIST */
#undef JMESSAGE

#ifndef LIBMESSAGE_H
#define LIBMESSAGE_H
#endif
