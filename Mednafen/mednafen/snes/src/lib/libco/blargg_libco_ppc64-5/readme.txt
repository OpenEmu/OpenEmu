Run run_tests to test things fairly well, including the embedded and
separate versions, 32-bit and 64-bit, and with or without FP and vector
support. It will stop on the first error. Verifies that proper registers
are saved/restored, and that no others are modified, and lists incorrect
ones. Runs a bunch of threads and randomly creates/destroys, verifying
that they all switch properly. Then runs benchmark. I'd have included
one of the other portable versions, but they all crash.

* Supports PowerPC 32-bit and 64-bit. Detects based on compiler
settings.

* Uses embedded PowerPC code. To use separate ppc.S, define
LIBCO_PPC_ASM.

* Saves floating-point registers by default. To avoid this and improve
performance, define LIBCO_PPC_NOFP.

* Saves Altivec registers only if compiler is generating Altivec code.

* Assembly code merely saves current state to buffer pointed to by r4,
then restores from r3. This eliminates almost all dependency on the ABI,
stack format, etc. and is easier to get right. C code does all the
setup.

* C code attempts to avoid depending on whether ABI stores pointers as
32-bit or 64-bit (independently from whether it's a 64-bit PPC).

* Everything outside of libco/ is just for
developing/testing/benchmarking.

* Code even works on my old Mac OS Classic machine.

-- 
Shay Green <gblargg@gmail.com>
