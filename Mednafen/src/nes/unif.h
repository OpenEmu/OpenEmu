int UNL6035052_Init(CartInfo *info);


int BIC43_Init(CartInfo *info);
int BIC48_Init(CartInfo *info);
int BIC62_Init(CartInfo *info);
int BIC64_Init(CartInfo *info);

int AGCI50282_Init(CartInfo *info);
int BioMiracleA_Init(CartInfo *info);
int NINA06_Init(CartInfo *info);

int AOROM_Init(CartInfo *info);
int BTR_Init(CartInfo *info);

int TCU01_Init(CartInfo *info);
int S8259B_Init(CartInfo *info);
int S8259A_Init(CartInfo *info);
int S74LS374N_Init(CartInfo *info);
int SA0161M_Init(CartInfo *info);

int SA72007_Init(CartInfo *info);
int SA72008_Init(CartInfo *info);
int SA0036_Init(CartInfo *info);
int SA0037_Init(CartInfo *info);

int H2288_Init(CartInfo *info);
int UNL8237_Init(CartInfo *info);

int HKROM_Init(CartInfo *info);

int ETROM_Init(CartInfo *info);
int EKROM_Init(CartInfo *info);
int ELROM_Init(CartInfo *info);
int EWROM_Init(CartInfo *info);

int SAROM_Init(CartInfo *info);
int SBROM_Init(CartInfo *info);
int SCROM_Init(CartInfo *info);
int SEROM_Init(CartInfo *info);
int SGROM_Init(CartInfo *info);
int SKROM_Init(CartInfo *info);
int SLROM_Init(CartInfo *info);
int SL1ROM_Init(CartInfo *info);
int SNROM_Init(CartInfo *info);
int SOROM_Init(CartInfo *info);

int NROM_Init(CartInfo *info);
int NROM256_Init(CartInfo *info);
int NROM128_Init(CartInfo *info);
int MHROM_Init(CartInfo *info);
int UNROM_Init(CartInfo *info);
int MALEE_Init(CartInfo *info);
int Supervision16_Init(CartInfo *info);
int Super24_Init(CartInfo *info);
int Novel_Init(CartInfo *info);
int CNROM_Init(CartInfo *info);
int CPROM_Init(CartInfo *info);
int GNROM_Init(CartInfo *info);

int TEROM_Init(CartInfo *info);
int TFROM_Init(CartInfo *info);
int TGROM_Init(CartInfo *info);
int TKROM_Init(CartInfo *info);
int TSROM_Init(CartInfo *info);
int TLROM_Init(CartInfo *info);
int TLSROM_Init(CartInfo *info);
int TKSROM_Init(CartInfo *info);
int TQROM_Init(CartInfo *info);
int TQROM_Init(CartInfo *info);

int MMC4_Init(CartInfo *info);
int PNROM_Init(CartInfo *info);

extern uint8 *UNIFchrrama;	// Meh.  So I can stop CHR RAM 
	 			// bank switcherooing with certain boards...
