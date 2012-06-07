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
	MDFNST_ENUM,	// Handled like a string, but validated against the enumeration list, and MDFN_GetSettingUI() returns the number in the enumeration list.

	MDFNST_ALIAS
} MDFNSettingType;


//#define MDFNST_EX_DRIVER = (1 << 16),    // If this is not set, the setting is assumed to be internal.  This...should probably be set automatically?
        
#define MDFNSF_NOFLAGS		0	  // Always 0, makes setting definitions prettier...maybe.

// TODO(cats)
#define MDFNSF_CAT_INPUT        (1 << 8)
#define MDFNSF_CAT_SOUND	(1 << 9)
#define MDFNSF_CAT_VIDEO	(1 << 10)

#define MDFNSF_EMU_STATE	(1 << 17) // If the setting affects emulation from the point of view of the emulated program
#define MDFNSF_UNTRUSTED_SAFE	(1 << 18) // If it's safe for an untrusted source to modify it, probably only used in conjunction with
                                          // MDFNST_EX_EMU_STATE and network play

#define MDFNSF_SUPPRESS_DOC	(1 << 19) // Suppress documentation generation for this setting.
#define MDFNSF_COMMON_TEMPLATE	(1 << 20) // Auto-generated common template setting(like nes.xscale, pce.xscale, vb.xscale, nes.enable, pce.enable, vb.enable)
// TODO:
// #define MDFNSF_WILL_BREAK_GAMES (1 << ) // If changing the value of the setting from the default value will break games/programs that would otherwise work.

// TODO(in progress):
#define MDFNSF_REQUIRES_RELOAD	(1 << 24)	// If a game reload is required for the setting to take effect.
#define MDFNSF_REQUIRES_RESTART	(1 << 25)	// If Mednafen restart is required for the setting to take effect.

typedef struct
{
	const char *string;
	int number;
	const char *description;	// Short
	const char *description_extra;	// Extra verbose text appended to the short description.
} MDFNSetting_EnumList;

typedef struct
{
        const char *name;
	uint32 flags;
        const char *description; // Short
	const char *description_extra;

        MDFNSettingType type;
        const char *default_value;
	const char *minimum;
	const char *maximum;
	bool (*validate_func)(const char *name, const char *value);
	void (*ChangeNotification)(const char *name);
	const MDFNSetting_EnumList *enum_list;
} MDFNSetting;

typedef struct __MDFNCS
{
        char *name;
        char *value;
	char *game_override;    // per-game setting override(netplay_override > game_override > value, in precedence)
        char *netplay_override; // "value" override for network play.

	const MDFNSetting *desc;
	void (*ChangeNotification)(const char *name);

        uint32 name_hash;
} MDFNCS;

#endif
