/*
 * Copyright 2008, Dominik Geyer
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
 */


#ifndef _WTABLE_H
#define _WTABLE_H

#include <QApplication>
#include <QWidget>
#include <QFont>
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QButtonGroup>
#include <QSlider>

class WTable : public QWidget
{
Q_OBJECT

public:
	WTable(QWidget *parent = 0);

	
private slots:
	

private:
	
};


class WSeat : public QWidget
{
Q_OBJECT

public:
	WSeat(unsigned int id, QWidget *parent = 0);

	
private slots:
	

private:
	
};


#endif /* _WTABLE_H */
