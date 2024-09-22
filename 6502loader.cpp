#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>
using namespace std;
vector<uint8_t> readBinary(const string& filename) {
  fstream fin(filename);
  vector<uint8_t> binary;
  while (fin.peek() != EOF) {
    binary.push_back((uint8_t)fin.get());
  }
  cout<<binary.size()<<endl;
  for (auto i : binary) {
    cout<<hex<<showbase<<(int)i<<' ';
  }
  cout<<endl;
  return binary;
}
