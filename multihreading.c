#ifndef BASE_MULTITHREADING_C
#define BASE_MULTITHREADING_C

#include "stdio.h"
#include "stdlib.h"

#include "profileapi.h"
#include "windows.h"
#include "winnt.h"
#include <minwindef.h>

typedef void (*fill_args_callback_type)(int, void *);

typedef DWORD WINAPI (*do_work_callback_type)(LPVOID param);

int multithreading_number_of_threads() {
  SYSTEM_INFO sys_info;
  GetSystemInfo(&sys_info);

  return (int)sys_info.dwNumberOfProcessors;
}

void multithreading_block_devide_work(
    do_work_callback_type do_work_callback,
    fill_args_callback_type fill_args_callback, int size_of_thread_args) {
  int number_of_threads = multithreading_number_of_threads();
  HANDLE *threads = (HANDLE *)malloc(number_of_threads);
  char *thread_args = (char *)malloc(number_of_threads * size_of_thread_args);

  LARGE_INTEGER freq, start, end;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&start);

  for (int i = 0; i < number_of_threads; i++) {
    fill_args_callback(i, &thread_args[i * size_of_thread_args]);

    threads[i] = CreateThread(NULL, 0, do_work_callback,
                              &thread_args[i * size_of_thread_args], 0, NULL);

    if (threads[i] == NULL) {
      fprintf(stderr, "Failed to create thread %d/n", i);
    }
  }

  WaitForMultipleObjects(number_of_threads, threads, TRUE, INFINITE);

  QueryPerformanceCounter(&end);
  double elapsed = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
  fprintf(stderr, "Render time: %.3f seconds \n", elapsed);

  for (int i = 0; i < number_of_threads; i++) {
    CloseHandle(threads[i]);
  }

  free(threads);
  free(thread_args);
}

#endif
