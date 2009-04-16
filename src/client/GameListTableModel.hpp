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


#ifndef _HOLDING_NUTS_GAME_LIST_TABLE_MODEL_H
#define _HOLDING_NUTS_GAME_LIST_TABLE_MODEL_H

#include <QAbstractTableModel>
#include <QStringList>

//! \brief Gamelist Table Model
class GameListTableModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	GameListTableModel(QObject *parent = 0);

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
		
	bool removeRows(
		int position,
		int rows,
		const QModelIndex &index = QModelIndex());
		
	void updateValue(int gid, int column, const QVariant& value);

	void updateGameName(int gid, const QString& value);
	void updateGameType(int gid, const QString& value);
	void updatePlayers(int gid, const QString& value);
	void updateGameState(int gid, const QString& value);
	void updatePassword(int gid, bool value);

	//! \brief clear's the list
	void clear();

	//! \brief translation methods (gid<->row)
#if 1
	int findGidByRow(int row) const;
#endif
	int findRowByGid(int gid) const;
	
	void dump() const;

private:
	QStringList				strlstHeaderLabels;
	
	struct dataitem{
		int			gid;
		// row[n,0] == name
		// row[n,1] == type
		// row[n,2] == players
		// row[n,3] == state
		// row[n,4] == password
		QList<QVariant>		cols;
	};
	
	QList<dataitem>		datarows;
};

#endif	/* _HOLDING_NUTS_GAME_LIST_TABLE_MODEL_H */
