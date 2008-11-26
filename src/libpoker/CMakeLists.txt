include_directories (${POKER_SOURCE_DIR}/src ${POKER_SOURCE_DIR}/src/libpoker)

add_library(Poker
	GameDebug.cpp
	Card.cpp Deck.cpp HoleCards.cpp CommunityCards.cpp
	GameLogic.cpp GameController.cpp
	Player.cpp
)
