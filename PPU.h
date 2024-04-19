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