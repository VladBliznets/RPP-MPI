#include <iostream>
#include <fstream>
#include <vector>
#include <mpi.h> // MPI - Message Passing Interface для паралельного програмування
#include <chrono> // Бібліотека для вимірювання часу
#include <algorithm> // Бібліотека для використання алгоритму сортування

using namespace std;
using namespace std::chrono;

// Функція для швидкого сортування масиву
void quickSort(vector<int>& arr, int left, int right) {
    int i = left, j = right;
    int tmp;
    int pivot = arr[(left + right) / 2];

    while (i <= j) {
        while (arr[i] < pivot)
            i++;
        while (arr[j] > pivot)
            j--;
        if (i <= j) {
            swap(arr[i], arr[j]); // Заміна елементів, які не відповідають умові
            i++;
            j--;
        }
    }

    if (left < j)
        quickSort(arr, left, j); // Рекурсивний виклик для лівої частини масиву
    if (i < right)
        quickSort(arr, i, right); // Рекурсивний виклик для правої частини масиву
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv); // Ініціалізація MPI

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Отримання номеру поточного процесу
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Отримання загальної кількості процесів

    auto start = high_resolution_clock::now(); // Початок вимірювання часу

    if (argc != 2) {
        if (rank == 0) {
            cout << "Usage: mpiexec -n <number of processes> mpiSort.exe <input file name>\n"; // Вивід повідомлення про правильний спосіб виклику програми
        }
        MPI_Finalize();
        return 1;
    }

    string filename = argv[1];
    ifstream inputFile(filename); // Відкриття файлу для зчитування
    if (!inputFile) {
        if (rank == 0) {
            cout << "Error: Unable to open file " << filename << endl; // Вивід повідомлення про помилку відкриття файлу
        }
        MPI_Finalize();
        return 1;
    }

    vector<int> numbers;
    int number;

    if (rank == 0) {
        while (inputFile >> number) {
            numbers.push_back(number); // Зчитування чисел з файлу у вектор
        }
    }

    inputFile.close(); // Закриття файлу

    int numbersSize = numbers.size();
    MPI_Bcast(&numbersSize, 1, MPI_INT, 0, MPI_COMM_WORLD); // Розсилка розміру вектора на всі процеси

    vector<int> localData(numbersSize / size); // Локальний вектор для кожного процесу
    MPI_Scatter(numbers.data(), numbersSize / size, MPI_INT, localData.data(), numbersSize / size, MPI_INT, 0, MPI_COMM_WORLD); // Розсилка даних кожному процесу

    // Виконання швидкого сортування на даних кожного процесу
    quickSort(localData, 0, localData.size() - 1);

    // Збір відсортованих даних
    MPI_Gather(localData.data(), localData.size(), MPI_INT, numbers.data(), localData.size(), MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        // Злиття відсортованих масивів
        vector<int> temp;
        merge(numbers.begin(), numbers.begin() + localData.size(), numbers.begin() + localData.size(), numbers.end(), back_inserter(temp));
        numbers = temp;

        // Запис відсортованого масиву у файл
        ofstream outputFile("output.txt");
        if (outputFile) {
            for (auto num : numbers) {
                outputFile << num << " ";
            }
            outputFile.close();
        }
    }

    auto stop = high_resolution_clock::now(); // Кінець вимірювання часу
    auto duration = duration_cast<milliseconds>(stop - start); // Обчислення тривалості
    cout << "Process " << rank << " finished in " << duration.count() << " milliseconds.\n"; // Виведення інформації про час виконання кожного процесу

    MPI_Finalize(); // Завершення роботи з MPI
    return 0;
}
