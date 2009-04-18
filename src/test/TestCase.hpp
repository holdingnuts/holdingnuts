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


#ifndef _TESTCASE_H
#define _TESTCASE_H

#include <string>


class TestCase
{
public:
	TestCase();
	virtual bool run() = 0;
	
	const std::string& name() const { return m_name; };
	unsigned int countResults() const { return m_count; };
	unsigned int countFailed() const { return m_fail; };
	unsigned int countSuccess() const { return countResults() - countFailed(); };

protected:
	bool test(bool result, const std::string& desc="");
	
	void setName(const std::string& name) { m_name = name; };
	
private:
	std::string m_name;
	unsigned int m_count;
	unsigned int m_fail;
};

#endif /* _TESTCASE_H */
