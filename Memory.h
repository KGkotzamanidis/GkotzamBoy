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
#pragma once

#include "LR35902.h"
#include "WRam.h"
#include "Symbols.h"
class LR35902_Interrupt;
class LR35902_Timer;
class Memory {
public:
	Memory(LR35902_Interrupt& Interrupts, LR35902_Timer& Timers, WRam& Wram);
	~Memory();

	vector<u8_t>ptrData;
	bool debug_CPU = false;

	u8_t readByte(u16_t address);
	u16_t readWord(u16_t address);

	void writeByte(u16_t address, u8_t data);
	void writeWord(u16_t address, u16_t data);

	void loadBios(string filepath);
	bool isloadBios;

private:
	LR35902_Interrupt* Interrupts;
	LR35902_Timer* Timers;
	WRam* Wram;

	vector<u8_t>ptrBIOS;
	streampos sizeBios;
};