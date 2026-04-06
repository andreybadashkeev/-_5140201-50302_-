from mpi4py import MPI
import random
import time

# Общее количество итераций
TOTAL_ITERATIONS = 10000000
# тут я просто добавлю комментрарий

def main():
    # 1. Инициализация MPI
    comm = MPI.COMM_WORLD
    rank = comm.Get_rank()
    size = comm.Get_size()

    # Засекаем время
    start_time = MPI.Wtime()

    # 2. Делим работу
    # Убеждаемся, что деление без остатка, или просто округляем
    # (в питоне 3 "/" возвращает float, поэтому используем "//")
    iterations_per_proc = TOTAL_ITERATIONS // size

    local_count = 0
    
    # Чтобы рандом был разным, можно сделать seed, 
    # но random.random() и так использует системное время,
    # а процессы запускаются чуть в разное время, так что обычно ок.
    # Но для надежности можно:
    random.seed(time.time() + rank)

    # 3. Вычисления
    # В Python циклы медленные, поэтому 100 млн итераций будут считаться 
    # ощутимо дольше, чем в C.
    for _ in range(iterations_per_proc):
        x = random.uniform(-1.0, 1.0)
        y = random.uniform(-1.0, 1.0)
        if x*x + y*y <= 1.0:
            local_count += 1

    # 4. Сбор результатов
    # reduce: берет данные (local_count), применяет операцию (MPI.SUM)
    # и возвращает результат на root (0).
    # Важно: mpi4py сам понимает типы данных!
    global_count = comm.reduce(local_count, op=MPI.SUM, root=0)

    end_time = MPI.Wtime()

    # 5. Вывод
    if rank == 0:
        real_total = iterations_per_proc * size
        pi_estimate = 4.0 * global_count / real_total
        
        # Вывод результатов (используем .format для надежности)
        print("Python MPI на {} процессах".format(size))
        print("Расчетное Pi: {}".format(pi_estimate))
        print("Время работы: {:.4f} сек".format(end_time - start_time))

        # --- ТЕСТ ---
        PI_REF = 3.141592653589793
        TOLERANCE = 0.01  # Допуск побольше, так как точек мало
        
        diff = abs(pi_estimate - PI_REF)
        
        if diff < TOLERANCE:
            print("[TEST PASSED] Result is within tolerance {}".format(TOLERANCE))
        else:
            print("[TEST FAILED] Error too high! Diff: {:.6f}".format(diff))
        # ------------
if __name__ == "__main__":
    main()
