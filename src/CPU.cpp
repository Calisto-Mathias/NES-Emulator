#include "../include/CPU.hpp"
#include "../include/Typedefs.hpp"

CPU::CPU()
{
    // Empty Constructor
}

CPU::~CPU()
{
    // Empty Destructor
}

void
CPU::Clock()
{
    if (CyclesLeftLeft == 0) {
        // If we have entered here, it means that the previous instruction has completed
        // its cycle count and we can move on to the next instruction.

        Opcode Opcode = FetchByteFromMemory(ProgramCounter);
        ++ProgramCounter;
        CyclesLeftLeft = GetNumberOfBaseClockCyclesLeftForOperation(Opcode);
        
        AddressingMode = OpcodeTable[Opcode].addressingMode;
        OperationFunction = OpcodeTable[Opcode].operation;

        CyclesLeftLeft +=  (this->*AddressingMode)() & (this->*OperationFunction)() ? 1 : 0;
    }

    --CyclesLeftLeft;
}

bool
CPU::ImplicitMode()
{
    FetchedDataData = Accumulator; // Reset the Byte;
    return false;
}

bool
CPU::ImmediateMode()
{
    AbsoluteAddress = ProgramCounter++;
    return false;
}

bool
CPU::AccumulatorMode()
{
    FetchedDataData = Accumulator;
    return false;
}

bool
CPU::ZeroPageMode()
{
    AbsoluteAddress = FetchByteFromMemory(ProgramCounter);
    ++ProgramCounter;
    AbsoluteAddress &= 0x00FF;
    return false;
}

bool
CPU::ZeroPageXMode()
{
    AbsoluteAddress = FetchByteFromMemory(ProgramCounter);
    ++ProgramCounter;
    AbsoluteAddress += X;
    AbsoluteAddress &= 0x00FF;
    return false;
}

bool
CPU::ZeroPageYMode()
{
    AbsoluteAddress = FetchByteFromMemory(ProgramCounter);
    ++ProgramCounter;
    AbsoluteAddress += Y;
    AbsoluteAddress &= 0x00FF;
    return false;
}

bool
CPU::AbsoluteMode()
{
    Byte lowByte = FetchByteFromMemory(ProgramCounter);
    ++ProgramCounter;
    Byte highByte = FetchByteFromMemory(ProgramCounter);
    ++ProgramCounter;
    AbsoluteAddress = (highByte << 8) | lowByte;
    return false;
}

bool
CPU::AbsoluteXMode()
{
    Byte lowByte = FetchByteFromMemory(ProgramCounter);
    ++ProgramCounter;
    Byte highByte = FetchByteFromMemory(ProgramCounter);
    ++ProgramCounter;
    AbsoluteAddress = (highByte << 8) | lowByte;
    AbsoluteAddress += X;
    
    bool hasPageChanged = (AbsoluteAddress & 0xFF00) != (highByte << 8);
    return hasPageChanged;
}

bool
CPU::AbsoluteYMode()
{
    Byte lowByte = FetchByteFromMemory(ProgramCounter);
    ++ProgramCounter;
    Byte highByte = FetchByteFromMemory(ProgramCounter);
    ++ProgramCounter;
    AbsoluteAddress = (highByte << 8) | lowByte;
    AbsoluteAddress += Y;
    
    bool hasPageChanged = (AbsoluteAddress & 0xFF00) != (highByte << 8);
    return hasPageChanged;
}

bool
CPU::IndirectMode()
{
    Byte lowByte = FetchByteFromMemory(ProgramCounter);
    ++ProgramCounter;
    Byte highByte = FetchByteFromMemory(ProgramCounter);
    ++ProgramCounter;
    
    Address indirectAddress = (highByte << 8) | lowByte;
    
    Byte indirectAddressLowByte = FetchByteFromMemory(indirectAddress);
    Byte indirectAddressHighByte = FetchByteFromMemory(indirectAddress + 1);
    AbsoluteAddress = (indirectAddressHighByte << 8) | indirectAddressLowByte;

    return false;
}

bool
CPU::IndirectXMode()
{
    Byte zeroPageBaseAddress = FetchByteFromMemory(ProgramCounter);
    ++ProgramCounter;

    Address indirectAddressForLowByte = static_cast<Address>(zeroPageBaseAddress) + static_cast<Address>(X);
    Address indirectAddressForHighByte = indirectAddressForLowByte + 1;

    indirectAddressForLowByte &= 0x00FF;
    indirectAddressForHighByte &= 0x00FF;

    AbsoluteAddress = (FetchByteFromMemory(indirectAddressForHighByte) << 8) | FetchByteFromMemory(indirectAddressForLowByte);

    return false;
}

bool
CPU::IndirectYMode()
{
    Byte zeroPageBaseAddress = FetchByteFromMemory(ProgramCounter);
    ++ProgramCounter;

    Byte lowByte = FetchByteFromMemory(zeroPageBaseAddress & 0x00FF);
    Byte highByte = FetchByteFromMemory((zeroPageBaseAddress + 1) & 0x00FF);
    AbsoluteAddress = (highByte << 8) | lowByte;
    AbsoluteAddress += Y;

    bool hasPageChanged = (AbsoluteAddress & 0xFF00) != (highByte << 8);
    return hasPageChanged;
}

bool
CPU::RelativeMode()
{
    RelativeAddress = FetchByteFromMemory(ProgramCounter);
    ++ProgramCounter;
    
    if (RelativeAddress & 0x80) {
        RelativeAddress |= 0xFF00;
    }

    return false;
}

Byte
CPU::FetchDataForOperation()
{
    if (OpcodeTable.at(CurrentOpcode).addressingMode != &CPU::ImplicitMode && OpcodeTable.at(CurrentOpcode).addressingMode != &CPU::AccumulatorMode) {
        FetchedData = FetchDataForOperation(ProgramCounter);
    }
    return FetchedData;
}

inline bool
CPU::GetFlagFromStatusRegister(const StatusRegisterFlags::Flags flag)
{
    return (StatusRegister & flag) != 0;
}

inline void
CPU::SetFlagInStatusRegister(const StatusRegisterFlags::Flags flag, const bool toSet)
{
    if (toSet) {
        StatusRegister |= flag;
    } else {
        StatusRegister &= ~flag;
    }
}

bool CPU::ADC() {
    FetchDataForOperation();
    TemporaryStorage = (uint16_t)Accumulator + (uint16_t)FetchedData + (uint16_t)GetFlagFromStatusRegister(StatusRegisterFlags::C);
    SetFlagInStatusRegister(StatusRegisterFlags::C, TemporaryStorage > 255);
    SetFlagInStatusRegister(StatusRegisterFlags::Z, (TemporaryStorage & 0x00FF) == 0);
    SetFlagInStatusRegister(StatusRegisterFlags::V, (~((uint16_t)Accumulator ^ (uint16_t)FetchedData) & ((uint16_t)Accumulator ^ (uint16_t)TemporaryStorage)) & 0x0080);
    SetFlagInStatusRegister(StatusRegisterFlags::N, TemporaryStorage & 0x80);
    Accumulator = TemporaryStorage & 0x00FF;
    return 1;
}

bool CPU::SBC() {
    FetchDataForOperation();
    uint16_t value = ((uint16_t)FetchedData) ^ 0x00FF;
    TemporaryStorage = (uint16_t)Accumulator + value + (uint16_t)GetFlagFromStatusRegister(StatusRegisterFlags::C);
    SetFlagInStatusRegister(StatusRegisterFlags::C, TemporaryStorage & 0xFF00);
    SetFlagInStatusRegister(StatusRegisterFlags::Z, ((TemporaryStorage & 0x00FF) == 0));
    SetFlagInStatusRegister(StatusRegisterFlags::V, (TemporaryStorage ^ (uint16_t)Accumulator) & (TemporaryStorage ^ value) & 0x0080);
    SetFlagInStatusRegister(StatusRegisterFlags::N, TemporaryStorage & 0x0080);
    Accumulator = TemporaryStorage & 0x00FF;
    return 1;
}

bool CPU::AND() {
    FetchDataForOperation();
    Accumulator = Accumulator & FetchedData;
    SetFlagInStatusRegister(StatusRegisterFlags::Z, Accumulator == 0x00);
    SetFlagInStatusRegister(StatusRegisterFlags::N, Accumulator & 0x80);
    return 1;
}

bool CPU::ASL() {
    FetchDataForOperation();
    TemporaryStorage = (uint16_t)FetchedData << 1;
    SetFlagInStatusRegister(StatusRegisterFlags::C, (TemporaryStorage & 0xFF00) > 0);
    SetFlagInStatusRegister(StatusRegisterFlags::Z, (TemporaryStorage & 0x00FF) == 0x00);
    SetFlagInStatusRegister(StatusRegisterFlags::N, TemporaryStorage & 0x80);
    if (OpcodeTable.at(CurrentOpcode).addressingMode == &CPU::ImplicitMode || OpcodeTable.at(CurrentOpcode).addressingMode == &CPU::AccumulatorMode)
        Accumulator = TemporaryStorage & 0x00FF;
    else
        WriteByteToMemory(AbsoluteAddress, TemporaryStorage & 0x00FF);
    return 0;
}

bool CPU::BCC() {
    if (GetFlagFromStatusRegister(StatusRegisterFlags::C) == 0) {
        CyclesLeft++;
        AbsoluteAddress = ProgramCounter + RelativeAddress;
        if((AbsoluteAddress & 0xFF00) != (ProgramCounter & 0xFF00))
            CyclesLeft++;
        ProgramCounter = AbsoluteAddress;
    }
    return 0;
}

bool CPU::BCS() {
    if (GetFlagFromStatusRegister(StatusRegisterFlags::C) == 1) {
        CyclesLeft++;
        AbsoluteAddress = ProgramCounter + RelativeAddress;
        if ((AbsoluteAddress & 0xFF00) != (ProgramCounter & 0xFF00))
            CyclesLeft++;
        ProgramCounter = AbsoluteAddress;
    }
    return 0;
}

bool CPU::BEQ() {
    if (GetFlagFromStatusRegister(StatusRegisterFlags::Z) == 1) {
        CyclesLeft++;
        AbsoluteAddress = ProgramCounter + RelativeAddress;
        if ((AbsoluteAddress & 0xFF00) != (ProgramCounter & 0xFF00))
            CyclesLeft++;
        ProgramCounter = AbsoluteAddress;
    }
    return 0;
}

bool CPU::BIT() {
    FetchDataForOperation();
    TemporaryStorage = Accumulator & FetchedData;
    SetFlagInStatusRegister(StatusRegisterFlags::Z, (TemporaryStorage & 0x00FF) == 0x00);
    SetFlagInStatusRegister(StatusRegisterFlags::N, FetchedData & (1 << 7));
    SetFlagInStatusRegister(StatusRegisterFlags::V, FetchedData & (1 << 6));
    return 0;
}

bool CPU::BMI() {
    if (GetFlagFromStatusRegister(StatusRegisterFlags::N) == 1) {
        CyclesLeft++;
        AbsoluteAddress = ProgramCounter + RelativeAddress;
        if ((AbsoluteAddress & 0xFF00) != (ProgramCounter & 0xFF00))
            CyclesLeft++;
        ProgramCounter = AbsoluteAddress;
    }
    return 0;
}

bool CPU::BNE() {
    if (GetFlagFromStatusRegister(StatusRegisterFlags::Z) == 0) {
        CyclesLeft++;
        AbsoluteAddress = ProgramCounter + RelativeAddress;
        if ((AbsoluteAddress & 0xFF00) != (ProgramCounter & 0xFF00))
            CyclesLeft++;
        ProgramCounter = AbsoluteAddress;
    }
    return 0;
}

bool CPU::BPL() {
    if (GetFlagFromStatusRegister(StatusRegisterFlags::N) == 0) {
        CyclesLeft++;
        AbsoluteAddress = ProgramCounter + RelativeAddress;
        if ((AbsoluteAddress & 0xFF00) != (ProgramCounter & 0xFF00))
            CyclesLeft++;
        ProgramCounter = AbsoluteAddress;
    }
    return 0;
}

bool CPU::BRK() {
    ProgramCounter++;
    SetFlagInStatusRegister(StatusRegisterFlags::I, 1);
    WriteByteToMemory(0x0100 + StackPointer, (ProgramCounter >> 8) & 0x00FF);
    StackPointer--;
    WriteByteToMemory(0x0100 + StackPointer, ProgramCounter & 0x00FF);
    StackPointer--;
    SetFlagInStatusRegister(StatusRegisterFlags::B, 1);
    WriteByteToMemory(0x0100 + StackPointer, status);
    StackPointer--;
    SetFlagInStatusRegister(StatusRegisterFlags::B, 0);
    ProgramCounter = (uint16_t)FetchByteFromMemory(0xFFFE) | ((uint16_t)FetchByteFromMemory(0xFFFF) << 8);
    return 0;
}

bool CPU::BVC() {
    if (GetFlagFromStatusRegister(StatusRegisterFlags::V) == 0) {
        CyclesLeft++;
        AbsoluteAddress = ProgramCounter + RelativeAddress;
        if ((AbsoluteAddress & 0xFF00) != (ProgramCounter & 0xFF00))
            CyclesLeft++;
        ProgramCounter = AbsoluteAddress;
    }
    return 0;
}

bool CPU::BVS() {
    if (GetFlagFromStatusRegister(StatusRegisterFlags::V) == 1) {
        CyclesLeft++;
        AbsoluteAddress = ProgramCounter + RelativeAddress;
        if ((AbsoluteAddress & 0xFF00) != (ProgramCounter & 0xFF00))
            CyclesLeft++;
        ProgramCounter = AbsoluteAddress;
    }
    return 0;
}

bool CPU::CLC() {
    SetFlagInStatusRegister(StatusRegisterFlags::C, false);
    return 0;
}

bool CPU::CLD() {
    SetFlagInStatusRegister(StatusRegisterFlags::D, false);
    return 0;
}

bool CPU::CLI() {
    SetFlagInStatusRegister(StatusRegisterFlags::I, false);
    return 0;
}

bool CPU::CLV() {
    SetFlagInStatusRegister(StatusRegisterFlags::V, false);
    return 0;
}

bool CPU::CMP() {
    FetchDataForOperation();
    TemporaryStorage = (uint16_t)a - (uint16_t)FetchedData;
    SetFlagInStatusRegister(StatusRegisterFlags::C, Accumulator >= FetchedData);
    SetFlagInStatusRegister(StatusRegisterFlags::Z, (TemporaryStorage & 0x00FF) == 0x0000);
    SetFlagInStatusRegister(StatusRegisterFlags::N, TemporaryStorage & 0x0080);
    return 1;
}

bool CPU::CPX() {
    FetchDataForOperation();
    TemporaryStorage = (uint16_t)x - (uint16_t)FetchedData;
    SetFlagInStatusRegister(StatusRegisterFlags::C, X >= FetchedData);
    SetFlagInStatusRegister(StatusRegisterFlags::Z, (TemporaryStorage & 0x00FF) == 0x0000);
    SetFlagInStatusRegister(StatusRegisterFlags::N, TemporaryStorage & 0x0080);
    return 0;
}

bool CPU::CPY() {
    FetchDataForOperation();
    TemporaryStorage = (uint16_t)y - (uint16_t)FetchedData;
    SetFlagInStatusRegister(StatusRegisterFlags::C, Y >= FetchedData);
    SetFlagInStatusRegister(StatusRegisterFlags::Z, (TemporaryStorage & 0x00FF) == 0x0000);
    SetFlagInStatusRegister(StatusRegisterFlags::N, TemporaryStorage & 0x0080);
    return 0;
}

bool CPU::DEC() {
    FetchDataForOperation();
    TemporaryStorage = FetchedData - 1;
    WriteByteToMemory(AbsoluteAddress, TemporaryStorage & 0x00FF);
    SetFlagInStatusRegister(StatusRegisterFlags::Z, (TemporaryStorage & 0x00FF) == 0x0000);
    SetFlagInStatusRegister(StatusRegisterFlags::N, TemporaryStorage & 0x0080);
    return 0;
}

bool CPU::DEX() {
    x--;
    SetFlagInStatusRegister(StatusRegisterFlags::Z, X == 0x00);
    SetFlagInStatusRegister(StatusRegisterFlags::N, X & 0x80);
    return 0;
}

bool CPU::DEY() {
    y--;
    SetFlagInStatusRegister(StatusRegisterFlags::Z, Y == 0x00);
    SetFlagInStatusRegister(StatusRegisterFlags::N, Y & 0x80);
    return 0;
}

bool CPU::EOR() {
    FetchDataForOperation();
    Accumulator = Accumulator ^ FetchedData;
    SetFlagInStatusRegister(StatusRegisterFlags::Z, Accumulator == 0x00);
    SetFlagInStatusRegister(StatusRegisterFlags::N, Accumulator & 0x80);
    return 1;
}

bool CPU::INC() {
    FetchDataForOperation();
    TemporaryStorage = FetchedData + 1;
    WriteByteToMemory(AbsoluteAddress, TemporaryStorage & 0x00FF);
    SetFlagInStatusRegister(StatusRegisterFlags::Z, (TemporaryStorage & 0x00FF) == 0x0000);
    SetFlagInStatusRegister(StatusRegisterFlags::N, TemporaryStorage & 0x0080);
    return 0;
}

bool CPU::INX() {
    x++;
    SetFlagInStatusRegister(StatusRegisterFlags::Z, X == 0x00);
    SetFlagInStatusRegister(StatusRegisterFlags::N, X & 0x80);
    return 0;
}

bool CPU::INY() {
    y++;
    SetFlagInStatusRegister(StatusRegisterFlags::Z, Y == 0x00);
    SetFlagInStatusRegister(StatusRegisterFlags::N, Y & 0x80);
    return 0;
}

bool CPU::JMP() {
    ProgramCounter = AbsoluteAddress;
    return 0;
}

bool CPU::JSR() {
    ProgramCounter--;
    WriteByteToMemory(0x0100 + StackPointer, (ProgramCounter >> 8) & 0x00FF);
    StackPointer--;
    WriteByteToMemory(0x0100 + StackPointer, ProgramCounter & 0x00FF);
    StackPointer--;
    ProgramCounter = AbsoluteAddress;
    return 0;
}

bool CPU::LDA() {
    FetchDataForOperation();
    Accumulator = FetchedData;
    SetFlagInStatusRegister(StatusRegisterFlags::Z, Accumulator == 0x00);
    SetFlagInStatusRegister(StatusRegisterFlags::N, Accumulator & 0x80);
    return 1;
}

bool CPU::LDX() {
    FetchDataForOperation();
    X = FetchedData;
    SetFlagInStatusRegister(StatusRegisterFlags::Z, X == 0x00);
    SetFlagInStatusRegister(StatusRegisterFlags::N, X & 0x80);
    return 1;
}

bool CPU::LDY() {
    FetchDataForOperation();
    Y = FetchedData;
    SetFlagInStatusRegister(StatusRegisterFlags::Z, Y == 0x00);
    SetFlagInStatusRegister(StatusRegisterFlags::N, Y & 0x80);
    return 1;
}

bool CPU::LSR() {
    FetchDataForOperation();
    SetFlagInStatusRegister(StatusRegisterFlags::C, FetchedData & 0x0001);
    TemporaryStorage = FetchedData >> 1;
    SetFlagInStatusRegister(StatusRegisterFlags::Z, (TemporaryStorage & 0x00FF) == 0x0000);
    SetFlagInStatusRegister(StatusRegisterFlags::N, TemporaryStorage & 0x0080);
    if (OpcodeTable.at(CurrentOpcode).addressingMode == &CPU::ImplicitMode || OpcodeTable.at(CurrentOpcode).addressingMode == &CPU::AccumulatorMode)
        Accumulator = TemporaryStorage & 0x00FF;
    else
        WriteByteToMemory(AbsoluteAddress, TemporaryStorage & 0x00FF);
    return 0;
}

bool CPU::NOP() {
    return 0;
}

bool CPU::ORA() {
    FetchDataForOperation();
    Accumulator = Accumulator | FetchedData;
    SetFlagInStatusRegister(StatusRegisterFlags::Z, Accumulator == 0x00);
    SetFlagInStatusRegister(StatusRegisterFlags::N, Accumulator & 0x80);
    return 1;
}

bool CPU::PHA() {
    WriteByteToMemory(0x0100 + StackPointer, Accumulator);
    StackPointer--;
    return 0;
}

bool CPU::PHP() {
    WriteByteToMemory(0x0100 + StackPointer, StatusRegister | B | U);
    SetFlagInStatusRegister(StatusRegisterFlags::B, 0);
    SetFlagInStatusRegister(StatusRegisterFlags::U, 0);
    StackPointer--;
    return 0;
}

bool CPU::PLA() {
    StackPointer++;
    Accumulator = FetchByteFromMemory(0x0100 + StackPointer);
    SetFlagInStatusRegister(StatusRegisterFlags::Z, Accumulator == 0x00);
    SetFlagInStatusRegister(StatusRegisterFlags::N, Accumulator & 0x80);
    return 0;
}

bool CPU::PLP() {
    StackPointer++;
    StatusRegister = FetchByteFromMemory(0x0100 + StackPointer);
    SetFlagInStatusRegister(StatusRegisterFlags::U, 1);
    return 0;
}

bool CPU::ROL() {
    FetchDataForOperation();
    TemporaryStorage = (uint16_t)(FetchedData << 1) | GetFlagFromStatusRegister(StatusRegisterFlags::C);
    SetFlagInStatusRegister(StatusRegisterFlags::C, TemporaryStorage & 0xFF00);
    SetFlagInStatusRegister(StatusRegisterFlags::Z, (TemporaryStorage & 0x00FF) == 0x0000);
    SetFlagInStatusRegister(StatusRegisterFlags::N, TemporaryStorage & 0x0080);
    if (OpcodeTable.at(CurrentOpcode).addressingMode == &CPU::ImplicitMode || OpcodeTable.at(CurrentOpcode).addressingMode == &CPU::AccumulatorMode)
        Accumulator = TemporaryStorage & 0x00FF;
    else
        WriteByteToMemory(AbsoluteAddress, TemporaryStorage & 0x00FF);
    return 0;
}

bool CPU::ROR() {
    FetchDataForOperation();
    TemporaryStorage = (uint16_t)(GetFlagFromStatusRegister(StatusRegisterFlags::C) << 7) | (FetchedData >> 1);
    SetFlagInStatusRegister(StatusRegisterFlags::C, FetchedData & 0x01);
    SetFlagInStatusRegister(StatusRegisterFlags::Z, (TemporaryStorage & 0x00FF) == 0x00);
    SetFlagInStatusRegister(StatusRegisterFlags::N, TemporaryStorage & 0x0080);
    if (OpcodeTable.at(CurrentOpcode).addressingMode == &CPU::ImplicitMode || OpcodeTable.at(CurrentOpcode).addressingMode == &CPU::AccumulatorMode)
        Accumulator = TemporaryStorage & 0x00FF;
    else
        WriteByteToMemory(AbsoluteAddress, TemporaryStorage & 0x00FF);
    return 0;
}

bool CPU::RTI() {
    StackPointer++;
    StatusRegister = FetchByteFromMemory(0x0100 + StackPointer);
    StatusRegister &= ~B;
    StatusRegister &= ~U;

    StackPointer++;
    ProgramCounter = (uint16_t)FetchByteFromMemory(0x0100 + StackPointer);
    StackPointer++;
    ProgramCounter |= (uint16_t)FetchByteFromMemory(0x0100 + StackPointer) << 8;
    return 0;
}

bool CPU::RTS() {
    StackPointer++;
    ProgramCounter = (uint16_t)FetchByteFromMemory(0x0100 + StackPointer);
    StackPointer++;
    ProgramCounter |= (uint16_t)FetchByteFromMemory(0x0100 + StackPointer) << 8;
    ProgramCounter++;
    return 0;
}

bool CPU::SEC() {
    SetFlagInStatusRegister(StatusRegisterFlags::C, true);
    return 0;
}

bool CPU::SED() {
    SetFlagInStatusRegister(StatusRegisterFlags::D, true);
    return 0;
}

bool CPU::SEI() {
    SetFlagInStatusRegister(StatusRegisterFlags::I, true);
    return 0;
}

bool CPU::STA() {
    WriteByteToMemory(AbsoluteAddress, Accumulator);
    return 0;
}

bool CPU::STX() {
    WriteByteToMemory(AbsoluteAddress, X);
    return 0;
}

bool CPU::STY() {
    WriteByteToMemory(AbsoluteAddress, Y);
    return 0;
}

bool CPU::TAX() {
    X = Accumulator;
    SetFlagInStatusRegister(StatusRegisterFlags::Z, X == 0x00);
    SetFlagInStatusRegister(StatusRegisterFlags::N, X & 0x80);
    return 0;
}

bool CPU::TAY() {
    Y = Accumulator;
    SetFlagInStatusRegister(StatusRegisterFlags::Z, Y == 0x00);
    SetFlagInStatusRegister(StatusRegisterFlags::N, Y & 0x80);
    return 0;
}

bool CPU::TSX() {
    X = StackPointer;
    SetFlagInStatusRegister(StatusRegisterFlags::Z, X == 0x00);
    SetFlagInStatusRegister(StatusRegisterFlags::N, X & 0x80);
    return 0;
}

bool CPU::TXA() {
    Accumulator = X;
    SetFlagInStatusRegister(StatusRegisterFlags::Z, Accumulator == 0x00);
    SetFlagInStatusRegister(StatusRegisterFlags::N, Accumulator & 0x80);
    return 0;
}

bool CPU::TXS() {
    StackPointer = X;
    return 0;
}

bool CPU::TYA() {
    Accumulator = Y;
    SetFlagInStatusRegister(StatusRegisterFlags::Z, Accumulator == 0x00);
    SetFlagInStatusRegister(StatusRegisterFlags::N, Accumulator & 0x80);
    return 0;
}

bool CPU::XXX() {
    return 0;
}