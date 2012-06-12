/******************************************************************************
*
* FILENAME: cart.h
*
* DESCRIPTION:  This contains function declartions for cart functions
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   02/05/07  bberlin      Creation
******************************************************************************/
#ifndef cart_h
#define cart_h

typedef enum {
	CART_MIN,
	CART_32K,
	CART_16K_TWO_CHIP,
	CART_16K_ONE_CHIP,
	CART_40K,
	CART_64K,
	CART_8K,
	CART_4K,
	CART_PC_8K,
	CART_PC_16K,
	CART_PC_32K,
	CART_PC_40K,
	CART_PC_RIGHT,
	CART_XEGS,
	CART_SWXEGS,
	CART_MEGA,
	CART_OSS,
	CART_OSS2,
	CART_ATMAX,
	CART_ATRAX,
	CART_WILL,
	CART_SDX,
	CART_EXP,
	CART_DIAMOND,
	CART_PHOENIX,
	CART_BLIZZARD,
	CART_CART,
	CART_FLOPPY,
	CART_CASSETTE,
	CART_EXE
} e_cart_type;

typedef struct {
	char filename[1024];
	e_cart_type mapping;
	int size;
	int loaded;
} t_cart;

int cart_open ( char *file, unsigned char *buffer, int size, unsigned long crc, 
                                                   e_cart_type mapping, int launch  );
char * cart_get_mapping_string ( e_cart_type type, char *desc );

#endif
