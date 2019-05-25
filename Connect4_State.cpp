//
// Created by dario on 24.05.19..
//

#include "Connect4_State.h"
#include <iostream>
#include <cassert>

Connect4_State::Connect4_State() {
    for (int i = LO_POSITION; i <= HI_POSITION; i++)
        state.push_back(std::vector<bool>());
}

bool Connect4_State::addCoin(int position, bool coin) {
    if (!isPositionValid(position))
        return false;
    if (!canAddInPosition(position))
        return false;

    state[position].push_back(coin);
    return true;
}

bool Connect4_State::removeCoin(int position) {
    if (!isPositionValid(position))
        return false;
    if (state[position].empty())
        return false;
    state[position].pop_back();
    return true;
}

int Connect4_State::checkForWinner() {
    for (int j = 0; j <= HI_POSITION; j++) {
        for (int i = 0; i < state[j].size(); i++) {
            bool tmpCoin = state[j][i];

            for (int k = 0; k < 4; k++) {
                int x = i;
                int y = j;
                int l;
                for (l = 0; l < 3; l++) {
                    x += dx[k];
                    y += dy[k];
                    if (!isThereACoin(y, x))
                        break;
                    if (state[y][x] != tmpCoin)
                        break;
                }
                if (l == 3)
                    return tmpCoin ? 2 : 1;
            }

        }
    }

    return 0;
}

bool Connect4_State::isPositionValid(int position) {
    if (position >= LO_POSITION && position <= HI_POSITION)
        return true;
    return false;
}


bool Connect4_State::canAddInPosition(int position) {
    if (state[position].size() == MAX_HEIGHT)
        return false;
    return true;
}

bool Connect4_State::isThereACoin(int position, int height) {
    if (!isPositionValid(position))
        return false;
    if (height < 0 || height >= state[position].size())
        return false;
    return true;
}

void Connect4_State::printState() {
    for (int i = 0; i < 9; i++)
        std::cout << "-";
    std::cout << std::endl;
    for (int i = MAX_HEIGHT - 1; i >= 0; i--) {
        std::cout << "|";
        for (int j = LO_POSITION; j <= HI_POSITION; j++) {
            if (!isThereACoin(j, i)) std::cout << " ";
            else {
                std::cout << (state[j][i] ? "#": "U");
            }
        }

        std::cout << "|" << std::endl;
    }
    for (int i = 0; i < 9; i++)
        std::cout << "-";
    std::cout << std::endl;
    std::cout << " ";
    for (int i = 0; i < 7; i++)
        std::cout << i;
    std::cout << " " << std::endl;
}

bool Connect4_State::full() {
    for (int i = 0; i <= HI_POSITION; i++)
        if (state[i].size() < MAX_HEIGHT)
            return false;
    return true;
}

Connect4_State::Connect4_State(int *arr) : Connect4_State() {
    for (int i = 0; i < 7; i++) {
        int tmp = arr[i];
        while (tmp > 0) {
            int rem = tmp % 3;
            assert(rem);
            state[i].push_back(rem == 2);
            tmp /= 3;
        }
    }
}

int *Connect4_State::serialize() {
    int* ret = (int*)malloc(7 * sizeof(int));

    int i = 0;
    for (const auto& col: state) {
        int tmp = 0;
        int pot = 1;
        for (const auto& elem: col) {
            if (elem)
                tmp += pot * 2;
            else
                tmp += pot;
            pot *= 3;
        }
        ret[i] = tmp;
        i++;
    }
    return ret;

}
