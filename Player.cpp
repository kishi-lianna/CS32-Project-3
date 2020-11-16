#include <string>
#include "Player.h"
#include <iostream>

Player::Player(std::string name)
// Create a Player with the indicated name.
{
    m_name = name;
}

std::string Player::name() const
// Return the name of the player.
{
    return m_name;
}

bool Player::isInteractive() const
// Return false if the player is a computer player. Return true if the player is human. Most
// kinds of players will be computer players.
{
    return false; // computer player
}

Player::~Player()
// Since this class is designed as a base class, it should have a virtual destructor.
{
}

HumanPlayer::HumanPlayer(std::string name) : Player(name)
// Create a HumanPlayer with the indicated name.
{
}

int HumanPlayer::chooseMove(const Board& b, Side s) const
// Returns the move the user enters (keeps prompting the user to enter a move until valid if possible)
// If no move is possible, return −1.
{
    int holeNumber;
    // check if any moves are possible
    if (b.beansInPlay(s) < 1) // no beans in the holes on that side
        return -1;
    // prompt the person running the program for a move until they enter a valid hole number
    for (;;)
    {
        std::cout << "Select a hole: ";
        std::cin >> holeNumber;
        if (b.beans(s, holeNumber) > 0) // beans in the hole, valid move
            return holeNumber;
        std::cout << std::endl;
    }
    return -1;
}

bool HumanPlayer::isInteractive() const
// Returns true bc human player
{
    return true; // human player
}

BadPlayer::BadPlayer(std::string name) : Player(name)
// Create a HumanPlayer with the indicated name.
{
}

int BadPlayer::chooseMove(const Board& b, Side s) const
// Returns the move the player would choose (leftmost hole with beans)
// If no move is possible, return −1.
{
    // go through the holes on the player's side
    for (int i = 0; i < b.holes(); i++)
    {
        if (b.beans(s, i + 1) > 0) // first hole with beans
            return i + 1;
    }
    return -1; // no possible moves
}

SmartPlayer::SmartPlayer(std::string name) : Player(name)
// Create a HumanPlayer with the indicated name.
{
}

int SmartPlayer::chooseMove(const Board& b, Side s) const
// Every concrete class derived from this class must implement this function so that if the
// player were to be playing side s and had to make a move given board b, the function returns
// the move the player would choose. If no move is possible, return −1.
{
    if (b.beansInPlay(s) == 0) // no move is possible; game is finished
        return -1;
    int bestHole;
    int value = 10; // arbitrary starting value
    AlarmClock ac(4900);
    minimax(b, s, bestHole, value, ac);
    return bestHole;
}

void SmartPlayer::minimax(const Board& b, const Side& s, int& bestHole, int& value, AlarmClock& ac) const
// assists with chooseMove
{
    // high value: good for south player, low value: good for north player
    // if game over
    Side initial_player;
    if (value == 10) // very first call of the function
        initial_player = s;
    if (b.beansInPlay(s) == 0 || b.beansInPlay(opponent(s)) == 0)
    {
        // value = value of this position (i.e., +∞, -∞, or 0)
        if (b.beansInPlay(SOUTH) + b.beans(SOUTH, 0) > b.beansInPlay(NORTH) + b.beans(NORTH, 0)) // south player won
            value = 1000000;
        else if (b.beansInPlay(SOUTH) + b.beans(SOUTH, 0) < b.beansInPlay(NORTH) + b.beans(NORTH, 0)) // north player won
            value = -1000000;
        else // tie
            value = 0;
        return;
    }
    // if the criterion says we should not search below this node
    if (ac.timedOut())
    {
        // value = value of this position
        // see if it's impossible for one of the sides to win
            // if the difference between the pots is > remaining beans in all holes
        if (abs(b.beans(s, 0) - b.beans(opponent(s), 0)) > (b.beansInPlay(s) + b.beansInPlay(opponent(s))))
        {
            // if south player can't win
            if (b.beans(SOUTH, 0) < b.beans(NORTH, 0))
                value = -1000000;
            else // north player can't win
                value = 1000000;
        }
        // south player's pot beans - north player's pot beans
        else
        {
            value = (b.beans(SOUTH, 0) - b.beans(NORTH, 0));
        }
        return;
    }
    // for every hole h the player can choose
    for (int i = 0; i < b.holes(); i++)
    {
        if (b.beans(s, i + 1) > 0) // hole has a bean, can choose
        {
            Side endside; int endHole;
            // "make" the move h
            Board copy = b; // set this to the board with the initial position
            Board perform = b;
            perform.sow(s, i + 1, endside, endHole);
            // If it was placed in the player's pot, he must take another turn
            if (endside == s && endHole == 0)
            {
                int value1 = 10;
                minimax(perform, s, bestHole, value1, ac);
                perform.sow(s, bestHole, endside, endHole);
            }
            // If capture occurred
            // placed in one of the player's own holes that was empty just a moment before
            // and if the opponent's hole directly opposite from that hole is not empty
            if (endside == s && endHole > 0 && perform.beans(endside, endHole) == 1
                && perform.beans(opponent(s), endHole) > 0)
            {
                // that bean and all beans in the opponent's hole directly opposite from that hole are put into the players pot
                perform.moveToPot(s, endHole, s);
                perform.moveToPot(opponent(s), endHole, s);
            }
            int bestHole1; int value1;
            Side nextturn = opponent(s);
            minimax(perform, nextturn, bestHole1, value1, ac);
            perform = copy; // "unmake" the move
            // set value to the first pathway value
            if (value == 10)
                value = value1;
            if (initial_player == SOUTH) // want the highest value
            {
                if (value1 >= value)
                {
                    bestHole = i + 1;
                    value = value1;
                }
            }
            else // want the lowest value
            {
                if (value1 <= value)
                {
                    bestHole = i + 1;
                    value = value1;
                }
            }
        }
    }
    return;
} 


