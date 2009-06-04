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

/* Howto test:
     Config.h: #define SERVER_TESTING  (without this gamecontroller test won't work)
     CMake: -DENABLE_DEBUG and -DENABLE_TEST
*/

#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

#include "Config.h"
#include "Platform.h"
#include "Logger.h"
#include "Debug.h"
#include "Tokenizer.hpp"

#include "Card.hpp"
#include "Deck.hpp"
#include "HoleCards.hpp"
#include "CommunityCards.hpp"
#include "GameLogic.hpp"

#include "GameController.hpp"
#include "Protocol.h"

#include "TestCase.hpp"

#ifndef SERVER_TESTING
#warning GameController tests will not work without SERVER_TESTING defined
#endif


using namespace std;


static int message_filter = -1;  // only show messages to this cid
static int stop_ticks_hand = -1; // stop ticks after hand x
static bool stop_ticks = false;


#ifdef DEBUG

//! \brief Specialized testcase for GameController
class TestCaseGameController : public TestCase
{
public:
	TestCaseGameController();
	~TestCaseGameController();

protected:
	void tick(unsigned int ticks=30);
	
	void setPlayerStake(int cid, chips_type stake) { game->findPlayer(cid)->stake = stake; };
	chips_type getPlayerStake(int cid) const { return game->findPlayer(cid)->stake; };
	
	void setCards(const std::vector<Card> *cardsvec) { game->debug_cards.insert(game->debug_cards.end(), cardsvec->rbegin(), cardsvec->rend()); };
	
	void setDealerSeat(unsigned int seat) { game->tables.begin()->second->dealer = seat; };
	int getDealerSeat() const { return game->tables.begin()->second->dealer; };
	int getSbSeat() const { return game->tables.begin()->second->sb; };
	int getBbSeat() const { return game->tables.begin()->second->bb; };
	
	HoleCards getPlayerHoleCards(int cid) const { return game->findPlayer(cid)->holecards; };
	Table::State getTableState() const { return game->tables.begin()->second->state; };
	
	GameController *game;
};

TestCaseGameController::TestCaseGameController()
{
	setName("TestCaseGameController");
	
	game = new GameController();
}

TestCaseGameController::~TestCaseGameController()
{
	delete game;
}

void TestCaseGameController::tick(unsigned int ticks)
{
	for (unsigned int i=0; i < ticks; i++)
	{
		if (stop_ticks)
		{
			log_msg("STOP", "do not tick anymore");
			return;
		}
		
		game->tick();
	}
}


////////////////////////////////////////////////////////////////////////////////

//! \brief Specialized testcase for headsup scenario
class TestHeadsup : public TestCaseGameController
{
public:
	TestHeadsup(const std::string& name,
			chips_type stake1,
			chips_type stake2,
			chips_type blinds,
			bool dealer,
			bool win1)
	{
		setName(string("Headsup-") + name);
		
		m_stake1 = stake1;
		m_stake2 = stake2;
		m_blinds = blinds;
		m_dealer = dealer;
		m_win1 = win1;
	};
	
	bool run();

private:
	chips_type	m_stake1;
	chips_type	m_stake2;
	chips_type	m_blinds;
	bool		m_dealer;
	bool		m_win1;
};



bool TestHeadsup::run()
{
	struct player
	{
		int id;
		chips_type stake;
	} players[] = {
		{ 111,	m_stake1 },
		{ 222,	m_stake2 },
	};
	const unsigned int players_count = sizeof(players) / sizeof(players[0]);
	
	
	message_filter = players[0].id;	// filter out Player1 messages
	stop_ticks_hand = 2;	// stop game after 1st hand
	stop_ticks = false;
	
	// game options
	game->setBlindsStart(m_blinds);  // blinds at 400/800
	game->setPlayerMax(10/*players_count*/);
	
	// players
	for (unsigned int i=0; i < players_count; i++)
	{
		player *pl = &(players[i]);
		game->addPlayer(pl->id);
		setPlayerStake(pl->id, pl->stake);
	}
	
	
	// use own cards
	const char *cards_array[] = {
		"Kc", "3h",	// player 1
		"Ah", "5c",	// player 2
		"2d", "7c", "8s", "9d", "Th"  // community cards
	};
	const unsigned int cards_count = sizeof(cards_array) / sizeof(cards_array[0]);
	
	
	if (m_win1)
	{
		// switch cards so player1 wins
		cards_array[2] = "3d";
	}
	
	
	vector<Card> cards;
	for (unsigned int i=0; i < cards_count; i++)
		cards.push_back(Card(cards_array[i]));
	
	setCards(&cards);
	
	/////////////////////
	
	// start the game (if not already started by max-players)
	game->start();
	
	/////////////////////
	tick(1);  // FIXME: this may change
	
	
	int expected_dealer = 0;
	int expected_sb = 0;
	int expected_bb = 1;
	
	if (m_dealer) // switch dealer button (0=normal, 1=switched)
	{
		log_msg("info", "switch dealer button");
		setDealerSeat(1);	// test with switched dealer_button
		
		expected_dealer = 1;
		expected_sb = 1;
		expected_bb = 0;
	}
	

	tick(1);  // FIXME: this may change
	test(getTableState() == Table::Blinds, "state after 1 tick: before blinds");
	
	//test(getPlayerStake(players[0].id) == players[0].stake, "player1 stake");
	test(getDealerSeat() == expected_dealer, "expected dealer seat");  // assume table seats _aren't_ shuffled
	
	// headsup-rule test
	test(getSbSeat() == expected_sb, "expected sb seat");
	test(getBbSeat() == expected_bb, "expected bb seat");


	
	/////////////////////
	
	tick(1);  // FIXME: this may change
	test(getTableState() == Table::Betting, "state after 1 tick: before betting");
	
	HoleCards hole = getPlayerHoleCards(players[expected_dealer].id);
	vector<Card> tmp;
	hole.copyCards(&tmp);
	
	if (test(tmp.size(), "hole cards available"))
		test(tmp[0] == cards[0], "first hole card == first deck card");
	
	/////////////////////

	// let the whole action take place
	tick();
	
	
	
	// case 1 (should not be possible)
	//game->setPlayerAction(expected_xxx, Player::Fold, 0);
	
	// case 2 (should not be neccessary)
	//game->setPlayerAction(expected_xxx, Player::Call, 0);
	
	//tick();

	
	
	return true;
}

#endif /*DEBUG*/

////////////////////////////////////////////////////////////////////////////////

bool client_chat(int from, int to, const char *msg)
{
	if (message_filter != -1 && message_filter != to)
		return true;
	
	log_msg("msg", "%d: %s", to, msg);
	
	return true;
}

bool client_chat(int from_gid, int from_tid, int to, const char *msg)
{
	return client_chat(-1, to, msg);
}


bool client_snapshot(int from_gid, int from_tid, int to, int sid, const char *msg)
{
	if (message_filter != -1 && message_filter != to)
		return true;
	
	Tokenizer t;
	t.parse(msg);
	
	const char *ssnaptype = "Unknown snaptype";
	switch (sid)
	{
		case SnapGameState:	ssnaptype = "SnapGameState";	break;
		case SnapTable:		ssnaptype = "SnapTable";	break;
		case SnapCards:		ssnaptype = "SnapCards";	break;
		case SnapWinPot:	ssnaptype = "SnapWinPot";	break;
		case SnapOddChips:	ssnaptype = "SnapOddChips";	break;
		case SnapPlayerAction:	ssnaptype = "SnapPlayerAction";	break;
		case SnapPlayerShow:	ssnaptype = "SnapPlayerShow";	break;
		
	}
	
	
	if (sid == SnapWinPot)
	{
		int cid, pot, amount;
		t >> cid >> pot >> amount;
		log_msg(ssnaptype, "cid=%d pot=%d amount=%d",
			 cid, pot, amount);
	}
	else if (sid == SnapGameState)
	{
		const unsigned int type = t.getNextInt();
		if (type == SnapGameStateNewHand && t.getNextInt() == stop_ticks_hand)
		{
			// dirty... but works ;)
			stop_ticks = true;
			log_msg(ssnaptype, "stopping ticks at this hand");
		}
		else
			log_msg(ssnaptype, "%d: [%d] %s", to, sid, msg);
	}
	else
		log_msg(ssnaptype, "%d: [%d] %s", to, sid, msg);
	
	return true;
}


int main(void)
{
	log_msg("main", "GameController test");
	
#ifndef SERVER_TESTING
	log_msg("main", "These tests are unlikely to work without SERVER_TESTING defined.");
#endif	
	
	// init PRNG
	srand((unsigned) time(NULL));
	
#ifdef DEBUG
	// perform these test cases
	TestCase *tests[] = {
		//					    stake1/2  blinds dealer win1
		new TestHeadsup("sb allin (less SB)", 		20, 500, 80, false, false),
		new TestHeadsup("sb allin (complete SB)",	40, 500, 80, false, false),
		new TestHeadsup("sb (more than SB)",		60, 500, 80, false, false),  // action needed
		new TestHeadsup("bb allin (less BB), win1", 	20, 500, 80, true, true),
		new TestHeadsup("bb allin (less BB), win1", 	40, 500, 80, true, true),
		new TestHeadsup("bb allin (complete BB), win1",	80, 500, 80, true, true), // action needed
	};
	
	unsigned int test_count = sizeof(tests) / sizeof(tests[0]);
	
	for (unsigned int i=0; i < test_count; i++)
	{
		TestCase *tc = tests[i];
		
		cerr << endl << ">>> BEGIN test (#" << (i+1) << ") " << tc->name() << " >>>" << endl;
		
		const bool retval = tc->run();
		
		cerr << "<<< END test (#" << (i+1) << ") " << tc->name() << 
			": RESULT=" << (retval ? "ok" : "err") << " OK=" << tc->countSuccess() <<
			" FAIL=" << tc->countFailed() << " <<<" << endl << endl;
	}
	
	
	cerr << endl << "Test results:" << endl;
	int failed_tests = 0;
	
	for (unsigned int i=0; i < test_count; i++)
	{
		TestCase *tc = tests[i];
		
		cerr << "Test (#" << (i+1) << ")" <<
			" OK=" << tc->countSuccess() <<
			" FAIL=" << tc->countFailed() <<
			"  " << tc->name() << endl;
		
		if (tc->countFailed())
			failed_tests++;
		
		delete tc;
	}
	
	if (failed_tests)
		cerr << "Overall: " << failed_tests << " out of " << test_count << " tests FAILED." << endl;
	else
		cerr << "All tests succeeded." << endl;

#else /* DEBUG */
	log_msg("main", "DEBUG needs to be defined. Tests depend on it.");
#endif /* DEBUG */

	return 0;
}
