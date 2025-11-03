#include "buffer.h"

void sem_op(int semid, int semnum, int op) {
    struct sembuf sb = {semnum, op, 0};
    if (semop(semid, &sb, 1) == -1) {
        perror("semop");
        exit(1);
    }
}

int main() {
    int shmid = shmget(SHM_KEY, sizeof(SharedBuffer), 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }
    SharedBuffer *buf = (SharedBuffer*)shmat(shmid, NULL, 0);
    if (buf == (void*)-1) {
        perror("shmat");
        exit(1);
    }

    int semid = semget(SEM_KEY, 3, 0666);
    if (semid == -1) {
        perror("semget");
        exit(1);
    }

    printf("Consumer started. Press Ctrl+C to exit.\n");
    while (1) {
        sem_op(semid, SEM_FULL, -1);  // wait full
        sem_op(semid, SEM_MUTEX, -1); // lock

        int item = buf->buffer[buf->out];
        buf->out = (buf->out + 1) % BUFFER_SIZE;
        buf->count--;
        printf("Consumed: %d (count=%d)\n", item, buf->count);
        print_buffer(buf); // Hiển thị trạng thái buffer

        sem_op(semid, SEM_MUTEX, 1);  // unlock
        sem_op(semid, SEM_EMPTY, 1); // signal empty
        sleep(2);
    }

    shmdt(buf);
    return 0;
}
