#include "../hpc_helpers.hpp"
#include <iostream>
#include <mutex>
#include <stdlib.h>
#include <thread>
#include <utility>

using namespace std;

int data_size;
int thread_size = 10;

vector<int> tsorted; // vector to hold data portion sorted by the threads
vector<int> fsorted; // vector to hold the final sorted data

mutex tsorted_mutex; // mutex to keep the vector tsorted safe

// function to print elements
void print_data(vector<int> data, int start, int end, bool newline = true) {
  for (int i = start; i < end; i++) {
    // if newline to show the thread portion (i has to be > 0)
    if (i != 0 && newline) {
      // calculate portion of that thread
      int portion = data_size / thread_size;
      // for every portion-th of data
      // print a new line
      if (i % portion == 0) {
        cout << endl;
      }
    }
    cout << data[i] << " ";
  }
  cout << endl;
}

// function to generate elements
void populate_data(vector<int> &data) {
  srand(time(NULL));
  for (int i = 0; i < data_size; i++)
    data.push_back(rand() % 100);
}

// selection sort (original)
vector<int> selection_sort(vector<int> data, int size) {
  int min; // hold current mininmum index
  for (int i = 0; i < size - 1; i++) {
    min = i;
    // find the minimum element in the right of i (unsorted)
    for (int j = i + 1; j < size; j++) {
      if (data[j] < data[min]) {
        min = j;
      }
    }
    // swap the current minumum element with the new minimum element
    if (min != i) {
      swap(data[i], data[min]);
    }
  }

  // return the vector data
  return data;
}

// selection sort (using r-value reference)
void rselection_sort(vector<int> &&data, int size, bool is_final = false,
                     bool is_threaded = false) {

  int min; // hold current mininmum index
  for (int i = 0; i < size - 1; i++) {
    min = i;
    // find the minimum element in the right of i (unsorted)
    for (int j = i + 1; j < size; j++) {
      if (data[j] < data[min]) {
        min = j;
      }
    }
    // swap the current minumum element with the new minimum element
    if (min != i) {
      swap(data[min], data[i]);
    }
  }

  // if threaded and the final thread to sort add the sorted to
  // the fully sorted vector (fsorted)
  if (is_threaded && is_final) {
    // move sorted data from begining to the end to the end of fsorted using
    // r-value reference using the move-iterator
    fsorted.insert(fsorted.end(), make_move_iterator(data.begin()),
                   make_move_iterator(data.end()));
  }

  // if threaded but not final thread add the sorted data to the thread sorted
  // vector (tsorted)
  if (is_threaded && !is_final) {
    // lock the tsorted vector using mutex so that other threads are not
    // inserting into the tsorted in order to keep sorted order
    lock_guard<mutex> vlock(tsorted_mutex);
    tsorted.insert(tsorted.end(), make_move_iterator(data.begin()),
                   make_move_iterator(data.end()));
  }
}

// multi-thread initializer
void init_thread(vector<int> &&data) {
  // calculate portion that every thread sorts
  int portion = data_size / thread_size;
  // vector to hold threads
  vector<thread> threads;

  // variable to determine starting and ending range of data
  int start, end;
  for (int i = 0; i < thread_size; i++) {
    // every portion-th is the start
    start = i * portion;
    // if it is about to have more elements than threads left assign the rest of
    // elements to the last thread
    if (i == thread_size - 1) {
      end = data_size;
    } else {
      // else assign i-th + 1 of portion
      end = (i + 1) * portion;
    }

    // vector to hold the portion of data that the thread is responsible for
    // sorting
    vector<int> portion_data(data.begin() + start, data.begin() + end);
    // add to the end of threads vector the portion data along with function and
    // paramters
    threads.emplace_back(rselection_sort, std::move(portion_data),
                         portion_data.size(), false, true);
  }

  // join all the threads
  for (auto &thread : threads) {
    thread.join();
  }
  // cout << "\nBEFORE F-SORT" << endl;
  // print_data(tsorted, 0, data_size, true);

  cout << endl;
  // final sorting
  // call the r-value reference sorting using the thread sorted vector (tsorted)
  thread t(rselection_sort, std::move(tsorted), data_size, true, true);
  t.join();
}

int main() {
  // lambda function to handle printing sorted function
  auto sorted_print = [](vector<int> data) {
    char s_print;
    cout << "\nPRINT SORTED? (y/n): ";
    cin >> s_print;
    if (s_print == 'y' || s_print == 'Y' || s_print == 'y') {
      cout << "\nSORTED: " << endl;
      print_data(data, 0, data_size, false);
    } else {
      return;
    }
  };
  // declare data vector of type int
  vector<int> data;
  cout << "ASSIGNMENT 3: SELECTION SORT" << endl;
  cout << "ENTER # OF ELEMENTS: ";
  // take user input and save it to data_size variable
  cin >> data_size;
  // fill vector with random data
  populate_data(data);
  // option menu
  int option;
optmenu:
  cout << "\nOPTION:\n\t1. Original Selection Sort\n\t2. Non-Threaded r-value "
          "Selection Sort \n\t3. Multi-Threaded "
          "r-value Selection Sort "
          "Selection Sort\n>>> ";
  // take user input and save it to option variable
  cin >> option;

  // switch-case to determine type of scenario
  switch (option) {
  // original
  case 1: {
    cout << "NOT SORTED: " << endl;
    print_data(data, 0, data_size, false);
    cout << endl;
    // start timer
    TIMERSTART(org);
    // call original selection sort
    data = selection_sort(data, data_size);
    // stop timer
    TIMERSTOP(org);
    // print sorted elements
    sorted_print(data);
    break;
  }
  // non-threaded r-value
  case 2: {
    cout << "NOT SORTED: " << endl;
    // print data elements
    print_data(data, 0, data_size, false);
    cout << endl;
    // start timer
    TIMERSTART(nontrval);
    // call r-value selection sort
    rselection_sort(std::move(data), data_size, false, false);
    // stop timer
    TIMERSTOP(nontrval);
    // print sorted elements
    sorted_print(data);
    break;
  }
  // multi-threaded r-value
  case 3: {
    cout << "NOT SORTED: " << endl;
    // print data elements
    print_data(data, 0, data_size, true);
    cout << endl;
    // start timer
    TIMERSTART(trval);
    // initalize threads
    init_thread(std::move(data));
    // stop timer
    TIMERSTOP(trval);
    // print sorted elements
    sorted_print(fsorted);
    break;
  }
  // invalid option case
  default: {
    cout << "\nINVALID OPTION! Please try again." << endl;
    // go to the option menu
    goto optmenu;
    break;
  }
  }

  return 0;
}
