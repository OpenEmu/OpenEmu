uint8 mmio_read(unsigned addr);
void mmio_write(unsigned addr, uint8 data);
Memory& mmio_access(unsigned &addr);

struct MMIO {
  //$2200 CCNT
  bool sa1_irq;
  bool sa1_rdyb;
  bool sa1_resb;
  bool sa1_nmi;
  uint8 smeg;

  //$2201 SIE
  bool cpu_irqen;
  bool chdma_irqen;

  //$2202 SIC
  bool cpu_irqcl;
  bool chdma_irqcl;

  //$2203,$2204 CRV
  uint16 crv;

  //$2205,$2206 CNV
  uint16 cnv;

  //$2207,$2208 CIV
  uint16 civ;

  //$2209 SCNT
  bool cpu_irq;
  bool cpu_ivsw;
  bool cpu_nvsw;
  uint8 cmeg;

  //$220a CIE
  bool sa1_irqen;
  bool timer_irqen;
  bool dma_irqen;
  bool sa1_nmien;

  //$220b CIC
  bool sa1_irqcl;
  bool timer_irqcl;
  bool dma_irqcl;
  bool sa1_nmicl;

  //$220c,$220d SNV
  uint16 snv;

  //$220e,$220f SIV
  uint16 siv;

  //$2210 TMC
  bool hvselb;
  bool ven;
  bool hen;

  //$2212,$2213
  uint16 hcnt;

  //$2214,$2215
  uint16 vcnt;

  //$2220 CXB
  bool cbmode;
  uint8 cb;

  //$2221 DXB
  bool dbmode;
  uint8 db;

  //$2222 EXB
  bool ebmode;
  uint8 eb;

  //$2223 FXB
  bool fbmode;
  uint8 fb;

  //$2224 BMAPS
  uint8 sbm;

  //$2225 BMAP
  bool sw46;
  uint8 cbm;

  //$2226 SBWE
  bool swen;

  //$2227 CBWE
  bool cwen;

  //$2228 BWPA
  uint8 bwp;

  //$2229 SIWP
  uint8 siwp;

  //$222a CIWP
  uint8 ciwp;

  //$2230 DCNT
  bool dmaen;
  bool dprio;
  bool cden;
  bool cdsel;
  bool dd;
  uint8 sd;

  //$2231 CDMA
  bool chdend;
  uint8 dmasize;
  uint8 dmacb;

  //$2232-$2234 SDA
  uint32 dsa;

  //$2235-$2237 DDA
  uint32 dda;

  //$2238,$2239 DTC
  uint16 dtc;

  //$223f BBF
  bool bbf;

  //$2240-224f BRF
  uint8 brf[16];

  //$2250 MCNT
  bool acm;
  bool md;

  //$2251,$2252 MA
  uint16 ma;

  //$2253,$2254 MB
  uint16 mb;

  //$2258 VBD
  bool hl;
  uint8 vb;

  //$2259-$225b VDA
  uint32 va;
  uint8 vbit;

  //$2300 SFR
  bool cpu_irqfl;
  bool chdma_irqfl;

  //$2301 CFR
  bool sa1_irqfl;
  bool timer_irqfl;
  bool dma_irqfl;
  bool sa1_nmifl;

  //$2302,$2303 HCR
  uint16 hcr;

  //$2304,$2305 VCR
  uint16 vcr;

  //$2306-230a MR
  uint64 mr;

  //$230b OF
  bool overflow;
} mmio;

void mmio_w2200(uint8);  //CCNT
void mmio_w2201(uint8);  //SIE
void mmio_w2202(uint8);  //SIC
void mmio_w2203(uint8);  //CRVL
void mmio_w2204(uint8);  //CRVH
void mmio_w2205(uint8);  //CNVL
void mmio_w2206(uint8);  //CNVH
void mmio_w2207(uint8);  //CIVL
void mmio_w2208(uint8);  //CIVH
void mmio_w2209(uint8);  //SCNT
void mmio_w220a(uint8);  //CIE
void mmio_w220b(uint8);  //CIC
void mmio_w220c(uint8);  //SNVL
void mmio_w220d(uint8);  //SNVH
void mmio_w220e(uint8);  //SIVL
void mmio_w220f(uint8);  //SIVH
void mmio_w2210(uint8);  //TMC
void mmio_w2211(uint8);  //CTR
void mmio_w2212(uint8);  //HCNTL
void mmio_w2213(uint8);  //HCNTH
void mmio_w2214(uint8);  //VCNTL
void mmio_w2215(uint8);  //VCNTH
void mmio_w2220(uint8);  //CXB
void mmio_w2221(uint8);  //DXB
void mmio_w2222(uint8);  //EXB
void mmio_w2223(uint8);  //FXB
void mmio_w2224(uint8);  //BMAPS
void mmio_w2225(uint8);  //BMAP
void mmio_w2226(uint8);  //SBWE
void mmio_w2227(uint8);  //CBWE
void mmio_w2228(uint8);  //BWPA
void mmio_w2229(uint8);  //SIWP
void mmio_w222a(uint8);  //CIWP
void mmio_w2230(uint8);  //DCNT
void mmio_w2231(uint8);  //CDMA
void mmio_w2232(uint8);  //SDAL
void mmio_w2233(uint8);  //SDAH
void mmio_w2234(uint8);  //SDAB
void mmio_w2235(uint8);  //DDAL
void mmio_w2236(uint8);  //DDAH
void mmio_w2237(uint8);  //DDAB
void mmio_w2238(uint8);  //DTCL
void mmio_w2239(uint8);  //DTCH
void mmio_w223f(uint8);  //BBF
void mmio_w2240(uint8);  //BRF0
void mmio_w2241(uint8);  //BRF1
void mmio_w2242(uint8);  //BRF2
void mmio_w2243(uint8);  //BRF3
void mmio_w2244(uint8);  //BRF4
void mmio_w2245(uint8);  //BRF5
void mmio_w2246(uint8);  //BRF6
void mmio_w2247(uint8);  //BRF7
void mmio_w2248(uint8);  //BRF8
void mmio_w2249(uint8);  //BRF9
void mmio_w224a(uint8);  //BRFA
void mmio_w224b(uint8);  //BRFB
void mmio_w224c(uint8);  //BRFC
void mmio_w224d(uint8);  //BRFD
void mmio_w224e(uint8);  //BRFE
void mmio_w224f(uint8);  //BRFF
void mmio_w2250(uint8);  //MCNT
void mmio_w2251(uint8);  //MAL
void mmio_w2252(uint8);  //MAH
void mmio_w2253(uint8);  //MBL
void mmio_w2254(uint8);  //MBH
void mmio_w2258(uint8);  //VBD
void mmio_w2259(uint8);  //VDAL
void mmio_w225a(uint8);  //VDAH
void mmio_w225b(uint8);  //VDAB

uint8 mmio_r2300();  //SFR
uint8 mmio_r2301();  //CFR
uint8 mmio_r2302();  //HCRL
uint8 mmio_r2303();  //HCRH
uint8 mmio_r2304();  //VCRL
uint8 mmio_r2305();  //VCRH
uint8 mmio_r2306();  //MR [00-07]
uint8 mmio_r2307();  //MR [08-15]
uint8 mmio_r2308();  //MR [16-23]
uint8 mmio_r2309();  //MR [24-31]
uint8 mmio_r230a();  //MR [32-40]
uint8 mmio_r230b();  //OF
uint8 mmio_r230c();  //VDPL
uint8 mmio_r230d();  //VDPH
uint8 mmio_r230e();  //VC
