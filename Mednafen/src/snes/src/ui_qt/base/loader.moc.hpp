class LoaderWindow : public QbWindow {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QGridLayout *grid;
  QLabel *baseLabel;
  QLineEdit *baseFile;
  QPushButton *baseBrowse;
  QPushButton *baseClear;
  QLabel *slot1Label;
  QLineEdit *slot1File;
  QPushButton *slot1Browse;
  QPushButton *slot1Clear;
  QLabel *slot2Label;
  QLineEdit *slot2File;
  QPushButton *slot2Browse;
  QPushButton *slot2Clear;
  QPushButton *load;
  QPushButton *cancel;

  void syncUi();
  void loadBsxSlottedCartridge(const char*, const char*);
  void loadBsxCartridge(const char*, const char*);
  void loadSufamiTurboCartridge(const char*, const char*, const char*);
  void loadSuperGameBoyCartridge(const char*, const char*);
  LoaderWindow();

  void selectBaseCartridge(const char*);
  void selectSlot1Cartridge(const char*);
  void selectSlot2Cartridge(const char*);

public slots:
  void selectBaseCartridge();
  void clearBaseCartridge();
  void selectSlot1Cartridge();
  void clearSlot1Cartridge();
  void selectSlot2Cartridge();
  void clearSlot2Cartridge();
  void onLoad();

private:
  SNES::Cartridge::Mode mode;
  void showWindow(const char *title);
};

extern LoaderWindow *loaderWindow;
