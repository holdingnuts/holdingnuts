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


#include "GameListSortFilterProxyModel.hpp"

#include <QStringList>
#include <QDebug>

GameListSortFilterProxyModel::GameListSortFilterProxyModel(QObject *parent)
:	QSortFilterProxyModel(parent),
	bShowPrivateGames(false)
{ }

void GameListSortFilterProxyModel::hideGameState(const QString& filter)
{
	if (filterGameState.contains(filter))
		return;
		
	filterGameState.append(filter);
	
	invalidateFilter();
}

void GameListSortFilterProxyModel::showGameState(const QString& filter)
{
	int i = filterGameState.indexOf(filter);
	
	if (i == -1)
		return;
		
	filterGameState.removeAt(i);

	invalidateFilter();
}

void GameListSortFilterProxyModel::showPrivateGames(bool value)
{
	bShowPrivateGames = value;

	invalidateFilter();
}
/*
void GameListSortFilterProxyModel::setFilterMinimumPlayers(int n)
{
	minPlayers = n;
	invalidateFilter();
}

void GameListSortFilterProxyModel::setFilterMaximumPlayers(int n)
{
	maxPlayers = n;
	invalidateFilter();
}

void GameListSortFilterProxyModel::setFilterMinimumRegisteredPlayers(int n)
{
	minRegisteredPlayers = n;
	invalidateFilter();
}

void GameListSortFilterProxyModel::setFilterMaximumRegisteredPlayers(int n)
{
	maxRegisteredPlayers = n;
	invalidateFilter();
}
*/
bool GameListSortFilterProxyModel::filterAcceptsRow(
	int sourceRow,
	const QModelIndex& sourceParent) const
{
	const QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent); // name
	const QModelIndex index1 = sourceModel()->index(sourceRow, 1, sourceParent); // gametype + gamemode
	const QModelIndex index2 = sourceModel()->index(sourceRow, 2, sourceParent); // players (current / max)
	const QModelIndex index3 = sourceModel()->index(sourceRow, 3, sourceParent); // gamestate
	const QModelIndex index4 = sourceModel()->index(sourceRow, 4, sourceParent); // password

	return (sourceModel()->data(index0).toString().contains(filterRegExp()) &&
			!filterGameState.contains(sourceModel()->data(index3).toString()) &&
			!(sourceModel()->data(index4).toBool() && bShowPrivateGames));
}

bool GameListSortFilterProxyModel::playersInRange(int i) const
{
	return (i > minPlayers && i < maxPlayers);
}
