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
    if (CyclesLeft == 0) {
        // If we have entered here, it means that the previous instruction has completed
        // its cycle count and we can move on to the next instruction.

        Opcode opcode = FetchByteFromMemory(ProgramCounter);
        ++ProgramCounter;
        CyclesLeft = GetNumberOfBaseClockCyclesForOperation(opcode);
        
        AddressingMode = OpcodeTable[opcode].addressingMode;
        OperationFunction = OpcodeTable[opcode].operation;

        CyclesLeft +=  (this->*AddressingMode)() & (this->*OperationFunction)() ? 1 : 0;
    }

    --CyclesLeft;
}

bool
CPU::ImplicitMode()
{
    FetchedData = Accumulator;
    return false;
}

bool
CPU::ImmediateMode()
{
    AbsoluteAddress = ProgramCounter++;
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
CPU::FetchDataForOperation(const Address)
{
    if (OpcodeTable.at(CurrentOpcode).addressingMode != &CPU::ImplicitMode) {
        FetchedData = FetchDataForOperation(ProgramCounter);
    }
    return FetchedData
}