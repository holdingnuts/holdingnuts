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


#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <map>
#include <sqlite3.h>


class QueryResult
{
friend class Database;

public:
	int numCols() const { return ncol; };
	int numRows() const { return nrow; };
	const char *getRow(int col, int row);
	const char *getCol(int col);
	
private:
	QueryResult(char **result, int nrow, int ncol);
	~QueryResult();
	char **result;
	int nrow, ncol;
};

class Database
{
public:
	Database();
	Database(const char *filename);
	~Database();
	
	int open(const char *filename);
	int query(const char *q, ...);
	int query(QueryResult **qr, const char *q, ...);
	
	void freeQueryResult(QueryResult **qr);
	
	char* createQueryString(const char *q, ...);
	void freeQueryString(char *q);
	
private:
	sqlite3 *db;
	
};

#endif /* DATABASE_H */
