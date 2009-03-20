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
 *     Dominik Geyer <dominik.geyer@holdingnuts.net>
 */
  
#include "EditableSlider.hpp"

#include <QSlider>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QVBoxLayout>

EditableSlider::EditableSlider(QWidget *parent)
:	QWidget(parent),
	m_nMin(0.0f),
	m_nMax(0.0f)
{
	m_pEdit = new QLineEdit("0", this);
	m_pEdit->setAlignment(Qt::AlignRight);
//	m_pEdit->setMinimumHeight(2* m_pEdit->fontInfo().pixelSize());
	
	m_pValidator = new QDoubleValidator(m_pEdit);
	
	m_pEdit->setValidator(m_pValidator);
	
	connect(m_pEdit, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
	connect(m_pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(textEdited(const QString&)));
	connect(m_pEdit, SIGNAL(returnPressed()), this, SIGNAL(returnPressed()));
			
	m_pSlider = new QSlider(Qt::Horizontal, this);
	m_pSlider->setTickInterval(10);
	m_pSlider->setSingleStep(1);
	m_pSlider->setRange(0, 100);
	m_pSlider->setValue(0);
	
	connect(m_pSlider, SIGNAL(sliderMoved(int)), this, SLOT(sliderMoved(int)));
	
	QVBoxLayout *layout = new QVBoxLayout(this);
		// qwidget has already margins
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSizeConstraint(QLayout::SetMinimumSize); 
		layout->addWidget(m_pEdit);
		layout->addWidget(m_pSlider);
	setLayout(layout);
}

EditableSlider::~EditableSlider() { }

void EditableSlider::setMinimum(float value)
{
	m_nMin = value;
	
	m_pEdit->setText(QString::number(value));
	m_pEdit->selectAll();
	m_pEdit->setFocus();
	
	m_pSlider->setValue(0);
	m_pValidator->setBottom(value);
}

void EditableSlider::setMaximum(float value)
{
	m_nMax = value;
}

float EditableSlider::value() const
{
	float value = m_pEdit->text().toFloat();
	
	if (value > m_nMax)
		value = m_nMax;
	
	return value;
}

bool EditableSlider::validValue() const
{
	QString temp(m_pEdit->text());
	int pos = 0;
	
	const QValidator::State state = m_pValidator->validate(temp, pos);

	return (state == QValidator::Acceptable);
}

void EditableSlider::sliderMoved(int value)
{
	float amount = .0f;
	
	if (value == 100)	// 100% == allin
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
		
		amount += (int)m_nMin;
	}
	
	QString str;
	
	if (amount > m_nMax)
		str.setNum(m_nMax);
	else
		str.setNum(amount);

	m_pEdit->setText(str);
	m_pEdit->selectAll();
	m_pEdit->setFocus();
	
	emit dataChanged();
}

void EditableSlider::textChanged(const QString& text)
{
	emit dataChanged();
}

void EditableSlider::textEdited(const QString& text)
{
	QString temp(text);
	int pos = 0;
		
	const QValidator::State state = m_pValidator->validate(temp, pos);

	if (state == QValidator::Acceptable)
	{
		const float value = text.toFloat();
		m_pSlider->setSliderPosition((int)(100 * value / m_nMax));
	}
}
