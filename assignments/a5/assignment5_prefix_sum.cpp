#include "../hpc_helpers.hpp"
#include <cmath>
#include <future>
#include <iostream>
#include <ostream>
#include <time.h>
#include <vector>

int main() {

  int element_size;   // element size
  int thread_size;    // thread size
  int implementation; // implementation type
  std::cout << "COST OPTIMAL PARALLEL PREFIX" << std::endl;
  std::cout << "Enter Element Size: ";
  std::cin >> element_size;
  std::cout << "\n1. Threaded \n2. Non-Threaded" << std::endl;
  std::cout << ">>> : ";
  std::cin >> implementation;

  srand(time(NULL));
  std::vector<int> elements; // vector for elements
  // generate random nums and fill vector
  for (int i = 0; i < element_size; i++)
    elements.push_back(rand() % 10);

  // non-threaded: k = full element size and thread_size  = 1
  int k = element_size;
  thread_size = 1;

  // threaded: k = element_size / thread_size and thread_size = 4
  if (implementation == 1) {
    thread_size = 4;
    k = element_size / thread_size;
  }

  // print elements in vector
  auto print = [&elements, k, element_size]() {
    std::cout << std::endl;
    for (int i = 0; i < element_size; i++) {
      if (i % k == 0)
        std::cout << std::endl;
      std::cout << elements[i] << " ";
    }
    std::cout << std::endl;
  };

  // stage 1: compute local prefix sums
  auto stage1 = [&](int start, int end) {
    for (int i = start; i < end; i++) {
      for (int j = 1; j < k; j++) {
        elements[i * k + j] += elements[i * k + j - 1];
      }
    }
  };

  // stage 2: compute the prefix sum of the right-most (last element) of each
  // chunk
  auto stage2 = [&thread_size, &elements, k, element_size]() {
    std::cout << thread_size << std::endl;
    int i = 0;
    for (int j = pow(2, i); j <= thread_size; j++) {
      int end = j * k - 1;
      int start = (j - pow(2, i)) * k - 1;
      if (end > 0 && start > 0) {
        elements[end] += elements[start];
      }
    }
  };

  // stage 3: compute the prefix sum using the right-most (last-element) from
  // the previous chunk above to the elements in chunk without including the
  // last element in chunk
  auto stage3 = [&](int i) {
    int start = i * k - 1;
    for (int j = 0; j < k - 1; j++) {
      int end = i * k + j;
      elements[end] += elements[start];
    }
  };

  // implementation switch-case
  switch (implementation) {
    // threaded implementation
  case 1: {
    std::cout << "\nCalculating " << element_size << " elements using 4 threads"
              << std::endl;

    // vector to hold futures using async
    std::vector<std::future<void>> futures;
    TIMERSTART(threaded); // start timer

    // assign async tasks in the vector for stage 1
    for (int i = 0; i < thread_size; i++)
      futures.push_back(std::async(std::launch::async, stage1, i, i + 1));

    // return all the future values to finish the tasks
    for (auto &f : futures)
      f.get();

    futures.clear(); // clear vector from any tasks

    stage2(); // execute stage 2

    // assign async tasks in the vector for stage 2
    for (int i = 1; i < thread_size; i++)
      futures.push_back(std::async(std::launch::async, stage3, i));

    // return all the future values to finish the tasks
    for (auto &f : futures)
      f.get();

    // stop timer
    TIMERSTOP(threaded);
    // std::cout << "\nAfter stage 3: ";
    //   print();
    break;
  }
  // non-threaded implementation
  case 2: {
    std::cout << "\nCalculating " << element_size
              << " elements without using threads " << std::endl;

    // print();
    //  start timer
    TIMERSTART(nonthreaded);
    // completely run the task using stage 1 and the full size for the chunk
    stage1(0, 1);
    // stop timer
    TIMERSTOP(nonthreaded);
    // print();
    break;
  }
    // error in choice of implementation
  default: {
    std::cout << "Error. Invalid option. Please try again" << std::endl;
    break;
  }
  }
  return 0;
}
