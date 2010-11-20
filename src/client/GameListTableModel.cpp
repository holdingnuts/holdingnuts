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
	strlstHeaderLabels << QString("ID")
		<< tr("Name") 
		<< tr("Gametype") 
		<< tr("Players") 
		<< tr("State")
		<< QString("Password protected");
}

int GameListTableModel::rowCount(const QModelIndex& parent) const
{
	if (parent.isValid())	// see qt-tip in doc
		return 0;

	return games.size();
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

	if (index.row() >= games.size() || index.row() < 0)
		return QVariant();

	if (role == Qt::DisplayRole)
	{
		gameinfo_type::const_reference data = games.at(index.row());

		switch (index.column())
		{
			case 0:
				return data.gid;
			case 1:
				return data.name;
			case 2:
				return data.type;
			case 3:
				return data.players;
			case 4:
				return data.state;
			case 5:
				return data.password;
			default:
				qDebug() << "GameListTableModel::data() invalid column ("<<index.column()<<")";
		}
	}

	if (
		role == Qt::DecorationRole && 
		index.column() == 1 &&
		games.at(index.row()).password)
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

	if (section >= this->columnCount() || section < 0)
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
		const int row = index.row();

		gameinfo_type::value_type data = games.value(row);

		switch (index.column())
		{
			case 0:
					data.gid = value.toInt();
				break;
			case 1:
					data.name = value.toString();
				break;
			case 2:
					data.type = value.toString();
				break;
			case 3:
					data.players = value.toString();
				break;
			case 4:
					data.state = value.toString();
				break;
			case 5:
					data.password = value.toBool();
				break;
			default:
			{
				qDebug() << "GameListTableModel::setData() invalid column ("<<index.column()<<")";
					return false;
			}
		}

		games.replace(row, data);

		emit(dataChanged(index, index));

		return true;
	}

	return false;
}

bool GameListTableModel::insertRows(int position, int rows, const QModelIndex& index)
{
	Q_UNUSED(index);
	
	beginInsertRows(QModelIndex(), position, position + rows - 1);

	for (int row = 0; row < rows; row++)
		games.insert(position, gameinfo_type::value_type());
	
	endInsertRows();
	
	return true;
}

bool GameListTableModel::removeRows(int position, int rows, const QModelIndex& index)
{
	Q_UNUSED(index);

	beginRemoveRows(QModelIndex(), position, position + rows - 1);

	for (int row = 0; row < rows; ++row)
		games.removeAt(position);

	endRemoveRows();

	return true;
}

void GameListTableModel::updateValue(int gid, int column, const QVariant& value)
{
	for (int i = 0; i < games.size(); ++i)
	{
		if (games.at(i).gid == gid)
		{
			this->setData(createIndex(i, column), value, Qt::EditRole);
				return;
		}
	}

	// gid not found --> add new entry
	this->insertRows(0, 1, QModelIndex());
	this->setData(createIndex(0, 0), gid, Qt::EditRole);
	this->setData(createIndex(0, column), value, Qt::EditRole);

	dump();
}

void GameListTableModel::updateGameName(int gid, const QString& value)
{
	updateValue(gid, 1, value);
}

void GameListTableModel::updateGameType(int gid, const QString& value)
{
	updateValue(gid, 2, value);
}

void GameListTableModel::updatePlayers(int gid, const QString& value)
{
	updateValue(gid, 3, value);
}

void GameListTableModel::updateGameState(int gid, const QString& value)
{
	updateValue(gid, 4, value);
}

void GameListTableModel::updatePassword(int gid, bool value)
{
	updateValue(gid, 5, value);
}

void GameListTableModel::clear()
{
	if (rowCount() > 0)
		removeRows(0, rowCount());
}

int GameListTableModel::findGidByRow(int row) const
{
	return games.at(row).gid;
}

void GameListTableModel::dump() const
{
#ifdef DEBUG

	qDebug() << "-----------------";	

	for (int i = 0; i < rowCount(); ++i)
		qDebug() << "row(" << i << ") " << games.at(i).gid << games.at(i).name;

	qDebug() << "-----------------";	

#endif
}

