//============================================================================
//
// MM     MM  6666  555555  0000   2222
// MMMM MMMM 66  66 55     00  00 22  22
// MM MMM MM 66     55     00  00     22
// MM  M  MM 66666  55555  00  00  22222  --  "A 6502 Microprocessor Emulator"
// MM     MM 66  66     55 00  00 22
// MM     MM 66  66 55  55 00  00 22
// MM     MM  6666   5555   0000  222222
//
// Copyright (c) 1995-2011 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id: M6502.hxx 2231 2011-05-10 15:04:19Z stephena $
//============================================================================

#ifndef M6502_HXX
#define M6502_HXX

class M6502;
class Debugger;
class CpuDebug;
class Expression;
class PackedBitArray;

#include "bspf.hxx"
#include "System.hxx"
#include "Array.hxx"
#include "StringList.hxx"
#include "Serializable.hxx"

typedef Common::Array<Expression*> ExpressionList;

/**
  The 6502 is an 8-bit microprocessor that has a 64K addressing space.
  This class provides a high compatibility 6502 microprocessor emulator.

  The memory accesses and cycle counts it generates are valid at the
  sub-instruction level and "false" reads are generated (such as the ones 
  produced by the Indirect,X addressing when it crosses a page boundary).
  This provides provides better compatibility for hardware that has side
  effects and for games which are very time sensitive.

  @author  Bradford W. Mott
  @version $Id: M6502.hxx 2231 2011-05-10 15:04:19Z stephena $
*/
class M6502 : public Serializable
{
  // The 6502 and Cart debugger classes are friends who need special access
  friend class CartDebug;
  friend class CpuDebug;

  public:
    /**
      Create a new 6502 microprocessor with the specified cycle 
      multiplier.  The cycle multiplier is the number of system cycles 
      per processor cycle.

      @param systemCyclesPerProcessorCycle The cycle multiplier
    */
    M6502(uInt32 systemCyclesPerProcessorCycle);

    /**
      Destructor
    */
    virtual ~M6502();

  public:
    /**
      Install the processor in the specified system.  Invoked by the
      system when the processor is attached to it.

      @param system The system the processor should install itself in
    */
    void install(System& system);

    /**
      Reset the processor to its power-on state.  This method should not 
      be invoked until the entire 6502 system is constructed and installed
      since it involves reading the reset vector from memory.
    */
    void reset();

    /**
      Request a maskable interrupt
    */
    void irq();

    /**
      Request a non-maskable interrupt
    */
    void nmi();

    /**
      Execute instructions until the specified number of instructions
      is executed, someone stops execution, or an error occurs.  Answers
      true iff execution stops normally.

      @param number Indicates the number of instructions to execute
      @return true iff execution stops normally
    */
    bool execute(uInt32 number);

    /**
      Tell the processor to stop executing instructions.  Invoking this 
      method while the processor is executing instructions will stop 
      execution as soon as possible.
    */
    void stop();

    /**
      Answer true iff a fatal error has occured from which the processor
      cannot recover (i.e. illegal instruction, etc.)

      @return true iff a fatal error has occured
    */
    bool fatalError() const { return myExecutionStatus & FatalErrorBit; }
  
    /**
      Get the 16-bit value of the Program Counter register.

      @return The program counter register
    */
    uInt16 getPC() const { return PC; }

    /**
      Answer true iff the last memory access was a read.

      @return true iff last access was a read
    */ 
    bool lastAccessWasRead() const { return myLastAccessWasRead; }

    /**                                                                    
      Return the last address that was part of a read/peek.  Note that
      reads which are part of a write are not considered here, unless
      they're not the same as the last write address.  This eliminates
      accesses that are part of a normal read/write cycle.

      @return The address of the last read
    */
    uInt16 lastReadAddress() const {
      return myLastPokeAddress ?
        (myLastPokeAddress != myLastPeekAddress ? myLastPeekAddress : 0) :
        myLastPeekAddress;
    }

    /**                                                                    
      Return the source of the address that was used for a write/poke.
      Note that this isn't the same as the address that is poked, but
      is instead the address of the *data* that is poked (if any).

      @return The address of the data used in the last poke, else 0
    */
    uInt16 lastDataAddressForPoke() const { return myDataAddressForPoke; }

    /**                                                                    
      Return the last data address used as part of a peek operation for
      the S/A/X/Y registers.  Note that if an address wasn't used (as in
      immediate mode), then the address is zero.

      @return The address of the data used in the last peek, else 0
    */
    uInt16 lastSrcAddressS() const { return myLastSrcAddressS; }
    uInt16 lastSrcAddressA() const { return myLastSrcAddressA; }
    uInt16 lastSrcAddressX() const { return myLastSrcAddressX; }
    uInt16 lastSrcAddressY() const { return myLastSrcAddressY; }

    /**
      Get the total number of instructions executed so far.

      @return The number of executed instructions
    */
    int totalInstructionCount() const { return myTotalInstructionCount; }

    /**
      Get the number of memory accesses to distinct memory locations

      @return The number of memory accesses to distinct memory locations
    */
    uInt32 distinctAccesses() const { return myNumberOfDistinctAccesses; }

    /**
      Saves the current state of this device to the given Serializer.

      @param out The serializer device to save to.
      @return The result of the save.  True on success, false on failure.
    */
    bool save(Serializer& out) const;

    /**
      Loads the current state of this device from the given Serializer.

      @param in The Serializer device to load from.
      @return The result of the load.  True on success, false on failure.
    */
    bool load(Serializer& in);

    /**
      Get a null terminated string which is the processor's name (i.e. "M6532")

      @return The name of the device
    */
    string name() const { return "M6502"; }

#ifdef DEBUGGER_SUPPORT
  public:
    /**
      Attach the specified debugger.

      @param debugger The debugger to attach to the microprocessor.
    */
    void attach(Debugger& debugger);

    void setBreakPoints(PackedBitArray* bp);
    void setTraps(PackedBitArray* read, PackedBitArray* write);

    unsigned int addCondBreak(Expression* e, const string& name);
    void delCondBreak(unsigned int brk);
    void clearCondBreaks();
    const StringList& getCondBreakNames() const;
    int evalCondBreaks();
#endif

  private:
    /**
      Get the byte at the specified address and update the cycle count.
      Addresses marked as code are hints to the debugger/disassembler to
      conclusively determine code sections, even if the disassembler cannot
      find them itself.

      @param address  The address from which the value should be loaded
      @param flags    Indicates that this address has the given flags
                      for type of access (CODE, DATA, GFX, etc)

      @return The byte at the specified address
    */
    uInt8 peek(uInt16 address, uInt8 flags);

    /**
      Change the byte at the specified address to the given value and
      update the cycle count.

      @param address  The address where the value should be stored
      @param value    The value to be stored at the address
    */
    void poke(uInt16 address, uInt8 value);

    /**
      Get the 8-bit value of the Processor Status register.

      @return The processor status register
    */
    uInt8 PS() const;

    /**
      Change the Processor Status register to correspond to the given value.

      @param ps The value to set the processor status register to
    */
    void PS(uInt8 ps);

    /**
      Called after an interrupt has be requested using irq() or nmi()
    */
    void interruptHandler();

  private:
    uInt8 A;    // Accumulator
    uInt8 X;    // X index register
    uInt8 Y;    // Y index register
    uInt8 SP;   // Stack Pointer
    uInt8 IR;   // Instruction register
    uInt16 PC;  // Program Counter

    bool N;     // N flag for processor status register
    bool V;     // V flag for processor status register
    bool B;     // B flag for processor status register
    bool D;     // D flag for processor status register
    bool I;     // I flag for processor status register
    bool notZ;  // Z flag complement for processor status register
    bool C;     // C flag for processor status register

    /** 
      Bit fields used to indicate that certain conditions need to be 
      handled such as stopping execution, fatal errors, maskable interrupts 
      and non-maskable interrupts (in myExecutionStatus)
    */
    enum 
    {
      StopExecutionBit = 0x01,
      FatalErrorBit = 0x02,
      MaskableInterruptBit = 0x04,
      NonmaskableInterruptBit = 0x08
    };
    uInt8 myExecutionStatus;
  
    /// Pointer to the system the processor is installed in or the null pointer
    System* mySystem;

    /// Indicates the number of system cycles per processor cycle 
    const uInt32 mySystemCyclesPerProcessorCycle;

    /// Table of system cycles for each instruction
    uInt32 myInstructionSystemCycleTable[256]; 

    /// Indicates if the last memory access was a read or not
    bool myLastAccessWasRead;

    /// The total number of instructions executed so far
    int myTotalInstructionCount;

    /// Indicates the numer of distinct memory accesses
    uInt32 myNumberOfDistinctAccesses;

    /// Indicates the last address which was accessed
    uInt16 myLastAddress;

    /// Indicates the last address which was accessed specifically
    /// by a peek or poke command
    uInt16 myLastPeekAddress, myLastPokeAddress;

    /// Indicates the last address used to access data by a peek command
    /// for the CPU registers (S/A/X/Y)
    uInt16 myLastSrcAddressS, myLastSrcAddressA,
           myLastSrcAddressX, myLastSrcAddressY;

    /// Indicates the data address used by the last command that performed
    /// a poke (currently, the last address used by STx)
    /// If an address wasn't used (ie, as in immediate mode), the address
    /// is set to zero
    uInt16 myDataAddressForPoke;

#ifdef DEBUGGER_SUPPORT
    /// Pointer to the debugger for this processor or the null pointer
    Debugger* myDebugger;

    PackedBitArray* myBreakPoints;
    PackedBitArray* myReadTraps;
    PackedBitArray* myWriteTraps;

    // Did we just now hit a trap?
    bool myJustHitTrapFlag;
    struct HitTrapInfo {
      string message;
      int address;
    };
    HitTrapInfo myHitTrapInfo;

    StringList myBreakCondNames;
    ExpressionList myBreakConds;
#endif

  private:
    /**
      Table of instruction processor cycle times.  In some cases additional 
      cycles will be added during the execution of an instruction.
    */
    static uInt32 ourInstructionCycleTable[256];
};

#endif
