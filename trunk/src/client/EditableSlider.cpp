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
 *     Michael Miller <michael.miller@holdingnuts.net>
 */
  
#include "EditableSlider.hpp"

#include <QSlider>
#include <QLineEdit>
#include <QIntValidator>
#include <QVBoxLayout>

EditableSlider::EditableSlider(QWidget *parent)
:	QWidget(parent)
{
	m_pEdit = new QLineEdit("0");
	m_pEdit->setAlignment(Qt::AlignRight);
	m_pEdit->setValidator(new QIntValidator(m_pEdit));
		
	m_pSlider = new QSlider(Qt::Horizontal);
	m_pSlider->setRange(0, 99);
	m_pSlider->setValue(0);
	
	connect(m_pSlider, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
	
	QVBoxLayout *layout = new QVBoxLayout;
		layout->setSizeConstraint(QLayout::SetMinimumSize); 
		layout->addWidget(m_pEdit);
		layout->addWidget(m_pSlider);
	setLayout(layout);
}

EditableSlider::~EditableSlider() { }

void EditableSlider::setMinimum(int value)
{
	QString str;

	m_pEdit->setText(str.setNum(value));
	m_pSlider->setMinimum(value);
}

void EditableSlider::setMaximum(int value)
{
	m_pSlider->setMaximum(value);
}

int EditableSlider::value() const
{
	return m_pEdit->text().toInt();
}

void EditableSlider::setValue(int value)
{
	QString str;

	m_pEdit->setText(str.setNum(value));
}
