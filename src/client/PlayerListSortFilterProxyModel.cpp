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


#include "PlayerListSortFilterProxyModel.hpp"

#include <QStringList>
#include <QDebug>

PlayerListSortFilterProxyModel::PlayerListSortFilterProxyModel(QObject *parent)
:	QSortFilterProxyModel(parent)
{ }

void PlayerListSortFilterProxyModel::filterListCid(const QVector<int>& lst)
{
	lstPlayersCid = lst;
	
	invalidateFilter();	
}

bool PlayerListSortFilterProxyModel::filterAcceptsRow(
	int sourceRow,
	const QModelIndex& sourceParent) const
{
	const QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent); // cid
	const QModelIndex index1 = sourceModel()->index(sourceRow, 1, sourceParent); // admin
	const QModelIndex index2 = sourceModel()->index(sourceRow, 2, sourceParent); // name
	const QModelIndex index3 = sourceModel()->index(sourceRow, 3, sourceParent); // location

	return (lstPlayersCid.contains(sourceModel()->data(index0).toInt()));
}
