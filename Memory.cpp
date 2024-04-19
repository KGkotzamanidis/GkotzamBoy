/*
 * Copyright (C) <2024>  <KGkotzamanidis>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "Memory.h"

Memory::Memory(LR35902_Interrupt& Interrupts, LR35902_Timer& Timers, WRam& Wram) :Interrupts(&Interrupts), Timers(&Timers), Wram(&Wram) {
	isloadBios = false;
	debug_CPU = false;
}
Memory::~Memory() {

}

u8_t Memory::readByte(u16_t address) {
	u8_t data = 0x00;
	if (debug_CPU) {
		/*
		* While this is true the program return address data of this vector
		*/
		if (address >= 0x0000 && address <= 0xFFFF) {
			data = ptrData[address];
		}
	}
	else {
		if (address >= 0x0000 && address <= 0x7FFF) {
			if (isloadBios && address <= 0x100) {
				data = ptrBIOS[address];
			}
		}
	}
	
	return data;
}
u16_t Memory::readWord(u16_t address) {
	u16_t data;
	data = readByte(address);
	data |= readByte(address + 1) << 8;
	return data;
}

void Memory::writeByte(u16_t address, u8_t data) {
	if (debug_CPU) {
		/*
		* While this is true the program write address data of this vector
		*/
		if (address >= 0x0000 && address <= 0xFFFF) {
			ptrData[address] = data;
		}
	}
	else {

	}
}
void Memory::writeWord(u16_t address, u16_t data) {
	writeByte(address, data & 0xFF);
	writeByte(address + 1, (data >> 8) & 0xFF);
}

void Memory::loadBios(string filepath) {
	ifstream readBios(filepath, ios::in | ios::binary | ios::ate);
	if (readBios.is_open()) {
		readBios.seekg(0, ios::end);
		sizeBios = readBios.tellg();
		readBios.seekg(0, ios::beg);
		ptrBIOS.resize(sizeBios);
		readBios.read(reinterpret_cast<char*>(ptrBIOS.data()), sizeBios);
		readBios.close();
		isloadBios = true;
	}
	else {
		isloadBios = false;
		sizeBios = 0;
	}
}