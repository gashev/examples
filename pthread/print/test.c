#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define COUNT 10

pthread_mutex_t print_mutex;

void *print(void *arg) {
    int i;
    int value = (long) arg;
    pthread_mutex_lock(&print_mutex);
    for (i = 0; i < COUNT; i++) {
        printf("%2d", value);
        sleep(1);
    }
    
    printf("\n");
    pthread_mutex_unlock(&print_mutex);
    pthread_exit((void*) 0);
}

int main(void) {
    long i;
    pthread_t print_thread[COUNT];
    void *status;

    pthread_mutex_init(&print_mutex, NULL);

    for (i = 0; i < COUNT; i++) {
        pthread_create(&print_thread[i], NULL, print, (void *)i);
    }

    for (i = 0; i < COUNT; i++) {
        pthread_join(print_thread[i], &status);
    }

    pthread_mutex_destroy(&print_mutex);

    return 0;
}
