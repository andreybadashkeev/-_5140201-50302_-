#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// Общее количество итераций
#define TOTAL_ITERATIONS 100000000

// Количество потоков (можно менять или считывать из аргументов)
#define NUM_THREADS 1

// Структура для передачи данных в поток
typedef struct {
    int thread_id;
    long iterations_per_thread;
    long count_in_circle; // Сюда поток запишет свой результат
} ThreadData;

// Функция, которую будет выполнять каждый поток
void* worker_function(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    long i;
    long local_count = 0;
    
    // Уникальный seed для каждого потока (очень важно для рандома!)
    // Используем time(NULL) + thread_id, чтобы seed отличался
    unsigned int seed = (unsigned int)time(NULL) ^ (data->thread_id << 16);

    for (i = 0; i < data->iterations_per_thread; i++) {
        // rand_r - потокобезопасная генерация
        double x = (double)rand_r(&seed) / RAND_MAX * 2.0 - 1.0;
        double y = (double)rand_r(&seed) / RAND_MAX * 2.0 - 1.0;

        if (x * x + y * y <= 1.0) {
            local_count++;
        }
    }

    // Записываем результат обратно в структуру
    data->count_in_circle = local_count;
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    long total_in_circle = 0;
    int i;
    
    // Засекаем время (используем clock_gettime для точности в Linux)
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    printf("Запуск Pthreads версии с %d потоками...\n", NUM_THREADS);

    // 1. Создание потоков
    long iterations_per_thread = TOTAL_ITERATIONS / NUM_THREADS;
    
    for (i = 0; i < NUM_THREADS; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].iterations_per_thread = iterations_per_thread;
        
        // Корректировка для последнего потока (если не делится нацело)
        if (i == NUM_THREADS - 1) {
            thread_data[i].iterations_per_thread += TOTAL_ITERATIONS % NUM_THREADS;
        }

        int rc = pthread_create(&threads[i], NULL, worker_function, (void*)&thread_data[i]);
        if (rc) {
            printf("Ошибка: невозможно создать поток, %d\n", rc);
            exit(-1);
        }
    }

    // 2. Ожидание завершения (Join) и сбор результатов
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        // Суммируем частичные результаты
        total_in_circle += thread_data[i].count_in_circle;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;

    double pi_estimate = 4.0 * (double)total_in_circle / TOTAL_ITERATIONS;

    printf("\nРезультаты:\n");
    printf("Расчетное Pi: %.10f\n", pi_estimate);
    printf("Время работы: %.4f сек\n", time_taken);

    return 0;
}
