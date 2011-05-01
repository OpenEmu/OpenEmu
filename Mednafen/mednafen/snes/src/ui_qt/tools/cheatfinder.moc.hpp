class CheatFinderWindow : public QWidget {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QTreeWidget *list;
  QGridLayout *controlLayout;
  QLabel *sizeLabel;
  QButtonGroup *sizeGroup;
  QRadioButton *size8bit;
  QRadioButton *size16bit;
  QRadioButton *size24bit;
  QRadioButton *size32bit;
  QLabel *compareLabel;
  QButtonGroup *compareGroup;
  QRadioButton *compareEqual;
  QRadioButton *compareNotEqual;
  QRadioButton *compareLessThan;
  QRadioButton *compareGreaterThan;
  QLabel *valueLabel;
  QHBoxLayout *actionLayout;
  QLineEdit *valueEdit;
  QPushButton *searchButton;
  QPushButton *resetButton;

  void synchronize();
  void refreshList();
  CheatFinderWindow();

public slots:
  void searchMemory();
  void resetSearch();

private:
  array<unsigned> addrList;
  array<unsigned> dataList;

  unsigned read(unsigned addr, unsigned size);
};

extern CheatFinderWindow *cheatFinderWindow;
