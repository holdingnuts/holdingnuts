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


#include "GameListTableModel.hpp"

#include <QDebug>


GameListTableModel::GameListTableModel(QObject *parent)
:	QAbstractTableModel(parent)
{
	strlstHeaderLabels << tr("Name") 
		<< tr("Gametype") 
		<< tr("Players") 
		<< tr("State");
}

int GameListTableModel::rowCount(const QModelIndex& parent) const
{
	return lstRows.count();
}

int GameListTableModel::columnCount(const QModelIndex& parent) const
{
	return strlstHeaderLabels.count();
}

QVariant GameListTableModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (index.row() > lstRows.size())
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();

	return lstRows.at(index.row()).at(index.column());
}

QVariant GameListTableModel::headerData(
	int section,
	Qt::Orientation orientation,
	int role) const
{
	if (role != Qt::DisplayRole)
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
	if (index.isValid() && role == Qt::EditRole)
	{
		lstRows[index.row()].replace(index.column(), value.toString());
		
		emit dataChanged(index, index);
		
		return true;
	}
	else
		qDebug() << "GameListItemModel::setData() invalided index= " << index;

	return false;
}

bool GameListTableModel::insertRows(int position, int rows, const QModelIndex& parent)
{
	beginInsertRows(QModelIndex(), position, position + rows);

	QStringList lstTemp;

	for (int j = 0; j < this->columnCount(); ++j)
		lstTemp.insert(j, "");

	for (int i = position; i < (position + rows); ++i)
		lstRows.insert(i, lstTemp);

	endInsertRows();
	
	return true;
}

bool GameListTableModel::appendRows(int rows, const QModelIndex& parent)
{
	return this->insertRows(this->rowCount(), rows, parent);
}

void GameListTableModel::updateRow(int gid, const QStringList& value)
{
	if (gid /*row*/ >= this->rowCount())
		appendRows(this->rowCount() - gid + 1);

	for (int j = 0; j < value.count(); ++j)
		this->setData(createIndex(gid, j), value.at(j));
}

void GameListTableModel::updateValue(int gid, int column, const QString& value)
{
	if (gid /*row*/ >= this->rowCount())
		appendRows(this->rowCount() - gid + 1);

	this->setData(createIndex(gid, column), value);
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

void GameListTableModel::clear()
{
	for (int i = 0; i < rowCount(); ++i)
		lstRows[i].clear();
		
	lstRows.clear();

	reset();
}

#ifdef DEBUG
void GameListTableModel::dump()
{
	for (int i = 0; i < rowCount(); ++i)
		qDebug() << "row(" << i << ") " << lstRows.at(i);
}
#endif



