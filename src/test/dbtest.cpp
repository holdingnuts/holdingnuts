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


#include <cstdio>
#include <cstdlib>

#include "Database.hpp"


using namespace std;


int main(int argc, char **argv)
{
	printf("Database test\n");
	
	Database db("test.db");
	QueryResult *result;
	int rc;
	rc = db.query(&result, "CREATE TABLE test (id INT NOT NULL PRIMARY KEY, value INT NOT NULL);");
	db.freeQueryResult(&result);
	
	rc = db.query(&result,  "INSERT INTO test (id,value) VALUES (1,99999);"
				"INSERT INTO test (id,value) VALUES (2,12345);");
	db.freeQueryResult(&result);
	
	rc = db.query(&result, "SELECT * FROM test;");
	
	for (int i=0; i < result->numCols(); i++)
		printf("col %d=%s\n", i, result->getCol(i));

	for (int i=0; i < result->numRows(); i++)
		printf("row %d=%s\n", i, result->getRow(1, i));
	
	db.freeQueryResult(&result);
	
	return 0;
}
