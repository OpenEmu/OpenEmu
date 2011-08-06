class sBus : public Bus {
public:
  bool load_cart();
  void unload_cart();

  void power();
  void reset();

  void serialize(serializer&);
  sBus();
  ~sBus();

private:
  void map_reset();
  void map_system();
  void map_generic();
  void map_generic_sram();
};

extern sBus bus;
