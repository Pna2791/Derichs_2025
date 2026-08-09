#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <Arduino.h>

class TaskQueue {
private:
    static const uint8_t MAX_SIZE = 20;

    uint8_t buffer[MAX_SIZE];
    uint8_t front;
    uint8_t rear;
    uint8_t count;

public:
    TaskQueue() {
        front = 0;
        rear = 0;
        count = 0;
    }

    bool enqueue(uint8_t value) {
        if (count >= MAX_SIZE)
            return false;

        buffer[rear] = value;
        rear = (rear + 1) % MAX_SIZE;
        count++;

        return true;
    }

    bool dequeue(uint8_t &value) {
        if (count == 0)
            return false;

        value = buffer[front];
        front = (front + 1) % MAX_SIZE;
        count--;

        return true;
    }

    bool peek(uint8_t &value) {
        if (count == 0)
            return false;

        value = buffer[front];
        return true;
    }

    bool isEmpty() {
        return count == 0;
    }

    bool isFull() {
        return count == MAX_SIZE;
    }

    uint8_t size() {
        return count;
    }

    void clear() {
        front = 0;
        rear = 0;
        count = 0;
    }
};

#endif