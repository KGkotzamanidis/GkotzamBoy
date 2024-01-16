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
	mainmemory.ptrData.push_back(0x00); //1-NOP
	mainmemory.ptrData.push_back(0x06); //2-LD B,n8
	mainmemory.ptrData.push_back(0x02); //3-Read addressh for data
	mainmemory.ptrData.push_back(0x00); //4-NOP
	mainmemory.ptrData.push_back(0x04); //5-INC B = 3
	mainmemory.ptrData.push_back(0x04); //6-INC B = 4
	mainmemory.ptrData.push_back(0x05); //7-DEC B = 3
	mainmemory.ptrData.push_back(0x00); //8-NOP
	mainmemory.ptrData.push_back(0x06); //9-LD B,n8
	mainmemory.ptrData.push_back(0x45); //10-Read addressh for data
	mainmemory.ptrData.push_back(0x00); //11-NOP

	CPU.B = 0x00;
	CPU.PC = 0x00;
	CPU.lastcyclecount = 0;
	CPU.reset();

	while (true) {
		
		CPU.run();
	}

	return 0;
}