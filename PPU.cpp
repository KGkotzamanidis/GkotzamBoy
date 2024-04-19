#include "PPU.h"

PPU::PPU(LR35902_Interrupt& Interrupts, WRam& Wram) : Interrupts(&Interrupts), Wram(&Wram) {

}

PPU::~PPU() {

}

void PPU::sendData(u16_t address, u8_t data) {

}

u8_t PPU::recieveData(u16_t address) {

}