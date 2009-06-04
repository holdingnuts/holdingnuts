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


#include <iostream>

#include "TestCase.hpp"


using namespace std;


TestCase::TestCase() :
	m_name("Unnamed test"),
	m_count(0), m_fail(0)
{
	
}

bool TestCase::test(bool result, const string& desc)
{
	m_count++;
	
	if (!result)
		m_fail++;
	
	cerr << "TEST " << (result ? "OK  " : "FAIL") << " [" << m_name << "] " << " " << desc  << endl;
	
	return result;
}
