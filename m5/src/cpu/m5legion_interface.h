
#include <unistd.h>

#define VERSION         0xA1000009
#define OWN_M5          0x000000AA
#define OWN_LEGION      0x00000055

/** !!!  VVV Increment VERSION on change VVV !!! **/

typedef struct {
    uint32_t flags;
    uint32_t version;

    uint64_t pc;
    uint64_t new_pc;
    uint64_t cycle_count;
    uint64_t new_cycle_count;
    uint32_t instruction;
    uint32_t new_instruction;
    uint64_t intregs[32];
    uint64_t fpregs[32];

    uint64_t tpc[8];
    uint64_t tnpc[8];
    uint64_t tstate[8];
    uint16_t tt[8];
    uint64_t tba;

    uint64_t hpstate;
    uint64_t htstate[8];
    uint64_t htba;
    uint16_t pstate;

    uint64_t y;
    uint64_t fsr;
    uint8_t ccr;
    uint8_t tl;
    uint8_t gl;
    uint8_t asi;
    uint8_t pil;

    uint8_t cwp;
    uint8_t cansave;
    uint8_t canrestore;
    uint8_t otherwin;
    uint8_t cleanwin;

    uint64_t itb[64];
    uint64_t dtb[64];

} SharedData;

/** !!! ^^^  Increment VERSION on change ^^^ !!! **/

