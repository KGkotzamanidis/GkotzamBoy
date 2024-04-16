#include "WRam.h"

WRam::WRam() {

}

WRam::~WRam() {

}

void WRam::sendData(u16_t address, u8_t data) {
	if (address >= 0xC000 && address <= 0xFDFF) {
		u16_t wramAddress = (address & 0x1FFF);
		if (wramAddress >= 0x1000) {
			wramAddress &= 0xFFF;
			wramAddress |= WRamBank << 12;
		}
		RAM[wramAddress] = data;
	}
	else if (address == 0xFF70) {
		WRamBank = data & 0x7;
		if (WRamBank == 0) {
			WRamBank = 1;
		}
	}
}

u8_t WRam::recieveData(u16_t address) {
	u8_t returnValue = 0xFF;
	if (address >= 0xC000 && address <= 0xFDFF) {
		u16_t wramAddress = (address & 0x1FFF);
		if (wramAddress >= 0x1000) {
			wramAddress &= 0xFFF;
			wramAddress |= WRamBank << 12;
		}
		returnValue = RAM[wramAddress];
	}
	else if (address == 0xFF70) {
		returnValue = WRamBank;
	}
	return returnValue;
}