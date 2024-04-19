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
#include "WRam.h"

void loadROM(Memory& mem, string file_path);
streampos sizeFile;

int main(int arga, char* argv[]) {

	LR35902_Interrupt Interrupts;
	LR35902_Timer Timers(Interrupts);
	WRam Wram;
	Memory mainmemory(Interrupts, Timers, Wram);
	LR35902 CPU(mainmemory);

	mainmemory.loadBios("D:\\GkotzamBoy\\Bios.bin");

	if (mainmemory.isloadBios) {
		cout << "Bios Loaded!" << endl;
	}

	CPU.reset();

	while (true) {
		
		CPU.run();
		u8_t opcode = CPU.returnOPCODE();

		int x = CPU.getlastcyclecount();
	}

	return 0;
}

void loadROM(Memory& mem, string file_path) {
	ifstream readFile(file_path, ios::in | ios::binary | ios::ate);
	if (readFile.is_open()) {
		readFile.seekg(0, ios::end);
		sizeFile = readFile.tellg();
		readFile.seekg(0, ios::beg);
		mem.ptrData.resize(sizeFile);
		readFile.read(reinterpret_cast<char*>(mem.ptrData.data()), sizeFile);
		readFile.close();
	}
	else {
		sizeFile = 0;
	}
}