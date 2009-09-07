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


#include <stdarg.h>

#include "Database.hpp"
#include "Debug.h"

using namespace std;

Database::Database()
{
	db = 0;
}

Database::Database(const char *filename)
{
	open(filename);
}

Database::~Database()
{
	if (db)
		sqlite3_close(db);
}

int Database::open(const char *filename)
{
	int rc;
	
	/* open sqlite database */
	rc = sqlite3_open(filename, &db);
	if (rc)
	{
		dbg_msg("sqlite", "Error: Can't open database: %s", sqlite3_errmsg(db));
		sqlite3_close(db);
		db = 0;
	}
	
	return rc;
}

int Database::query(const char *q, ...)
{
	if (!db)
		return SQLITE_ERROR;
	
	va_list args;
	
	char *zErrMsg;
	
	va_start(args, q);
	char *qstr = sqlite3_vmprintf(q, args);
	va_end(args);
	
	dbg_msg("sqlite", "query= %s", qstr);
	
	int rc = sqlite3_exec(db, qstr, 0, 0, &zErrMsg);
	
	sqlite3_free(qstr);
	
	if (rc != SQLITE_OK)
	{
		dbg_msg("sqlite", "Error: %s", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	
	return rc;
}

int Database::query(QueryResult **qr, const char *q, ...)
{
	if (!db)
		return SQLITE_ERROR;
	
	va_list args;
	
	char **result;
	char *zErrMsg;
	int nrow, ncol;
	
	va_start(args, q);
	char *qstr = sqlite3_vmprintf(q, args);
	va_end(args);
	
	dbg_msg("sqlite", "query= %s", qstr);
	
	int rc = sqlite3_get_table(
		db,              /* An open database */
		qstr,       /* SQL to be executed */
		&result,       /* Result written to a char *[]  that this points to */
		&nrow,             /* Number of result rows written here */
		&ncol,          /* Number of result columns written here */
		&zErrMsg          /* Error msg written here */
		);
	
	sqlite3_free(qstr);
	
	if (rc != SQLITE_OK)
	{
		dbg_msg("sqlite", "Error: %s", zErrMsg);
		sqlite3_free(zErrMsg);
		*qr = 0;
	}
	else
		*qr = new QueryResult(result, nrow, ncol);
	
	return rc;
}

void Database::freeQueryResult(QueryResult **qr)
{
	if (qr)
		return;
	
	delete qr;
	qr = 0;
}

char* Database::createQueryString(const char *q, ...)
{
	va_list args;
	char *qstr;
	
	va_start(args, q);
	qstr = sqlite3_vmprintf(q, args);
	va_end(args);
	
	return qstr;
}

void Database::freeQueryString(char *q)
{
	sqlite3_free(q);
}

QueryResult::QueryResult(char **result, int nrow, int ncol)
{
	this->result = result;
	this->nrow = nrow;
	this->ncol = ncol;
}

const char* QueryResult::getCol(int col)
{
	return result[col];
}

const char* QueryResult::getRow(int col, int row)
{
	return result[this->ncol + this->ncol * row + col];
}

QueryResult::~QueryResult()
{
	sqlite3_free_table(result);
}
