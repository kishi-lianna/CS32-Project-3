#include "Game.h"
#include "Side.h"
#include "Board.h"
#include "Player.h"
#include <iostream>
class Player;
using namespace std;

Game::Game(const Board& b, Player* south, Player* north)
// Construct a Game to be played with the indicated players on a copy of the board b. The
// player on the south side always moves first.
:m_board(b)
{
    m_south = south;
    m_north = north;
    m_turn = SOUTH;
}

void Game::display() const
// Display the game's board in a manner of your choosing, provided you show the names of the
// players and a reasonable representation of the state of the board.
{
    cout << "-----" << endl;
    if (m_turn == NORTH)
        cout << m_north->name() << "'s move:" << endl << endl;
    else
        cout << m_south->name() << "'s move:" << endl << endl;
    cout << '\t' << '\t' << '\t' << m_north->name() << endl;
    // display the north holes
    cout << '\t' << '\t';
    for (int i = 0; i < m_board.holes(); i++)
        cout << beans(NORTH, i + 1) << '\t';
    cout << endl;
    // display the pots
    cout << '\t' << beans(NORTH, 0);
    for (int i = 0; i < m_board.holes(); i++)
        cout << '\t';
    cout << '\t' << beans(SOUTH, 0) << endl;
    // display the south holes
    cout << '\t' << '\t';
    for (int i = 0; i < m_board.holes(); i++)
        cout << beans(SOUTH, i + 1) << '\t';
    cout << endl;
    cout << '\t' << '\t' << '\t' << m_south->name() << endl;
}

void Game::status(bool& over, bool& hasWinner, Side& winner) const
// If the game isn't over (i.e., more moves are possible), set over to false and do not change
// anything else. Otherwise, set over to true and hasWinner to true if the game has a winner,
// or false if it resulted in a tie. If hasWinner is set to false, leave winner unchanged;
// otherwise, set it to the winning side.
{
    // Game Over: all of the holes on one side of the board empty
    if (m_board.beansInPlay(NORTH) == 0 || m_board.beansInPlay(SOUTH) == 0)
    {
        over = true;
        // check for a winner (the player with the most beans in their pot)
        if ((m_board.beansInPlay(NORTH) + m_board.beans(NORTH, 0)) ==
            (m_board.beansInPlay(SOUTH) + m_board.beans(SOUTH, 0))) // tie
            hasWinner = false;
        else // there's a winner
        {
            hasWinner = true;
            if ((m_board.beansInPlay(NORTH) + m_board.beans(NORTH, 0)) >
                (m_board.beansInPlay(SOUTH) + m_board.beans(SOUTH, 0))) // north wins
                winner = NORTH;
            else // south wins
                winner = SOUTH;
        }
    }
    else // Game isn't over
        over = false;
}

bool Game::move()
// If the game is over, return false. Otherwise, make a complete move for the player whose
// turn it is (so that it becomes the other player's turn) and return true. "Complete" means
// that the player sows the seeds from a hole and takes any additional turns required or
// completes a capture. If the player gets an additional turn, you should display the board so
// someone looking at the screen can follow what's happening.
{
    bool over; bool hasWinner; Side winner;
    status(over, hasWinner, winner);
    if (over == true) // game is over
    {
        // game is over
        // sweep any beans in the holes to the pots
        cout << endl;
        for (int i = 0; i < m_board.holes(); i++)
        {
            m_board.moveToPot(NORTH, i + 1, NORTH);
            m_board.moveToPot(SOUTH, i + 1, SOUTH);
        }
        display(); cout << endl;
        return false;
    }
    int hole; Side endSide; int endHole;
    if (m_turn == NORTH) // north player's turn
        hole = m_north->chooseMove(m_board, NORTH);
    else // south player's turn
        hole = m_south->chooseMove(m_board, SOUTH);
    m_board.sow(m_turn, hole, endSide, endHole);
    // If it was placed in the player's pot, he must take another turn if the game isn't over
    status(over, hasWinner, winner);
    while (endSide == m_turn && endHole == 0)
    {
        status(over, hasWinner, winner);
        if (!over)
        {
            display();
            cout << endl;
            if (m_turn == NORTH) // north player's turn
                hole = m_north->chooseMove(m_board, NORTH);
            else // south player's turn
                hole = m_south->chooseMove(m_board, SOUTH);
            m_board.sow(m_turn, hole, endSide, endHole);
        }
        else
        {
            // game is over
            // sweep any beans in the holes to the pots
            cout << endl;
            for (int i = 0; i < m_board.holes(); i++)
            {
                m_board.moveToPot(NORTH, i + 1, NORTH);
                m_board.moveToPot(SOUTH, i + 1, SOUTH);
            }
            display(); cout << endl;
            return false;
        }
    }
    // If capture occurred
    // placed in one of the player's own holes that was empty just a moment before
    // and if the opponent's hole directly opposite from that hole is not empty
    if (endSide == m_turn && endHole > 0 && m_board.beans(endSide, endHole) == 1
        && m_board.beans(opponent(m_turn), endHole) > 0)
    {
        // that bean and all beans in the opponent's hole directly opposite from that hole are put into the players pot
        m_board.moveToPot(m_turn, endHole, m_turn);
        m_board.moveToPot(opponent(m_turn), endHole, m_turn);
        status(over, hasWinner, winner);
        if (over)
        {
            // game is over
            // sweep any beans in the holes to the pots
            cout << endl;
            for (int i = 0; i < m_board.holes(); i++)
            {
                m_board.moveToPot(NORTH, i + 1, NORTH);
                m_board.moveToPot(SOUTH, i + 1, SOUTH);
            }
            display(); cout << endl;
            return false;
        }
    }
    // Turn ends
    m_turn = opponent(m_turn);
    return true;
}

void Game::play()
// Play the game. Display the progress of the game in a manner of your choosing, provided that
// someone looking at the screen can follow what's happening. If neither player is
// interactive, then to keep the display from quickly scrolling through the whole game, it
// would be reasonable periodically to prompt the viewer to press ENTER to continue and not
// proceed until ENTER is pressed. (The ignore function for input streams is useful here.)
// Announce the winner at the end of the game. You can apportion to your liking the
// responsibility for displaying the board between this function and the move function. (Note:
// If when this function is called, South has no beans in play, so can't make the first move,
// sweep any beans on the North side into North's pot and act as if the game is thus over.)
{
    bool GameOver; bool hasWinner; Side winner;
    display(); cout << endl;
    status(GameOver, hasWinner, winner);
    if (GameOver)
    {
        // sweep beans
        cout << endl;
        for (int i = 0; i < m_board.holes(); i++)
        {
            m_board.moveToPot(NORTH, i + 1, NORTH);
            m_board.moveToPot(SOUTH, i + 1, SOUTH);
        }
        display(); cout << endl;
    }
    while (!GameOver) // while the game isn't over
    {
        if (!m_north->isInteractive() && !m_south->isInteractive()) // neither interactive
        {
            // prompt the viewer to press ENTER to continue and not proceed until ENTER is pressed
            cout << "Press enter to continue." << endl;
            cin.ignore(10000,'\n');
        }
        move(); // player makes move
        display();
        cout << endl;
        status(GameOver, hasWinner, winner); // check game status after move
    }
    // check for tie
    if (hasWinner == false)
    {
        cout << "-----" << endl;
        cout << "Tie." << endl;
    }
    else
    {
    // announce the winner
        cout << "-----" << endl;
        cout << "The winner is ";
        if (winner == SOUTH)
            cout << m_south->name();
        else
            cout << m_north->name();
        cout << endl;
    }
}

int Game::beans(Side s, int hole) const
// Return the number of beans in the indicated hole or pot of the game's board, or −1 if the
// hole number is invalid. This function exists so that we and you can more easily test your
// program.
{
    return m_board.beans(s, hole);
}
