//
// Created by dario on 24.05.19..
//

#ifndef INC_2LAB_MESSAGE_H
#define INC_2LAB_MESSAGE_H

#include <mpi.h>
#include <cstring>

#define MAX_DATA_SIZE 200


enum MessageType{
    WAKE, READY, SLEEP, EXIT, TASK, SOLUTION
};

struct Message{
    MessageType type;
    char data[MAX_DATA_SIZE];
    int data_size;

    int send(int to){
        return MPI_Send(this, sizeof(Message), MPI_BYTE, to, 0, MPI_COMM_WORLD);
    }

    int receive(int from, MPI_Status &stat){
        return MPI_Recv(this, sizeof(Message), MPI_BYTE, from, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
    }

    int broadcast(int root){
        return MPI_Bcast(this, sizeof(Message), MPI_BYTE, root, MPI_COMM_WORLD);
    }

    Message *wake(){
        type = WAKE;
        memset(data, 0, sizeof(data));
        data_size = 0;
        return this;
    }

    Message *ready(){
        type = READY;
        memset(data, 0, sizeof(data));
        data_size = 0;
        return this;
    }

    Message *sleep(){
        type = SLEEP;
        memset(data, 0, sizeof(data));
        data_size = 0;
        return this;
    }

    Message *exit(){
        type = EXIT;
        memset(data, 0, sizeof(data));
        data_size = 0;
        return this;
    }

    Message *task(void *task_data, int size){
        type = TASK;
        memcpy(data, task_data, size);
        data_size = size;
        return this;
    }

    Message *solution(void *sol_data, int size){
        type = SOLUTION;
        memcpy(data, sol_data, size);
        data_size = size;
        return this;
    }

};
#endif //INC_2LAB_MESSAGE_H
