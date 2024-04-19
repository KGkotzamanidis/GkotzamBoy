#pragma once

#include "LR35902.h"
#include "Symbols.h"
#include "WRam.h"
class PPU {
public:
	PPU(LR35902_Interrupt&Interrupts,WRam&Wram);
	~PPU();

	void sendData(u16_t address, u8_t data);
	u8_t recieveData(u16_t address);

private:
	LR35902_Interrupt* Interrupts;
	WRam* Wram;
protected:
};