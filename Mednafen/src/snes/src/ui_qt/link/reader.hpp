class Reader : public library {
public:
  string compressionList;
  string extensionList;

  function<const char* ()> supported;
  function<bool (string&, uint8_t*&, unsigned&)> load;

  const char* direct_supported();
  bool direct_load(string&, uint8_t*&, unsigned&);

  Reader();
};

extern Reader reader;
