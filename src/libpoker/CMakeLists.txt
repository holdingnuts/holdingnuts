include_directories (
	${HOLDINGNUTS_SOURCE_DIR}/src
	${HOLDINGNUTS_SOURCE_DIR}/src/libpoker
)

add_library(Poker
	GameDebug.cpp
	Card.cpp Deck.cpp HoleCards.cpp CommunityCards.cpp
	GameLogic.cpp
	Player.cpp
)
