#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

// Общее количество итераций
#define NUM_ITERATIONS 100000000

int main() {
    long i;
    long count_in_circle = 0;
    
    double start_time, end_time;

    // Инициализация генератора случайных чисел (хотя в параллельной версии это сложнее,
    // для простого примера используем rand_r внутри потоков)
    
    printf("Запуск симуляции с %d итерациями...\n", NUM_ITERATIONS);
    printf("Используется потоков: %d\n", omp_get_max_threads());

    start_time = omp_get_wtime(); // Засекаем время OpenMP таймером

    // #pragma omp parallel: создает команду потоков
    // reduction(+:count_in_circle): каждый поток считает свою локальную переменную,
    // а в конце они все суммируются в общую переменную count_in_circle.
    #pragma omp parallel reduction(+:count_in_circle)
    {
        // У каждого потока должен быть свой seed для рандома, 
        // иначе они будут генерировать одинаковые числа
        unsigned int seed = (unsigned int)time(NULL) ^ omp_get_thread_num();

        // Директива for автоматически делит цикл между потоками
        #pragma omp for
        for (i = 0; i < NUM_ITERATIONS; i++) {
            // Генерируем случайные x и y от -1 до 1
            // rand_r - потокобезопасная версия rand()
            double x = (double)rand_r(&seed) / RAND_MAX * 2.0 - 1.0;
            double y = (double)rand_r(&seed) / RAND_MAX * 2.0 - 1.0;

            // Проверяем, попала ли точка в круг (x^2 + y^2 <= 1)
            if (x * x + y * y <= 1.0) {
                count_in_circle++;
            }
        }
    }

    end_time = omp_get_wtime();
    double pi_estimate = 4.0 * (double)count_in_circle / NUM_ITERATIONS;

    printf("\nРезультаты:\n");
    printf("Расчетное Pi: %.10f\n", pi_estimate);
    printf("Реальное Pi:  3.1415926535\n");
    printf("Время работы: %.4f сек\n", end_time - start_time);

    return 0;
}
