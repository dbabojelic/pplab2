//
// Created by dario on 24.05.19..
//


#ifndef INC_2LAB_CONNECT4_STATE_H
#define INC_2LAB_CONNECT4_STATE_H

#include <vector>

/*
 * Information about connect4 game state.
 */
class Connect4_State {
public:

    static const int LO_POSITION = 0;
    static const int HI_POSITION = 6;
    static const int MAX_HEIGHT = 6;
    constexpr static int dy[] = {1, 1, 1, 0};
    constexpr static int dx[] = {1, 0, -1, -1};

    /*
     * Make empty state. No coins inside.
     */
    Connect4_State();
    /*
     * Make a state that is a copy of other.
     */
    //Connect4_State(const Connect4_State& other);

    /*
     * Make a state from serialization; arr should contain 7 numbers,
     * representing columns of state.
     */
    Connect4_State(int* arr);

    /*
     * Add coin in column with index position.
     * coin: true if coin is mine (computer), false if players
     * Returns: true if coin is added, false if not for whatever reason
     */
    bool addCoin(int position, bool coin);

    /*
     * Removes a coin from column with index position.
     */
    bool removeCoin(int position);


    /*
     * Checks if someone has 4 connected.
     * Returns:
     *      0 if there is no winner
     *      1 if user won
     *      2 if computer won
     */
    int checkForWinner();


    bool isPositionValid(int position);

    bool canAddInPosition(int position);

    /*
     * Prints state to stdout.
     */
    void printState();

    bool full();

    /*
     * Serialize state to array[7] so I can send it with MPI_Send
     */
    int* serialize();

private:
    bool isThereACoin(int position, int height);


    std::vector<std::vector<bool>> state;


};


#endif //INC_2LAB_CONNECT4_STATE_H
