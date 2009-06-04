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
		const QModelIndex& index = QModelIndex());

	bool removeRows(
		int position,
		int rows,
		const QModelIndex& index = QModelIndex());

	//! \brief 
	//! \param value
	void updateValue(int cid, int column, const QVariant& value);

	void updatePlayerAdmin(int cid, bool value);
	void updatePlayerName(int cid, const QString& value);
	void updatePlayerLocation(int cid, const QString& value);

	QString getPlayerName(int cid) const;
	QString getLocation(int cid) const;

	QString name(int cid) const;
	QString location(int cid) const;

	bool containsId(int cid) const;

	unsigned nameColumn() const;

	void clear();

	void dump();

private:
	QStringList				strlstHeaderLabels;

	struct playerinfo
	{
		//! \brief Default c-tor
		playerinfo()
		:	cid(-1),
			name("???"),
			location("")
		{ }
		
		//! \brief ID; ColumnIndex 0
		int			cid;
		//! \brief Admin of Games; ColumnIndex 1 
		QList<int>	admin;
		//! \brief Playername; ColumnIndex 2
		QString		name;
		//! \brief Player Location; ColumnIndex 3
		QString		location;
	};

	typedef QList<playerinfo>	players_type;
	
	players_type				players;
};

#endif	/* _HOLDING_NUTS_GAME_LIST_TABLE_MODEL_H */
