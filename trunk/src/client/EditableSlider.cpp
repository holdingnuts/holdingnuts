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
:	QWidget(parent),
	m_nMin(0),
	m_nMax(0)
{
	m_pEdit = new QLineEdit("0");
	m_pEdit->setAlignment(Qt::AlignRight);
		
	m_pValidator = new QIntValidator(m_pEdit);
	
	m_pEdit->setValidator(m_pValidator);
			
	m_pSlider = new QSlider(Qt::Horizontal);
	m_pSlider->setTickInterval(10);
	m_pSlider->setSingleStep(1);
	m_pSlider->setRange(0, 100);
	m_pSlider->setValue(0);
	
	connect(m_pSlider, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
	
	QVBoxLayout *layout = new QVBoxLayout(parent);
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
	m_pSlider->setSliderPosition(0);
	m_pValidator->setBottom(value);
	
	m_nMin = value;
}

void EditableSlider::setMaximum(int value)
{
	m_nMax = value;
}

int EditableSlider::value() const
{
	int value = m_pEdit->text().toInt();
	
	if (value > m_nMax)
		value = m_nMax;
	
	return value;
}

void EditableSlider::setValue(int value)
{
	int amount = 0;
	
	if (!value)
		amount = 0;
	else if (value == 100)	// 100% == allin
		amount = m_nMax;
	else
	{
		if (value <= 40)
			amount = (int)((m_nMax * .25f) * value * .02f);
		else if (value > 40 && value <= 50)
			amount = (int)((m_nMax * .35f) * value * .02f);
		else if (value > 50 && value <= 60)
			amount = (int)((m_nMax * .45f) * value * .02f);
		else
			amount = (int)(m_nMax * value / 100);
		
	}

	amount += m_nMin;
	
	if (amount > m_nMax)
		amount = m_nMax;

	QString str;

	m_pEdit->setText(str.setNum(amount));
}
