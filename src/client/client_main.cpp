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


#include "Config.h"
#include "Logger.h"
#include "Debug.h"
#include "SysAccess.h"
#include "ConfigParser.hpp"


#include <cstdlib>
#include <cstdio>
#include <string>

#include <QApplication>
#include <QDir>
#include <QString>
#include <QUuid>


ConfigParser config;

bool config_load()
{
	// include config defaults
	#include "client_variables.hpp"
	
	char cfgfile[1024];
	snprintf(cfgfile, sizeof(cfgfile), "%s/client.cfg", sys_config_path());
	
	if (config.load(cfgfile))
		log_msg("config", "Loaded configuration from %s", cfgfile);
	else
	{
		// override defaults
		
		// determine system username and use it as default playername
		const char *name = sys_username();
		if (name)
			config.set("player_name", std::string(name));
		
		// generate an UUID
		QString suuid = QUuid::createUuid().toString();
		suuid = suuid.mid(1, suuid.length() - 2);
		config.set("uuid", suuid.toStdString());
		
		if (config.save(cfgfile))
			log_msg("config", "Saved initial configuration to %s", cfgfile);
	}
	
	return true;
}

int main(int argc, char **argv)
{
	log_set(stdout, 0);
	
	log_msg("main", "HoldingNuts pclient (version %d.%d.%d; svn %s; Qt version %s)",
		VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION,
		VERSIONSTR_SVN,
		qVersion());
	
	
	// the app instance
	//PClient app(argc, argv);
	
	
	// load config
	config_load();
	config.print();
	
	// start logging
	filetype *fplog = NULL;
	if (config.getBool("log"))
	{
		char logfile[1024];
		snprintf(logfile, sizeof(logfile), "%s/client.log", sys_config_path());
		fplog = file_open(logfile, config.getBool("log_append")
				? mode_append
				: mode_write);
		
		// log destination
		log_set(stdout, fplog);
		
		// log timestamp
		if (config.getBool("log_timestamp"))
			log_use_timestamp(1);
	}
	
#if defined(DEBUG) && defined(PLATFORM_WINDOWS)
	char dbgfile[1024];
	snprintf(dbgfile, sizeof(dbgfile), "%s/client.debug", sys_config_path());
	/*filetype *dbglog = */ file_reopen(dbgfile, mode_write, stderr);  // omit closing
#endif
	
	
	//if (app.init())
	//	return 1;
	//
	//int retval = app.exec();
	
	
	// close log-file
	if (fplog)
		file_close(fplog);
	
	return 0;
}
