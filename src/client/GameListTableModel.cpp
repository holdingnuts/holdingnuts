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


#include "GameListTableModel.hpp"

#include <QDebug>
#include <QIcon>

GameListTableModel::GameListTableModel(QObject *parent)
:	QAbstractTableModel(parent)
{
	strlstHeaderLabels << tr("Name") 
		<< tr("Gametype") 
		<< tr("Players") 
		<< tr("State")
		<< QString("Password protected");
}

int GameListTableModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);

	return datarows.count();
}

int GameListTableModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);

	return strlstHeaderLabels.count();
}

QVariant GameListTableModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (index.row() > datarows.size())
		return QVariant();

	if (index.column() > datarows.at(index.row()).cols.size())
	{
		qDebug() << "GameListTableModel::data() index.column("<<index.column()<<") greater than datarows.column("<<datarows.at(index.row()).cols.size()<<")";
			return QVariant();
	}

	if (role == Qt::DisplayRole)
		return datarows.at(index.row()).cols.at(index.column());

	if (
		role == Qt::DecorationRole && 
		index.column() == 0 &&
		datarows.at(index.row()).cols.at(4).toBool())
			return QVariant(QIcon("gfx/foyer/lock.png"));
	
	return QVariant();
}

QVariant GameListTableModel::headerData(
	int section,
	Qt::Orientation orientation,
	int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (section >= this->columnCount())
		return QVariant();

	if (orientation == Qt::Horizontal)
		return strlstHeaderLabels.at(section);

	return QVariant();
}

bool GameListTableModel::setData(
	const QModelIndex& index,
	const QVariant& value,
	int role)
{
	if (!index.isValid())
	{
		qDebug() << "GameListTableModel::setData() invalided index= " << index;
			return false;
	}

	if (index.row() > datarows.size())
	{
		qDebug() << "GameListTableModel::setData() index.row("<<index.row()<<") greater than datarows.size("<<datarows.size()<<")";
			return false;
	}


	if (index.column() > datarows.at(index.row()).cols.size())
	{
		qDebug() << "GameListTableModel::setData() index.column("<<index.column()<<") greater than datarows.column("<<datarows.at(index.row()).cols.size()<<")";
			return false;
	}

	if (role != Qt::EditRole)
		return false;

	datarows[index.row()].cols.replace(index.column(), value);
		
	emit dataChanged(index, index);
		
	return true;
}

bool GameListTableModel::insertRows(int position, int rows, const QModelIndex& index)
{
	Q_UNUSED(index);
	
	beginInsertRows(QModelIndex(), position, position + rows - 1);

	dataitem di;
	di.gid = -1;
	
	// name, gametype, players, state
	for (int j = 0; j < this->columnCount() - 1; ++j)
		di.cols.insert(j, QString());

	di.cols.insert(this->columnCount() + 0, false);	// password

	for (int i = position; i < (position + rows); ++i)
		datarows.insert(i, di);
	
	endInsertRows();
	
	return true;
}

bool GameListTableModel::removeRows(int position, int rows, const QModelIndex& index)
{
	Q_UNUSED(index);

	beginRemoveRows(QModelIndex(), position, position + rows - 1);

	for (int row = 0; row < rows; ++row)
		datarows.removeAt(position);

	endRemoveRows();

	return true;
}

void GameListTableModel::updateValue(int gid, int column, const QVariant& value)
{
	if (findRowByGid(gid) == -1)
	{
		insertRow(this->rowCount()); // 0

		datarows[this->rowCount() -1].gid = gid;
	}
	
	const int row = findRowByGid(gid);

	this->setData(createIndex(row, column, static_cast<quint32>(gid)), value);
	
#if 0
	// why doesn't this work?
	qDebug() << "id after set:" << this->index(row, column).internalId();
#endif
}

void GameListTableModel::updateGameName(int gid, const QString& value)
{
	updateValue(gid, 0, value);
}

void GameListTableModel::updateGameType(int gid, const QString& value)
{
	updateValue(gid, 1, value);
}

void GameListTableModel::updatePlayers(int gid, const QString& value)
{
	updateValue(gid, 2, value);
}

void GameListTableModel::updateGameState(int gid, const QString& value)
{
	updateValue(gid, 3, value);
}

void GameListTableModel::updatePassword(int gid, bool value)
{
	updateValue(gid, 4, value);
}

void GameListTableModel::clear()
{
	if (!this->rowCount()) return;
	
	beginRemoveRows(QModelIndex(), 0, this->rowCount() - 1);
	datarows.clear();
	endRemoveRows();
	
	reset();
}

#if 1
int GameListTableModel::findGidByRow(int row) const
{
	return datarows.at(row).gid;
}
#endif

int GameListTableModel::findRowByGid(int gid) const
{
	for (int i = 0; i < rowCount(); ++i)
		if (datarows.at(i).gid == gid)
			return i;

	return -1;
}

void GameListTableModel::dump() const
{
#ifdef DEBUG
	for (int i = 0; i < rowCount(); ++i)
		qDebug() << "row(" << i << "gid" << datarows.at(i).gid << ") " << datarows.at(i).cols;
#endif
}

