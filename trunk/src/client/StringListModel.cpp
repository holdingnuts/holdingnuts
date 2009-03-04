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


#include "StringListModel.hpp"

StringListModel::StringListModel(QObject *parent)
:	QAbstractListModel(parent)
{ }

int StringListModel::rowCount(const QModelIndex& parent) const
{
	return stringList.count();
}

QVariant StringListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();
 
	if (index.row() >= stringList.size())
		return QVariant();
 
	if (role == Qt::DisplayRole)
		return stringList.at(index.row());
	else
		return QVariant();
}

QVariant StringListModel::headerData(
	int section,
	Qt::Orientation orientation,
	int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal)
		return QString("Column %1").arg(section);
	else
		return QString("Row %1").arg(section);
}

Qt::ItemFlags StringListModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	return QAbstractItemModel::flags(index) | Qt::ItemIsSelectable;
}

bool StringListModel::setData(
	const QModelIndex &index,
	const QVariant &value,
	int role)
{
	if (index.isValid() && role == Qt::EditRole)
	{
		stringList.replace(index.row(), value.toString());
		
		emit dataChanged(index, index);
		
		return true;
	}
	return false;
}

bool StringListModel::insertRows(int position, int rows, const QModelIndex &parent)
{
	beginInsertRows(QModelIndex(), position, position + rows);

	for (int i = position; i < (position + rows); ++i)
		stringList.insert(i, "");

	endInsertRows();
	
	return true;
}

bool StringListModel::appendRows(int rows, const QModelIndex &parent)
{
	return this->insertRows(this->rowCount(), rows, parent);
}

void StringListModel::add(const QVariant& value)
{
	if (this->appendRows(1))
		this->setData(createIndex(this->rowCount() - 1, 0), value);
}

void StringListModel::set(const QStringList& value)
{
	clear();

	// not the best solution, but a save way to add variables
	for (int i = 0; i < value.count(); ++i)
		add(value.at(i));
}

void StringListModel::clear()
{
	stringList.clear();

	reset();
}
