#include "LR35902.h"

#pragma region Interrupt
LR35902_Interrupt::LR35902_Interrupt() {

}
LR35902_Interrupt::~LR35902_Interrupt() {

}
#pragma endregion Interrupt

#pragma region Timer

LR35902_Timer::LR35902_Timer(LR35902_Interrupt& Interrupts):Interrupts(&Interrupts) {

}
LR35902_Timer::~LR35902_Timer() {

}
void LR35902_Timer::sendData(u16_t address, u8_t data) {
	switch (address) {
		/* DIV Address */
	case 0xFF04:
		DIV = 0;
		break;
		/* TIMA Address */
	case 0xFF05:
		TIMA = data;
		break;
		/* TMA Address */
	case 0xFF06:
		TMA = data;
		break;
		/* TAC Address */
	case 0xFF07:
		TAC = data;
		break;
	default:
		break;
	}
}
u8_t LR35902_Timer::recieveData(u16_t address) {
	u8_t data = 0x00;

	switch (address) {
		/* DIV Address */
	case 0xFF04:
		data = DIV;
		break;
		/* TIMA Address */
	case 0xFF05:
		data = TIMA;
		break;
		/* TMA Address */
	case 0xFF06:
		data = TMA;
		break;
		/* TAC Address */
	case 0xFF07:
		data = TAC;
		break;
	default:
		break;
	}
	return data;
}
void LR35902_Timer::updateTimers(int lastCycleCount) {
	DIVcyclecount += lastCycleCount;
	
	if (DIVcyclecount >= 256) {
		DIVcyclecount -= 256;
		DIV++;
	}

	if ((TAC & 0x4) != 0) {
		TIMAcyclecount += lastCycleCount;
		int clockRateNum = clockRate(TAC & 0x3);

		while (TIMAcyclecount >= clockRateNum) {
			TIMAcyclecount -= clockRateNum;
			TIMA++;

			if (TIMA == 0) {
				TIMA = TMA;
				Interrupts->IF = Interrupts->IF | (0xE0 | 0x04);
			}
		}
	}
}
int LR35902_Timer::clockRate(int code) {
	int value = 0;
	switch (code) {
	case 0:
		value = 1024;
		break;
	case 1:
		value = 16;
		break;
	case 2:
		value = 64;
		break;
	case 3:
		value = 256;
		break;
	default:
		break;
	}
	return value;
}
#pragma endregion Timer

#pragma region CPU
#define Flag_Z 0x80
#define Flag_N 0x40
#define Flag_H 0x20
#define Flag_C 0x10

#define Flag_Z_reset 0x7F
#define Flag_N_reset 0xBF
#define Flag_H_reset 0xDF
#define Flag_C_reset 0xE0

LR35902::LR35902(Memory& mem) :mem(&mem) {
	IME, IMEhold, ishalt, EIDIFlag, DoubleSpeed = false;
	A = B = C = D = E = F = H = L = 0x00;
	PC = SP = 0x0000;
	lastcyclecount = 0;
}
LR35902::~LR35902() {

}

const int LR35902::instructionCount[] = {
	4,12,8,8,4,4,8,4,20,8,8,8,4,4,8,4,
	4,12,8,8,4,4,8,4,12,8,8,8,4,4,8,4,
	8,12,8,8,4,4,8,4,8,8,8,8,4,4,8,4,
	8,12,8,8,12,12,12,4,8,8,8,8,4,4,8,4,
	4,4,4,4,4,4,8,4,4,4,4,4,4,4,8,4,
	4,4,4,4,4,4,8,4,4,4,4,4,4,4,8,4,
	4,4,4,4,4,4,8,4,4,4,4,4,4,4,8,4,
	8,8,8,8,8,8,4,8,4,4,4,4,4,4,8,4,
	4,4,4,4,4,4,8,4,4,4,4,4,4,4,8,4,
	4,4,4,4,4,4,8,4,4,4,4,4,4,4,8,4,
	4,4,4,4,4,4,8,4,4,4,4,4,4,4,8,4,
	4,4,4,4,4,4,8,4,4,4,4,4,4,4,8,4,
	8,12,12,16,12,16,8,16,8,16,12,4,12,24,8,16,
	8,12,12,0,12,16,8,16,8,16,12,0,12,0,8,16,
	12,12,8,0,0,16,8,16,16,4,16,0,0,0,8,16,
	12,12,8,4,0,16,8,16,12,8,16,4,0,0,8,16,
};
const int LR35902::prefixedinstructionCount[] = {
	8,8,8,8,8,8,16,8,8,8,8,8,8,8,16,8,
	8,8,8,8,8,8,16,8,8,8,8,8,8,8,16,8,
	8,8,8,8,8,8,16,8,8,8,8,8,8,8,16,8,
	8,8,8,8,8,8,16,8,8,8,8,8,8,8,16,8,
	8,8,8,8,8,8,12,8,8,8,8,8,8,8,12,8,
	8,8,8,8,8,8,12,8,8,8,8,8,8,8,12,8,
	8,8,8,8,8,8,12,8,8,8,8,8,8,8,12,8,
	8,8,8,8,8,8,12,8,8,8,8,8,8,8,12,8,
	8,8,8,8,8,8,16,8,8,8,8,8,8,8,16,8,
	8,8,8,8,8,8,16,8,8,8,8,8,8,8,16,8,
	8,8,8,8,8,8,16,8,8,8,8,8,8,8,16,8,
	8,8,8,8,8,8,16,8,8,8,8,8,8,8,16,8,
	8,8,8,8,8,8,16,8,8,8,8,8,8,8,16,8,
	8,8,8,8,8,8,16,8,8,8,8,8,8,8,16,8,
	8,8,8,8,8,8,16,8,8,8,8,8,8,8,16,8,
	8,8,8,8,8,8,16,8,8,8,8,8,8,8,16,8,
};

void LR35902::reset() {
	A = B = C = D = E = F = H = L = 0x00;
	PC = SP = 0x0000;
}
void LR35902::run() {
	executeInstruction(mem->readByte(PC));
}

int LR35902::getlastcyclecount() {
	return lastcyclecount;
}

bool LR35902::getDoubleSpeed() {
	return DoubleSpeed;
}

#pragma region CPU execute Instruction
/* For more information about the SM83 Instruction read this
   https://gbdev.io/gb-opcodes/optables/
*/
void LR35902::executeInstruction(u8_t opcode) {
	PC++;
	lastcyclecount = instructionCount[opcode];

	switch (opcode) {
	case 0x00:
		nop();
		break;
	case 0x01:
		setBC(mem->readWord(PC));
		PC += 2;
		break;
	case 0x02:
		setBC(mem->readByte(A));
		break;
	case 0x03:
		setBC(BC() + 1);
		break;
	case 0x04:
		B = alu8bitINC(B);
		break;
	case 0x05:
		B = alu8bitDEC(B);
		break;
	case 0x06:
		B = mem->readByte(PC);
		PC++;
		break;
	case 0x07:
		A = RLC(A);
		break;
	case 0x08:
		mem->writeWord(mem->readWord(PC), SP);
		PC += 2;
		break;
	case 0x09:
		alu16bitADD(BC());
		break;
	case 0x0A:
		A = mem->readByte(BC());
		break;
	case 0x0B:
		setBC(BC() - 1);
		break;
	case 0x0C:
		C = alu8bitINC(C);
		break;
	case 0x0D:
		C = alu8bitDEC(C);
		break;
	case 0x0E:
		C = mem->readByte(PC);
		PC++;
		break;
	case 0x0F:
		A = RRC(A);
		break;
	case 0x10:
		stop();
		PC++;
		break;
	case 0x11:
		setDE(mem->readWord(PC));
		PC += 2;
		break;
	case 0x12:
		setDE(mem->readByte(A));
		break;
	case 0x13:
		setDE(DE() + 1);
		break;
	case 0x14:
		D = alu8bitINC(D);
		break;
	case 0x15:
		D = alu8bitDEC(D);
		break;
	case 0x16:
		D = mem->readByte(PC);
		PC++;
		break;
	case 0x17:
		A = RL(A);
		break;
	case 0x18:
		jr(true);
		PC++;
		break;
	case 0x19:
		alu16bitADD(DE());
		break;
	case 0x1A:
		A = mem->readByte(DE());
		break;
	case 0x1B:
		setDE(DE() - 1);
		break;
	case 0x1C:
		E = alu8bitINC(E);
		break;
	case 0x1D:
		E = alu8bitDEC(E);
		break;
	case 0x1E:
		E = mem->readByte(PC);
		PC++;
		break;
	case 0x1F:
		A = RR(A);
		break;
	case 0x20:
		jr(!getFlagZ());
		PC++;
		break;
	case 0x21:
		setHL(mem->readWord(PC));
		PC += 2;
		break;
	case 0x22:
		mem->writeByte(HL(), A);
		setHL(HL() + 1);
		break;
	case 0x23:
		setHL(HL() + 1);
		break;
	case 0x24:
		H = alu8bitINC(H);
		break;
	case 0x25:
		H = alu8bitDEC(H);
		break;
	case 0x26:
		H = mem->readByte(PC);
		PC++;
		break;
	case 0x27:
		logic8bitDAA();
		break;
	case 0x28:
		jr(getFlagZ());
		PC++;
		break;
	case 0x29:
		alu16bitADD(HL());
		break;
	case 0x2A:
		A = mem->readByte(HL());
		setHL(HL() + 1);
		break;
	case 0x2B:
		setHL(HL() - 1);
		break;
	case 0x2C:
		L = alu8bitINC(L);
		break;
	case 0x2D:
		L = alu8bitDEC(L);
		break;
	case 0x2E:
		L = mem->readByte(PC);
		PC++;
		break;
	case 0x2F:
		logic8bitCPL();
		break;
	case 0x30:
		jr(!getFlagC());
		PC++;
		break;
	case 0x31:
		SP = mem->readWord(PC);
		PC += 2;
		break;
	case 0x32:
		mem->writeByte(HL(), A);
		setHL(HL() - 1);
		break;
	case 0x33:
		SP++;
		break;
	case 0x34:
		mem->writeByte(HL(), alu8bitINC(mem->readByte(HL())));
		break;
	case 0x35:
		mem->writeByte(HL(), alu8bitDEC(mem->readByte(HL())));
		break;
	case 0x36:
		mem->writeWord(HL(), mem->readByte(PC));
		PC++;
		break;
	case 0x37:
		scf();
		break;
	case 0x38:
		jr(getFlagC());
		break;
	case 0x39:
		alu16bitADD(SP);
		break;
	case 0x3A:
		A = mem->readByte(HL());
		setHL(HL() - 1);
		break;
	case 0x3B:
		SP--;
		break;
	case 0x3C:
		A = alu8bitINC(A);
		break;
	case 0x3D:
		A = alu8bitDEC(A);
		break;
	case 0x3E:
		A = mem->readByte(PC);
		PC++;
		break;
	case 0x3F:
		ccf();
		break;
	case 0x40:
		B = B;
		break;
	case 0x41:
		B = C;
		break;
	case 0x42:
		B = D;
		break;
	case 0x43:
		B = E;
		break;
	case 0x44:
		B = H;
		break;
	case 0x45:
		B = L;
		break;
	case 0x46:
		B = mem->readByte(HL());
		break;
	case 0x47:
		B = A;
		break;
	case 0x48:
		C = B;
		break;
	case 0x49:
		C = C;
		break;
	case 0x4A:
		C = D;
		break;
	case 0x4B:
		C = E;
		break;
	case 0x4C:
		C = H;
		break;
	case 0x4D:
		C = L;
		break;
	case 0x4E:
		C = mem->readByte(HL());
		break;
	case 0x4F:
		C = A;
		break;
	case 0x50:
		D = B;
		break;
	case 0x51:
		D = C;
		break;
	case 0x52:
		D = D;
		break;
	case 0x53:
		D = E;
		break;
	case 0x54:
		D = H;
		break;
	case 0x55:
		D = L;
		break;
	case 0x56:
		D = mem->readByte(HL());
		break;
	case 0x57:
		D = A;
		break;
	case 0x58:
		E = B;
		break;
	case 0x59:
		E = C;
		break;
	case 0x5A:
		E = D;
		break;
	case 0x5B:
		E = E;
		break;
	case 0x5C:
		E = H;
		break;
	case 0x5D:
		E = L;
		break;
	case 0x5E:
		E = mem->readByte(HL());
		break;
	case 0x5F:
		E = A;
		break;
	case 0x60:
		H = B;
		break;
	case 0x61:
		H = C;
		break;
	case 0x62:
		H = D;
		break;
	case 0x63:
		H = E;
		break;
	case 0x64:
		H = H;
		break;
	case 0x65:
		H = L;
		break;
	case 0x66:
		H = mem->readByte(HL());
		break;
	case 0x67:
		H = A;
		break;
	case 0x68:
		L = B;
		break;
	case 0x69:
		L = C;
		break;
	case 0x6A:
		L = D;
		break;
	case 0x6B:
		L = E;
		break;
	case 0x6C:
		L = H;
		break;
	case 0x6D:
		L = L;
		break;
	case 0x6E:
		L = mem->readByte(HL());
		break;
	case 0x6F:
		L = A;
		break;
	case 0x70:
		mem->writeByte(HL(), B);
		break;
	case 0x71:
		mem->writeByte(HL(), C);
		break;
	case 0x72:
		mem->writeByte(HL(), D);
		break;
	case 0x73:
		mem->writeByte(HL(), E);
		break;
	case 0x74:
		mem->writeByte(HL(), H);
		break;
	case 0x75:
		mem->writeByte(HL(), L);
		break;
	case 0x76:
		halt();
	case 0x77:
		mem->writeByte(HL(), A);
		break;
	case 0x78:
		A = B;
		break;
	case 0x79:
		A = C;
		break;
	case 0x7A:
		A = D;
		break;
	case 0x7B:
		A = E;
		break;
	case 0x7C:
		A = H;
		break;
	case 0x7D:
		A = L;
		break;
	case 0x7E:
		A = mem->readByte(HL());
		break;
	case 0x7F:
		A = A;
		break;
	case 0x80:
		alu8bitADD(B);
		break;
	case 0x81:
		alu8bitADD(C);
		break;
	case 0x82:
		alu8bitADD(D);
		break;
	case 0x83:
		alu8bitADD(E);
		break;
	case 0x84:
		alu8bitADD(H);
		break;
	case 0x85:
		alu8bitADD(L);
		break;
	case 0x86:
		alu8bitADD(mem->readByte(HL()));
		break;
	case 0x87:
		alu8bitADD(A);
		break;
	case 0x88:
		alu8bitADC(B,getFlagC());
		break;
	case 0x89:
		alu8bitADC(C, getFlagC());
		break;
	case 0x8A:
		alu8bitADC(D, getFlagC());
		break;
	case 0x8B:
		alu8bitADC(E, getFlagC());
		break;
	case 0x8C:
		alu8bitADC(H, getFlagC());
		break;
	case 0x8D:
		alu8bitADC(L, getFlagC());
		break;
	case 0x8E:
		alu8bitADC(mem->readByte(HL()), getFlagC());
		break;
	case 0x8F:
		alu8bitADC(A, getFlagC());
		break;
	case 0x90:
		alu8bitSUB(B);
		break;
	case 0x91:
		alu8bitSUB(C);
		break;
	case 0x92:
		alu8bitSUB(D);
		break;
	case 0x93:
		alu8bitSUB(E);
		break;
	case 0x94:
		alu8bitSUB(H);
		break;
	case 0x95:
		alu8bitSUB(L);
		break;
	case 0x96:
		alu8bitSUB(mem->readByte(HL()));
		break;
	case 0x97:
		alu8bitSUB(A);
		break;
	case 0x98:
		alu8bitSBC(B, getFlagC());
		break;
	case 0x99:
		alu8bitSBC(C, getFlagC());
		break;
	case 0x9A:
		alu8bitSBC(D, getFlagC());
		break;
	case 0x9B:
		alu8bitSBC(E, getFlagC());
		break;
	case 0x9C:
		alu8bitSBC(H, getFlagC());
		break;
	case 0x9D:
		alu8bitSBC(L, getFlagC());
		break;
	case 0x9E:
		alu8bitSBC(mem->readByte(HL()), getFlagC());
		break;
	case 0x9F:
		alu8bitSBC(A, getFlagC());
		break;
	case 0xA0:
		logic8bitAND(B);
		break;
	case 0xA1:
		logic8bitAND(C);
		break;
	case 0xA2:
		logic8bitAND(D);
		break;
	case 0xA3:
		logic8bitAND(E);
		break;
	case 0xA4:
		logic8bitAND(H);
		break;
	case 0xA5:
		logic8bitAND(L);
		break;
	case 0xA6:
		logic8bitAND(mem->readByte(HL()));
		break;
	case 0xA7:
		logic8bitAND(A);
		break;
	case 0xA8:
		logic8bitXOR(B);
		break;
	case 0xA9:
		logic8bitXOR(C);
		break;
	case 0xAA:
		logic8bitXOR(D);
		break;
	case 0xAB:
		logic8bitXOR(E);
		break;
	case 0xAC:
		logic8bitXOR(H);
		break;
	case 0xAD:
		logic8bitXOR(L);
		break;
	case 0xAE:
		logic8bitXOR(mem->readByte(HL()));
		break;
	case 0xAF:
		logic8bitXOR(A);
		break;
	case 0xB0:
		logic8bitOR(B);
		break;
	case 0xB1:
		logic8bitOR(C);
		break;
	case 0xB2:
		logic8bitOR(D);
		break;
	case 0xB3:
		logic8bitOR(E);
		break;
	case 0xB4:
		logic8bitOR(H);
		break;
	case 0xB5:
		logic8bitOR(L);
		break;
	case 0xB6:
		logic8bitOR(mem->readByte(HL()));
		break;
	case 0xB7:
		logic8bitOR(A);
		break;
	case 0xB8:
		logic8bitCP(B);
		break;
	case 0xB9:
		logic8bitCP(C);
		break;
	case 0xBA:
		logic8bitCP(D);
		break;
	case 0xBB:
		logic8bitCP(E);
		break;
	case 0xBC:
		logic8bitCP(H);
		break;
	case 0xBD:
		logic8bitCP(L);
		break;
	case 0xBE:
		logic8bitCP(mem->readByte(HL()));
		break;
	case 0xBF:
		logic8bitCP(A);
		break;
	case 0xC0:
		ret(!getFlagZ());
		break;
	case 0xC1:
		setBC(mem->readWord(SP));
		SP += 2;
		break;
	case 0xC2:
		jp(!getFlagZ());
		break;
	case 0xC3:
		jp(true);
		break;
	case 0xC4:
		call(!getFlagZ());
		break;
	case 0xC5:
		SP -= 2;
		mem->writeWord(SP, BC());
		break;
	case 0xC6:
		alu8bitADD(mem->readByte(PC));
		PC++;
		break;
	case 0xC7:
		rst(0x00);
		break;
	case 0xC8:
		ret(getFlagZ());
		break;
	case 0xC9:
		ret(true);
		break;
	case 0xCA:
		jp(getFlagZ());
		break;
	case 0xCB:
		executePrefixedInstruction(mem->readByte(PC));
		break;
	case 0xCC:
		call(getFlagZ());
		break;
	case 0xCD:
		call(true);
		break;
	case 0xCE:
		alu8bitADC(mem->readByte(PC), getFlagC());
		PC++;
		break;
	case 0xCF:
		rst(0x08);
		break;
	case 0xD0:
		ret(!getFlagC());
		break;
	case 0xD1:
		setDE(mem->readWord(SP));
		SP += 2;
		break;
	case 0xD2:
		jp(!getFlagC());
		break;
	case 0xD3:
		/* Empty Instruction */
		break;
	case 0xD4:
		call(!getFlagC());
		break;
	case 0xD5:
		SP -= 2;
		mem->writeWord(SP, DE());
		break;
	case 0xD6:
		alu8bitSUB(mem->readByte(PC));
		PC++;
		break;
	case 0xD7:
		rst(0x10);
		break;
	case 0xD8:
		ret(getFlagC());
	case 0xD9:
		reti();
		break;
	case 0xDA:
		jp(getFlagC());
		break;
	case 0xDB:
		/* Empty Instruction */
		break;
	case 0xDC:
		call(getFlagC());
		break;
	case 0xDD:
		/* Empty Instruction */
		break;
	case 0xDE:
		alu8bitSBC(mem->readByte(PC), getFlagC());
		PC++;
		break;
	case 0xDF:
		rst(0x18);
		break;
	case 0xE0:
		mem->writeByte(0xFF00 + mem->readByte(PC), A);
		PC++;
		break;
	case 0xE1:
		setHL(mem->readWord(SP));
		SP += 2;
		break;
	case 0xE2:
		mem->writeByte(0xFF00 + C, A);
		break;
	case 0xE3:
		/* Empty Instruction */
		break;
	case 0xE4:
		/* Empty Instruction */
		break;
	case 0xE5:
		SP -= 2;
		mem->writeWord(SP, HL());
		break;
	case 0xE6:
		logic8bitAND(mem->readByte(PC));
		PC++;
		break;
	case 0xE7:
		rst(0x20);
		break;
	case 0xE8:
		alu16bitSPADD(mem->readByte(PC));
		PC++;
		break;
	case 0xE9:
		PC = HL();
		break;
	case 0xEA:
		mem->writeByte(mem->readWord(PC), A);
		PC += 2;
		break;
	case 0xEB:
		/* Empty Instruction */
		break;
	case 0xEC:
		/* Empty Instruction */
		break;
	case 0xED:
		/* Empty Instruction */
		break;
	case 0xEE:
		logic8bitXOR(mem->readByte(PC));
		PC++;
		break;
	case 0xEF:
		rst(0x28);
		break;
	case 0xF0:
		A = mem->readByte(0xFF00 + mem->readByte(PC));
		PC++;
		break;
	case 0xF1:
		setAF(mem->readWord(SP));
		SP += 2;
		break;
	case 0xF2:
		A = mem->readByte(0xFF00 + C);
		break;
	case 0xF3:
		di();
		break;
	case 0xF4:
		/* Empty Instruction */
	case 0xF5:
		SP -= 2;
		mem->writeWord(SP, AF());
		break;
	case 0xF6:
		rst(0x30);
		break;
	case 0xF7:
		rst(0x30);
		break;
	case 0xF8: {
		i8_t data = mem->readByte(PC);
		u16_t result = SP + data;

		setFlag(Flag_Z, false);
		setFlag(Flag_N, false);
		setFlag(Flag_H, ((SP & 0x0F) + (data & 0x0F)) > 0x0F);
		setFlag(Flag_C, result > 0xFFFF);

		setHL(result);
		PC++;
		break;
	}
	case 0xF9:
		SP = HL();
		break;
	case 0xFA:
		A = mem->readByte(mem->readWord(PC));
		break;
	case 0xFB:
		ei();
		break;
	case 0xFC:
		/* Empty Instruction */
		break;
	case 0xFD:
		/* Empty Instruction */
		break;
	case 0xFE:
		logic8bitCP(mem->readByte(PC));
		PC++;
		break;
	case 0xFF:
		rst(0x38);
		break;
	default:
		break;
	}
}
#pragma endregion

#pragma region CPU execute Prefixed Instruction
void LR35902::executePrefixedInstruction(u8_t opcode) {
	PC++;
	lastcyclecount = prefixedinstructionCount[opcode];
	switch (opcode) {
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
	case 0x4:
	case 0x5:
	case 0x6:
	case 0x7:
	case 0x8:
	case 0x9:
	case 0xA:
	case 0xB:
	case 0xC:
	case 0xD:
	case 0xE:
	case 0xF:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
	case 0x20:
	case 0x21:
	case 0x22:
	case 0x23:
	case 0x24:
	case 0x25:
	case 0x26:
	case 0x27:
	case 0x28:
	case 0x29:
	case 0x2A:
	case 0x2B:
	case 0x2C:
	case 0x2D:
	case 0x2E:
	case 0x2F:
	case 0x30:
	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x36:
	case 0x37:
	case 0x38:
	case 0x39:
	case 0x3A:
	case 0x3B:
	case 0x3C:
	case 0x3D:
	case 0x3E:
	case 0x3F:
	case 0x40:
	case 0x41:
	case 0x42:
	case 0x43:
	case 0x44:
	case 0x45:
	case 0x46:
	case 0x47:
	case 0x48:
	case 0x49:
	case 0x4A:
	case 0x4B:
	case 0x4C:
	case 0x4D:
	case 0x4E:
	case 0x4F:
	case 0x50:
	case 0x51:
	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
	case 0x56:
	case 0x57:
	case 0x58:
	case 0x59:
	case 0x5A:
	case 0x5B:
	case 0x5C:
	case 0x5D:
	case 0x5E:
	case 0x5F:
	case 0x60:
	case 0x61:
	case 0x62:
	case 0x63:
	case 0x64:
	case 0x65:
	case 0x66:
	case 0x67:
	case 0x68:
	case 0x69:
	case 0x6A:
	case 0x6B:
	case 0x6C:
	case 0x6D:
	case 0x6E:
	case 0x6F:
	case 0x70:
	case 0x71:
	case 0x72:
	case 0x73:
	case 0x74:
	case 0x75:
	case 0x76:
	case 0x77:
	case 0x78:
	case 0x79:
	case 0x7A:
	case 0x7B:
	case 0x7C:
	case 0x7D:
	case 0x7E:
	case 0x7F:
	case 0x80:
	case 0x81:
	case 0x82:
	case 0x83:
	case 0x84:
	case 0x85:
	case 0x86:
	case 0x87:
	case 0x88:
	case 0x89:
	case 0x8A:
	case 0x8B:
	case 0x8C:
	case 0x8D:
	case 0x8E:
	case 0x8F:
	case 0x90:
	case 0x91:
	case 0x92:
	case 0x93:
	case 0x94:
	case 0x95:
	case 0x96:
	case 0x97:
	case 0x98:
	case 0x99:
	case 0x9A:
	case 0x9B:
	case 0x9C:
	case 0x9D:
	case 0x9E:
	case 0x9F:
	case 0xA0:
	case 0xA1:
	case 0xA2:
	case 0xA3:
	case 0xA4:
	case 0xA5:
	case 0xA6:
	case 0xA7:
	case 0xA8:
	case 0xA9:
	case 0xAA:
	case 0xAB:
	case 0xAC:
	case 0xAD:
	case 0xAE:
	case 0xAF:
	case 0xB0:
	case 0xB1:
	case 0xB2:
	case 0xB3:
	case 0xB4:
	case 0xB5:
	case 0xB6:
	case 0xB7:
	case 0xB8:
	case 0xB9:
	case 0xBA:
	case 0xBB:
	case 0xBC:
	case 0xBD:
	case 0xBE:
	case 0xBF:
	case 0xC0:
	case 0xC1:
	case 0xC2:
	case 0xC3:
	case 0xC4:
	case 0xC5:
	case 0xC6:
	case 0xC7:
	case 0xC8:
	case 0xC9:
	case 0xCA:
	case 0xCB:
	case 0xCC:
	case 0xCD:
	case 0xCE:
	case 0xCF:
	case 0xD0:
	case 0xD1:
	case 0xD2:
	case 0xD3:
	case 0xD4:
	case 0xD5:
	case 0xD6:
	case 0xD7:
	case 0xD8:
	case 0xD9:
	case 0xDA:
	case 0xDB:
	case 0xDC:
	case 0xDD:
	case 0xDE:
	case 0xDF:
	case 0xE0:
	case 0xE1:
	case 0xE2:
	case 0xE3:
	case 0xE4:
	case 0xE5:
	case 0xE6:
	case 0xE7:
	case 0xE8:
	case 0xE9:
	case 0xEA:
	case 0xEB:
	case 0xEC:
	case 0xED:
	case 0xEE:
	case 0xEF:
	case 0xF0:
	case 0xF1:
	case 0xF2:
	case 0xF3:
	case 0xF4:
	case 0xF5:
	case 0xF6:
	case 0xF7:
	case 0xF8:
	case 0xF9:
	case 0xFA:
	case 0xFB:
	case 0xFC:
	case 0xFD:
	case 0xFE:
	case 0xFF:
	default:
		break;
	}
}
#pragma endregion

#pragma region CPU Registers
u16_t LR35902::AF() {
	u16_t word;
	word = F;
	word |= A << 8;
	return word;
}
u16_t LR35902::BC() {
	u16_t word;
	word = C;
	word |= B << 8;
	return word;
}
u16_t LR35902::DE() {
	u16_t word;
	word = E;
	word |= D << 8;
	return word;
}
u16_t LR35902::HL() {
	u16_t word;
	word = L;
	word |= H << 8;
	return word;
}
void LR35902::setAF(u16_t data) {
	F = data & 0xF0;
	A = (data >> 8) & 0xFF;
}
void LR35902::setBC(u16_t data) {
	C = data & 0xFf;
	B = (data >> 8) & 0xFF;
}
void LR35902::setDE(u16_t data) {
	E = data & 0xFF;
	D = (data >> 8) & 0xFF;
}
void LR35902::setHL(u16_t data) {
	L = data & 0xFF;
	H = (data >> 8) & 0xFF;
}

void LR35902::setFlag(u8_t flag, bool state) {
	if (state) {
		F |= (1 << flag);
	}
	else {
		F &= ~(1 << flag);
	}
}
bool LR35902::getFlag(u8_t flag) {
	return (F & (1 << flag)) != 0;
}
void LR35902::setFlagZ(void) {
	F |= Flag_Z;
}
void LR35902::setFlagN(void) {
	F |= Flag_N;
}
void LR35902::setFlagH(void) {
	F |= Flag_H;
}
void LR35902::setFlagC(void) {
	F |= Flag_C;
}
bool LR35902::getFlagZ() {
	u8_t data = F;

	data &= Flag_Z;
	data = data >> 7;

	if (data == 0x1) {
		return true;
	}
	return false;
}
bool LR35902::getFlagN() {
	u8_t data = F;

	data &= Flag_N;
	data = data >> 7;

	if (data == 0x1) {
		return true;
	}
	return false;
}
bool LR35902::getFlagH() {
	u8_t data = F;

	data &= Flag_H;
	data = data >> 7;

	if (data == 0x1) {
		return true;
	}
	return false;
}
bool LR35902::getFlagC() {
	u8_t data = F;

	data &= Flag_C;
	data = data >> 7;

	if (data == 0x1) {
		return true;
	}
	return false;
}
#pragma endregion

#pragma region ALU
void LR35902::alu16bitADD(u16_t data) {
	u32_t result = HL() + data;

	setHL(HL() + data);

	setFlag(Flag_N, false);
	setFlag(Flag_H, (((HL() & 0x0FFF) + (data & 0x0FFF)) & 0xF000) != 0);
	setFlag(Flag_C, result > 0xFFFF);
}
void LR35902::alu16bitSPADD(u16_t data) {
	u8_t immediate = data;
	u16_t tmp = SP;
	u16_t result = SP + immediate;

	setFlag(Flag_Z, false);
	setFlag(Flag_N, false);
	setFlag(Flag_H, ((tmp & 0x0F) + (immediate & 0x0F)) > 0x0F);
	setFlag(Flag_C, result > 0xFFFF);

	SP = result;
}
void LR35902::alu8bitADD(u8_t data) {
	u8_t tmp = A;
	
	A += data;

	setFlag(Flag_Z, (A == 0));
	setFlag(Flag_N, false);
	setFlag(Flag_H, ((tmp & 0x0F) + (data & 0x0F)) > 0x0F);
	setFlag(Flag_C, (A < tmp));
}
void LR35902::alu8bitADC(u8_t data, bool carry) {
	u8_t tmp = A;
	u8_t carryValue = carry ? 1 : 0;

	A += data + carryValue;

	setFlag(Flag_Z, (A == 0));
	setFlag(Flag_N, false);
	setFlag(Flag_H, ((tmp & 0x0F) + (data & 0x0F) + carryValue) > 0x0F);
	setFlag(Flag_C, (A < tmp));
}
void LR35902::alu8bitSUB(u8_t data) {
	u8_t tmp = A;
	
	A -= data;

	setFlag(Flag_Z, (A == 0));
	setFlag(Flag_N, true);
	setFlag(Flag_H, (tmp & 0x0F) < (data & 0x0F));
	setFlag(Flag_C, (A > tmp));
}
void LR35902::alu8bitSBC(u8_t data, bool carry) {
	u8_t tmp = A;
	u8_t carryValue = carry ? 1 : 0;

	A -= (data + carryValue);

	setFlag(Flag_Z, (A == 0));
	setFlag(Flag_N, true);
	setFlag(Flag_H, ((tmp & 0x0F) < ((data & 0x0F) + carryValue)));
	setFlag(Flag_C, (A > tmp) || (tmp < (data + carryValue)));
}
u8_t LR35902::alu8bitINC(u8_t data) {
	u8_t tmp = data;
	
	data++;

	setFlag(Flag_Z, (data == 0));
	setFlag(Flag_N, false);
	setFlag(Flag_H, (tmp & 0x0F) == 0x0F);

	return data;
}
u8_t LR35902::alu8bitDEC(u8_t data) {
	u8_t tmp = data;
	
	data--;

	setFlag(Flag_Z, (data == 0));
	setFlag(Flag_N, true);
	setFlag(Flag_H, (tmp & 0x0F) == 0);

	return data;
}
void LR35902::logic8bitAND(u8_t data) {
	A &= data;
	
	setFlag(Flag_Z, (A == 0));
	setFlag(Flag_N, false);
	setFlag(Flag_H, true);
	setFlag(Flag_C, false);
}
void LR35902::logic8bitOR(u8_t data) {
	A |= data;

	setFlag(Flag_Z, (A == 0));
	setFlag(Flag_N, false);
	setFlag(Flag_H, false);
	setFlag(Flag_C, false);
}
void LR35902::logic8bitXOR(u8_t data) {
	A ^= data;

	setFlag(Flag_Z, (A == 0));
	setFlag(Flag_N, false);
	setFlag(Flag_H, true);
	setFlag(Flag_C, false);
}
void LR35902::logic8bitCP(u8_t data) {
	u8_t result = A - data;

	setFlag(Flag_Z, (result == 0));
	setFlag(Flag_N, true);
	setFlag(Flag_H, ((A & 0x0F) < (data & 0x0F)));
	setFlag(Flag_C, (A < data));
}
void LR35902::logic8bitDAA() {
	u8_t correction = 0;

	if (getFlag(Flag_H) || (!getFlag(Flag_N) && (A & 0x0F) > 9)) {
		correction |= 0x06;
	}

	if (getFlag(Flag_C) || (!getFlag(Flag_N) && A > 0x99)) {
		correction |= 0x60;
		setFlag(Flag_C, true);
	}

	if (!getFlag(Flag_N) && ((A & 0x0F) > 9 || getFlag(Flag_H))) {
		A += correction;
	}
	else if (getFlag(Flag_N) && getFlag(Flag_H)) {
		A = (A - 6) & 0xFF;
	}

	setFlag(Flag_Z, (A == 0));
	setFlag(Flag_H, false);
}
void LR35902::logic8bitCPL() {
	A = ~A;

	setFlag(Flag_N, true);
	setFlag(Flag_H, true);
}
#pragma endregion

#pragma region Rotate and Shift
u8_t LR35902::RLC(u8_t data) {
	u8_t result = data;
	bool carry = (result & 0x80) != 0;

	result = (result << 1) | (u8_t)carry;

	setFlag(Flag_Z, (result == 0));
	setFlag(Flag_N, false);
	setFlag(Flag_H, false);
	setFlag(Flag_C, carry);

	return result;
}
u8_t LR35902::RL(u8_t data) {
	u8_t result = data;
	bool carryFlagBit = getFlagC();
	bool carry = (result & 0x80) != 0;

	result = (result << 1) | (u8_t)carryFlagBit;

	setFlag(Flag_Z, (result == 0));
	setFlag(Flag_N, false);
	setFlag(Flag_H, false);
	setFlag(Flag_C, carry);

	return result;
}
u8_t LR35902::RRC(u8_t data) {
	u8_t result = data;
	bool carry = (result & 0x1) != 0;

	result = (result >> 1) | (carry << 7);

	setFlag(Flag_Z, (result == 0));
	setFlag(Flag_N, false);
	setFlag(Flag_H, false);
	setFlag(Flag_C, carry);

	return result;
}
u8_t LR35902::RR(u8_t data) {
	u8_t result = data;
	bool carryFlagBit = getFlagC();
	bool carry = (result & 0x1) != 0;

	result = (result >> 1) | (carryFlagBit << 7);

	setFlag(Flag_Z, (result == 0));
	setFlag(Flag_N, false);
	setFlag(Flag_H, false);
	setFlag(Flag_C, carry);

	return result;
}
u8_t LR35902::SLA(u8_t data) {
	u8_t result = data;
	bool carry = (result & 0x80) != 0;

	result = result << 1;

	setFlag(Flag_Z, (result == 0));
	setFlag(Flag_N, false);
	setFlag(Flag_H, false);
	setFlag(Flag_C, carry);

	return result;
}
u8_t LR35902::SRA(u8_t data) {
	i8_t result = data;
	bool carry = (result & 0x01) != 0;

	result = result >> 1;

	setFlag(Flag_Z, (result == 0));
	setFlag(Flag_N, false);
	setFlag(Flag_H, false);
	setFlag(Flag_C, carry);

	return (u8_t)result;
}
u8_t LR35902::SRL(u8_t data) {
	u8_t result = data;
	bool carry = (result & 0x01) != 0;

	result = result >> 1;

	setFlag(Flag_Z, (result == 0));
	setFlag(Flag_N, false);
	setFlag(Flag_H, false);
	setFlag(Flag_C, carry);

	return (u8_t)result;
}
u8_t LR35902::SWAP(u8_t data) {
	u8_t result = (data & 0xF0) >> 4;

	result |= data << 4;
	
	setFlag(Flag_Z, (result == 0));
	setFlag(Flag_N, false);
	setFlag(Flag_H, false);
	setFlag(Flag_C, false);

	return result;
}
void LR35902::BIT(u8_t data, int bit) {
	bool result = (data & (1 << bit)) == 0;

	setFlag(Flag_Z, result);
	setFlag(Flag_N, false);
	setFlag(Flag_H, true);
}
u8_t LR35902::SET(u8_t data, int bit) {
	u8_t setBit = 1 << bit;
	u8_t result = data | setBit;

	return result;
}
u8_t LR35902::RES(u8_t data, int bit) {
	u8_t resetBit = ~(1 << bit);
	u8_t result = data & resetBit;
	
	return result;
}
#pragma endregion

#pragma region Jump Instruction
void LR35902::jp(bool state) {
	if (state) {
		lastcyclecount = 16;
		PC = mem->readWord(PC);
		PC += 2;
	}
	else {
		PC += 2;
	}
}
void LR35902::jr(bool state) {
	if (state) {
		lastcyclecount = 12;
		PC = PC + (i8_t)mem->readByte(PC);
		PC++;
	}
	else {
		PC++;
	}
}
void LR35902::call(bool state) {
	if (state) {
		SP -= 2;
		mem->writeWord(SP, PC + 2);
		jp(true);
		lastcyclecount = 24;
		PC += 2;
	}
	else {
		PC += 2;
	}
}
void LR35902::ret(bool state) {
	if (state) {
		lastcyclecount = 20;
		PC = mem->readWord(SP);
		SP += 2;
	}
}
void LR35902::reti(void) {
	ret(true);
	IME = true;
	IMEhold = true;
	lastcyclecount = 16;
}
void LR35902::rst(u8_t data) {
	SP -= 2;
	mem->writeWord(SP, PC);
	PC = data;
}
#pragma endregion

#pragma region Control Instruction
void LR35902::ccf(void) {
	setFlag(Flag_C, !getFlagC());
	setFlag(Flag_N, false);
	setFlag(Flag_H, false);
}
void LR35902::scf(void) {
	setFlag(Flag_N, false);
	setFlag(Flag_H, false);
	setFlag(Flag_C, true);
}
void LR35902::nop(void) {

}
void LR35902::halt(void) {
	ishalt = true;
	if (!IME) {

	}
}
void LR35902::stop(void) {
	u8_t key1 = mem->readByte(0xFF4D);
	
	if ((key1 & 0x1) == 0x1) {
		DoubleSpeed = !DoubleSpeed;
	}

	mem->writeByte(0xFF4D, (key1 & 0x7E) | ((DoubleSpeed ? 1 : 0) << 7));
}
void LR35902::di(void) {
	IMEhold = false;
	EIDIFlag = true;
}
void LR35902::ei(void) {
	IMEhold = true;
	EIDIFlag = true;
}
#pragma endregion

#pragma endregion CPU