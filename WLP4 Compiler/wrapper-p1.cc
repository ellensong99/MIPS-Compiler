#include <iostream>
#include <cstdint>
#include "a10p1.h"

using namespace std;

int64_t wain(int64_t* a, int64_t n) {
  cout << "n: " << n << endl;
  for (int i = 0; i < n; i++) {
      arena()[a[i]]++;
  }
  int max = 0;
  for (int i = 0; i < n; i++) {
      if (arena()[a[i]] > max) {
          max = arena()[a[i]];
      }
  }
  return max; 
}

int main(int argc, char *argv[]) {
  int64_t l, c;
  int64_t* a;
  std::cout << "Enter length of array: ";
  std::cin >> l; 
  a = new int64_t[l];
  for(int64_t i = 0; i < l; ++i) {
    std::cout << "Enter value of array element " << i << " ";
    std::cin >> a[i];
  }
  c = wain(a,l);
  delete [] a;
  std::cerr << c << std::endl;
  return c;
}
