#include <iostream>
#include <cstdint>
#include "a10p2.h"

using namespace std;

int64_t wain(int64_t*, int64_t) {
  char c;
  cin >> noskipws >> c;
  if (cin.fail()) {
      return 0;
  }
  int bytes = 1;
  int numC = c;
  int64_t *words = cons(numC, NULL);
  int64_t *head = words;
  cout << c;

  while (cin >> noskipws >> c) {
      numC = c;
      bytes++;
      int64_t *next = cons(numC, NULL);
      setcdr(words, next);
      words = next;
      cout << c;
  }

  int64_t *prev = head;
  while (head) {
      cout << (char)car(head);
      head = cdr(head);
      snoc(prev);
      prev = head;
  }
  return bytes;
}

int main(int argc, char *argv[]) {
  int ret = wain(0,0);
  std::cerr << ret << std::endl;
  return ret;
}
