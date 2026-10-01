#include <cstdlib>
#include <iostream>
#include <sys/stat.h>
#include <bits/stdc++.h>
#include <chrono>
// check if chrono works aswell instead of omp

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
// e.g. double computationEnd = omp_get_wtime();

using namespace std;

int M = 2048;
int N = 64;


int THREADS = 1;
int REP = 1;
int writeMatrixToFile = 0; 
string OUTPUT_DIR = "results";


void matrixGenerator(int seed, float *mat) {
    srand(seed);
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < M; ++j) {
            float randomNum = rand();
            mat[i * M + j] = randomNum*0.000000001;
        }
    }
}


void writeMatrix(float *mat, const char *filename) {
    struct stat st;
    if (stat("results", &st) != 0) {
        mkdir("results", 0755);
    }
    FILE *f = fopen(filename, "w");
    for (int y = 0; y < M; y++) {
        for (int x = 0; x < M; x++) {
            fprintf(f, "%f ", mat[y * M + x]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

void initZeroMatrix(float *mat) {
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < M; ++j) {
            mat[i * M + j] = 0;
        }
    }
}

int main(int argc, char* argv[]){
    if (argc == 6) {
        OUTPUT_DIR = argv[1];
        M = std::atoi(argv[2]);
        N = std::atoi(argv[3]);
        THREADS = std::atoi(argv[4]);
        REP = std::atoi(argv[5]);
    } else {
        std::cout << "Arg. read failed\n";
        return 1;
    }
    printf("REP:%d, M:%d, N:%d, THREADS:%d \n", REP, M, N, THREADS);
    auto E2E_start = omp_get_wtime();
    bool conceptualTest = 0;

    if (M<N || M % N  != 0) {
        cout << "M is smaller than N, or M modulo N is not 0 for M, N: " << M << ", " << N << "Done" << endl;
        return 1;
    }

    char filenameA[256];
    char filenameB[256];
    char filenameC[256];
    snprintf(filenameA, sizeof(filenameA), "%s/seq_matA_%d_%d_%d_%d_t_v2_openmp.txt", OUTPUT_DIR.c_str(),  REP, M, N, THREADS);
    snprintf(filenameB, sizeof(filenameB), "%s/seq_matB_%d_%d_%d_%d_t_v2_openmp.txt", OUTPUT_DIR.c_str(),  REP, M, N, THREADS);
    snprintf(filenameC, sizeof(filenameC), "%s/seq_matC_%d_%d_%d_%d_t_v2_openmp.txt", OUTPUT_DIR.c_str(),  REP, M, N, THREADS);
    
    float* matrixA = new float[M * M];
    float* matrixB = new float[M * M];
    float* matrixC = new float[M * M];

    float* blockA = new float[N * N];
    float* blockB = new float[N * N];
    float* blockC = new float[N * N];


    auto startCreateA = omp_get_wtime();
    matrixGenerator(16, matrixA);
    auto endCreateA = omp_get_wtime();
    auto elapsed_secondsA = (endCreateA - startCreateA);
    cout << "Time for gen. A: " << elapsed_secondsA << endl;

    auto startCreateB = omp_get_wtime();
    matrixGenerator(44, matrixB);        
    auto endCreateB = omp_get_wtime();
    auto elapsed_secondsB = (endCreateB-startCreateB);
    cout << "Time for gen. and write B: " << elapsed_secondsB << endl;
    
    auto startCreateC = omp_get_wtime();
    initZeroMatrix(matrixC);
    auto endCreateC = omp_get_wtime();
    auto elapsed_secondsC = (endCreateC-startCreateC);
    cout << "Time for gen. C: " << elapsed_secondsC << endl;

    auto computationStart = omp_get_wtime();
    auto init_matrix = (computationStart-E2E_start);


    #pragma omp parallel for
    for (int ii = 0; ii < M; ii += N) {
       // if (ii % (2*N) == 0 ) { // changed to M from N
       //     cout << "Progress outter loop: " << (float) ii / M << endl;
       // }
        for (int kk = 0; kk < M; kk += N) {
            for (int jj = 0; jj < M; jj += N) {
                for (int i = ii; i < ii + N; ++i) {
                    for (int k = kk; k < kk + N; ++k) {
                        const float a = matrixA[i * M + k];
                        for (int j = jj; j < jj + N; ++j) {
                            matrixC[i * M + j] += a * matrixB[k * M + j];
                        }
                    }
                }
            }
        }
    }
    if (writeMatrixToFile == 1 && REP == 55) {writeMatrix(matrixC, filenameC);}
    auto computationEnd = omp_get_wtime();
    auto elapsedComputationTime = (computationEnd-computationStart);

    delete[] blockA;
    delete[] blockB;
    delete[] blockC;

    delete[] matrixA;
    delete[] matrixB;
    delete[] matrixC;

    
    cout << "End" << endl;
    auto E2E_End = omp_get_wtime();

    auto E2E = (E2E_End-E2E_start);
    std::cout << "Writing file" << std::endl;
    if (!std::filesystem::exists("./results")) {
        std::filesystem::create_directory("./results");
    }
        std::ofstream fresults("results/v0_openmp_R" + std::to_string(REP) + "_M" + std::to_string(M) + "_N" + std::to_string(N) + "_T" + std::to_string(THREADS) + ".benchmark");
        fresults << "Repetition=" << REP << "\n";
        fresults << "Dimension=" << M << "\n";
        fresults << "BlockDimension=" << N << "\n";
        fresults << "Threads="  << THREADS << "\n";
        fresults << "Initialization=" << init_matrix << "\n";
        fresults << "E2E=" << E2E << "\n"; 
        fresults.close();

}
