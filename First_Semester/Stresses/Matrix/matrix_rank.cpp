#include "../../D/matrix.h"
#include <random>
#include <fstream>
#include <time.h>

int get_random(int min, int max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

Rational get_random_rat(int zz, int nn) {
  Rational ans;
  ans.z = get_random(-abs(zz), abs(zz));
  ans.n = get_random(1, nn);
  return ans;
}

template <size_t N, size_t M, typename Field>
void random_fill(Matrix<N, M, Field>& matrix, int min, int max) {
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < M; ++j) {
      matrix[i, j] = get_random(min, max);
    }
  }
}

template <size_t N, size_t M>
void random_fill(Matrix<N, M, Rational>& matrix, int zz, int nn) {
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < M; ++j) {
      matrix[i, j] = get_random_rat(zz, nn);
    }
  }
}

// template <size_t N, size_t M, typename Field=Rational>
// void copy_fill(const Matrix<N, M, Field>& matrix, Correct::Matrix<N, M, Field>& cmatrix) {
//   for (size_t i = 0; i < N; ++i) {
//     for (size_t j = 0; j < M; ++j) {
//       cmatrix[i][j] = matrix[i, j];
//     }
//   }
// }

struct Diapason {
  int min;
  int max;
};

int main() {
  static constexpr size_t N = 20;
  int nn = 100;
  int zz = 1000000;

  time_t start, end;

  time(&start);
  Matrix<N, N> matrix;
  random_fill<N, N>(matrix, zz, nn);

  Matrix<N, N> ans_matrix = matrix.transposed() * matrix;
  std::cout << "\n\n" << ans_matrix;
  time(&end);

  std::cout << difftime(end, start);
}