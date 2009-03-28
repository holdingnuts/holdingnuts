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


#ifndef _HOLDING_NUTS_EDITABLE_SLIDER_H
#define _HOLDING_NUTS_EDITABLE_SLIDER_H

#include <QWidget>
#include <QSize>

class QSlider;
class QLineEdit;
class QDoubleValidator;

class EditableSlider : public QWidget
{
	Q_OBJECT

public:
	EditableSlider(QWidget *parent = 0);

	virtual ~EditableSlider();
		
	void setMinimum(float value);
	void setMaximum(float value);

	float value() const;

	bool validValue() const;
	void setFocus();
	
protected:
	int valueToSliderPosition(float value) const;

signals:
	void dataChanged();

	void returnPressed();

private slots:
	void sliderValueChanged(int value);
	void sliderMoved(int value);
	void textChanged(const QString& text);
	void textEdited(const QString& text);
	
private:
	QSlider			*m_pSlider;
	QLineEdit		*m_pEdit;
	QDoubleValidator	*m_pValidator;
			
	float		m_nMin;
	float		m_nMax;
};

#endif /* _HOLDING_NUTS_EDITABLE_SLIDER_H */

