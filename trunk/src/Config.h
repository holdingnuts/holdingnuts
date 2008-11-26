#ifndef _CONFIG_H
#define _CONFIG_H

/* version(major,minor,revision) */
#define VERSION_MAJOR     0
#define VERSION_MINOR     0
#define VERSION_REVISION  1

#define DEFAULT_SERVER_PORT  12345
#define MAX_PLAYERS  10


/* TODO: move to other file
   Example: 2.12.123 = 2*100000 + 12*1000 + 123 = 212123
 */
#define VERSION_CREATE(major,minor,revision) \
	(major*100000 + minor*1000 + revision)

#define VERSION_GETMAJOR(version) \
	(version / 100000)

#define VERSION_GETMINOR(version) \
	((version - VERSION_GETMAJOR(version)*100000) / 1000)

#define VERSION_GETREVISION(version) \
	((version - VERSION_GETMAJOR(version)*100000) - VERSION_GETMINOR(version)*1000)

#endif /* _CONFIG_H */
