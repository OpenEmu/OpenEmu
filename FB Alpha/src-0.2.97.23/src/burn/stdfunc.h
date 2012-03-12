// Standard ROM/input/DIP info functions

// A function to pick a rom, or return NULL if i is out of range
#define STD_ROM_PICK(Name)												\
static struct BurnRomInfo* Name##PickRom(UINT32 i)						\
{																		\
	if (i >= sizeof(Name##RomDesc) / sizeof(Name##RomDesc[0])) {		\
		return NULL;													\
	}																	\
	return Name##RomDesc + i;											\
}

#define STDROMPICKEXT(Name, Info1, Info2)								\
static struct BurnRomInfo* Name##PickRom(UINT32 i)						\
{																		\
	if (i >= 0x80) {													\
		i &= 0x7F;														\
		if (i >= sizeof(Info2##RomDesc) / sizeof(Info2##RomDesc[0])) {	\
			return NULL;												\
		}																\
		return Info2##RomDesc + i;										\
	} else {															\
		if (i >= sizeof(Info1##RomDesc) / sizeof(Info1##RomDesc[0])) {	\
			return emptyRomDesc + 0;									\
		}																\
		return Info1##RomDesc + i;										\
	}																	\
}

// Standard rom functions for returning Length, Crc, Type and one one Name
#define STD_ROM_FN(Name)												\
static INT32 Name##RomInfo(struct BurnRomInfo* pri, UINT32 i)			\
{																		\
	struct BurnRomInfo* por = Name##PickRom(i);							\
	if (por == NULL) {													\
		return 1;														\
	}																	\
	if (pri) {															\
		pri->nLen = por->nLen;											\
		pri->nCrc = por->nCrc;											\
		pri->nType = por->nType;										\
	}																	\
	return 0;															\
}																		\
																		\
static INT32 Name##RomName(char** pszName, UINT32 i, INT32 nAka)		\
{											   		 					\
	struct BurnRomInfo *por = Name##PickRom(i);							\
	if (por == NULL) {													\
		return 1;														\
	}																	\
	if (nAka) {															\
		return 1;														\
	}																	\
	*pszName = por->szName;												\
	return 0;															\
}

#define STDINPUTINFO(Name)												\
static INT32 Name##InputInfo(struct BurnInputInfo* pii, UINT32 i)		\
{																		\
	if (i >= sizeof(Name##InputList) / sizeof(Name##InputList[0])) {	\
		return 1;														\
	}																	\
	if (pii) {															\
		*pii = Name##InputList[i];										\
	}																	\
	return 0;															\
}

#define STDINPUTINFOSPEC(Name, Info1)									\
static INT32 Name##InputInfo(struct BurnInputInfo* pii, UINT32 i)		\
{																		\
	if (i >= sizeof(Info1) / sizeof(Info1[0])) {						\
		return 1;														\
	}																	\
	if (pii) {															\
		*pii = Info1[i];												\
	}																	\
	return 0;															\
}

#define STDDIPINFO(Name)												\
static INT32 Name##DIPInfo(struct BurnDIPInfo* pdi, UINT32 i)			\
{																		\
	if (i >= sizeof(Name##DIPList) / sizeof(Name##DIPList[0])) {		\
		return 1;														\
	}																	\
	if (pdi) {															\
		*pdi = Name##DIPList[i];										\
	}																	\
	return 0;															\
}

#define STDDIPINFOEXT(Name, Info1, Info2)								\
static INT32 Name##DIPInfo(struct BurnDIPInfo* pdi, UINT32 i)			\
{																		\
	if (i >= sizeof(Info1##DIPList) / sizeof(Info1##DIPList[0])) {		\
		i -= sizeof(Info1##DIPList) / sizeof(Info1##DIPList[0]);		\
		if (i >= sizeof(Info2##DIPList) / sizeof(Info2##DIPList[0])) {	\
			return 1;													\
		}																\
		if (pdi) {														\
			*pdi = Info2##DIPList[i];									\
		}																\
		return 0;														\
	}																	\
	if (pdi) {															\
		*pdi = Info1##DIPList[i];										\
	}																	\
	return 0;															\
}

// sample support
#define STD_SAMPLE_PICK(Name)											\
static struct BurnSampleInfo* Name##PickSample(UINT32 i)				\
{																		\
	if (i >= sizeof(Name##SampleDesc) / sizeof(Name##SampleDesc[0])) {	\
		return NULL;													\
	}																	\
	return Name##SampleDesc + i;										\
}

#define STD_SAMPLE_FN(Name)												\
static INT32 Name##SampleInfo(struct BurnSampleInfo* pri, UINT32 i)		\
{																		\
	struct BurnSampleInfo* por = Name##PickSample(i);					\
	if (por == NULL) {													\
		return 1;														\
	}																	\
	if (pri) {															\
		pri->nFlags = por->nFlags;										\
	}																	\
	return 0;															\
}																		\
																		\
static INT32 Name##SampleName(char** pszName, UINT32 i, INT32 nAka)		\
{											   		 					\
	struct BurnSampleInfo *por = Name##PickSample(i);					\
	if (por == NULL) {													\
		return 1;														\
	}																	\
	if (nAka) {															\
		return 1;														\
	}																	\
	*pszName = por->szName;												\
	return 0;															\
}
