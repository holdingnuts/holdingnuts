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


#ifndef _HOLDING_NUTS_STRING_LIST_MODEL_H
#define _HOLDING_NUTS_STRING_LIST_MODEL_H

#include <QAbstractListModel>
#include <QStringList>

//! \brief simple, non-hierarchical, stringbased Modellist class
class StringListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	StringListModel(QObject *parent = 0);

	int rowCount(const QModelIndex& parent = QModelIndex()) const;

	QVariant data(const QModelIndex& index, int role) const;

	QVariant headerData(
		int section,
		Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;
		
	Qt::ItemFlags flags(const QModelIndex& index) const;
	
	bool setData(
		const QModelIndex& index,
		const QVariant& value,
		int role = Qt::EditRole);

	bool insertRows(
		int position,
		int rows,
		const QModelIndex &index = QModelIndex());
		
	bool appendRows(
		int rows,
		const QModelIndex &index = QModelIndex());

	//! \brief add's a new value to list
	//! \param value
	void add(const QVariant& value);
	
	//! \brief clear's the list
	void clear();

private:
	QStringList		stringList;
};

#endif	/* _HOLDING_NUTS_STRING_LIST_MODEL_H */
