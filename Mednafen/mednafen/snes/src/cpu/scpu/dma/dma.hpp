  struct {
    //$420b
    bool   dma_enabled;

    //$420c
    bool   hdma_enabled;

    //$43x0
    uint8  dmap;
    bool   direction;
    bool   hdma_indirect;
    bool   reversexfer;
    bool   fixedxfer;
    uint8  xfermode;

    //$43x1
    uint8  destaddr;

    //$43x2-$43x3
    uint16 srcaddr;

    //$43x4
    uint8  srcbank;

    //$43x5-$43x6
    union {
      uint16 xfersize;
      uint16 hdma_iaddr;
    };

    //$43x7
    uint8  hdma_ibank;

    //$43x8-$43x9
    uint16 hdma_addr;

    //$43xa
    uint8  hdma_line_counter;

    //$43xb/$43xf
    uint8  unknown;

    //internal variables
    bool   hdma_completed;
    bool   hdma_do_transfer;
  } channel[8];

  void dma_add_clocks(unsigned clocks);
  bool dma_addr_valid(uint32 abus);
  uint8 dma_read(uint32 abus);
  void dma_transfer(bool direction, uint8 bbus, uint32 abus);

  uint8 dma_bbus(uint8 i, uint8 index);
  uint32 dma_addr(uint8 i);
  uint32 hdma_addr(uint8 i);
  uint32 hdma_iaddr(uint8 i);

  uint8 dma_enabled_channels();
  void dma_run();

  bool hdma_active(uint8 i);
  bool hdma_active_after(uint8 i);
  uint8 hdma_enabled_channels();
  uint8 hdma_active_channels();
  void hdma_update(uint8 i);
  void hdma_run();
  void hdma_init_reset();
  void hdma_init();

  void dma_power();
  void dma_reset();
