#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h> // Главная библиотека MPI

// Общее количество итераций
#define TOTAL_ITERATIONS 100000000

int main(int argc, char** argv) {
    int rank, size;
    long i;
    long local_count = 0;
    long global_count = 0;
    
    // 1. Инициализация MPI
    MPI_Init(&argc, &argv);
    // Узнаем свой номер (rank) и общее число процессов (size)
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Засекаем время (только на главном процессе для красоты)
    double start_time = MPI_Wtime();

    // 2. Делим работу
    // Каждый процесс делает равную долю
    long iterations_per_proc = TOTAL_ITERATIONS / size;

    // Уникальный seed для каждого процесса!
    unsigned int seed = (unsigned int)time(NULL) ^ (rank << 16);

    // 3. Вычисления (каждый в своей песочнице)
    for (i = 0; i < iterations_per_proc; i++) {
        double x = (double)rand_r(&seed) / RAND_MAX * 2.0 - 1.0;
        double y = (double)rand_r(&seed) / RAND_MAX * 2.0 - 1.0;

        if (x * x + y * y <= 1.0) {
            local_count++;
        }
    }

    // 4. Сбор результатов (Магия MPI)
    // MPI_Reduce берет local_count у ВСЕХ процессов,
    // применяет операцию MPI_SUM (сложение)
    // и кладет результат в global_count ТОЛЬКО на процессе с root=0
    MPI_Reduce(&local_count, &global_count, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    // 5. Вывод результатов (только Master)
    double end_time = MPI_Wtime();
    
    if (rank == 0) {
        // Поскольку мы могли потерять остаток от деления, пересчитаем реальное число итераций
        long real_total_iterations = iterations_per_proc * size;
        double pi_estimate = 4.0 * (double)global_count / real_total_iterations;
        
        printf("MPI версия на %d процессах\n", size);
        printf("Расчетное Pi: %.10f\n", pi_estimate);
        printf("Время работы: %.4f сек\n", end_time - start_time);
    }

    // 6. Завершение работы MPI
    MPI_Finalize();
    return 0;
}
