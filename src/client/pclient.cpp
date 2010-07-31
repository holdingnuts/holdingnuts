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
 *     Dominik Geyer <dominik.geyer@holdingnuts.net>
 */


#include "Config.h"
#include "Logger.h"
#include "Debug.h"
#include "SysAccess.h"
#include "ConfigParser.hpp"

#include "Table.hpp"   // needed for reading snapshots // FIXME: should be all in protocol.h

#include "pclient.hpp"
#include "PlayerListTableModel.hpp"

#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <string>

#include <QUuid>
#include <QMessageBox>
#include <QDateTime>
#include <QPixmapCache>    // only needed for applying workaround

#ifndef NOAUDIO
# include "Audio.h"
#endif
#include "data.h"


ConfigParser config;


// server command PSERVER <version> <client-id> <time>
void PClient::serverCmdPserver(Tokenizer &t)
{
	srv.version = t.getNextInt();
	srv.cid = t.getNextInt();
	
	const unsigned int time_remote = t.getNextInt();
	srv.time_remote_delta = time_remote - QDateTime::currentDateTime().toTime_t();
	
	srv.introduced = true;
		
	wMain->addLog(tr("Server running version %1.%2.%3. Your client ID is %4.")
			  .arg(VERSION_GETMAJOR(srv.version))
			  .arg(VERSION_GETMINOR(srv.version))
			  .arg(VERSION_GETREVISION(srv.version))
			  .arg(srv.cid));
	
	
	// there is a newer version available
	if (srv.version > VERSION)
	{
		const QString sversion =
			tr("There is a newer version of HoldingNuts available (at least %1.%2.%3)")
				.arg(VERSION_GETMAJOR(srv.version))
				.arg(VERSION_GETMINOR(srv.version))
				.arg(VERSION_GETREVISION(srv.version));
		wMain->addServerMessage(sversion);
	}
	else if (srv.version < VERSION_COMPAT)
	{
		QMessageBox::critical(wMain, tr("Error"),
			tr("The version of the server isn't compatible anymore "
				"with this client version. Please either use "
				"an older client version or request the server "
				"admin to update the server version."));
		doClose();
		return;
	}
	
	
	// send user info
	char msg[1024];
	snprintf(msg, sizeof(msg), "INFO \"name:%s\" \"location:%s\"",
		  config.get("player_name").c_str(),
		  config.get("info_location").c_str());
		
	netSendMsg(msg);
	
	
	// request initial game-list and start update-timer
	requestGamelist();
	
	// request initial server stats
	requestServerStats();
}

// server command ERR [<code>] [<text>]
void PClient::serverCmdErr(Tokenizer &t)
{
	const int err_code = t.getNextInt();
	const std::string smsg = t.getTillEnd();
	
	log_msg("cmd", "error executing command (id=%d code=%d): %s",
		srv.last_msgid, err_code, smsg.c_str());
	
	wMain->addServerErrorMessage(err_code, QString::fromStdString(smsg));
	
	// check for version mismatch
	if (err_code == ErrWrongVersion)
		QMessageBox::critical(wMain, tr("Error"),
			tr("The version of this client isn't compatible anymore "
				"with the server. Please download a recent version."));
}

// server command MSG <from> <sender-name> <message>
void PClient::serverCmdMsg(Tokenizer &t)
{
	const std::string idfrom = t.getNext();

	Tokenizer ft(":");
	ft.parse(idfrom);
		
	int from = -1;
	int gid = -1;
	int tid = -1;
	int cid = -1;

	if (ft.count() == 2)
	{
		// message from table
		gid = ft.getNextInt();
		tid = ft.getNextInt();
	}
	else if (ft.count() == 3)
	{
		// client message from table
		gid = ft.getNextInt();
		tid = ft.getNextInt();
		cid = ft.getNextInt();
	}
	else
	{
		// message from foyer or client
		from = ft.getNextInt();
	}

	// playername
	const QString qsfrom(QString::fromStdString(t.getNext()));
	// chatmessage
	QString qchatmsg(QString::fromStdString(t.getTillEnd()));

	// replace all occurrences of "[cid]" in server-msg with player-name
	if (from == -1)
	{
		QRegExp rx("\\[(\\d+)\\]");
			
		while(rx.indexIn(qchatmsg) != -1)
		{
			const int mcid = rx.cap(1).toInt();
			const QString name = modelPlayerList->name(mcid);
			
			qchatmsg.replace(rx, name);
		}
	}

	// handle escape sequences
	qchatmsg.replace("\\\"", "\"");

	
	if (gid != -1 && tid != -1)
	{
		const tableinfo *tinfo = getTableInfo(gid, tid);
		
		// silently drop message if there is no table-info
		if (!tinfo)
			return;

		if (cid == -1) // message from server to table
			tinfo->window->addServerMessage(qchatmsg);
		else // message from user to table
		{
			if (config.getInt("chat_verbosity_table") & 0x4)
				tinfo->window->addChat(qsfrom, qchatmsg);
		}
	}
	else
	{
		if (from == -1) // message from server to foyer
			wMain->addServerMessage(qchatmsg);
		else // message from user to foyer
		{
			if (config.getInt("chat_verbosity_foyer") & 0x8)
				wMain->addChat(qsfrom, qchatmsg);
		}
	}
}

// table snapshot
void PClient::serverCmdSnapTable(Tokenizer &t, int gid, int tid, tableinfo* tinfo)
{
	// silently drop message if there is no table-info
	if (!tinfo)
		return;
	
	table_snapshot &table = tinfo->snap;
	HoleCards &holecards = tinfo->holecards;
	
	Tokenizer st(":");
	
	// state:betting_round
	std::string tmp = t.getNext();
	st.parse(tmp);
	
	table.state = st.getNextInt();
	table.betting_round = st.getNextInt();
	
	// dealer:sb:bb:current:lastbet
	tmp = t.getNext();
	st.parse(tmp);
	table.s_dealer = st.getNextInt();
	table.s_sb = st.getNextInt();
	table.s_bb = st.getNextInt();
	table.s_cur = st.getNextInt();
	table.s_lastbet = st.getNextInt();
	
	// community-cards
	{
		std::string board = t.getNext().substr(3);
		CommunityCards &cc = table.communitycards;
		
		Tokenizer ct(":");
		ct.parse(board);
		
		if (ct.count() == 0)
			cc.clear();
		
		if (ct.count() >= 3)
		{
			Card cf1(ct.getNext().c_str());
			Card cf2(ct.getNext().c_str());
			Card cf3(ct.getNext().c_str());
			
			cc.setFlop(cf1, cf2, cf3);
		}
		
		if (ct.count() >= 4)
		{
			Card ct1(ct.getNext().c_str());
			
			cc.setTurn(ct1);
		}
		
		if (ct.count() == 5)
		{
			Card cr1(ct.getNext().c_str());
			
			cc.setRiver(cr1);
		}
	}
	
	// table.seats
	table.my_seat = -1;
	table.nomoreaction = false;
	
	const unsigned int seat_max = 10;
	memset(table.seats, 0, seat_max*sizeof(seatinfo));
	
	tmp = t.getNext();
	do {
		//dbg_msg("seat", "%s", tmp.c_str());
		
		Tokenizer st(":");
		st.parse(tmp);
		
		unsigned int seat_no = Tokenizer::string2int(st.getNext().substr(1));
		
		seatinfo si;
		memset(&si, 0, sizeof(si));
		
		si.valid = true;
		si.client_id = st.getNextInt();
		
		if (si.client_id == srv.cid)
			table.my_seat = seat_no;
		
		int pstate = st.getNextInt();
		if (pstate & PlayerInRound)
			si.in_round = true;
		if (pstate & PlayerSitout)
			si.sitout = true;
		
		si.stake = st.getNextInt();
		si.bet = st.getNextInt();
		si.action = (Player::PlayerAction) st.getNextInt();
		
		std::string shole = st.getNext();
		if (shole.length() == 4)
		{
			Card h1(shole.substr(0, 2).c_str());
			Card h2(shole.substr(2, 2).c_str());
			si.holecards.setCards(h1, h2);
			
			// if there are hole-cards in the snapshot
			// then there's no further action possible
			table.nomoreaction = true;
		}
		else
			si.holecards.clear();
		
		if (seat_no < seat_max)
			table.seats[seat_no] = si;
		
		tmp = t.getNext();
	} while (tmp[0] == 's');
	
	
	table.pots.clear();
	
	// pots
	do {
		//dbg_msg("pot", "%s", tmp.c_str());
		Tokenizer pt(":");
		pt.parse(tmp);
		
		pt.getNext();   // pot-no; unused
		chips_type potsize = pt.getNextInt();
		table.pots.push_back(potsize);
		
		tmp = t.getNext();
	} while (tmp[0] == 'p');
	
	
	table.minimum_bet = Tokenizer::string2int(tmp);
	
	
	if (table.state == Table::NewRound)
		holecards.clear();
	
	if (tinfo->window)
		tinfo->window->updateView();
}

// snapshot with gamestate info
void PClient::serverCmdSnapGamestate(Tokenizer &t, int gid, int tid, tableinfo* tinfo)
{
	snap_gamestate_type type = (snap_gamestate_type) t.getNextInt();
	
	if (type == SnapGameStateStart)
	{
		if (config.getInt("chat_verbosity_foyer") & 0x4)
			wMain->addServerMessage(
				QString(tr("Game (%1) has been started.").arg(gid)));
		
		addTable(gid, tid);
	}
	else if (type == SnapGameStateEnd && config.getInt("chat_verbosity_foyer") & 0x4)
	{
		wMain->addServerMessage(
			QString(tr("Game (%1) has been ended.").arg(gid)));
	}
	else if (type == SnapGameStateNewHand)
	{
		const unsigned int hand_no = t.getNextInt();
		
		// add this table to known tables list
		if (!tinfo)
		{
			addTable(gid, tid);
			tinfo = getTableInfo(gid, tid);
		}
		
		tinfo->window->addServerMessage(
			QString(tr("A new hand (#%1) begins.").arg(hand_no)));
	}
	else if (type == SnapGameStateBlinds)
	{
		const chips_type blind_small = t.getNextInt();
		const chips_type blind_big = t.getNextInt();
		
		// silently drop message if there is no table-info
		if (!tinfo)
			return;
		
		tinfo->window->addServerMessage(
			QString(tr("Blinds are now at %1/%2.")
				.arg(blind_small)
				.arg(blind_big)));
	}
	else if (type == SnapGameStateBroke)
	{
		const int cid = t.getNextInt();
		const int position = t.getNextInt();
		const QString player_name = modelPlayerList->getPlayerName(cid);
		
		// silently drop message if there is no table-info
		if (!tinfo)
			return;
		
		tinfo->window->addServerMessage(
			tr("Player %1 broke.")
				.arg(player_name) +
			((position != -1) ? QString(" #%1").arg(position) : QString()));
	}
	// TODO: else if (type == "your_seat")
}

// cards snapshot
void PClient::serverCmdSnapCards(Tokenizer &t, int gid, int tid, tableinfo* tinfo)
{
	snap_cards_type type = (snap_cards_type) t.getNextInt();
	
	if (type == SnapCardsHole)
	{
		if (!tinfo)
			return;
		
		HoleCards &h = tinfo->holecards;
		std::string card1 = t.getNext();
		std::string card2 = t.getNext();
		Card ch1(card1.c_str());
		Card ch2(card2.c_str());
		
		h.setCards(ch1, ch2);
		
		if (tinfo->window)
		{
			if (config.getInt("chat_verbosity_table") & 0x2)
				tinfo->window->addServerMessage(
					QString(tr("Your hole cards: [%1 %2].")
						.arg(QString::fromStdString(card1))
						.arg(QString::fromStdString(card2))));
				
			tinfo->window->updateView();
			tinfo->window->playSound(SOUND_DEAL_1);
		}
	}
	else if (type == SnapCardsFlop || type == SnapCardsTurn || type == SnapCardsRiver)
	{
		if (!(config.getInt("chat_verbosity_table") & 0x2))
			return;

		// silently drop message if there is no table-info
		if (!tinfo || !tinfo->window)
			return;
		
		std::string card1, card2, card3;
		t >> card1;
		if (type == SnapCardsFlop)
			t >> card2 >> card3;
		
		QString smsg;
		switch ((int) type)
		{
			case SnapCardsFlop:
				smsg = QString(tr("The flop: [%1 %2 %3].")
					.arg(QString::fromStdString(card1))
					.arg(QString::fromStdString(card2))
					.arg(QString::fromStdString(card3)));
				break;
			case SnapCardsTurn:
				smsg = QString(tr("The turn: [%1].")
					.arg(QString::fromStdString(card1)));
				break;
			case SnapCardsRiver:
				smsg = QString(tr("The river: [%1].")
					.arg(QString::fromStdString(card1)));
				break;
		}
		
		tinfo->window->addServerMessage(smsg);
	}
}

// snapshot with player action info
void PClient::serverCmdSnapPlayerAction(Tokenizer &t, int gid, int tid, tableinfo* tinfo)
{
	// silently drop message if there is no table-info
	if (!tinfo || !tinfo->window)
		return;
	
	const snap_playeraction_type type = (snap_playeraction_type) t.getNextInt();
	const unsigned int cid = t.getNextInt();
	
	const QString player_name = modelPlayerList->name(cid);
	
	QString smsg;
	if (type == SnapPlayerActionFolded || type == SnapPlayerActionChecked)
	{
		const unsigned int auto_action = t.getNextInt();
		unsigned int sound = SOUND_FOLD_1;
		
		if (type == SnapPlayerActionFolded)
		{
			if (auto_action)
				smsg = QString(tr("%1 was folded.").arg(player_name));
			else if (type == SnapPlayerActionFolded)
				smsg = QString(tr("%1 folded.").arg(player_name));
			
			sound = SOUND_FOLD_1;
		}
		else if (type == SnapPlayerActionChecked)
		{
			if (auto_action)
				smsg = QString(tr("%1 was checked.").arg(player_name));
			else if (type == SnapPlayerActionChecked)
				smsg = QString(tr("%1 checked.").arg(player_name));
			
			sound = SOUND_CHECK_1;
		}
		
		if (tinfo->window)
			tinfo->window->playSound(sound);
	}
	else
	{
		const chips_type amount = t.getNextInt();
		unsigned int sound = SOUND_CHIP_2;
		
		if (type == SnapPlayerActionCalled)
		{
			smsg = QString(tr("%1 called %2.")
				.arg(player_name)
				.arg(amount));
			
			sound = SOUND_CHIP_1;
		}
		else if (type == SnapPlayerActionBet)
		{
			smsg = QString(tr("%1 bet to %2.")
				.arg(player_name)
				.arg(amount));
		}
		else if (type == SnapPlayerActionRaised)
		{
			smsg = QString(tr("%1 raised to %2.")
				.arg(player_name)
				.arg(amount));
		}
		else if (type == SnapPlayerActionAllin)
		{
			smsg = QString(tr("%1 is allin with %2.")
				.arg(player_name)
				.arg(amount));
		}
		
		if (tinfo->window)
			tinfo->window->playSound(sound);
	}
	
	if (config.getInt("chat_verbosity_table") & 0x1)
		tinfo->window->addServerMessage(smsg);
}

void PClient::serverCmdSnapPlayerShow(Tokenizer &t, int gid, int tid, tableinfo* tinfo)
{
	// silently drop message if there is no table-info
	if (!tinfo || !tinfo->window)
		return;
	
	unsigned int cid = t.getNextInt();
	
	const QString player_name = modelPlayerList->name(cid);
	
	std::vector<Card> allcards;
	
	std::string scard;
	while (t.getNext(scard))
	{
		Card c(scard.c_str());
		
		allcards.push_back(c);
	}
	
	
	HandStrength strength;
	GameLogic::getStrength(&allcards, &strength);
	const QString sstrength = WTable::buildHandStrengthString(&strength, 1);
	
	tinfo->window->addServerMessage(
		QString(tr("%1 shows %2.")
			.arg(player_name)
			.arg(sstrength)));
}

void PClient::serverCmdSnapFoyer(Tokenizer &t)
{
	const snap_foyer_type type = (snap_foyer_type) t.getNextInt();
	const int cid = t.getNextInt();
	const std::string cname = t.getNext();
	
	if (type == SnapFoyerJoin && config.getInt("chat_verbosity_foyer") & 0x2)
	{
		wMain->addServerMessage(tr("%2 (%1) joined foyer.")
			.arg(cid)
			.arg(QString::fromStdString(cname)));
	}
	else if (type == SnapFoyerLeave && config.getInt("chat_verbosity_foyer") & 0x2)
	{
		wMain->addServerMessage(tr("%2 (%1) left foyer.")
			.arg(cid)
			.arg(QString::fromStdString(cname)));
	}
}

// server cmd SNAP
void PClient::serverCmdSnap(Tokenizer &t)
{
	const std::string from = t.getNext();
	Tokenizer ft(":");
	ft.parse(from);
	const int gid = ft.getNextInt();
	const int tid = ft.getNextInt();
	tableinfo *tinfo = getTableInfo(gid, tid);
	
	snaptype snap = (snaptype)t.getNextInt();
	
	switch ((int)snap)
	{
	case SnapGameState:
		serverCmdSnapGamestate(t, gid, tid, tinfo);
		break;
	
	case SnapTable:
		serverCmdSnapTable(t, gid, tid, tinfo);
		break;
	
	case SnapCards:
		serverCmdSnapCards(t, gid, tid, tinfo);
		break;
	
	case SnapPlayerCurrent:
		// silently drop message if there is no table-info
		if (!tinfo || !tinfo->window)
			return;
		
#if 0
		tinfo->window->addServerMessage(
			QString(tr("%1, it's your turn!")
				.arg(modelPlayerList->name(srv.cid))));
#endif
		break;
	case SnapPlayerAction:
		serverCmdSnapPlayerAction(t, gid, tid, tinfo);
		break;
	case SnapWinAmount:
	case SnapWinPot:
	case SnapOddChips:
		{
			// silently drop message if there is no table-info
			if (!tinfo || !tinfo->window)
				return;
			
			const int cid = t.getNextInt();
			const unsigned int poti = t.getNextInt();
			const chips_type amount = t.getNextInt();
			
			QString smsg;
			if (snap == SnapWinPot)
				smsg = QString(tr("%1 receives pot #%2 with %3.")
					.arg(modelPlayerList->name(cid))
					.arg(poti+1)
					.arg(amount));
			else if (snap == SnapWinAmount)
				smsg = QString(tr("%1 wins %2.")
					.arg(modelPlayerList->name(cid))
					.arg(amount));
			else
				smsg = QString(tr("%1 receives %3 odd chips of split pot #%2.")
					.arg(modelPlayerList->name(cid))
					.arg(poti+1)
					.arg(amount));
			
			tinfo->window->addServerMessage(smsg);
		}
		break;
	case SnapPlayerShow:
		serverCmdSnapPlayerShow(t, gid, tid, tinfo);
		break;
	case SnapFoyer:
		serverCmdSnapFoyer(t);
		break;
	}
}

// server command PLAYERLIST <gid> <client-id> [...]
void PClient::serverCmdPlayerlist(Tokenizer &t)
{
	char msg[1024];
	
	const int gid = t.getNextInt();

	// find game
	games_type::iterator git = games.find(gid);
	
	Q_ASSERT_X(git != games.end(), Q_FUNC_INFO, "game not found");
	
	// clear current player list
	git->second.players.clear();
	
	// client-list
	std::string sreq_orig = t.getTillEnd();
	std::string sreq_clean;
	
	Tokenizer tcid(" ");
	tcid.parse(sreq_orig);
	
	std::string token;
	while (tcid.getNext(token))
	{
		unsigned int cid = Tokenizer::string2int(token);
		
		// append cid to player list
		git->second.players.push_back(cid);
		
		// skip this client for the request... if we already know him
//		if (getPlayerInfo(cid))
//				continue;
		
		sreq_clean += token + " ";
	}
	
	// only request client-info if there are unknown clients left
	if (sreq_clean.length())
	{
		snprintf(msg, sizeof(msg), "REQUEST clientinfo %s", sreq_clean.c_str());
		netSendMsg(msg);
	}
	
	wMain->playerListFilter()->filterListCid(
		QVector<int>::fromStdVector(git->second.players));
}

// server command CLIENTINFO <client-id> <type>:<value> [...]
void PClient::serverCmdClientinfo(Tokenizer &t)
{
	Q_ASSERT_X(modelPlayerList, Q_FUNC_INFO, "invalid modelPlayerList pointer");

	const int cid = t.getNextInt();
	
	std::string sinfo;
	while (t.getNext(sinfo))
	{
		Tokenizer it(":");
		it.parse(sinfo);
		
		std::string itype = it.getNext();
		std::string ivalue = it.getNext();
		
		if (itype == "name")
			modelPlayerList->updatePlayerName(cid, QString::fromStdString(ivalue));
		else if (itype == "location")
			modelPlayerList->updatePlayerLocation(cid, QString::fromStdString(ivalue));
	}
}

// server command GAMEINFO <gid> <type>:<value> [...]
void PClient::serverCmdGameinfo(Tokenizer &t)
{
	const int gid = t.getNextInt();
	
	games_type::iterator git = games.find(gid);
	
	if (git == games.end())
	{
		// if no game found add a new one to the list
		git = games.insert(
			git, 
			games_type::value_type(gid, games_type::mapped_type()));
	}
	
	gameinfo *gi = &(git->second);
	
	
	// unpack info
	const std::string sinfo = t.getNext();
	
	Tokenizer it(":");
	it.parse(sinfo);
	
	gi->type = (gametype) it.getNextInt();
	gi->mode = (gamemode) it.getNextInt();
	gi->state = (gamestate) it.getNextInt();
	
	unsigned int flags = it.getNextInt();
	gi->registered = flags & GameInfoRegistered;
	gi->subscribed = flags & GameInfoSubscribed;
	gi->password = flags & GameInfoPassword;
	gi->owner = flags & GameInfoOwner;
	
//	qDebug() << "[" << getMyCId() << "]" << gid << "owner" << gi->owner;
	
	gi->players_max = it.getNextInt();
	gi->players_count = it.getNextInt();
	gi->player_timeout = it.getNextInt();
	gi->initial_stakes = it.getNextInt();
	
	
	// unpack blinds-rule
	const std::string sblinds = t.getNext();
	it.parse(sblinds);
	
	gi->blinds_start = it.getNextInt();
	gi->blinds_factor = it.getNextInt() / 10.0;
	gi->blinds_time = it.getNextInt();
	
	
	// game name
	gi->name = QString::fromStdString(t.getNext());
	
	
	// notify WMain there's an updated gameinfo available
	Q_ASSERT_X(wMain, Q_FUNC_INFO, "invalid mainwindow pointer");

	wMain->notifyGameinfo(gid);
}

// server command GAMELIST <gid> [...]
void PClient::serverCmdGamelist(Tokenizer &t)
{
	// game-list
	std::string sreq;
	
	gamelist.clear();
	
	std::string sgid;
	while (t.getNext(sgid))
	{
		gamelist.push_back(Tokenizer::string2int(sgid));
		sreq += sgid + " ";
	}
	
	// get game info
	requestGameinfo(sreq.c_str());
	
	wMain->notifyGamelist();
}

void PClient::serverCmdServerinfo(Tokenizer &t)
{
	std::string spair;
	
	int clients_count = -1;
	int games_count = -1;
	
#if 1
	// legacy: older versions use incompatible syntax
	if (t.count() < 2)
		return;
#endif
	
	while (t.getNext(spair))
	{
		Tokenizer ti(":");
		ti.parse(spair);
		
		const unsigned int code = ti.getNextInt();
		const unsigned int value = ti.getNextInt();
		
		switch (code)
		{
		case StatsClientCount:
			clients_count = value;
			break;
		case StatsGamesCount:
			games_count = value;
			break;
		}
	}
	
	// update the server stats label if valid response
	if (clients_count != -1 && games_count != -1)
		wMain->updateServerStatsLabel(clients_count, games_count);
}

int PClient::serverExecute(const char *cmd)
{
	Tokenizer t(" ");
	t.parse(cmd);  // parse the command line
	
	if (!t.count())
		return 0;
	
#ifdef DEBUG
	if (config.getBool("dbg_srv_cmd"))
		dbg_msg("server_execute", "cmd= %s", cmd);
#endif
	
	// extract message-id if present
	const char firstchar = t[0][0];
	if (firstchar >= '0' && firstchar <= '9')
		srv.last_msgid = t.getNextInt();
	else
		srv.last_msgid = -1;
	
	
	// get command argument
	const std::string command = t.getNext();
	
	
	if (!srv.introduced)   // state: not introduced
	{
		if (command == "PSERVER")
			serverCmdPserver(t);
		else if (command == "ERR")
		{
			serverCmdErr(t);
			
			doClose();
		}
		else if (command == "OK")
		{
			// do nothing
		}
		else
		{
			// protocol error: maybe this isn't a pserver
			log_msg("introduce", "protocol error");
			wMain->addServerErrorMessage(
				ErrProtocol, tr("Protocol error. The remote host does not seem to be a HoldingNuts server."));
			
			// FIXME: don't do a regular/clean close (avoid the QUIT sequence)
			doClose();
		}
	}
	else if (command == "OK")
	{
		
	}
	else if (command == "ERR")
		serverCmdErr(t);
	else if (command == "MSG")
		serverCmdMsg(t);
	else if (command == "SNAP")
		serverCmdSnap(t);
	else if (command == "PLAYERLIST")
		serverCmdPlayerlist(t);
	else if (command == "CLIENTINFO")
		serverCmdClientinfo(t);
	else if (command == "GAMEINFO")
		serverCmdGameinfo(t);
	else if (command == "GAMELIST")
		serverCmdGamelist(t);
	else if (command == "SERVERINFO")
		serverCmdServerinfo(t);

	return 0;
}

// returns zero if no cmd was found or no bytes remaining after exec
int PClient::serverParsebuffer()
{
	//log_msg("clientsock", "(%d) parse (bufferlen=%d)", srv.sock, srv.buflen);
	
	int found_nl = -1;
	for (int i=0; i < srv.buflen; i++)
	{
		if (srv.msgbuf[i] == '\r')
			srv.msgbuf[i] = ' ';  // space won't hurt
		else if (srv.msgbuf[i] == '\n')
		{
			found_nl = i;
			break;
		}
	}
	
	int retval = 0;
	
	// is there a command in queue?
	if (found_nl != -1)
	{
		// extract command
		char cmd[sizeof(srv.msgbuf)];
		memcpy(cmd, srv.msgbuf, found_nl);
		cmd[found_nl] = '\0';
		
		//log_msg("clientsock", "(%d) command: '%s' (len=%d)", srv.sock, cmd, found_nl);
		if (serverExecute(cmd) != -1)
		{
			// move the rest to front
			memmove(srv.msgbuf, srv.msgbuf + found_nl + 1, srv.buflen - (found_nl + 1));
			srv.buflen -= found_nl + 1;
			//log_msg("clientsock", "(%d) new buffer after cmd (bufferlen=%d)", srv.sock, srv.buflen);
			
			retval = srv.buflen;
		}
		else
			retval = 0;
	}
	else
		retval = 0;
	
	return retval;
}

bool PClient::addTable(int gid, int tid)
{
	gameinfo *game = getGameInfo(gid);
	
	if (!game)
	{
		games[gid];  // FIXME: better way of adding item
		game = getGameInfo(gid);
	}
	
	tableinfo *table = getTableInfo(gid, tid);
	
	if (!table)
	{
		game->tables[tid];  // FIXME: better way of adding item
		table = getTableInfo(gid, tid);
		
		table->sitting = true;
		table->subscribed = true;
		table->window = new WTable(gid, tid);
		table->window->setWindowTitle(tr("HoldingNuts Table - [") + game->name + "]");
		
		// show table after some delay (give time to retrieve player-info)
		QTimer::singleShot(2000, table->window, SLOT(slotShow()));
	}
	
	
	// request the gameinfo
	requestGameinfo(gid);
	
	// request the player-list of the game
	requestPlayerlist(gid);
	
	return true;
}

bool PClient::doConnect(QString strServer, unsigned int port)
{
	log_msg("net", "Connecting to %s:%d...", strServer.toStdString().c_str(), port);
	wMain->addLog(tr("Connecting..."));
	
	tcpSocket->connectToHost(strServer, port);
	
	connecting = true;
	
	wMain->updateConnectionStatus();
	
	return true;
}

void PClient::doClose()
{
	if (!connecting)
		netSendMsg("QUIT");
	
	tcpSocket->abort();
	tcpSocket->close();
}

void PClient::doRegister(int gid, bool bRegister, bool subscription, const QString& password)
{
	char msg[1024];
	
	if (!connected)
		return;
	
	if (!subscription)
	{
		if (bRegister)
			snprintf(msg, sizeof(msg), "REGISTER %d %s", gid, password.toStdString().c_str());
		else
			snprintf(msg, sizeof(msg), "UNREGISTER %d", gid);
	}
	else
	{
		if (bRegister)
			snprintf(msg, sizeof(msg), "SUBSCRIBE %d %s", gid, password.toStdString().c_str());
		else
			snprintf(msg, sizeof(msg), "UNSUBSCRIBE %d", gid);
	}
	
	netSendMsg(msg);
}

void PClient::doStartGame(int gid)
{
	char msg[1024];
	
	if (!connected)
		return;
	
	snprintf(msg, sizeof(msg), "REQUEST start %d", gid);
		
	netSendMsg(msg);
}

bool PClient::doSetAction(int gid, Player::PlayerAction action, chips_type amount)
{
	if (!connected)
		return false;
	
	const char *saction = "";
	bool bAmount = false;
	
	switch ((int) action)
	{
	case Player::Fold:
		saction = "fold";
		break;
	case Player::Call:
		saction = "call";
		bAmount = true;
		break;
	case Player::Raise:
		saction = "raise";
		bAmount = true;
		break;
	case Player::Allin:
		saction = "allin";
		break;
	case Player::Show:
		saction = "show";
		break;
	case Player::Muck:
		saction = "muck";
		break;
	case Player::ResetAction:
		saction = "reset";
		break;
	case Player::Back:
		saction = "back";
		break;
	case Player::Sitout:
		saction = "sitout";
		break;
	}
	
	char msg[1024];
	if (bAmount)
		snprintf(msg, sizeof(msg), "ACTION %d %s %d",
			gid, saction, amount);
	else
		snprintf(msg, sizeof(msg), "ACTION %d %s",
			gid, saction);
	
	netSendMsg(msg);
	
	return true;
}

void PClient::chatAll(const QString& text)
{
	// foyer chat
	if (!connected)
		return;

	QString strMsg = "CHAT -1 " + text.simplified();

	netSendMsg(strMsg.toStdString().c_str());
}

void PClient::chat(const QString& text, int gid, int tid)
{
	if (!connected)
		return;

	QString strMsg = QString("CHAT %1:%2 %3")
		.arg(gid).arg(tid).arg(text.simplified());

	netSendMsg(strMsg.toStdString().c_str());
}

bool PClient::createGame(gamecreate *createinfo)
{
	char msg[1024];
	
	snprintf(msg, sizeof(msg), "CREATE players:%d stake:%d timeout:%d "
		"blinds_start:%d blinds_factor:%d blinds_time:%d password:%s "
		"\"name:%s\"",
		createinfo->max_players,
		createinfo->stake,
		createinfo->timeout,
		createinfo->blinds_start,
		int(createinfo->blinds_factor * 10 + .05),
		createinfo->blinds_time,
		createinfo->password.simplified().toStdString().c_str(),
		createinfo->name.simplified().toStdString().c_str());
	netSendMsg(msg);
	
	return true;
}

#if 0
const gamelist_type& PClient::getGameList()
{
	return gamelist;
}
#endif

gameinfo* PClient::getGameInfo(int gid)
{
	if (games.find(gid) != games.end())
		return &(games[gid]);
	else
		return 0;
}

tableinfo* PClient::getTableInfo(int gid, int tid)
{
	if (games.find(gid) != games.end() && games[gid].tables.find(tid) != games[gid].tables.end())
		return &(games[gid].tables[tid]);
	else
		return 0;
}

int PClient::getMyCId()
{
	return srv.cid;
}

#ifdef DEBUG
void PClient::slotDbgRegister()
{
	const int gid = config.getInt("dbg_register");
	doRegister(gid);
}
#endif

void PClient::sendDebugMsg(const QString& msg)
{
	netSendMsg(msg.toStdString().c_str());
}

QDateTime PClient::getServerTime()
{
	if (!connected)
		return QDateTime();
	
	QDateTime timeServer;
	timeServer.setTime_t(QDateTime::currentDateTime().toTime_t() + srv.time_remote_delta);
	return timeServer;
}

int PClient::netSendMsg(const char *msg)
{
	char buf[1024];
	const int len = snprintf(buf, sizeof(buf), "%s\n", msg);
	
#ifdef DEBUG
	if (config.getBool("dbg_srv_cmd"))
		dbg_msg("netSendMsg", "req= %s", msg);
#endif
	
	const int bytes = tcpSocket->write(buf, len);
	
	// FIXME: send remaining bytes if not all have been sent
	
	if (len != bytes)
		log_msg("connectsock", "warning: not all bytes written (%d != %d)", len, bytes);
	
	return bytes;
}

void PClient::netRead()
{
	char buf[1024];
	int bytes;
	
	do
	{
		// return early if there's nothing to read
		if ((bytes = tcpSocket->read(buf, sizeof(buf))) <= 0)
			return;
		
		//log_msg("connectsock", "(%d) DATA len=%d", sock, bytes);
		
		if (srv.buflen + bytes > (int)sizeof(srv.msgbuf))
		{
			log_msg("connectsock", "error: buffer size exceeded");
			srv.buflen = 0;
		}
		else
		{
			memcpy(srv.msgbuf + srv.buflen, buf, bytes);
			srv.buflen += bytes;
			
			// parse and execute all commands in queue
			while (serverParsebuffer());
		}
	} while (sizeof(buf) == bytes);
	
	return;
}

void PClient::netError(QAbstractSocket::SocketError socketError)
{
	log_msg("net", "Connection error: %s", tcpSocket->errorString().toStdString().c_str());
	wMain->addLog(tr("Connection error: %1.").arg(tcpSocket->errorString()));
	
	connected = false;
	connecting = false;
	
	wMain->updateConnectionStatus();
}

void PClient::netConnected()
{
	log_msg("net", "Connection established");
	wMain->addLog(tr("Connected."));
	
	memset(&srv, 0, sizeof(srv));
	
	connected = true;
	connecting = false;
	
	wMain->updateConnectionStatus();
	
	// send protocol introduction
	char msg[1024];
	snprintf(msg, sizeof(msg), "PCLIENT %d %s",
		VERSION,
		config.get("uuid").c_str());
	
	netSendMsg(msg);
}

void PClient::netDisconnected()
{
	log_msg("net", "Connection closed");
	wMain->addLog(tr("Connection closed."));
	
	connected = false;
	connecting = false;
	
	wMain->updateConnectionStatus();
	
	
	// reset all game related data and close table-windows
	for (games_type::iterator e = games.begin(); e != games.end(); e++)
	{
		tables_type &tables = e->second.tables;
		for (tables_type::iterator t = tables.begin(); t != tables.end(); t++)
		{
			const tableinfo *table = &(t->second);
			
			if (table->window)
			{
				table->window->close();
				delete table->window;
			}
		}
	}
	
	modelPlayerList->clear();
	games.clear();
	gamelist.clear();
}

void PClient::requestPlayerlist(int gid)
{
	char msg[256];
		
	// request the player-list of the game
	snprintf(msg, sizeof(msg), "REQUEST playerlist %d", gid);
	netSendMsg(msg);
}

void PClient::requestGamelist()
{
	// query gamelist
	netSendMsg("REQUEST gamelist");
}

void PClient::requestServerStats()
{
	// query server stats
	netSendMsg("REQUEST serverinfo");
}

bool PClient::isGameInList(int gid)
{
	for (gamelist_type::const_iterator e = gamelist.begin(); e != gamelist.end(); e++) 
		if (*e == gid) 
			return true;
	
	return false;
}

void PClient::requestGameinfo(const char *glist)
{
	char msg[1024];
	
	// get game infos
	snprintf(msg, sizeof(msg), "REQUEST gameinfo %s", glist);
	netSendMsg(msg);
}

void PClient::requestGameinfo(int gid)
{
	char msg[1024];
	
	// get game info
	snprintf(msg, sizeof(msg), "REQUEST gameinfo %d", gid);
	netSendMsg(msg);
}

bool PClient::isTableWindowRemaining()
{
	// FIXME: make use of QApplication::lastWindowClosed(), but Qt::WA_QuitOnClose flag needed for table-windows
	
	// check if there are open windows
	for (games_type::iterator e = games.begin(); e != games.end(); e++)
	{
		tables_type &tables = e->second.tables;
		for (tables_type::iterator t = tables.begin(); t != tables.end(); t++)
		{
			const tableinfo *table = &(t->second);
			
			if (table->window)
			{
				if (table->window->isVisible())
					return true;
			}
		}
	}
	
	return false;
}

PClient::PClient(int &argc, char **argv) : QApplication(argc, argv)
{
	connected = false;
	connecting = false;
	
	tcpSocket = new QTcpSocket(this);
	connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(netRead()));
	connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
		this, SLOT(netError(QAbstractSocket::SocketError)));
	connect(tcpSocket, SIGNAL(connected()), this, SLOT(netConnected()));
	connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(netDisconnected()));
	
	// app icon
	Q_INIT_RESOURCE(pclient);
	
	// app info for settings
	QCoreApplication::setOrganizationName(CONFIG_APPNAME);
	QCoreApplication::setOrganizationDomain("www.holdingnuts.net");
	QCoreApplication::setApplicationName(CONFIG_APPNAME);
	
	// model player
	modelPlayerList = new PlayerListTableModel;
	
	// use config-directory set on command-line
	if (argc >= 3 && (argv[1][0] == '-' && argv[1][1] == 'c'))
	{
		// we need an absolute path because we chdir into data-dir
		QString path(argv[2]);
		QDir dir(path);
		
		path = dir.absolutePath();
		
		sys_set_config_path(path.toStdString().c_str());
		log_msg("config", "Using manual config-directory '%s'", path.toStdString().c_str());
	}
	
	
	// create config-dir if it doesn't yet exist
	QDir config_dir;
	config_dir.mkpath(QString(sys_config_path()));
}

PClient::~PClient()
{
	Q_CLEANUP_RESOURCE(pclient);
}

int PClient::init()
{
	// workaround QTBUG-6840 and QTBUG-8606:
	// disable pixmap-cache for Qt 4.6.0 and 4.6.2
	if ((QT_VERSION == QT_VERSION_CHECK(4, 6, 0)) || (QT_VERSION == QT_VERSION_CHECK(4, 6, 2)))
	{
		log_msg("main", "Working around QTBUG-6840 and QTBUG-8606");
		QPixmapCache::setCacheLimit(0);
	}

	// change into data-dir
	const char *datadir = sys_data_path();
	if (datadir)
	{
		log_msg("main", "Using data-directory: %s", datadir);
		
		QString sdatadir(datadir);
		QDir::setCurrent(sdatadir);
	}
	else
	{
		log_msg("main", "Error: data-directory was not found");
		QMessageBox::critical(NULL, "Error", "The data directory was not found.");
		
		return 1;
	}
	
	
	// load locale
	QString locale;
	if (config.get("locale").length())
		locale = QString::fromStdString(config.get("locale"));
	else
	{
		locale = QLocale::system().name().left(2);
		log_msg("main", "Auto-detected locale: %s", locale.toStdString().c_str());
	}
	
	
	if (locale != "en")  // no locale
	{
		QTranslator *translator = new QTranslator();
		if (translator->load("i18n/hn_" + locale))
		{
			installTranslator(translator);
			log_msg("main", "Using locale: %s", locale.toStdString().c_str());
		}
		else
			log_msg("main", "Error: Cannot load locale: %s", locale.toStdString().c_str());
		
		// load qt localization; first try system version, then our own copy
		QTranslator *qt_translator = new QTranslator();
		if (qt_translator->load("qt_" + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath)) ||
			qt_translator->load("i18n/qt_" + locale))
		{
			installTranslator(qt_translator);
		}
	}
	
	
#ifndef NOAUDIO
	// load sounds
	struct sound {
		unsigned int id;
		const char *file;
	} sounds[] = {
		{ SOUND_TEST_1,		"audio/test.wav" },
		{ SOUND_DEAL_1,		"audio/deal.wav" },
		{ SOUND_CHIP_1,		"audio/chip1.wav" },
		{ SOUND_CHIP_2,		"audio/chip2.wav" },
		{ SOUND_CHECK_1,	"audio/check1.wav" },
		{ SOUND_FOLD_1,		"audio/fold1.wav" },
		{ SOUND_REMINDER_1,	"audio/reminder.wav" },
	};
	
	const unsigned int sounds_count = sizeof(sounds) / sizeof(sounds[0]);
	
	for (unsigned int i=0; i < sounds_count; i++)
	{
		dbg_msg("audio", "Loading sound '%s' (%d)", sounds[i].file, sounds[i].id);
		if (audio_load(sounds[i].id, sounds[i].file))
			log_msg("audio", "Failed loading sound (%d)", sounds[i].id);
	}
#endif /* NOAUDIO */
	
#ifdef DEBUG
	// set a random suffix, useful for debugging with multiple clients
	if (config.getBool("dbg_name"))
	{
		srand(time(NULL));

		char name[128];
		snprintf(name, sizeof(name), "%s_%d",
			config.get("player_name").c_str(),
			(int)(rand() % 100));
		config.set("player_name", name);
	}
#endif
	
	// main window
	wMain = new WMain();
	wMain->updateConnectionStatus();
	wMain->show();
	
	
	// automatically connect to default server
	if (config.getBool("auto_connect"))
	{
		doConnect(config.get("default_host").c_str(),
			config.getInt("default_port"));
	}
	
#if 1
	// temporary fix for localized chat
	if (config.get("encoding").length())
		QTextCodec::setCodecForCStrings(QTextCodec::codecForName(
			config.get("encoding").c_str()));
#endif

#ifdef DEBUG
	// automatically register to the game  (auto_connect must be set)
	if (config.getInt("dbg_register") != -1)
		QTimer::singleShot(1000, this, SLOT(slotDbgRegister()));
#endif
	
	return 0;
}

bool config_load()
{
	// include config defaults
	#include "client_variables.hpp"
	
	char cfgfile[1024];
	snprintf(cfgfile, sizeof(cfgfile), "%s/client.cfg", sys_config_path());
	
	if (config.load(cfgfile))
		log_msg("config", "Loaded configuration from %s", cfgfile);
	else
	{
		// override defaults
		
		// determine system username and use it as default playername
		const char *name = sys_username();
		if (name)
			config.set("player_name", std::string(name));
		
		// generate an UUID
		QString suuid = QUuid::createUuid().toString();
		suuid = suuid.mid(1, suuid.length() - 2);
		config.set("uuid", suuid.toStdString());
		
		if (config.save(cfgfile))
			log_msg("config", "Saved initial configuration to %s", cfgfile);
	}
	
	return true;
}

int main(int argc, char **argv)
{
	log_set(stdout, 0);
	
	log_msg("main", "HoldingNuts pclient (version %d.%d.%d; svn %s; Qt version %s)",
		VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION,
		VERSIONSTR_SVN,
		qVersion());
	
	
	// the app instance
	PClient app(argc, argv);
	
	
	// load config
	config_load();
	config.print();
	
	// start logging
	filetype *fplog = NULL;
	if (config.getBool("log"))
	{
		char logfile[1024];
		snprintf(logfile, sizeof(logfile), "%s/client.log", sys_config_path());
		fplog = file_open(logfile, config.getBool("log_append")
				? mode_append
				: mode_write);
		
		// log destination
		log_set(stdout, fplog);
		
		// log timestamp
		if (config.getBool("log_timestamp"))
			log_use_timestamp(1);
	}
	
#if defined(DEBUG) && defined(PLATFORM_WINDOWS)
	char dbgfile[1024];
	snprintf(dbgfile, sizeof(dbgfile), "%s/client.debug", sys_config_path());
	/*filetype *dbglog = */ file_reopen(dbgfile, mode_write, stderr);  // omit closing
#endif
	
	// initialize SDL for audio
#ifndef NOAUDIO
	audio_init();
#endif
	
	if (app.init())
		return 1;
	
	int retval = app.exec();
	
	
	// close log-file
	if (fplog)
		file_close(fplog);
	
#ifndef NOAUDIO
	// cleanup SDL
	audio_deinit();
#endif
	
	return retval;
}
