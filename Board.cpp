#include "Board.h"
#include "Side.h"

Board::Board(int nHoles, int nInitialBeansPerHole)
// Construct a Board with the indicated number of holes per side (not counting the pot) and
// initial number of beans per hole. If nHoles is not positive, act as if it were 1; if
// nInitialBeansPerHole is negative, act as if it were 0.
{
    if (nHoles < 1) // if nHoles is not pos
        nHoles = 1;
    if (nInitialBeansPerHole < 0) // if nInitialBeansPerHole is neg
        nInitialBeansPerHole = 0;
    m_holes = nHoles;
    // create a dummy Node for the linked list
    m_head = new Node;
    m_head->m_next = m_head;
    m_head->m_prev = m_head;
    // m_items = 0;
    // create NORTH pot
    insertNode(m_head, 0, NORTH, 0);
    // create NORTH holes
    for (int i = 0; i < nHoles; i++)
    {
        Node* previous = m_head->m_prev; // points to previous last item
        insertNode(previous, nInitialBeansPerHole, NORTH, i + 1);
    }
    // create SOUTH pot
    insertNode(m_head->m_prev, 0, SOUTH, 0);
    // create SOUTH holes
    for (int i = nHoles; i > 0; i--)
    {
        Node* previous = m_head->m_prev; // points to previous last item
        insertNode(previous, nInitialBeansPerHole, SOUTH, i);
    }
}

Board::Board(const Board &obj)
{
    m_items = obj.m_items;
    m_holes = obj.m_holes;
    if (obj.m_head == nullptr)
    {
        m_head = nullptr;
        return;
    }
    m_head = new Node; // dummy
    Node* temp = m_head;
    for (Node* p = obj.m_head->m_next; p != obj.m_head; p = p->m_next)
    {
        temp->m_next = new Node;
        temp->m_next->m_prev = temp;
        temp->m_next->m_beans = p->m_beans; // set value
        temp->m_next->m_side = p->m_side;
        temp->m_next->m_holeNum = p->m_holeNum;
        temp = temp->m_next; // point to newly created Node
    }
    temp->m_next = m_head; // make last item point to the head
    m_head->m_prev = temp;
}

int Board::holes() const
// Return the number of holes on a side (not counting the pot).
{
    return m_holes;
}

int Board::beans(Side s, int hole) const
// Return the number of beans in the indicated hole or pot, or −1 if the hole number is
// invalid.
{
    for (Node* p = m_head->m_next; p != m_head; p = p->m_next)
    {
        if (p->m_holeNum == hole && p->m_side == s) // found indicated hole/pot
            return p->m_beans;
    }
    return -1; // hole/pot number invalid
}

int Board::beansInPlay(Side s) const
// Return the total number of beans in all the holes on the indicated side, not counting the
// beans in the pot.
{
    int totalBeans = 0;
    for (Node* p = m_head->m_next; p != m_head; p = p->m_next)
    {
        if (p->m_side == s && p->m_holeNum != 0) // find correct side and exclude pots
            totalBeans += p->m_beans;   // count the beans in the holes
    }
    return totalBeans;
}

int Board::totalBeans() const
// Return the total number of beans in the game, including any in the pots.
{
    int totalBeans = 0;
    for (Node* p = m_head->m_next; p != m_head; p = p->m_next)
        totalBeans += p->m_beans;   // count the beans in the holes
    return totalBeans;
}

bool Board::sow(Side s, int hole, Side& endSide, int& endHole)
// If the hole indicated by (s,hole) is empty or invalid or a pot, this function returns false
// without changing anything. Otherwise, it will return true after sowing the beans: the beans
// are removed from hole (s,hole) and sown counterclockwise, including s's pot if encountered,
// but skipping s's opponent's pot. The parameters endSide and endHole are set to the side and
// hole where the last bean was placed. (This function does not make captures or multiple
// turns; different Kalah variants have different rules about these issues, so dealing with
// them should not be the responsibility of the Board class.)
{
    for (Node* p = m_head->m_next; p != m_head; p = p->m_next)
    {
        if (p->m_holeNum == hole && p->m_side == s && p->m_holeNum != 0 && p->m_beans > 0)
        {
            // sow the beans
            int beansToSow = p->m_beans;
            // beans are removed from the hole
            p->m_beans = 0;
            Node* k = p->m_prev;
            while(beansToSow > 0)
            {
                // if dummy node, don't do anything
                if (k == m_head)
                {
                    k = k->m_prev;
                    continue;
                }
                // if opponent's pot, don't do anything
                if (k->m_side != s && k->m_holeNum == 0)
                {
                    k = k->m_prev;
                    continue;
                }
                // else, place a bean in the hole
                k->m_beans++;
                beansToSow--;
                k = k->m_prev; // move on to the next hole
            }
            // finished sowing the beans
            endSide = k->m_next->m_side;
            endHole = k->m_next->m_holeNum;
            return true;
        }
    }
    // indicated hole is empty/invalid/pot
    return false;
}

bool Board::moveToPot(Side s, int hole, Side potOwner)
// If the indicated hole is invalid or a pot, return false without changing anything.
// Otherwise, move all the beans in hole (s,hole) into the pot belonging to potOwner and
// return true.
{
    for (Node* p = m_head->m_next; p != m_head; p = p->m_next)
    {
        if (p->m_holeNum == hole && p->m_side == s && p->m_holeNum != 0) // found indicated hole
        {
            // find the pot
            for (Node* k = m_head->m_next; k != m_head; k = k-> m_next)
            {
                if (k->m_holeNum == 0 && k->m_side == potOwner)
                {
                    k->m_beans += p->m_beans; // move all beans in hole to the pot
                    p->m_beans = 0;
                    return true;
                }
            }
        }
    }
    return false; // hole invalid or a pot
}

bool Board::setBeans(Side s, int hole, int beans)
// If the indicated hole is invalid or beans is negative, this function returns false without
// changing anything. Otherwise, it will return true after setting the number of beans in the
// indicated hole or pot to the value of the third parameter. (This may change what
// beansInPlay and totalBeans return if they are called later.) This function exists solely so
// that we and you can more easily test your program: None of your code that implements the
// member functions of any class is allowed to call this function directly or indirectly.
// (We'll show an example of its use below.)
{
    if (beans < 0) // beans is neg
        return false;
    for (Node* p = m_head->m_next; p != m_head; p = p->m_next)
    {
        if (p->m_holeNum == hole && p->m_side == s)
        {
            // set the number of beans to the 3rd parameter
            p->m_beans = beans;
            return true;
        }
    }
    // indicated hole is invalid
    return false;
}

//////////

void Board::insertNode(Node* p, const int& beans, const Side& side, const int& holeNum) // adds a new Node to the linked list
{
    Node* newNode = new Node; // create a new hole
    newNode->m_beans = beans; // assign beans
    newNode->m_side = side; // assign side
    newNode->m_holeNum = holeNum; // assign holeNum
    newNode->m_next = m_head;
    newNode->m_prev = p;
    newNode->m_prev->m_next = newNode;
    newNode->m_next->m_prev = newNode;
    m_items++;
}
