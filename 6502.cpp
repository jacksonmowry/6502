#include <cstdint>
#include <iostream>
#include <iomanip>
#include <bit>
#include <numeric>
#include "6502loader.cpp"
#include "6502ppu.cpp"
using namespace std;
//SR Negative, Overflow, Ignored, Break, Decimal, Interrupt, Zero, Carry
enum {
  SR = 0,
  AC,
  X,
  Y,
  SP
};
void loadProgram(vector<uint8_t> program, uint8_t* memory) {
  for (size_t i = 0; i < program.size(); i++) {
    memory[0x0600+i] = program[i];
  }
}

void hexDump(uint8_t* program, uint8_t* memory) {
  uint16_t PC = 0x0600;
  int i = 0;
  while (memory[PC]) {
    cout<<"Program: "<<hex<<(int)program[i++]<<" Memory: "<<hex<<int(memory[PC++])<<endl;
  }
}
void regDump(uint8_t reg[]) {
  cout<<"SR = "<<hex<<(int)reg[SR]<<" AC = "<<hex<<(int)reg[AC]<<" X = "<<hex<<(int)reg[X]<<" Y = "<<hex<<(int)reg[Y]<<" SP = "<<hex<<(int)reg[SP]<<endl;
}
void printMemory(uint8_t* memory, uint16_t startAddress) {
  uint8_t i = 0;
  while (i < 16) {
    cout<<setw(2)<<setfill('0')<<hex<<(int)memory[startAddress+i++]<<' ';
  }
  cout<<endl;
}
void updateNZFlags(int8_t result, uint8_t* reg) {
  if (result < 0) {
    reg[SR] |= (1 << 7);
    reg[SR] &= 0b11111101;
  }
  else if (result == 0) {
    reg[SR] |= (1<<1);
    reg[SR] &= 0b01111111;
  }
  else {
    reg[SR] &= 0b01111101;
  }
}

int main() {
  uint8_t reg[5] = {0, 0, 0, 0, 0xFF};
  uint16_t PC = 0x0600;
  cout<<"PC = "<<hex<<(int)PC<<endl;
  uint8_t memory[0xFFFF];
  loadProgram(readBinary("bin.txt"), memory);
  // hexDump(program, memory);
  bool running = true;
  // Loop
  while (running) {
    uint8_t instr = memory[PC++];
    switch(instr) {
      // ADC add with carry immediate
      case(0x69): {
        uint8_t imm = memory[PC++];
        if (reg[AC] > 0 && imm > 255-reg[AC]) {
          reg[SR] &= 0x01;
        }
        reg[AC] += imm;
      }
      break;
      // ADC add with carry zeropage
      case(0x65): {
        uint8_t zeropage = memory[PC++];
        uint8_t value = memory[zeropage];
        if (reg[AC] > 0 && value > 255-reg[AC]) {
          reg[SR] &= 0x01;
        }
        reg[AC] += value;
      }
      break;
      // ADC add with carry zeropage,X
      case(0x75): {
        uint8_t zeropage = memory[PC++];
        uint8_t value = memory[zeropage+reg[X]];
        if (reg[AC] > 0 && value > 255-reg[AC]) {
          reg[SR] &= 0x01;
        }
        reg[AC] += value;
      }
      break;
      // ADC add with carry absolute LLHH
      case(0x6D): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint16_t absolute = high + low;
        uint8_t value = memory[absolute];
        if (reg[AC] > 0 && value > 255-reg[AC]) {
          reg[SR] &= 0x01;
        }
        reg[AC] += value;
      }
      break;
      // ADC add with carry absolute,X LLHH offset by X
      case(0x7D): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint16_t absolute = high + low + reg[X];
        uint8_t value = memory[absolute];
        if (reg[AC] > 0 && value > 255-reg[AC]) {
          reg[SR] &= 0x01;
        }
        reg[AC] += value;
      }
      break;
      // ADC add with carry absolute,Y LLHH offset by Y
      case(0x79): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint16_t absolute = high + low + reg[Y];
        uint8_t value = memory[absolute];
        if (reg[AC] > 0 && value > 255-reg[AC]) {
          reg[SR] &= 0x01;
        }
        reg[AC] += value;
      }
      break;
      // ADC add with carry (indirect,X) indirect is zeropage
      // (inderect+x indirect+x+1)
      case(0x61): {
        uint8_t indirect = memory[PC++];
        uint8_t low = memory[indirect+reg[X]];
        uint16_t high = memory[indirect+reg[X]+1] << 8;
        uint16_t absolute = high+low;
        uint8_t value = memory[absolute];
        if (reg[AC] > 0 && value > 255-reg[AC]) {
          reg[SR] &= 0x01;
        }
        reg[AC] += value;
      }
      break;
      // ADC add with carry (indirect),Y
      // (indirect indirect+1)+Y
      case(0x71): {
        uint8_t indirect = memory[PC++];
        uint8_t low = memory[indirect];
        uint16_t high = memory[indirect+1] << 8;
        uint16_t absolute = high+low+reg[Y];
        uint8_t value = memory[absolute];
        if (reg[AC] > 0 && value > 255-reg[AC]) {
          reg[SR] &= 0x01;
        }
        reg[AC] += value;
      }
      break;
      // AND and memory with AC immediate
      case(0x29): {
        uint8_t imm = memory[PC++];
        reg[AC] &= imm;
      }
      break;
      // AND and memory with AC zeropage
      case(0x25): {
        uint8_t zeropage = memory[PC++];
        uint8_t value = memory[zeropage];
        reg[AC] &= value;
      }
      break;
      // AND and memory with AC zeropage,X offset by X
      case(0x35): {
        uint8_t zeropage = memory[PC++];
        uint8_t value = memory[zeropage+reg[X]];
        reg[AC] &= value;
      }
      break;
      // AND and memory with AC absolute LLHH
      case(0x2D): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint8_t value = memory[high + low];
        reg[AC] &= value;
      }
      break;
      // AND and memory with AC absolute,X LLHH + X
      case(0x3D): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint8_t value = memory[high + low + reg[X]];
        reg[AC] &= value;
      }
      break;
      // AND and memory with AC absolute,Y LLHH + Y
      case(0x39): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint8_t value = memory[high + low + reg[Y]];
        reg[AC] &= value;
      }
      break;
      // AND and memory with AC (indirect,X) (zeropage+X zeropage+X+1)
      case(0x21): {
        uint8_t indirect = memory[PC++];
        uint8_t low = memory[indirect+reg[X]];
        uint16_t high = memory[indirect+reg[X]+1] << 8;
        uint16_t absolute = high+low;
        uint8_t value = memory[absolute];
        reg[AC] &= value;
      }
      break;
      // AND and memory with AC (indirect),Y (zeropage zeropage+1)+Y
      case(0x31): {
        uint8_t indirect = memory[PC++];
        uint8_t low = memory[indirect];
        uint16_t high = memory[indirect+1] << 8;
        uint16_t absolute = high+low+reg[Y];
        uint8_t value = memory[absolute];
        reg[AC] &= value;
      }
      break;
      // ASL shift left one bit AC
      case(0x0A): {
        reg[AC] = reg[AC] << 1;
      }
      break;
      // ASL shift left one bit zeropage memory
      case(0x06): {
        uint8_t zeropage = memory[PC++];
        reg[zeropage] = reg[zeropage] << 1;
      }
      break;
      // ASL shift left one bit zeropage offset X
      case(0x16): {
        uint8_t zeropage = memory[PC++];
        zeropage += reg[X];
        reg[zeropage] = reg[zeropage] << 1;
      }
      break;
      // ASL shift left one bit absolute
      case(0x0E): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint16_t address = high + low;
        reg[address] = reg[address] << 1;
      }
      break;
      // ASL shift left one bit absolute offset x
      case(0x1E): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint16_t address = high + low + reg[X];
        reg[address] = reg[address] << 1;
      }
      break;
      // BCC branch on carry clear
      case(0x90): {
        int8_t relative = memory[PC++];
        if (reg[SR] & 0x1) break;
        PC += relative;
      }
      break;
      // BCS branch on carry set
      case(0xB0): {
        int8_t relative = memory[PC++];
        if (reg[SR] & 0x1) PC += relative;
      }
      break;
      // BEQ branch on result 0
      case(0xF0): {
        int8_t relative = memory[PC++];
        if (reg[SR] >> 1 & 0x1) {
          PC += relative;
        }
      }
      break;
      //
      // Leaving space for BIT dont understand its use
      //
      // BMI branch on reuslt minus
      case(0x30): {
        int8_t relative = memory[PC++];
        if (reg[SR] >> 7 & 0x1) PC += relative;
      }
      break;
      // BNE branch not equal
      case(0xD0): {
        int8_t relative = memory[PC++];
        if (!(reg[SR] >> 1 & 0x1)) PC += relative;
      }
      break;
      // BPL branch if result positive
      case(0x10): {
        int8_t relative = memory[PC++];
        if (!(reg[SR] >> 7 & 0x1)) PC += relative;
      }
      break;
      //
      // Leaving space for BRK force break
      //
      // BVC branch on overflow clear
      case(0x50): {
        int8_t relative = memory[PC++];
        if (!(reg[SR] >> 6 & 0x1)) PC += relative;
      }
      break;
      // BVS branch on overflow set
      case(0x70): {
        int8_t relative = memory[PC++];
        if (reg[SR] >> 6 & 0x1) PC += relative;
      }
      break;
      // CLC clear carry flag
      case(0x18): {
        reg[SR] &= 0b11111110;
      }
      break;
      // CLD clear decimal mode
      case(0xD8): {
        reg[SR] &= 0b11110111;
      }
      break;
      // CLI clear interrupt disable bit
      case(0x58): {
        reg[SR] &= 0b11111011;
      }
      break;
      // CLV clear overflow flag
      case(0xB8): {
        reg[SR] &= 0b10111111;
      }
      break;
      // CMP compare memory with AC immediate
      case(0xC9): {
        uint8_t imm = memory[PC++];
        int8_t result = reg[AC] - imm;
        updateNZFlags(result, reg);
      }
      break;
      // CMP compare memory with AC zeropage
      case(0xC5): {
        uint8_t zeropage = memory[PC++];
        uint8_t value = memory[zeropage];
        int8_t result = reg[AC] - value;
        updateNZFlags(result, reg);
      }
      break;
      // CMP compare memory with AC zeropage offset by X
      case(0xD5): {
        uint8_t zeropage = memory[PC++];
        zeropage += reg[X];
        uint8_t value = memory[zeropage];
        int8_t result = reg[AC] - value;
        updateNZFlags(result, reg);
      }
      break;
      // CMP compare memory with AC absolute LLHH
      case(0xCD): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint16_t address = low + high;
        uint8_t value = memory[address];
        int8_t result = reg[AC] - value;
        updateNZFlags(result, reg);
      }
      break;
      // CMP compare memory with AC absolute offset by X LLHH
      case(0xDD): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint16_t address = low + high + reg[X];
        uint8_t value = memory[address];
        int8_t result = reg[AC] - value;
        updateNZFlags(result, reg);
      }
      break;
      // CMP compare memory with AC absolute offset by Y LLHH
      case(0xD9): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint16_t address = low + high + reg[Y];
        uint8_t value = memory[address];
        int8_t result = reg[AC] - value;
        updateNZFlags(result, reg);
      }
      break;
      // CMP compare memory with AC (indirect,X)
      case(0xC1): {
        uint8_t indirect = memory[PC++];
        uint8_t low = memory[indirect + reg[X]];
        uint16_t high = memory[indirect + reg[X] + 1];
        uint16_t address = low + high;
        uint8_t value = memory[address];
        int8_t result = reg[AC] - value;
        updateNZFlags(result, reg);
      }
      break;
      // CMP compare memory with AC (indirect),Y
      case(0xD1): {
        uint8_t indirect = memory[PC++];
        uint8_t low = memory[indirect];
        uint16_t high = memory[indirect + 1];
        uint16_t address = low + high + reg[Y];
        uint8_t value = memory[address];
        int8_t result = reg[AC] - value;
        updateNZFlags(result, reg);
      }
      break;
      // CPX compare memory and index X immediate
      case(0xE0): {
        uint8_t imm = memory[PC++];
        int8_t result = reg[X] - imm;
        updateNZFlags(result, reg);
      }
      break;
      // CPX compare memory and index X zeropage
      case(0xE4): {
        uint8_t zeropage = memory[PC++];
        int8_t result = reg[X] - memory[zeropage];
        updateNZFlags(result, reg);
      }
      break;
      // CPX compare memory and index X absolute
      case(0xEC): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint16_t absolute = high+low;
        int8_t result = reg[X] - memory[absolute];
        updateNZFlags(result, reg);
      }
      break;
      // CPY compare memory and index Y immediate
      case(0xC0): {
        uint8_t imm = memory[PC++];
        int8_t result = reg[Y] - imm;
        updateNZFlags(result, reg);
      }
      break;
      // CPY compare memory and index Y zeropage
      case(0xC4): {
        uint8_t zeropage = memory[PC++];
        int8_t result = reg[Y] - memory[zeropage];
        updateNZFlags(result, reg);
      }
      break;
      // CPY compare memory and index Y absolute
      case(0xCC): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint16_t absolute = high+low;
        int8_t result = reg[Y] - memory[absolute];
        updateNZFlags(result, reg);
      }
      break;
      // DEC decrement memory by 1 zeropage
      case(0xC6): {
        uint8_t zeropage = memory[PC++];
        memory[zeropage]--;
      }
      break;
      // DEC decrement memory by 1 zeropage offset by X
      case(0xD6): {
        uint8_t zeropage = memory[PC++];
        memory[zeropage + reg[X]]--;
      }
      break;
      // DEC decrement memory by 1 absolute
      case(0xCE): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint8_t address = high + low;
        memory[address]--;
      }
      break;
      // DEC decrement memory by 1 absolute offset by X
      case(0xDE): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint8_t address = high + low + reg[X];
        memory[address]--;
      }
      break;
      // DEX decrement X by 1 implied
      case(0xCA): {
        reg[X]--;
      }
      break;
      // DEY decrement Y by 1 implied
      case(0x88): {
        reg[Y]--;
      }
      break;
      // EOR xor memory with AC immediate
      case(0x49): {
        uint8_t imm = memory[PC++];
        reg[AC] ^= imm;
      }
      break;
      // EOR xor memory with AC zeropage
      case(0x45): {
        uint8_t zeropage = memory[PC++];
        uint8_t value = memory[zeropage];
        reg[AC] ^= value;
      }
      break;
      // EOR or memory with AC zeropage,X offset by X
      case(0x55): {
        uint8_t zeropage = memory[PC++];
        uint8_t value = memory[zeropage+reg[X]];
        reg[AC] ^= value;
      }
      break;
      // EOR xor memory with AC absolute LLHH
      case(0x4D): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint8_t value = memory[high + low];
        reg[AC] ^= value;
      }
      break;
      // EOR xor memory with AC absolute,X LLHH + X
      case(0x5D): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint8_t value = memory[high + low + reg[X]];
        reg[AC] ^= value;
      }
      break;
      // EOR xor memory with AC absolute,Y LLHH + Y
      case(0x59): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint8_t value = memory[high + low + reg[Y]];
        reg[AC] ^= value;
      }
      break;
      // EOR xor memory with AC (indirect,X) (zeropage+X zeropage+X+1)
      case(0x41): {
        uint8_t indirect = memory[PC++];
        uint8_t low = memory[indirect+reg[X]];
        uint16_t high = memory[indirect+reg[X]+1] << 8;
        uint16_t absolute = high+low;
        uint8_t value = memory[absolute];
        reg[AC] ^= value;
      }
      break;
      // EOR xor memory with AC (indirect),Y (zeropage zeropage+1)+Y
      case(0x51): {
        uint8_t indirect = memory[PC++];
        uint8_t low = memory[indirect];
        uint16_t high = memory[indirect+1] << 8;
        uint16_t absolute = high+low+reg[Y];
        uint8_t value = memory[absolute];
        reg[AC] ^= value;
      }
      break;
      // INC increment memory by 1 zeropage
      case(0xE6): {
        uint8_t zeropage = memory[PC++];
        memory[zeropage]++;
      }
      break;
      // INC increment memory by 1 zeropage offset by X
      case(0xF6): {
        uint8_t zeropage = memory[PC++];
        memory[zeropage + reg[X]]++;
      }
      break;
      // INC increment memory by 1 absolute
      case(0xEE): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint8_t address = high + low;
        memory[address]++;
      }
      break;
      // INC increment memory by 1 absolute offset by X
      case(0xFE): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint8_t address = high + low + reg[X];
        memory[address]++;
      }
      break;
      // INX increment X by 1 implied
      case(0xE8): {
        reg[X]++;
      }
      break;
      // INY increment Y by 1 implied
      case(0xC8): {
        reg[Y]++;
      }
      break;
      // JMP jump to new location absolute
      case(0x4C): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        PC = high + low;
      }
      break;
      // JMP jump to new location indirect
      case(0x6C): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint16_t address = high + low;
        low = memory[address];
        high = memory[address+1];
        PC = low + high;
      }
      break;
      // JSR jump subroutine (PC + 2)
      // PC is pushed onto the stack as the 3rd byte of this instruction
      // later it is popped off, incremented by 1, and thats the new PC
      case(0x20): {
        uint8_t ll = memory[PC++];
        uint16_t hh = memory[PC++] << 8;
        uint16_t address = ll + hh;
        uint16_t tmpSP = SP;
        uint8_t low = tmpSP & 0xFF;
        uint8_t high = tmpSP >> 4;
        memory[0x0100 + reg[SP]--] = high;
        memory[0x0100 + reg[SP]--] = low;
        PC = address;
      }
      break;
      // LDA load accumulator with memory immediate
      case (0xA9): {
        reg[AC] = memory[PC++];
        updateNZFlags(reg[AC], reg);
      }
      break;
      // LDA load accumulator with memory zeropage
      case (0xA5): {
        uint8_t zeropage = memory[PC++];
        reg[AC] = memory[zeropage];
        updateNZFlags(reg[AC], reg);
      }
      break;
      // LDA load accumulator with memory zeropage,X
      case (0xB5): {
        uint8_t zeropage = memory[PC++];
        reg[AC] = memory[zeropage+reg[X]];
        updateNZFlags(reg[AC], reg);
      }
      break;
      // LDA load accumulator with memory absolute
      case (0xAD): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] >> 8;
        reg[AC] = memory[high + low];
        updateNZFlags(reg[AC], reg);
      }
      break;
      // LDA load accumulator with memory absolute,X offset by X
      case (0xBD): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] >> 8;
        reg[AC] = memory[high + low + reg[X]];
        updateNZFlags(reg[AC], reg);
      }
      break;
      // LDA load accumulator with memory absolute,Y offset by Y
      case (0xB9): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] >> 8;
        reg[AC] = memory[high + low + reg[Y]];
        updateNZFlags(reg[AC], reg);
      }
      break;
      // LDA load accumulator with memory (indirect,X)
      case (0xA1): {
        uint8_t indirect = memory[PC++];
        uint8_t low = memory[indirect + reg[X]];
        uint16_t high = memory[indirect + reg[X] + 1] << 8;
        reg[AC] = memory[high + low];
        updateNZFlags(reg[AC], reg);
      }
      break;
      // LDA load accumulator with memory (indirect),Y
      case (0xB1): {
        uint8_t indirect = memory[PC++];
        uint8_t low = memory[indirect];
        uint16_t high = memory[indirect + 1] << 8;
        reg[AC] = memory[high + low + reg[Y]];
        updateNZFlags(reg[AC], reg);
      }
      break;
      // LDX load X with memory immediate
      case(0xA2): {
        uint8_t imm = memory[PC++];
        reg[X] = imm;
        updateNZFlags(reg[X], reg);
      }
      break;
      // LDX load X with memory zeropage
      case(0xA6): {
        uint8_t zeropage = memory[PC++];
        uint8_t value = memory[zeropage];
        reg[X] = value;
        updateNZFlags(reg[X], reg);
      }
      break;
      // LDX load X with memory,Y offset by Y
      case(0xB6): {
        uint8_t zeropage = memory[PC++];
        uint8_t value = memory[zeropage + reg[Y]];
        reg[X] = value;
        updateNZFlags(reg[X], reg);
      }
      break;
      // LDX load X with memory absolute
      case(0xAE): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint8_t value = memory[low + high];
        reg[X] = value;
        updateNZFlags(reg[X], reg);
      }
      break;
      // LDX load X with memory absolute,Y offset by Y
      case(0xBE): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint8_t value = memory[low + high + reg[Y]];
        reg[X] = value;
        updateNZFlags(reg[X], reg);
      }
      break;
      // LDY load Y with memory immediate
      case(0xA0): {
        uint8_t imm = memory[PC++];
        reg[Y] = imm;
        updateNZFlags(reg[Y], reg);
      }
      break;
      // LDY load Y with memory zeropage
      case(0xA4): {
        uint8_t zeropage = memory[PC++];
        uint8_t value = memory[zeropage];
        reg[Y] = value;
        updateNZFlags(reg[Y], reg);
      }
      break;
      // LDY load Y with memory,X offset by X
      case(0xB4): {
        uint8_t zeropage = memory[PC++];
        uint8_t value = memory[zeropage + reg[X]];
        reg[Y] = value;
        updateNZFlags(reg[Y], reg);
      }
      break;
      // LDY load Y with memory absolute
      case(0xAC): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint8_t value = memory[low + high];
        reg[Y] = value;
        updateNZFlags(reg[Y], reg);
      }
      break;
      // LDY load Y with memory absolute,X offset by X
      case(0xBC): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint8_t value = memory[low + high + reg[X]];
        reg[Y] = value;
        updateNZFlags(reg[Y], reg);
      }
      break;
      // LSR shift right one bit AC
      case(0x4A): {
        reg[AC] = reg[AC] >> 1;
      }
      break;
      // LSR shift right one bit zeropage memory
      case(0x46): {
        uint8_t zeropage = memory[PC++];
        reg[zeropage] = reg[zeropage] >> 1;
      }
      break;
      // LSR shift right one bit zeropage offset X
      case(0x56): {
        uint8_t zeropage = memory[PC++];
        zeropage += reg[X];
        reg[zeropage] = reg[zeropage] >> 1;
      }
      break;
      // LSR shift right one bit absolute
      case(0x4E): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint16_t address = high + low;
        reg[address] = reg[address] >> 1;
      }
      break;
      // LSR shift right one bit absolute offset x
      case(0x5E): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint16_t address = high + low + reg[X];
        reg[address] = reg[address] >> 1;
      }
      break;
      // NOP
      case(0xEA): {
        break;
      }
      // ORA OR memory with AC immediate
      case(0x09): {
        uint8_t imm = memory[PC++];
        reg[AC] |= imm;
        updateNZFlags(reg[AC], reg);
      }
      break;
      // ORA OR memory with AC zeropage
      case(0x05): {
        uint8_t zeropage = memory[PC++];
        reg[AC] |= memory[zeropage];
        updateNZFlags(reg[AC], reg);
      }
      break;
      // ORA OR memory with AC zeropage,X offset by X
      case(0x15): {
        uint8_t zeropage = memory[PC++];
        reg[AC] |= memory[zeropage + reg[X]];
        updateNZFlags(reg[AC], reg);
      }
      break;
      // ORA OR memory with AC absolute
      case(0x0D): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        reg[AC] |= memory[low + high];
        updateNZFlags(reg[AC], reg);
      }
      break;
      // ORA OR memory with AC absolute,X offset by X
      case(0x1D): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        reg[AC] |= memory[low + high + reg[X]];
        updateNZFlags(reg[AC], reg);
      }
      break;
      // ORA OR memory with AC absolute,Y offset by Y
      case(0x19): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        reg[AC] |= memory[low + high + reg[Y]];
        updateNZFlags(reg[AC], reg);
      }
      break;
      // ORA OR memory with AC (indirect,X)
      case(0x01): {
        uint8_t indirect = memory[PC++];
        uint8_t low = memory[indirect + reg[X]];
        uint16_t high = memory[indirect + reg[X] + 1] << 8;
        reg[AC] |= memory[low + high];
        updateNZFlags(reg[AC], reg);
      }
      break;
      // ORA OR memory with AC (indirect),Y
      case(0x11): {
        uint8_t indirect = memory[PC++];
        uint8_t low = memory[indirect];
        uint16_t high = memory[indirect + 1] << 8;
        reg[AC] |= memory[low + high + reg[Y]];
        updateNZFlags(reg[AC], reg);
      }
      break;
      // PHA push AC onto stack
      case(0x48): {
        memory[0x100 + reg[SP]--] = reg[AC];
      }
      break;
      // PHP push SR onto stack, break = 1, ignored = 1
      case(0x08): {
        uint8_t tmpSR = SR;
        tmpSR |= 0b00110000;
        memory[0x100 + reg[SP]--] = reg[SR];
      }
      break;
      // PLA pull AC from stack
      case(0x68): {
        reg[AC] = memory[0x100 + reg[SP]++];
        updateNZFlags(reg[AC], reg);
      }
      break;
      // PLP pull SR from stack, break = ignored, ignored = ignored
      case(0x28): {
        uint8_t tmpSR = memory[0x100 + reg[SP]++];
        tmpSR &= 0b11001111;
        reg[SR] = tmpSR;
      }
      break;
      // ROL rotate one bit left AC
      case(0x2A): {
        reg[AC] = rotl(reg[AC], 1);
        updateNZFlags(reg[AC], reg);
      }
      break;
      // ROL rotate one bit left zeropage
      case(0x26): {
        uint8_t zeropage = memory[PC++];
        memory[zeropage] = rotl(memory[zeropage], 1);
        updateNZFlags(memory[zeropage], reg);
      }
      break;
      // ROL rotate one bit left zeropage,X offset by X
      case(0x36): {
        uint8_t zeropage = memory[PC++];
        zeropage += reg[X];
        memory[zeropage] = rotl(memory[zeropage], 1);
        updateNZFlags(memory[zeropage], reg);
      }
      break;
      // ROL rotate one bit left absolute
      case(0x2E): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        memory[low + high] = rotl(memory[low + high], 1);
        updateNZFlags(memory[low+high], reg);
      }
      break;
      // ROL rotate one bit left absolute,X offset by X
      case(0x3E): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        memory[low + high + reg[X]] = rotl(memory[low + high + reg[X]], 1);
        updateNZFlags(memory[low+high+reg[X]], reg);
      }
      break;
      // ROR rotate one bit right AC
      case(0x6A): {
        reg[AC] = rotr(reg[AC], 1);
        updateNZFlags(reg[AC], reg);
      }
      break;
      // ROR rotate one bit right zeropage
      case(0x66): {
        uint8_t zeropage = memory[PC++];
        memory[zeropage] = rotr(memory[zeropage], 1);
        updateNZFlags(memory[zeropage], reg);
      }
      break;
      // ROR rotate one bit right zeropage,X offset by X
      case(0x76): {
        uint8_t zeropage = memory[PC++];
        zeropage += reg[X];
        memory[zeropage] = rotr(memory[zeropage], 1);
        updateNZFlags(memory[zeropage], reg);
      }
      break;
      // ROR rotate one bit right absolute
      case(0x6E): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        memory[low + high] = rotr(memory[low + high], 1);
        updateNZFlags(memory[low+high], reg);
      }
      break;
      // ROR rotate one bit right absolute,X offset by X
      case(0x7E): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        memory[low + high + reg[X]] = rotr(memory[low + high + reg[X]], 1);
        updateNZFlags(memory[low+high+reg[X]], reg);
      }
      break;
      // RTI return from interrupt
      // pop SR, pop PC
      case(0x40): {
        uint8_t tmpSR = memory[0x100 + reg[SP]++];
        tmpSR &= 0b11001111;
        reg[SR] = tmpSR;
        uint8_t PCLow = memory[0x100 + reg[SP]++];
        uint16_t PCHigh = memory[0x100 + reg[SP]++] << 8;
        PC = PCHigh + PCLow;
      }
      break;
      // RTS return from subroutine
      // Pop PC, add 1, set PC
      case(0x60): {
        uint8_t PCLow = memory[0x100 + reg[SP]++];
        uint16_t PCHigh = memory[0x100 + reg[SP]++];
        PC = PCHigh + PCLow + 1;
      }
      break;
      // SBC subtract memory from AC with borrow immediate
      case(0xE9): {
        uint8_t imm = memory[PC++];
        if (reg[AC] - imm > reg[AC]) {
          reg[SR] |= 0b10000001;
        }
        reg[AC] -= imm;
      }
      break;
      // SBC subtract memory from AC with borrow zeropage
      case(0xE5): {
        uint8_t zeropage = memory[PC++];
        uint8_t value = memory[zeropage];
        if (reg[AC] - value > reg[AC]) {
          reg[SR] |= 0b10000001;
        }
        reg[AC] -= value;
      }
      break;
      // SBC subtract memory from AC with borrow zeropage,X offset by X
      case(0xF5): {
        uint8_t zeropage = memory[PC++];
        uint8_t value = memory[zeropage + reg[X]];
        if (reg[AC] - value > reg[AC]) {
          reg[SR] |= 0b10000001;
        }
        reg[AC] -= value;
      }
      break;
      // SBC subtract memory from AC with borrow absolute
      case(0xED): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint8_t value = memory[low + high];
        if (reg[AC] - value > reg[AC]) {
          reg[SR] |= 0b10000001;
        }
        reg[AC] -= value;
      }
      break;
      // SBC subtract memory from AC with borrow absolute,X offset by X
      case(0xFD): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint8_t value = memory[low + high + reg[X]];
        if (reg[AC] - value > reg[AC]) {
          reg[SR] |= 0b10000001;
        }
        reg[AC] -= value;
      }
      break;
      // SBC subtract memory from AC with borrow absolute,Y offset by Y
      case(0xF9): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        uint8_t value = memory[low + high + reg[Y]];
        if (reg[AC] - value > reg[AC]) {
          reg[SR] |= 0b10000001;
        }
        reg[AC] -= value;
      }
      break;
      // SBC subtract memory from AC with borrow (indirect,X)
      case(0xE1): {
        uint8_t indirect = memory[PC++];
        uint8_t low = memory[indirect + reg[X]];
        uint16_t high = memory[indirect + reg[X] + 1] << 8;
        uint8_t value = memory[low + high];
        if (reg[AC] - value > reg[AC]) {
          reg[SR] |= 0b10000001;
        }
        reg[AC] -= value;
      }
      break;
      // SBC subtract memory from AC with borrow (indirect),Y
      case(0xF1): {
        uint8_t indirect = memory[PC++];
        uint8_t low = memory[indirect];
        uint16_t high = memory[indirect + 1] << 8;
        uint8_t value = memory[low + high + reg[Y]];
        if (reg[AC] - value > reg[AC]) {
          reg[SR] |= 0b10000001;
        }
        reg[AC] -= value;
      }
      break;
      // SEC set carry flag
      case(0x3B): {
        reg[SR] |= 0x1;
      }
      break;
      // SED set decimal flag
      case(0xF8): {
        reg[SR] |= (1 << 3);
      }
      break;
      // SEI set interrupt disable status
      case(0x7B): {
        reg[SR] |= (1 << 2);
      }
      break;
      // STA store AC in memory zeropage
      case(0x85): {
        uint8_t zeropage = memory[PC++];
        memory[zeropage] = reg[AC];
      }
      break;
      // STA store AC in memory zeropage,X offset by X
      case(0x95): {
        uint8_t zeropage = memory[PC++];
        memory[zeropage+reg[X]] = reg[AC];
      }
      break;
      // STA store AC in memory absolute
      case(0x8D): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        memory[low + high] = reg[AC];
      }
      break;
      // STA store AC in memory absolute,X
      case(0x9D): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        memory[low + high + reg[X]] = reg[AC];
      }
      break;
      // STA store AC in memory absolute,Y
      case(0x99): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        memory[low + high + reg[Y]] = reg[AC];
      }
      break;
      // STA store AC in memory (indirect,X)
      case(0x81): {
        uint8_t indirect = memory[PC++];
        uint8_t low = memory[indirect + reg[X]];
        uint16_t high = memory[indirect + reg[X] + 1] << 8;
        memory[low + high] = reg[AC];
      }
      break;
      // STA store AC in memory (indirect),Y
      case(0x91): {
        uint8_t indirect = memory[PC++];
        uint8_t low = memory[indirect];
        uint16_t high = memory[indirect + 1] << 8;
        memory[low + high + reg[Y]] = reg[AC];
      }
      break;
      // STX store x in memory zeropage
      case(0x86): {
        uint8_t zeropage = memory[PC++];
        memory[zeropage] = reg[X];
      }
      break;
      // STX store x in memory zeropage,Y offset by Y
      case(0x96): {
        uint8_t zeropage = memory[PC++];
        memory[zeropage + reg[Y]] = reg[X];
      }
      break;
      // STX store x in memory absolute
      case(0x8E): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        memory[low + high] = reg[X];
      }
      break;
      // STY store y in memory zeropage
      case(0x84): {
        uint8_t zeropage = memory[PC++];
        memory[zeropage] = reg[Y];
      }
      break;
      // STY store y in memory zeropage,X offset by X
      case(0x94): {
        uint8_t zeropage = memory[PC++];
        memory[zeropage + reg[X]] = reg[Y];
      }
      break;
      // STY store y in memory absolute
      case(0x8C): {
        uint8_t low = memory[PC++];
        uint16_t high = memory[PC++] << 8;
        memory[low + high] = reg[Y];
      }
      break;
      // TAX transfer AC to X
      case(0xAA): {
        reg[X] = reg[AC];
        updateNZFlags(reg[X], reg);
      }
      break;
      // TAY transfer AC to Y
      case(0xA8): {
        reg[X] = reg[AC];
        updateNZFlags(reg[Y], reg);
      }
      break;
      // TSX transfer SP to Y
      case(0xBA): {
        reg[X] = reg[SP];
        updateNZFlags(reg[X], reg);
      }
      break;
      // TXA transfer X to AC
      case(0x8A): {
        reg[AC] = reg[X];
        updateNZFlags(reg[AC], reg);
      }
      break;
      // TXS transfer X to SP
      case(0x9A): {
        reg[SP] = reg[X];
      }
      break;
      // TYA transfer Y to AC
      case(0x9B): {
        reg[AC] = reg[Y];
        updateNZFlags(reg[AC], reg);
      }
      break;
      default:
        running = false;
        // break;
    }
  }
  regDump(reg);
  cout<<"ZP = ";
  printMemory(memory, 0x0000);
  cout<<"ST = ";
  printMemory(memory, 0x01F0);
  cout<<"PG = ";
  printMemory(memory, 0x0600);
  printMemory(memory, 0x0610);
  printMemory(memory, 0x0620);
  ppuLoop(memory);
  cout<<"PC = "<<hex<<(int)PC<<endl;
  return 0;
}

//https://www.masswerk.at/6502/6502_instruction_set.html#LDA
//https://www.masswerk.at/6502/assembler.html
