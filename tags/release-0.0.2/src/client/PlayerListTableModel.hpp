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


#ifndef _HOLDING_NUTS_PLAYER_LIST_TABLE_MODEL_H
#define _HOLDING_NUTS_PLAYER_LIST_TABLE_MODEL_H

#include <QAbstractTableModel>
#include <QStringList>

//! \brief Playerlist Table Model
class PlayerListTableModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	PlayerListTableModel(QObject *parent = 0);

	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	int columnCount(const QModelIndex& parent = QModelIndex()) const;

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	QVariant headerData(
		int section,
		Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;	
	
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

	//! \brief 
	//! \param value
	void updateRow(int row, const QStringList& value);

	void updateValue(int row, int column, const QString& value);

	void updatePlayerName(int row, const QString& value);

	//! \brief clear's the list
	void clear();

	void dump();

private:
	QStringList				strlstHeaderLabels;
	
	QList<QStringList>		lstRows; // QStringList == row[n,0], row[n,1],... 
};

#endif	/* _HOLDING_NUTS_GAME_LIST_TABLE_MODEL_H */
