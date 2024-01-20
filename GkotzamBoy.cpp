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
#include "LR35902.h"

int main(int arga, char* argv[]) {
	LR35902_Interrupt Interrupts;
	LR35902_Timer Timers(Interrupts);
	Memory mainmemory(Interrupts, Timers);
	LR35902 CPU(mainmemory);

	mainmemory.loadBios("D:\\GkotzamBoy\\Bios.bin");
	
	/* Test Program */
	mainmemory.ptrData.clear();
	mainmemory.ptrData.push_back(0x00); // NOP
	mainmemory.ptrData.push_back(0x3E); // LD A n8
	mainmemory.ptrData.push_back(0x04); // n8 = 0x04
	mainmemory.ptrData.push_back(0xCB); //Prefix Instruction
	mainmemory.ptrData.push_back(0xFF); //SET 7,A
	mainmemory.ptrData.push_back(0x00); // NOP
	mainmemory.ptrData.push_back(0x00); // NOP
	mainmemory.ptrData.push_back(0x04); // INC B
	mainmemory.ptrData.push_back(0x00); // NOP

	CPU.reset();

	while (true) {
		
		CPU.run();
		u8_t opcode = CPU.returnOPCODE();
	}

	return 0;
}