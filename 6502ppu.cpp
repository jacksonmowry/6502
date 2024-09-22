#include <iostream>
#include <cstdint>
using namespace std;
#define RED     "\033[31m"      // Red
#define GREEN   "\033[32m"      // Green
#define BLUE    "\033[34m"      // Blue
#define RESET   "\033[0m"
void ppuLoop(uint8_t* memory) {
  string block = "█";
  for (uint16_t row = 0xFF10; row <= 0xFFEF; row += 0x10) {
    for (int i = 0; i < 16; i++) {
      switch(memory[row+i]) {
      case(0x1): {
        cout<<RED<<block<<' ';
        break;
      }
      case(0x2): {
        cout<<GREEN<<block<<' ';
        break;
      }
      case(0x3): {
        cout<<BLUE<<block<<' ';
        break;
      }
      default: {
        cout<<RESET<<block<<' ';
        break;
      }
      }
    }
    cout<<RESET<<endl;
  }
}
