// Note: The size of each these structs shouldn't exceed 256 bytes(current worst-case is about 56 bytes with the 18x18 font).

typedef struct
{
        uint16 glyph_num;
        uint8 data[5];
} font4x5;

typedef struct
{
        uint16 glyph_num;
        uint8 data[7];
} font5x7;

typedef struct
{
        uint16 glyph_num;
        uint8 data[13];
} font6x13;

typedef struct
{
        uint16 glyph_num;
        uint8 data[18 * 2];
} font9x18;

extern const font4x5 FontData4x5[];
extern const font5x7 FontData5x7[];
extern const font6x13 FontData6x13[];
extern const font9x18 FontData9x18[];

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
        uint16_t glyph_num;
        uint8_t data[13 * 2];
} font12x13;

typedef struct
{
        uint16 glyph_num;
        uint8 data[18 * 3];
} font18x18;

extern font12x13 FontData12x13[];
extern font18x18 FontData18x18[];

extern const int FontData12x13_Size;
extern const int FontData18x18_Size;

#ifdef __cplusplus
} /* extern "C" */
#endif


extern const int FontData4x5_Size;
extern const int FontData5x7_Size;
extern const int FontData6x13_Size;
extern const int FontData9x18_Size;

