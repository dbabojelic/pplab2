#include <iostream>
#include <mpi.h>
#include "Connect4_State.h"
#include "Message.h"
#include <unordered_map>
#include <cassert>

#define PPRINTF(f, ...)                                               \
  printf("%*sPROC %d: " f, ID * 50, "", ID, ##__VA_ARGS__)

int DEPTH_SEARCH;
int ID;
int N;

struct Task {
    int id;
    int task_state[7];
};

void test_state() {
    Message msg;
    if (ID == 0) {
        Connect4_State state;
        state.printState();

        for (int j = 0; j < 7; j++) {
            for (int i = 0; i < 3; i++)
                state.addCoin(j, (i + j) % 2 == 0);
        }
        state.printState();

        std::cout << state.checkForWinner() << std::endl;

        state.addCoin(4, true);

        state.printState();

        std::cout << state.checkForWinner() << std::endl;

        state.addCoin(5, true);

        state.printState();

        std::cout << state.checkForWinner() << std::endl;

        std::cout << "POSLAO:" << std::endl;
        int *bla = state.serialize();
        for (int i = 0; i < 7; i++)
            std::cout << bla[i] << " ";
        std::cout << std::endl;

        msg.task(bla, 7 * sizeof(int))->send(1);

    }

    else {
        MPI_Status mpi_status;
        msg.receive(0, mpi_status);
        std::cout << "PRIMIO" << std::endl;
        int *rec = (int*)msg.data;

        for (int i = 0; i < 7; i++)
            std::cout << rec[i] << " ";
        std::cout << std::endl;

    }


}


float calculate_fitness(const std::vector<float>& children_fits, bool my_turn) {
    if ((int)children_fits.size() == 0) return 0.;

    bool exists1 = false;
    bool existsm1 = false;
    bool all1 = true;
    bool allm1 = true;
    float ret = 0.;

    for (const float& elem: children_fits) {
        ret += elem;
        if (elem == 1.) exists1 = true;
        if (elem == -1.) existsm1 = true;
        if (elem != 1.) all1 = false;
        if (elem != -1.) allm1 = false;
    }

    if (all1 || (my_turn && exists1)) return 1.;
    if (allm1 || (!my_turn && existsm1)) return -1.;
    return ret / (int)children_fits.size();
}

float dfs(Connect4_State& state, int d, bool my_turn) {
    int winner = state.checkForWinner();
    if (winner) {
        return (winner == 2) ? 1. : -1.;
    }

    if (!d)
        return 0.;

    std::vector<float> fits;
    for (int i = 0; i < 7; i++) {
        if (!state.addCoin(i, my_turn)) continue;
        fits.push_back(dfs(state, d - 1, !my_turn));
        state.removeCoin(i);
    }

    return calculate_fitness(fits, my_turn);
}

float solve_task(Task& task) {
    Connect4_State state(&task.task_state[0]);
    return dfs(state, DEPTH_SEARCH, true);
}


int calculate_move(const Connect4_State& _curr_state) {
    Connect4_State curr_state = _curr_state;

    //generate all states after two moves
    std::vector<Task> tasks;
    for (int first_move = 0; first_move < 7; first_move++) {
        if (!curr_state.addCoin(first_move, true)) continue;
        for (int second_move = 0; second_move < 7; second_move++) {
            if (!curr_state.addCoin(second_move, false)) continue;
            Task new_task;
            new_task.id = first_move * 10 + second_move;
            memcpy(new_task.task_state, curr_state.serialize(), 7 * sizeof(int));
            tasks.push_back(new_task);
            curr_state.removeCoin(second_move);
        }
        curr_state.removeCoin(first_move);
    }

    Message msg;
    MPI_Status stat;
    std::unordered_map<int, float> solutions; // task_id -> fitness
    std::unordered_map<int, int> who_solves_what; // process_id -> task_id

    msg.wake()->broadcast(0);
    int awake = N - 1;

    //special case if N == 1
    if (!awake) {
        for (auto& task: tasks)
            solutions[task.id] = solve_task(task);
    }

    int solved_cnt = 0;

    while (awake > 0) {
       msg.receive(MPI_ANY_SOURCE, stat);
       int who = stat.MPI_SOURCE;

       if (msg.type == READY) {
           if (solved_cnt == (int)tasks.size()) {
//               PPRINTF("Saljem procesu %d SLEEP\n", who);
               msg.sleep()->send(who);
               awake--;
               continue;
           }

           who_solves_what[who] = tasks[solved_cnt].id;
//           PPRINTF("Saljem procesu %d task %d\n", who, tasks[solved_cnt].id);
           msg.task(&tasks[solved_cnt], sizeof(Task))->send(who);
           solved_cnt++;
       }
       else if (msg.type == SOLUTION) {
           float sol;
           memcpy(&sol, msg.data, msg.data_size);
           solutions[who_solves_what[who]] = sol;
//           PPRINTF("Primio sam solution za task %d\n", who_solves_what[who]);
       }
       else {
           assert(0);
       }
    }

    float max_fit = -2;
    int ret = -1;

    for (int first_move = 0; first_move < 7; first_move++) {
       if (!curr_state.addCoin(first_move, true)) continue;
       float fit;
       int winner = curr_state.checkForWinner();

       if (winner) {
           fit = (winner == 2) ? 1 : -1;
       }
       else {
           std::vector<float> fits;
           for (int second_move = 0; second_move < 7; second_move++) {
               int task_id = first_move * 10 + second_move;
               if (solutions.find(task_id) == solutions.end()) continue;
               fits.push_back(solutions[task_id]);
           }
           fit = calculate_fitness(fits, false);
       }

       if (fit > max_fit) {
           max_fit = fit;
           ret = first_move;
       }


       curr_state.removeCoin(first_move);
    }

    return ret;


}

void game_engine() {
    Connect4_State currState;

    // time test
    bool turn = true;
//    bool turn = false;

    while (!currState.checkForWinner() && !currState.full()) {
        std::cout << "CURRENT STATE:" << std::endl;
        currState.printState();

        if (turn) {
            std::cout << "COMPUTER PLAYS" << std::endl << std::endl;
            int move = calculate_move(currState);
            // time test
            break;
            currState.addCoin(move, true);
        }

        else {
            std::cout << "YOU PLAY, enter position (0-6)" << std::endl;
            while (true) {
                std::cout << "> ";
                int pos;
                std::cin >> pos;
                if (currState.addCoin(pos, false))
                    break;
                else {
                    std::cout << "Invalid position! Try again." << std::endl;
                }
            }
        }

        turn = !turn;
    }

    std::cout << std::endl << std::endl;
    std::cout << "FINAL STATE:" << std::endl;
    currState.printState();

    int winner = currState.checkForWinner();
    if (!winner)
        std::cout << "GAME OVER, NO WINNER!";
    else if (winner == 1)
        std::cout << "CONGRATZ, YOU WON!";
    else
        std::cout << "YOU ARE A LOSER, COMPUTER SMASHED YOU!";
    std::cout << std::endl;

    // kill all workers
    Message msg;
    msg.exit()->broadcast(0);
}

void work() {

    Message msg;
    MPI_Status stat;

    while (true) {
        msg.broadcast(0);

        if (msg.type == EXIT)
            break;
        if (msg.type != WAKE)
            assert(0);

        while (true) {
            msg.ready()->send(0);
            msg.receive(0, stat);

            if (msg.type == SLEEP) {
//                PPRINTF("Mogu na spavanje!\n");
                break;
            }

            if (msg.type != TASK)
                assert(0);

            Task task;
            memcpy(&task, msg.data, msg.data_size);
//            PPRINTF("Dobio sam task %d\n", task.id);

            float solution = solve_task(task);

//            PPRINTF("Saljem rjesenje!\n");
            msg.solution(&solution, sizeof(float))->send(0);
        }


    }
}

int main(int argc, char** argv) {
    MPI_Init(NULL, NULL);
    if (argc < 2) {
        PPRINTF("Program expects one argument, integer representing DEPTH of solution search for every task. Depth for task generation is fixed at 2.");

        return 1;
    }

    DEPTH_SEARCH = atoi(argv[1]);

    MPI_Comm_rank(MPI_COMM_WORLD, &ID);
    MPI_Comm_size(MPI_COMM_WORLD, &N);

    if (ID == 0) {
        game_engine();
    }
    else {
        work();
    }

//    test_state();

    MPI_Finalize();
    return 0;
}