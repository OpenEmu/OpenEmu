#ifndef _MDFN_SETTINGS_COMMON_H
#define _MDFN_SETTINGS_COMMON_H
typedef enum
{
	// Actual base types
        MDFNST_INT = 0,     // (signed), int8, int16, int32, int64(saved as)
        MDFNST_UINT,    // uint8, uint16, uint32, uint64(saved as)
        MDFNST_BOOL,    // bool. bool. bool!
        MDFNST_FLOAT,   // float, double(saved as).
	MDFNST_STRING,
	MDFNST_BASE_MASK = 0xFFFF,

	// OR'd with base type to express additional information.
	MDFNST_EX_DRIVER = (1 << 16),	 // If this is not set, the setting is assumed to be internal.  This...should probably be set automatically?
        MDFNST_EX_EMU_STATE = (1 << 17), // If the setting affects emulation from the point of view of the emulated program
	MDFNST_EX_UNTRUSTED_SAFE = (1 << 18), // If it's safe for an untrusted source to modify it, probably only used in conjunction with 
					      // MDFNST_EX_EMU_STATE and network play
} MDFNSettingType;


typedef struct
{
        const char *name;
        const char *description; // Short
        MDFNSettingType type;
        const char *default_value;
	const char *minimum;
	const char *maximum;
	bool (*validate_func)(const char *name, const char *value);
	void (*ChangeNotification)(const char *name);
} MDFNSetting;

typedef struct __MDFNCS
{
        char *name;
        char *value;
        char *netplay_override; // "value" override for network play.

	const MDFNSetting *desc;
        uint64 name_hash;
	void (*ChangeNotification)(const char *name);
} MDFNCS;

#endif
