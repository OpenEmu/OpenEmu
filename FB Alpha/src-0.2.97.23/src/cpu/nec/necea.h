
static UINT32 EA;
static UINT16 EO;
static UINT16 E16;

static unsigned EA_000(nec_state_t *nec_state) { EO=Wreg(BW)+Wreg(IX); EA=DefaultBase(DS0)+EO; return EA; }
static unsigned EA_001(nec_state_t *nec_state) { EO=Wreg(BW)+Wreg(IY); EA=DefaultBase(DS0)+EO; return EA; }
static unsigned EA_002(nec_state_t *nec_state) { EO=Wreg(BP)+Wreg(IX); EA=DefaultBase(SS)+EO; return EA; }
static unsigned EA_003(nec_state_t *nec_state) { EO=Wreg(BP)+Wreg(IY); EA=DefaultBase(SS)+EO; return EA; }
static unsigned EA_004(nec_state_t *nec_state) { EO=Wreg(IX); EA=DefaultBase(DS0)+EO; return EA; }
static unsigned EA_005(nec_state_t *nec_state) { EO=Wreg(IY); EA=DefaultBase(DS0)+EO; return EA; }
static unsigned EA_006(nec_state_t *nec_state) { EO=FETCH(); EO+=FETCH()<<8; EA=DefaultBase(DS0)+EO; return EA; }
static unsigned EA_007(nec_state_t *nec_state) { EO=Wreg(BW); EA=DefaultBase(DS0)+EO; return EA; }

static unsigned EA_100(nec_state_t *nec_state) { EO=(Wreg(BW)+Wreg(IX)+(INT8)FETCH()); EA=DefaultBase(DS0)+EO; return EA; }
static unsigned EA_101(nec_state_t *nec_state) { EO=(Wreg(BW)+Wreg(IY)+(INT8)FETCH()); EA=DefaultBase(DS0)+EO; return EA; }
static unsigned EA_102(nec_state_t *nec_state) { EO=(Wreg(BP)+Wreg(IX)+(INT8)FETCH()); EA=DefaultBase(SS)+EO; return EA; }
static unsigned EA_103(nec_state_t *nec_state) { EO=(Wreg(BP)+Wreg(IY)+(INT8)FETCH()); EA=DefaultBase(SS)+EO; return EA; }
static unsigned EA_104(nec_state_t *nec_state) { EO=(Wreg(IX)+(INT8)FETCH()); EA=DefaultBase(DS0)+EO; return EA; }
static unsigned EA_105(nec_state_t *nec_state) { EO=(Wreg(IY)+(INT8)FETCH()); EA=DefaultBase(DS0)+EO; return EA; }
static unsigned EA_106(nec_state_t *nec_state) { EO=(Wreg(BP)+(INT8)FETCH()); EA=DefaultBase(SS)+EO; return EA; }
static unsigned EA_107(nec_state_t *nec_state) { EO=(Wreg(BW)+(INT8)FETCH()); EA=DefaultBase(DS0)+EO; return EA; }

static unsigned EA_200(nec_state_t *nec_state) { E16=FETCH(); E16+=FETCH()<<8; EO=Wreg(BW)+Wreg(IX)+(INT16)E16; EA=DefaultBase(DS0)+EO; return EA; }
static unsigned EA_201(nec_state_t *nec_state) { E16=FETCH(); E16+=FETCH()<<8; EO=Wreg(BW)+Wreg(IY)+(INT16)E16; EA=DefaultBase(DS0)+EO; return EA; }
static unsigned EA_202(nec_state_t *nec_state) { E16=FETCH(); E16+=FETCH()<<8; EO=Wreg(BP)+Wreg(IX)+(INT16)E16; EA=DefaultBase(SS)+EO; return EA; }
static unsigned EA_203(nec_state_t *nec_state) { E16=FETCH(); E16+=FETCH()<<8; EO=Wreg(BP)+Wreg(IY)+(INT16)E16; EA=DefaultBase(SS)+EO; return EA; }
static unsigned EA_204(nec_state_t *nec_state) { E16=FETCH(); E16+=FETCH()<<8; EO=Wreg(IX)+(INT16)E16; EA=DefaultBase(DS0)+EO; return EA; }
static unsigned EA_205(nec_state_t *nec_state) { E16=FETCH(); E16+=FETCH()<<8; EO=Wreg(IY)+(INT16)E16; EA=DefaultBase(DS0)+EO; return EA; }
static unsigned EA_206(nec_state_t *nec_state) { E16=FETCH(); E16+=FETCH()<<8; EO=Wreg(BP)+(INT16)E16; EA=DefaultBase(SS)+EO; return EA; }
static unsigned EA_207(nec_state_t *nec_state) { E16=FETCH(); E16+=FETCH()<<8; EO=Wreg(BW)+(INT16)E16; EA=DefaultBase(DS0)+EO; return EA; }

static unsigned (*const GetEA[192])(nec_state_t *)={
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,

	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,
	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,
	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,
	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,
	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,
	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,
	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,
	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,

	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207,
	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207,
	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207,
	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207,
	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207,
	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207,
	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207,
	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207
};
