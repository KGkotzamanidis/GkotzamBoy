#pragma once

#include "Memory.h"
#include "Symbols.h"

class LR35902_Interrupt {
public:
	LR35902_Interrupt();
	~LR35902_Interrupt();

	u8_t IF = 0x00;
	u8_t IE = 0x00;
private:
};

class LR35902_Timer {
public:
	LR35902_Timer(LR35902_Interrupt& Interrupts);
	~LR35902_Timer();

	void sendData(u16_t address, u8_t data);
	u8_t recieveData(u16_t address);
	void updateTimers(int lastCycleCount);
	int clockRate(int code);
private:
	LR35902_Interrupt* Interrupts = nullptr;

	u8_t DIV = 0x00;
	u8_t TIMA = 0x00;
	u8_t TMA = 0x00;
	u8_t TAC = 0x00;

	int DIVcyclecount = 0;
	int TIMAcyclecount = 0;
};

class Memory;
class LR35902 {
public:
	LR35902(Memory& mainmemory);
	~LR35902();

	static const int instructionCount[];
	static const int prefixedinstructionCount[];

	void reset();
	void run();

	int getlastcyclecount();
private:
	void executeInstruction(u8_t opcode);
	void executePrefixedInstruction(u8_t opcode);
	/* Debug CPU Functions */
public:
	u8_t returnOPCODE();
private:
	Memory* mem;

	bool IME, IMEhold, ishalt, EIDIFlag, DoubleSpeed;
	int lastcyclecount;
	bool getDoubleSpeed();

	/* CPU Registers functions */
public:
	u8_t A, B, C, D, E, F, H, L;
	u16_t PC, SP;
private:
	u16_t AF();
	u16_t BC();
	u16_t DE();
	u16_t HL();
	
	void setAF(u16_t data);
	void setBC(u16_t data);
	void setDE(u16_t data);
	void setHL(u16_t data);

	void setFlag(u8_t flag, bool state);
	bool getFlag(u8_t flag);

	void setFlagZ(void);
	void setFlagN(void);
	void setFlagH(void);
	void setFlagC(void);

	bool getFlagZ();
	bool getFlagN();
	bool getFlagH();
	bool getFlagC();

	/* Arithemtic and Logical Unit (ALU) */
private:
	void alu16bitADD(u16_t data);
	void alu16bitSPADD(u16_t data);

	void alu8bitADD(u8_t data);
	void alu8bitADC(u8_t data, bool carry);
	void alu8bitSUB(u8_t data);
	void alu8bitSBC(u8_t data, bool carry);

	u8_t alu8bitINC(u8_t data);
	u8_t alu8bitDEC(u8_t data);

	void logic8bitAND(u8_t data);
	void logic8bitOR(u8_t data);
	void logic8bitXOR(u8_t data);
	void logic8bitCP(u8_t data);
	void logic8bitDAA();
	void logic8bitCPL();

	/* Rotate and Shift */
private:
	u8_t RLC(u8_t data);
	u8_t RL(u8_t data);
	u8_t RRC(u8_t data);
	u8_t RR(u8_t data);
	u8_t SLA(u8_t data);
	u8_t SRA(u8_t data);
	u8_t SRL(u8_t data);
	u8_t SWAP(u8_t data);

	void BIT(int bit, u8_t data);
	u8_t SET(int bit, u8_t data);
	u8_t RES(int bit, u8_t data);

	/* CPU Control Instruction */
private:
	void ccf(void);
	void scf(void);
	void nop(void);
	void halt(void);
	void stop(void);
	void di(void);
	void ei(void);

	/* Jump Instruction */
private:
	void jp(bool state);
	void jr(bool state);
	void call(bool state);
	void ret(bool state);
	void reti(void);
	void rst(u8_t data);
};