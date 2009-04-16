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


#ifndef _HOLDING_NUTS_GAMELIST_SORT_FILTER_PROXY_MODEL_H
#define _HOLDING_NUTS_GAMELIST_SORT_FILTER_PROXY_MODEL_H

#include <QSortFilterProxyModel>
#include <QStringList>

class GameListSortFilterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	GameListSortFilterProxyModel(QObject *parent = 0);

	// name filter with setFilterRegExp(...); from baseclass

	void hideGameState(const QString& filter);
	void showGameState(const QString& filter);
	
	void showPrivateGames(bool value);

	//void setFilterMinimumPlayers(int n);
	//void setFilterMaximumPlayers(int n);

	//void setFilterMinimumRegisteredPlayers(int n);
	//void setFilterMaximumRegisteredPlayers(int n);

protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
	bool playersInRange(int i) const;

private:
	QStringList	filterGameState;
	
	bool		bShowPrivateGames;

	int			minPlayers;
	int			maxPlayers;
	
	int			minRegisteredPlayers;
	int			maxRegisteredPlayers;
};

#endif /* _HOLDING_NUTS_GAMELIST_SORT_FILTER_PROXY_MODEL_H */
