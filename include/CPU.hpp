#ifndef CPU_HPP
#define CPU_HPP

#include "Typedefs.hpp"

class CPU
{
    public:
        CPU() = default;
        ~CPU() = default;

        // Input Signals into the CPU are Public
        void Clock();
        void Reset();
        void InterruptRequest();
        void NonMaskableInterrupt();

    private:
        const Byte FetchByteFromMemory(const Address);

        // TODO: Change the return types to use std::optional<Byte> rather than Byte!
        // Addressing Modes
        bool ImplicitMode();
        bool AccumulatorMode();
        bool ImmediateMode();
        bool ZeroPageMode();
        bool ZeroPageXMode();
        bool ZeroPageYMode();
        bool AbsoluteMode();
        bool AbsoluteXMode();
        bool AbsoluteYMode();
        bool IndirectMode();
        bool IndirectXMode();
        bool IndirectYMode();
        bool RelativeMode();

        // Opcode Functions
        bool ADC(); // Add with Carry
        bool AND(); // And with Accumulator
        bool ASL(); // Arithmetic Shift Left
        bool BCC(); // Branch on Carry Clear
        bool BCS(); // Branch on Carry Set
        bool BEQ(); // Branch on Equal
        bool BIT(); // Bit Test
        bool BMI(); // Branch on Minus
        bool BNE(); // Branch on Not Equal
        bool BPL(); // Branch on Plus
        bool BRK(); // Force Break
        bool BVC(); // Branch on Overflow Clear
        bool BVS(); // Branch on Overflow Set
        bool CLC(); // Clear Carry Flag
        bool CLD(); // Clear Decimal Mode
        bool CLI(); // Clear Interrupt Disable
        bool CLV(); // Clear Overflow Flag
        bool CMP(); // Compare Accumulator
        bool CPX(); // Compare X Register
        bool CPY(); // Compare Y Register
        bool DEC(); // Decrement Memory
        bool DEX(); // Decrement X Register
        bool DEY(); // Decrement Y Register
        bool EOR(); // Exclusive Or with Accumulator
        bool INC(); // Increment Memory
        bool INX(); // Increment X Register
        bool INY(); // Increment Y Register
        bool JMP(); // Jump to Address
        bool JSR(); // Jump to Subroutine
        bool LDA(); // Load Accumulator
        bool LDX(); // Load X Register
        bool LDY(); // Load Y Register
        bool LSR(); // Logical Shift Right
        bool NOP(); // No Operation
        bool ORA(); // Or with Accumulator
        bool PHA(); // Push Accumulator
        bool PHP(); // Push Processor Status
        bool PLA(); // Pull Accumulator
        bool PLP(); // Pull Processor Status
        bool ROL(); // Rotate Left
        bool ROR(); // Rotate Right
        bool RTI(); // Return from Interrupt
        bool RTS(); // Return from Subroutine
        bool SBC(); // Subtract with Carry
        bool SEC(); // Set Carry Flag
        bool SED(); // Set Decimal Mode
        bool SEI(); // Set Interrupt Disable
        bool STA(); // Store Accumulator
        bool STX(); // Store X Register
        bool STY(); // Store Y Register
        bool TAX(); // Transfer Accumulator to X
        bool TAY(); // Transfer Accumulator to Y
        bool TSX(); // Transfer Stack Pointer to X
        bool TXA(); // Transfer X to Accumulator
        bool TXS(); // Transfer X to Stack Pointer
        bool TYA(); // Transfer Y to Accumulator
        bool XXX(); // Catches all illegal Instructions!

        // Utility Functions
        inline uint8_t GetNumberOfBaseClockCyclesForOperation(const Opcode);

    private:
        Register Accumulator;
        Register X;
        Register Y;
        Register StackPointer;
        Register StatusRegister;
        LargeRegister ProgramCounter;
    
        Byte FetchedData;
        Address AbsoluteAddress;
        Address RelativeAddress;
        uint8_t CyclesLeft;
};

#endif