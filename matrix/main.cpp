
#define DEBUG
#include "matrix.h"
#include <random>

int main() {
    Matrix<double> m(1000, 1000), n(1000, 1000);

    for (int i = 0; i < m.rows(); i++)
        for (int j = 0; j < m.cols(); j++)
            m[i][j] = rand() % 1000 - 500;
    for (int i = 0; i < n.rows(); i++)
        for (int j = 0; j < n.cols(); j++)
            n[i][j] = rand() % 1000 - 500;

    m * n;
    
    return 0;
}
