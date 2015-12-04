#include <stdio.h>
#include <pthread.h>

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutex;

int odd = 1;

void *print_odd(void *arg) {
    int n = 1;

    while(1) {
        pthread_mutex_lock(&mutex);
        while (odd == 1) {
            printf("Odd: %d\n", n);
            n += 2;
            odd = 0;
            pthread_cond_wait(&cond, &mutex);
        }
        pthread_mutex_unlock(&mutex);
    }
}

void *print_even(void *arg) {
    int n = 2;

    while (1) {
        pthread_mutex_lock(&mutex);
        while (odd == 0) {
            printf("Even: %d\n", n);
            n += 2;
            odd = 1;
            pthread_cond_signal(&cond);
        }
        pthread_mutex_unlock(&mutex);
    }
}

int main(void) {
    pthread_t threads[2];

    pthread_mutex_init(&mutex, NULL);

    pthread_create(&threads[0], NULL, print_odd, NULL);
    pthread_create(&threads[1], NULL, print_even, NULL);
  
    pthread_join(threads[0], NULL); 
    pthread_join(threads[1], NULL); 
    
    return 0; 
}
