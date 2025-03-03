#ifndef CPU_HPP
#define CPU_HPP

#include "Typedefs.hpp"

class CPU
{
    public:
        CPU() = default;
        ~CPU() = default;

    private:
        Register Accumulator;
        Register X;
        Register Y;
        Register StackPointer;
        Register StatusRegister;
        LargeRegister ProgramCounter;
};

#endif