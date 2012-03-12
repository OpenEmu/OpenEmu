extern const UINT8 gunforce_decryption_table[];
extern const UINT8 bomberman_decryption_table[];
extern const UINT8 lethalth_decryption_table[];
extern const UINT8 dynablaster_decryption_table[];
extern const UINT8 mysticri_decryption_table[];
extern const UINT8 majtitl2_decryption_table[];
extern const UINT8 hook_decryption_table[];
extern const UINT8 rtypeleo_decryption_table[];
extern const UINT8 inthunt_decryption_table[];
extern const UINT8 gussun_decryption_table[];
extern const UINT8 leagueman_decryption_table[];
extern const UINT8 psoldier_decryption_table[];
extern const UINT8 dsoccr94_decryption_table[];
extern const UINT8 matchit2_decryption_table[];

extern const UINT8 test_decryption_table[];

extern void irem_cpu_decrypt(INT32 cpu,const UINT8 *decryption_table, UINT8 *src,UINT8 *dest, INT32 size);
