#include "../../include/work_que.h"

void queInit(pQue_t que, int capacity) {
    que->capacity = capacity;
    que->queSize = 0;
    que->queHead = que->queTail = NULL;
    pthread_mutex_init(&que->mutex, NULL);
}

int queInsert(pQue_t que, pNode_t pNew) {
    if (que->queSize == 0) {
        que->queHead = que->queTail = pNew;
        ++que->queSize;
        return 0;
    }
    if (que->capacity <= que->queSize) {
        return -1;
    }
    que->queTail->next = pNew;
    que->queTail = pNew;
    ++que->queSize;
    return 0;
}

int queGet(pQue_t que, pNode_t *pGet) {
    if (que->queSize == 0) {
        return -1;
    }
    *pGet = que->queHead;
    que->queHead = que->queHead->next;
    if (que->queHead == NULL) {
        que->queTail = NULL;
    }
    --que->queSize;
    return 0;
}