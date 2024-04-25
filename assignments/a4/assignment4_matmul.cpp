#include <iostream>
#include <stdlib.h>
#include <thread>
#include <vector>
#include "../hpc_helpers.hpp"

// sequential matrix multiplication
void sequential_matmul(std::vector<int>& A, std::vector<int>& B, std::vector<int>& C, int m, int n, int l) {  
	// iterate over each row (A)
  for(int row=0; row < m; row++) {
    // iterate over each column (B)
		for(int col= 0; col < n; col++) {
      // init accumulator
			int accum = 0;
      // iterate over the shared dimension to get each element (i-th row and j-th column) 
      for (int k = 0; k < l; k++) {         			
        int A_idx = row * l + k; // A[row][col]
        int B_idx = k * n + col; // B[k][col]
        accum += A[A_idx] * B[B_idx]; // calculate the product 
      }
      C[row * n + col] = accum; // set the product to C[row][col]
    }
  }
}

// print elements in vector with matrix style
void print(std::vector<int>& d, int m, int n) {
	std::cout << std::endl;
  // iterate over each row
	for(int row=0; row < m; row++) {
    // iterate over each column
		for(int col=0; col < n; col++) {
      int idx = row * n + col; // A[row][col]
			std::cout << d[idx] << " ";
		}
    // new line every col-th
		std::cout << std::endl;
	}
}

// create an identity matrix (for testing)
std::vector<int> create_identity(int m) {
	std::vector<int> identity(m * m, 0); // init a vector to hold the identity matrix
  // iterate over each row
	for (int row = 0; row < m; row++) {
		identity[row * (m + 1)] = 1; // calculate the diagonal and set it to 1
  }
  return identity;
}
// populate vector with random nums
std::vector<int> create_random(int m, int n) {
  std::vector<int> data(m * n, 0); // init a vector to hold the values
	// iterate over each row
  for(int i=0; i < m * n; i++) {
		data[i] = 1 + rand() % 9; // fill the i-th index with values(1 - 9)
	}
	return data;	
}

int main() {
  // user size input
  sizeopt:
	int a_row, a_col, b_row, b_col;
	std::cout << "MATRIX A: " << std::endl;
	std::cout << "(A) Enter rows: ";
	std::cin >> a_row;
	std::cout << "(A) Enter cols: ";
	std::cin >> a_col;

	std::cout << "\nMATRIX B: " << std::endl;
	std::cout << "(B) Enter rows: ";
	std::cin >> b_row;
	std::cout << "(B) Enter cols: ";
	std::cin >> b_col;
	

	if(a_col != b_row) {
		std::cout << "\nCANNOT multiply (A's columns need to equal B's row). Please try again!\n" << std::endl;
	 	goto sizeopt; 
	}

  // the col of a and row of b must match
	int m = a_row; // common between a & c since c = a_row * b_col
	int n = b_col; // common between b & c since c = a_row * b_col
	int l = a_col; // common between a & b
	
  std::vector<int> a = create_random(m, l); // populate with random values
	std::vector<int> b = create_random(n, l); // populate with random values
	std::vector<int> c(m*n, 0); // populate with zero for a_row * b_col
	
  // size of threads (for block & cyclic)
  int num_threads = 4;

  // user implementation input
	impopt:
	std::cout << std::endl;
	int option;
	std::cout << "OPTION: \n1. SEQUENTIAL\n2. BLOCK PARALLEL\n3. CYCLIC PARALLEL " << std::endl;
	std::cout << ">> : ";
	std::cin >> option;
	
  // switch case for implementation
	switch(option) {
    // sequential
		case 1: {
		std::cout << std::endl;
		std::cout << "\nSEQUENTIAL MATRIX MULTIPLICATION: " << std::endl;	
		TIMERSTART(sequential); // timer start
		sequential_matmul(a, b, c, m, n, l); // pass the a, b, c vectors with m, n, l sizes
		TIMERSTOP(sequential); // timer start
		break;
		}
    // block parallel
		case 2: {
    std::cout << std::endl;
		std::cout << "\nBLOCK PARALLEL MATRIX MULTIPLICATION: " << std::endl;	
    // timer start
		TIMERSTART(block);
    // init vector with threads
		std::vector<std::thread> threads;
    // iterate for the size of threads
		for(int i = 0; i < num_threads; i++) {
      // pass the vectors by reference and every available outside variable by value
			threads.emplace_back([=, &a, &b, &c]() {
				//std::cout << i << std::endl;
        const int chunk = SDIV(m, num_threads); // calculate chunk for every thread
        const int lower = i * chunk; // starting index
        const int upper = std::min(lower + chunk, m); // ending index
				//std::cout << "chunk: " << chunk << " lower: " << lower << " upper: " << upper << std::endl;	
				// iterate every row between lower and upper (A)
        for(int row = lower; row < upper; row++) {
            // iterate every column (B)
            for(int col = 0; col < n; col++) { 
                int accum = 0;	// init accumulator
                // iterate every shared dimension
                for(int k = 0; k < l; k++) {
                  int A_idx = row * l + k; // A[row][col]
                  int B_idx = k * n + col; // B[k][col]
							    accum += a[A_idx] * b[B_idx]; // calculate the product 
						    }		
						    c[row * n + col] = accum; // set the result to c[row][col]
					   }
        } 
    	});
		}
    // join every thread
		for(auto& thread: threads) thread.join();
		TIMERSTOP(block); // end timer
		break;
		} 
    // cyclic parallel
		case 3: {
    // start timer
		TIMERSTART(cyclic);
		std::cout << "\nCYCLIC PARALLEL MATRIX MULTIPLICATION (w/ 4 THREADS): "<< std::endl;
		std::vector<std::thread> threads; // init vector with threads
    // iterate for the size of threads
		for(int i = 0; i < num_threads; i++) {
      // pass the vectors by referencee and others by values
			threads.emplace_back([=, &a, &b, &c]() {
      		// std::cout << i << std::endl;
          // [0][1][2][3][0][1][2][3].... <- thread
          // [0][1][2][3][4][5][6][7].... <- task
          // iterate for every row until the thread does not have any element assigned to compute (A)
          for(int row = i; row < m; row += num_threads) {
              // iterate every column (B)
              for(int col = 0; col < n; col++) {
                  int accum = 0; // init accumulator
                  for(int k=0; k < l; k++) {
                      int A_idx = row * l + k; // A[row][col]
                      int B_idx = k * n + col; // A[k][col]
                      accum += a[A_idx] * b[B_idx]; // calculate the product
                  }
                  c[row * n + col] = accum; // set the result to c[row][col]
              }
          }
      });
		}
    // join threads
		for(auto& thread: threads) thread.join();
		TIMERSTOP(cyclic); // stop timer
		break;
		}
    // invalid option
		default: {
		std::cout << "ERR: Invalid option. Please try again!" << std::endl;
		goto impopt; // take user to the implementation option input
		break;
		}
	}
	


	// printing the values
	char prntopt;
	std::cout << "\nPrint? (y/n): ";
	std::cin >> prntopt;
	if(tolower(prntopt) == 'y') {
		std::cout << "\nA: ";
		print(a, a_row, a_col);
		std::cout << "\nB: ";
		print(b, b_row, b_col);
		std::cout << "\nC: ";
		print(c, m, n);
	}		
	return 0;
}
