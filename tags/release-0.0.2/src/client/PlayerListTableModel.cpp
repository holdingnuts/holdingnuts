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


PlayerListTableModel::PlayerListTableModel(QObject *parent)
:	QAbstractTableModel(parent)
{
	strlstHeaderLabels << tr("Name"); 
}

int PlayerListTableModel::rowCount(const QModelIndex& parent) const
{
	if (parent.isValid())	// see qt-tip in doc
		return 0;

	return lstRows.count();
}

int PlayerListTableModel::columnCount(const QModelIndex& parent) const
{
	return strlstHeaderLabels.count();
}

QVariant PlayerListTableModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (index.row() > lstRows.size())
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();

	return lstRows.at(index.row()).at(index.column());
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
		lstRows[index.row()].replace(index.column(), value.toString());
		
		emit dataChanged(index, index);
		
		return true;
	}
	else
		qDebug() << "PlayerListTableModel::setData() invalided index= " << index;

	return false;
}

bool PlayerListTableModel::insertRows(int position, int rows, const QModelIndex& parent)
{
	beginInsertRows(parent, position, position + rows - 1);

	QStringList lstTemp;

	for (int j = 0; j < this->columnCount(); ++j)
		lstTemp.insert(j, "");

	for (int i = position; i < (position + rows); ++i)
		lstRows.insert(i, lstTemp);

	endInsertRows();
	
	return true;
}

bool PlayerListTableModel::appendRows(int rows, const QModelIndex& parent)
{
	return this->insertRows(this->rowCount(), rows, parent);
}

void PlayerListTableModel::updateRow(int row, const QStringList& value)
{
	if (row >= this->rowCount())
		appendRows(this->rowCount() - row + 1);

	for (int j = 0; j < value.count(); ++j)
		this->setData(createIndex(row, j), value.at(j));
}

void PlayerListTableModel::updateValue(int row, int column, const QString& value)
{
	if (row >= this->rowCount())
		appendRows(this->rowCount() - row + 1);

	this->setData(createIndex(row, column), value);
}

void PlayerListTableModel::updatePlayerName(int row, const QString& value)
{
	updateValue(row, 0, value);
}

void PlayerListTableModel::clear()
{
	if (!this->rowCount()) return;
	
	beginRemoveRows(QModelIndex(), 0, this->rowCount() - 1);
	
	//for (int i = 0; i < rowCount(); ++i)
	//	lstRows[i].clear();
		
	lstRows.clear();
	
	endRemoveRows();
	
	reset();
}

void PlayerListTableModel::dump()
{
#ifdef DEBUG
	for (int i = 0; i < rowCount(); ++i)
		qDebug() << "row(" << i << ") " << lstRows.at(i);
#endif
}




