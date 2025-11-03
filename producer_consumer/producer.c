#include "buffer.h"
#include <time.h>

void sem_op(int semid, int semnum, int op) {
    struct sembuf sb = {semnum, op, 0};
    if (semop(semid, &sb, 1) == -1) {
        perror("semop");
        exit(1);
    }
}

int main() {
    int shmid = shmget(SHM_KEY, sizeof(SharedBuffer), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }
    SharedBuffer *buf = (SharedBuffer*)shmat(shmid, NULL, 0);
    if (buf == (void*)-1) {
        perror("shmat");
        exit(1);
    }

    int semid = semget(SEM_KEY, 3, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        exit(1);
    }

    // Khởi tạo buffer và semaphore nếu lần đầu
    if (buf->in == 0 && buf->out == 0 && buf->count == 0) {
        buf->in = buf->out = buf->count = 0;
        semctl(semid, SEM_MUTEX, SETVAL, 1);
        semctl(semid, SEM_EMPTY, SETVAL, BUFFER_SIZE);
        semctl(semid, SEM_FULL,  SETVAL, 0);
    }

    srand(time(NULL) ^ getpid());
    printf("Producer started. Press Ctrl+C to exit.\n");
    int value = 1;
    while (1) {
        int item = rand() % 1000;
        sem_op(semid, SEM_EMPTY, -1); // wait empty
        sem_op(semid, SEM_MUTEX, -1); // lock

        buf->buffer[buf->in] = item;
        buf->in = (buf->in + 1) % BUFFER_SIZE;
        buf->count++;
        printf("Produced: %d (count=%d)\n", item, buf->count);

        sem_op(semid, SEM_MUTEX, 1);  // unlock
        sem_op(semid, SEM_FULL, 1);  // signal full
        sleep(1);
    }

    shmdt(buf);
    return 0;
}
