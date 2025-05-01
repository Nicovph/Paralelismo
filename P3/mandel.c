/*The Mandelbrot set is a fractal that is defined as the set of points c
in the complex plane for which the sequence z_{n+1} = z_n^2 + c
with z_0 = 0 does not tend to infinity.*/

/*This code computes an image of the Mandelbrot set.*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

#define DEBUG 1

#define X_RESN 1024 /* x resolution */
#define Y_RESN 1024 /* y resolution */

/* Boundaries of the mandelbrot set */
#define X_MIN -2.0
#define X_MAX 2.0
#define Y_MIN -2.0
#define Y_MAX 2.0

/* More iterations -> more detailed image & higher computational cost */
#define maxIterations 1000

typedef struct complextype
{
  float real, imag;
} Compl;

static inline double get_seconds(struct timeval t_ini, struct timeval t_end)
{
  return (t_end.tv_usec - t_ini.tv_usec) / 1E6 +
         (t_end.tv_sec - t_ini.tv_sec);
}

int main(int argc, char *argv[])
{

  MPI_Init(&argc, &argv);
  /* Mandelbrot variables */
  int i, j, k;
  Compl z, c;
  float lengthsq, temp;
  int *vres, *res[Y_RESN];
  int rank, nprocs, local_rows = 0;
  int *vres_local, *res_local[Y_RESN];
  int padding = 0, N = Y_RESN;
  struct timeval ti, tf;
  struct timeval ti_2, tf_2;
  double local_time = 0, total_time = 0;

  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (N % nprocs != 0)
  {
    padding = nprocs - (N % nprocs); // padding if not divisible
  }

  N = N + padding;         // new size of the image
  local_rows = N / nprocs; // rows per process

  /* Allocate result matrix of Y_RESN x X_RESN */
  if (rank == 0)
  {

    vres = (int *)malloc(N * X_RESN * sizeof(int)); // matrix
    if (!vres)
    {
      fprintf(stderr, "Error allocating memory\n");
      return 1;
    }
    for (i = 0; i < N; i++)
      res[i] = vres + i * Y_RESN; // pointer to each row
  }

  for (int i = 0; i < nprocs; i++)
  {
    vres_local = (int *)malloc(local_rows * X_RESN * sizeof(int)); // local matrix
  }

  if (!vres_local)
  {
    fprintf(stderr, "Error allocating memory\n");
    return 1;
  }

  for (i = 0; i < local_rows; i++)
    res_local[i] = vres_local + i * X_RESN; // pointer to each row

  MPI_Scatter(vres, local_rows * X_RESN, MPI_INT, vres_local, local_rows * X_RESN, MPI_INT, 0, MPI_COMM_WORLD);

  /* Start measuring time */
  gettimeofday(&ti, NULL);

  /* Calculate and draw points */
  for (i = 0; i < local_rows; i++)
  {
    int global_i = rank * local_rows + i; //for process 2, N=17 y P=5:  2 * 4 + i = 8 + i ⇒ rows 8, 9, 10, 11
    if (global_i >= Y_RESN)
      continue;
    

    for (j = 0; j < X_RESN; j++)
    {
      z.real = z.imag = 0.0;
      c.real = X_MIN + j * (X_MAX - X_MIN) / X_RESN;
      c.imag = Y_MAX - i * (Y_MAX - Y_MIN) / N;
      k = 0;

      do
      { /* iterate for pixel color */
        temp = z.real * z.real - z.imag * z.imag + c.real;
        z.imag = 2.0 * z.real * z.imag + c.imag;
        z.real = temp;
        lengthsq = z.real * z.real + z.imag * z.imag;
        k++;
      } while (lengthsq < 4.0 && k < maxIterations);

      if (k >= maxIterations)
        res_local[i][j] = 0;
      else
        res_local[i][j] = k;
    }
  }

  /* End measuring time */
  gettimeofday(&tf, NULL);
  fprintf(stderr, "Process %d: Computation time (seconds) = %lf\n", rank, get_seconds(ti, tf));

  gettimeofday(&ti_2, NULL);
  MPI_Gather(vres_local, local_rows * X_RESN, MPI_INT, vres, local_rows * X_RESN, MPI_INT, 0, MPI_COMM_WORLD);
  gettimeofday(&tf_2, NULL);
  fprintf(stderr, "Process %d: Communication time (seconds) = %lf\n", rank, get_seconds(ti_2, tf_2));
  local_time = get_seconds(ti, tf) + get_seconds(ti_2, tf_2);
  fprintf(stderr, "Process %d: Total local time (seconds) = %lf\n", rank, local_time);

  MPI_Reduce(&local_time, &total_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  if (rank == 0)
  {
    fprintf(stderr, "Total Time (seconds) = %lf\n", total_time);
    if (DEBUG)
    {
      for (i = 0; i < N; i++)
      {
        for (j = 0; j < X_RESN; j++)
          printf("%3d ", res[i][j]);
        printf("\n");
      }
    }
  }

  /* Free memory */

  if (rank == 0)
  {
    free(vres);
  }
  free(vres_local);

  MPI_Finalize();
  return 0;
}
