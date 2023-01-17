#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MIN3(a, b, c)                                                          \
  ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))
#define MIN2(a, b) ((a) < (b) ? (a) : (b))
#define GET_MACRO(_1, _2, _3, NAME, ...) NAME
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(...) GET_MACRO(__VA_ARGS__, MIN3, MIN2)(__VA_ARGS__)

void allocate_matrix(unsigned int **matrix, int rows, int cols) {
  unsigned int i;

  matrix = (unsigned int **)malloc(rows * sizeof(unsigned int));
  for (i = 0; i < rows; i++) {
    matrix[i] = (unsigned int *)malloc(cols * sizeof(unsigned int));
  }
}

void deallocate_matrix(unsigned int **matrix, unsigned int rows) {
  int i;

  for (i = 0; i < rows; i++)
    free(matrix[i]);
  free(matrix);
}

int serial_levenshtein(char *s1, char *s2, int NUM_THREADS) {
  unsigned int i, x, y, s1len, s2len, j;
  s1len = strlen(s1);
  s2len = strlen(s2);

  unsigned int **matrix = malloc((s2len + 1) * sizeof(unsigned int *));
  for (i = 0; i < s2len + 1; i++)
    matrix[i] = malloc((s1len + 1) * sizeof(unsigned int));

  matrix[0][0] = 0;
  for (x = 1; x <= s2len; x++)
    matrix[x][0] = matrix[x - 1][0] + 1;
  for (y = 1; y <= s1len; y++)
    matrix[0][y] = matrix[0][y - 1] + 1;
  for (x = 1; x <= s2len; x++)
    for (y = 1; y <= s1len; y++)
      matrix[x][y] =
          MIN(matrix[x - 1][y] + 1, matrix[x][y - 1] + 1,
              matrix[x - 1][y - 1] + (s1[y - 1] == s2[x - 1] ? 0 : 1));

  unsigned int return_val = matrix[s2len][s1len];
  deallocate_matrix(matrix, s2len + 1);
  return return_val;
}

int diagonal_levenshtein(char *s1, char *s2, int NUM_THREADS) {
  unsigned int i, x, y, s1len, s2len, i_x, i_y, start, stop;
  s1len = strlen(s1);
  s2len = strlen(s2);
  unsigned int **matrix = malloc((s2len + 1) * sizeof(unsigned int *));
  for (i = 0; i < s2len + 1; i++)
    matrix[i] = malloc((s1len + 1) * sizeof(unsigned int));

  matrix[0][0] = 0;
  for (x = 1; x <= s2len; x++)
    matrix[x][0] = matrix[x - 1][0] + 1;
  for (y = 1; y <= s1len; y++)
    matrix[0][y] = matrix[0][y - 1] + 1;
  for (i_x = 2; i_x <= s1len + s2len; i_x++) {
    for (i_y = i_x - 1; i_y >= 1; i_y--) {
      y = i_x - i_y;
      x = i_y;
      if ((x > s2len) || (y > s1len)) {
        continue;
      }
      matrix[x][y] =
          MIN(matrix[x - 1][y] + 1, matrix[x][y - 1] + 1,
              matrix[x - 1][y - 1] + ((s1[y - 1] == s2[x - 1]) ? 0 : 1));
    }
  }
  unsigned int return_val = matrix[s2len][s1len];
  deallocate_matrix(matrix, s2len + 1);
  return return_val;
}

int parallel_diagonal_levenshtein(char *s1, char *s2, int NUM_THREADS) {
  unsigned int i, x, y, s1len, s2len, i_x, i_y;
  s1len = strlen(s1);
  s2len = strlen(s2);

  unsigned int **matrix = malloc((s2len + 1) * sizeof(unsigned int *));
  for (i = 0; i < s2len + 1; i++)
    matrix[i] = malloc((s1len + 1) * sizeof(unsigned int));

  matrix[0][0] = 0;
  for (x = 1; x <= s2len; x++)
    matrix[x][0] = matrix[x - 1][0] + 1;
  for (y = 1; y <= s1len; y++)
    matrix[0][y] = matrix[0][y - 1] + 1;
  for (i_x = 2; i_x <= s1len + s2len; i_x++) {
#pragma omp parallel for firstprivate(i_x, x, y) shared(matrix)                \
                                                                               \
    num_threads(NUM_THREADS)
    for (i_y = i_x - 1; i_y >= 1; i_y--) {
      y = i_x - i_y;
      x = i_y;
      if ((x > s2len) || (y > s1len)) {
        continue;
      }
      matrix[x][y] =
          MIN(matrix[x - 1][y] + 1, matrix[x][y - 1] + 1,
              matrix[x - 1][y - 1] + ((s1[y - 1] == s2[x - 1]) ? 0 : 1));
    }
  }

  unsigned int return_val = matrix[s2len][s1len];
  deallocate_matrix(matrix, s2len + 1);
  return return_val;
}

int index_of_letter(char letter, char *alphabet) {
  int i;

  for (i = 0; i < strlen(alphabet); ++i) {
    if (alphabet[i] == letter) {
      return i;
    }
  }
  return -1;
}

int parallel_friendly_algorithm(char *s1, char *s2, int NUM_THREADS) {
  unsigned int i, j = 0, x, y, s1len, s2len, u = 0, lmi;
  char *alphabet;
  s1len = strlen(s1);
  s2len = strlen(s2);
  int hash[128] = {0};

  // reading each character of str[]
  for (i = 0; i < strlen(s1); ++i) {
    // set the position corresponding
    // to the ASCII value of str[i] in hash[] to 1
    hash[s1[i]] = 1;
  }

  // counting number of unique characters
  // repeated elements are only counted once
  for (i = 0; i < 128; ++i) {
    u += hash[i];
  }
  alphabet = malloc(sizeof(int) * (u + 2));

  for (i = 0; i < 128; ++i) {
    if (hash[i] == 1) {
      alphabet[j] = i;
      ++j;
    }
  }
  alphabet[j] = '\0';

  unsigned int **mi = malloc((u + 1) * sizeof(unsigned int *));
  for (i = 0; i < u + 1; i++)
    mi[i] = malloc((s2len + 1) * sizeof(unsigned int));

  for (i = 0; i < u; i++) {
    for (j = 0; j < s2len; j++) {
      if (j == 0) {
        mi[i][j] = (s2[j] == alphabet[i]) ? 0 : -1;
      } else if (s2[j] == alphabet[i]) {
        mi[i][j] = j;
      } else {
        mi[i][j] = mi[i][j - 1];
      }
    }
  }
  unsigned int **matrix = malloc((s1len + 1) * sizeof(unsigned int *));
  for (i = 0; i < s1len + 1; i++)
    matrix[i] = malloc((s2len + 1) * sizeof(unsigned int));
  for (j = 0; j < s2len + 1; j++) {
    matrix[0][j] = j;
  }
  for (i = 0; i <= s1len; i++) {
    matrix[i][0] = i;
  }
  for (i = 1; i <= s1len; i++) {
    int a = index_of_letter(s1[i - 1], alphabet);
    for (j = 1; j <= s2len; j++) {
      lmi = mi[a][j - 1] + 1;
      if (j == lmi) {
        matrix[i][j] = matrix[i - 1][j - 1];
      } else if ((lmi == -1) || (lmi == 0)) {
        matrix[i][j] = MIN(matrix[i - 1][j - 1] + 1, matrix[i - 1][j] + 1);
      } else if (j > lmi) {
        matrix[i][j] = MIN(matrix[i - 1][j - 1] + 1, matrix[i - 1][j] + 1,
                           matrix[i - 1][lmi - 1] + (j - lmi));
      }
    }
  }
  free(alphabet);
  unsigned int return_val = matrix[s1len][s2len];

  deallocate_matrix(mi, u + 1);
  deallocate_matrix(matrix, s1len + 1);
  return return_val;
}

int parallelised_friendly_algorithm(char *s1, char *s2, int NUM_THREADS) {
  unsigned int i, j = 0, x, y, s1len, s2len, u = 0, lmi;
  char *alphabet;
  s1len = strlen(s1);
  s2len = strlen(s2);
  int hash[128] = {0};

  // reading each character of str[]
  for (i = 0; i < strlen(s1); ++i) {
    // set the position corresponding
    // to the ASCII value of str[i] in hash[] to 1
    hash[s1[i]] = 1;
  }

  // counting number of unique characters
  // repeated elements are only counted once
  for (i = 0; i < 128; ++i) {
    u += hash[i];
  }
  alphabet = malloc(sizeof(int) * (u + 2));

  for (i = 0; i < 128; ++i) {
    if (hash[i] == 1) {
      alphabet[j] = i;
      ++j;
    }
  }
  alphabet[j] = '\0';

  unsigned int **mi = malloc((u + 1) * sizeof(unsigned int *));
  for (i = 0; i < u + 1; i++)
    mi[i] = malloc((s2len + 1) * sizeof(unsigned int));

#pragma omp parallel for private(j) shared(s2len, mi, s2, alphabet)            \
    num_threads(NUM_THREADS)
  for (i = 0; i < u; i++) {
    for (j = 0; j < s2len; j++) {
      if (j == 0) {
        mi[i][j] = (s2[j] == alphabet[i]) ? 0 : -1;
      } else if (s2[j] == alphabet[i]) {
        mi[i][j] = j;
      } else {
        mi[i][j] = mi[i][j - 1];
      }
    }
  }
  unsigned int **matrix = malloc((s1len + 1) * sizeof(unsigned int *));
  for (i = 0; i < s1len + 1; i++)
    matrix[i] = malloc((s2len + 1) * sizeof(unsigned int));
#pragma omp parallel for num_threads(NUM_THREADS)
  for (j = 0; j < s2len + 1; j++) {
    matrix[0][j] = j;
  }
#pragma omp parallel for num_threads(NUM_THREADS)
  for (i = 0; i <= s1len; i++) {
    matrix[i][0] = i;
  }
  for (i = 1; i <= s1len; i++) {
    int a = index_of_letter(s1[i - 1], alphabet);
#pragma omp parallel for shared(i, s2len, mi) private(lmi)                     \
    num_threads(NUM_THREADS)
    for (j = 1; j <= s2len; j++) {
      lmi = mi[a][j - 1] + 1;
      if (j == lmi) {
        matrix[i][j] = matrix[i - 1][j - 1];
      } else if ((lmi == -1) || (lmi == 0)) {
        matrix[i][j] = MIN(matrix[i - 1][j - 1] + 1, matrix[i - 1][j] + 1);
      } else if (j > lmi) {
        matrix[i][j] = MIN(matrix[i - 1][j - 1] + 1, matrix[i - 1][j] + 1,
                           matrix[i - 1][lmi - 1] + (j - lmi));
      }
    }
  }
  free(alphabet);
  unsigned int return_val = matrix[s1len][s2len];

  deallocate_matrix(mi, u + 1);
  deallocate_matrix(matrix, s1len + 1);
  return return_val;
}

void benchmark(char *s1, char *s2, int (*f)(char *, char *, int),
               int num_threads) {
  clock_t start, stop;
  double t = 0.0;
  start = clock();
  int distance = f(s1, s2, num_threads);
  stop = clock();
  t = (double)(stop - start) / CLOCKS_PER_SEC;
  // printf("Exec time is : %f \n", t);
  // printf("Edit Diagonal Distance is %d \n", distance);
  printf("%f %d \n", t, distance);
}

int main(int argc, char **argv) {
  FILE *in = fopen(argv[3], "r");
  int len1 = atoi(argv[1]);
  int len2 = atoi(argv[2]);
  int num_threads = atoi(argv[4]);
  char s1[len1];
  char s2[len2];
  fscanf(in, "%s %s", s1, s2);
  fclose(in);
  benchmark(s1, s2, serial_levenshtein, num_threads);
  benchmark(s1, s2, diagonal_levenshtein, num_threads);
  benchmark(s1, s2, parallel_diagonal_levenshtein, num_threads);
  benchmark(s1, s2, parallel_friendly_algorithm, num_threads);
  benchmark(s1, s2, parallelised_friendly_algorithm, num_threads);

  return 0;
}