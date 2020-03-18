#include <iostream>
#include <array>
#include <thread>

const int threadsCount = 4;

const int matrixSize = 16;
std::array<std::array<int, matrixSize>, matrixSize> leftMatrix; 
std::array<std::array<int, matrixSize>, matrixSize> rightMatrix; 

const int outputRowCount = matrixSize;
const int outputColCount = matrixSize;
const int inputCommonSize = matrixSize;
std::array<std::array<int, outputRowCount>, outputColCount> outputMatrix;

void mult(int startIndexRow, int endIndexRow, int colCount) {
    std::cout << "I am in thread" << std::endl;
    for (int i=startIndexRow; i<endIndexRow; i++) {
        for (int j=0; j<colCount; j++) {
            int sum = 0;
            for (int k=0; k<colCount; k++) {
                sum += leftMatrix[i][k] * rightMatrix[k][j];
            }
            outputMatrix[i][j] = sum;
        }
    }
}

int main () {
    // wypelnienie macierzy danymi
    int a = 0;
    for (int i=0; i<matrixSize; i++) {
        for (int j=0; j<matrixSize; j++) {
            leftMatrix[i][j] = a;
            rightMatrix[i][j] = a++;
        } 
    }

    // Wyswietlenie macierzy
    std::cout << "Left matrix:" << std::endl;
    for (const auto& row: leftMatrix) {
        for (const auto& el: row) {
           std::cout << el << "\t";
        }
        std::cout << std::endl;
    }

    std::cout << "Right matrix:" << std::endl;
    for (const auto& row: rightMatrix) {
        for (const auto& el: row) {
           std::cout << el << "\t";
        }
        std::cout << std::endl;
    }

    // Mnozenie macierzy
    int rowsPerThread = outputRowCount/threadsCount;
    std::thread multtiplicationThreads[threadsCount];

    for (int i=0; i<threadsCount; i++) {
        int startIndexRow = i*rowsPerThread;
        int endIndexRow = startIndexRow + rowsPerThread;
        int colCount = outputColCount;
        multtiplicationThreads[i] = std::thread(mult, startIndexRow, endIndexRow, colCount);
    }

    for (int i=0; i<threadsCount; i++) {
        multtiplicationThreads[i].join();
    }

    // Wyswietlenie macierzy wynikowej
    std::cout << "Output matrix" << std::endl;
    for (const auto& row: outputMatrix) {
        for (const auto& el: row) {
           std::cout << el << "\t";
        }
        std::cout << std::endl;
    }
    return 0;
}
