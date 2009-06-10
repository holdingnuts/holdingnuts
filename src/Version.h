/*
 * Copyright 2008, 2009, Dominik Geyer
 *
 * This file is part of HoldingNuts.
 *
 * HoldingNuts is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HoldingNuts is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HoldingNuts.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Dominik Geyer <dominik.geyer@holdingnuts.net>
 */


#ifndef _VERSION_H
#define _VERSION_H

/* version(major,minor,revision) */
#define VERSION_MAJOR     0
#define VERSION_MINOR     0
#define VERSION_REVISION  5


/* Example: Version 2.12.123 = 2*100000 + 12*1000 + 123 = 212123 */
#define VERSION_CREATE(major,minor,revision) \
	(major*100000 + minor*1000 + revision)

#define VERSION_GETMAJOR(version) \
	(version / 100000)

#define VERSION_GETMINOR(version) \
	((version - VERSION_GETMAJOR(version)*100000) / 1000)

#define VERSION_GETREVISION(version) \
	((version - VERSION_GETMAJOR(version)*100000) - VERSION_GETMINOR(version)*1000)


#define VERSION VERSION_CREATE(VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION)

/* Last compatible version */
#define VERSION_COMPAT VERSION_CREATE(0, 0, 4)

/* provided SVN revision */
#ifdef SVN_REV
# define VERSIONSTR_SVN	"r" SVN_REV
#else
# define VERSIONSTR_SVN	"-"
#endif

#endif /* _VERSION_H */
