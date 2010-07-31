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


#include "PlayerListTableModel.hpp"

#include <QDebug>
#include <QIcon>

PlayerListTableModel::PlayerListTableModel(QObject *parent)
:	QAbstractTableModel(parent)
{
	strlstHeaderLabels << tr("ID") << tr("Admin") << tr("Name") << tr("Location"); 
}

int PlayerListTableModel::rowCount(const QModelIndex& parent) const
{
	if (parent.isValid())	// see qt-tip in doc
		return 0;

	return players.size();
}

int PlayerListTableModel::columnCount(const QModelIndex& parent) const
{
	return strlstHeaderLabels.count();
}

QVariant PlayerListTableModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (index.row() >= players.size() || index.row() < 0)
		return QVariant();

	if (role == Qt::DisplayRole)
	{
		playerinfo data = players.at(index.row());

		switch (index.column())
		{
			case 0:
				return data.cid;
			case 1:
				return QVariant(); // TODO: data.admin;
			case 2:
				return data.name;
			case 3:
				return data.location;
			default:
				qDebug() << "PlayerListTableModel::data() invalid column ("<<index.column()<<")";
		}
	}

// TODO
//	if (
//		role == Qt::DecorationRole && 
//		index.column() == 1 &&
//		players.at(index.row()).admin)
//			return QVariant(QIcon("gfx/foyer/admin.png"));

	return QVariant();
}

QVariant PlayerListTableModel::headerData(
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

bool PlayerListTableModel::setData(
	const QModelIndex& index,
	const QVariant& value,
	int role)
{
	if (index.isValid() && role == Qt::EditRole)
	{
		const int row = index.row();

		playerinfo data = players.value(row);

		switch (index.column())
		{
			case 0:
					data.cid = value.toInt();
				break;
// TODO				
//			case 1:
//					data.admin = value.toBool();
//				break;
			case 2:
					data.name = value.toString();
				break;
			case 3:
					data.location = value.toString();
				break;
			default:
			{
				qDebug() << "PlayerListTableModel::setData() invalid column ("<<index.column()<<")";
					return false;
			}
		}

		players.replace(row, data);

		emit(dataChanged(index, index));

		return true;
	}

	return false;
}

bool PlayerListTableModel::insertRows(int position, int rows, const QModelIndex& index)
{
	Q_UNUSED(index);
	
	beginInsertRows(QModelIndex(), position, position + rows - 1);

	for (int row = 0; row < rows; row++)
		players.insert(position, playerinfo());

	endInsertRows();
	
	return true;
}

bool PlayerListTableModel::removeRows(int position, int rows, const QModelIndex& index)
{
	Q_UNUSED(index);
	
	beginRemoveRows(QModelIndex(), position, position + rows - 1);

	for (int row = 0; row < rows; ++row)
         players.removeAt(position);

	endRemoveRows();

	return true;
}

void PlayerListTableModel::updateValue(int cid, int column, const QVariant& value)
{
	for (int i = 0; i < players.size(); ++i)
	{
		if (players.at(i).cid == cid)
		{
			this->setData(createIndex(i, column), value, Qt::EditRole);
				return;
		}
	}

	// cid not found --> add new entry
	this->insertRows(0, 1, QModelIndex());
	this->setData(createIndex(0, 0), cid, Qt::EditRole);
	this->setData(createIndex(0, column), value, Qt::EditRole);
}

void PlayerListTableModel::updatePlayerAdmin(int cid, bool value)
{
	updateValue(cid, 1, value);
}

void PlayerListTableModel::updatePlayerName(int cid, const QString& value)
{
	updateValue(cid, 2, value);
}

void PlayerListTableModel::updatePlayerLocation(int cid, const QString& value)
{
	updateValue(cid, 3, value);
}

QString PlayerListTableModel::getPlayerName(int cid) const
{
	players_type::const_iterator it;
	
	for (it = players.constBegin(); it != players.constEnd(); ++it)
		if (it->cid == cid)
			return it->name;

	return QString("??? (%1)").arg(cid);
}

QString PlayerListTableModel::getLocation(int cid) const
{
	players_type::const_iterator it;
	
	for (it = players.constBegin(); it != players.constEnd(); ++it)
		if (it->cid == cid)
			return it->location;

	return QString();
}

QString PlayerListTableModel::name(int cid) const
{
	players_type::const_iterator it;
	
	for (it = players.constBegin(); it != players.constEnd(); ++it)
		if (it->cid == cid)
			return it->name;

	return QString("??? (%1)").arg(cid);
}

QString PlayerListTableModel::location(int cid) const
{
	players_type::const_iterator it;
	
	for (it = players.constBegin(); it != players.constEnd(); ++it)
		if (it->cid == cid)
			return it->location;

	return QString();
}

bool PlayerListTableModel::containsId(int cid) const
{
	players_type::const_iterator it;
	
	for (it = players.constBegin(); it != players.constEnd(); ++it)
		if (it->cid == cid)
			return true;
			
	return false;			
}

unsigned PlayerListTableModel::nameColumn() const { return 2; }

void PlayerListTableModel::clear()
{
	if (rowCount() > 0)
		removeRows(0, rowCount());
}

void PlayerListTableModel::dump()
{
#ifdef DEBUG

	qDebug() << "-----------------";	

	for (int i = 0; i < rowCount(); ++i)
		qDebug() << "row(" << i << ") " << players.at(i).cid << players.at(i).name << players.at(i).location;

	qDebug() << "-----------------";	
		
#endif
}

