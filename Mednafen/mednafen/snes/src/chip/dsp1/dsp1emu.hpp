// DSP-1's emulation code
//
// Based on research by Overload, The Dumper, Neviksti and Andreas Naive
// Date: June 2006

#ifndef __DSP1EMUL_H
#define __DSP1EMUL_H

#define DSP1_VERSION 0x0102

class Dsp1
{
   public:
      // The DSP-1 status register has 16 bits, but only
      // the upper 8 bits can be accessed from an external device, so all these
      // positions are referred to the upper byte (bits D8 to D15)
      enum SrFlags {DRC=0x04, DRS=0x10, RQM=0x80};

      // According to Overload's docs, these are the meanings of the flags:
      // DRC: The Data Register Control (DRC) bit specifies the data transfer length to and from the host CPU.
      //   0: Data transfer to and from the DSP-1 is 16 bits.
      //   1: Data transfer to and from the DSP-1 is 8 bits.
      // DRS: The Data Register Status (DRS) bit indicates the data transfer status in the case of transfering 16-bit data.
      //   0: Data transfer has terminated.
      //   1: Data transfer in progress.
      // RQM: The Request for Master (RQM) indicates that the DSP1 is requesting host CPU for data read/write.
      //   0: Internal Data Register Transfer.
      //   1: External Data Register Transfer.

      Dsp1();
      uint8 getSr();            // return the status register's high byte
      uint8 getDr();
      void setDr(uint8 iDr);
      void reset();

      void serialize(serializer&);

   private:
      enum FsmMajorState {WAIT_COMMAND, READ_DATA, WRITE_DATA};
      enum MaxDataAccesses {MAX_READS=7, MAX_WRITES=1024};

      struct Command {
         void (Dsp1::*callback)(int16 *, int16 *);
         unsigned int reads;
         unsigned int writes;
      };

      static const Command mCommandTable[];
      static const int16 MaxAZS_Exp[16];
      static const int16 SinTable[];
      static const int16 MulTable[];
      static const uint16 DataRom[];

      struct SharedData { // some RAM variables shared between commands
         int16 MatrixA[3][3];          // attitude matrix A
         int16 MatrixB[3][3];
         int16 MatrixC[3][3];
         int16 CentreX, CentreY, CentreZ;   // center of projection
         int16 CentreZ_C, CentreZ_E;
         int16 VOffset;                     // vertical offset of the screen with regard to the centre of projection
         int16 Les, C_Les, E_Les;
         int16 SinAas, CosAas;
         int16 SinAzs, CosAzs;
         int16 SinAZS, CosAZS;
         int16 SecAZS_C1, SecAZS_E1;
         int16 SecAZS_C2, SecAZS_E2;
         int16 Nx, Ny, Nz;    // normal vector to the screen (norm 1, points toward the center of projection)
         int16 Gx, Gy, Gz;    // center of the screen (global coordinates)
         int16 Hx, Hy;        // horizontal vector of the screen (Hz=0, norm 1, points toward the right of the screen)
         int16 Vx, Vy, Vz;    // vertical vector of the screen (norm 1, points toward the top of the screen) 

      } shared;

      uint8 mSr;            // status register
      int mSrLowByteAccess;
      uint16 mDr;           // "internal" representation of the data register
      unsigned mFsmMajorState;     // current major state of the FSM
      uint8 mCommand;                  // current command processed by the FSM
      uint8 mDataCounter;                 // #uint16 read/writes counter used by the FSM
      int16 mReadBuffer[MAX_READS];
      int16 mWriteBuffer[MAX_WRITES];
      bool mFreeze;                   // need explanation?  ;)

      void fsmStep(bool read, uint8 &data);            // FSM logic

      // commands
      void memoryTest(int16 *input, int16 *output);
      void memoryDump(int16 *input, int16 *output);
      void memorySize(int16 *input, int16 *output);
      void multiply(int16* input, int16* output);
      void multiply2(int16* input, int16* output);
      void inverse(int16 *input, int16 *output);
      void triangle(int16 *input, int16 *output);
      void radius(int16 *input, int16 *output);
      void range(int16 *input, int16 *output);
      void range2(int16 *input, int16 *output);
      void distance(int16 *input, int16 *output);
      void rotate(int16 *input, int16 *output);
      void polar(int16 *input, int16 *output);
      void attitudeA(int16 *input, int16 *output);
      void attitudeB(int16 *input, int16 *output);
      void attitudeC(int16 *input, int16 *output);
      void objectiveA(int16 *input, int16 *output);
      void objectiveB(int16 *input, int16 *output);
      void objectiveC(int16 *input, int16 *output);
      void subjectiveA(int16 *input, int16 *output);
      void subjectiveB(int16 *input, int16 *output);
      void subjectiveC(int16 *input, int16 *output);
      void scalarA(int16 *input, int16 *output);
      void scalarB(int16 *input, int16 *output);
      void scalarC(int16 *input, int16 *output);
      void gyrate(int16 *input, int16 *output);
      void parameter(int16 *input, int16 *output);
      void raster(int16 *input, int16 *output);
      void target(int16 *input, int16 *output);
      void project(int16 *input, int16 *output);

      // auxiliar functions
      int16 sin(int16 Angle);
      int16 cos(int16 Angle);
      void inverse(int16 Coefficient, int16 Exponent, int16 &iCoefficient, int16 &iExponent);
      int16 denormalizeAndClip(int16 C, int16 E);
      void normalize(int16 m, int16 &Coefficient, int16 &Exponent);
      void normalizeDouble(int32 Product, int16 &Coefficient, int16 &Exponent);
      int16 shiftR(int16 C, int16 E);
}; 

#endif

