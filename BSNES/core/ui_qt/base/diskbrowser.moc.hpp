class PathSettingWidget;

class FolderCreator : public QbWindow {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QLabel *label;
  QLineEdit *name;
  QHBoxLayout *controlLayout;
  QPushButton *ok;
  QPushButton *cancel;

  FolderCreator();

public slots:
  void show();
  void createFolder();
} *folderCreator;

class DiskBrowserView : public QTreeView {
  Q_OBJECT

public:
  void keyPressEvent(QKeyEvent*);

signals:
  void cdUp();
  void changed(const QModelIndex&);

public slots:
  void currentChanged(const QModelIndex&, const QModelIndex&);
};

class DiskBrowserImage : public QWidget {
public:
  string name;
  void paintEvent(QPaintEvent*);
};

class DiskBrowser : public QbWindow {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QHBoxLayout *topLayout;
  QVBoxLayout *browseLayout;
  QHBoxLayout *pathLayout;
  QComboBox *path;
  QPushButton *newFolder;
  DiskBrowserView *view;
  QVBoxLayout *panelLayout;
  QGroupBox *group;
  QVBoxLayout *groupLayout;
  QLabel *info;
  DiskBrowserImage *image;
  QWidget *spacer;
  QCheckBox *applyPatch;
  QHBoxLayout *controlLayout;
  QComboBox *filter;
  QPushButton *options;
  QPushButton *ok;
  QPushButton *cancel;
  QFileSystemModel *model;
  PathSettingWidget *activePath;

  QMenu *menu;
  QbCheckAction *showPanel;

  void chooseFolder(PathSettingWidget*, const char*);
  void loadCartridge();
  void loadBaseCartridge();
  void loadBsxCartridge();
  void loadSufamiTurboCartridge1();
  void loadSufamiTurboCartridge2();
  void loadSuperGameBoyCartridge();
  string queryImageInformation();

  void setPath(const QString&);
  void setNameFilters(const QString&);

  DiskBrowser();

public slots:
  void cdUp();
  void updatePath();
  void updateFilter();
  void activateItem(const QModelIndex&);
  void changeItem(const QModelIndex&);
  void loadSelected();

  void toggleShowPanel();

private:
  enum BrowseMode {
    Folder,
    Cartridge,
    BaseCartridge,
    BsxCartridge,
    SufamiTurboCartridge1,
    SufamiTurboCartridge2,
    SuperGameBoyCartridge,
  } browseMode;

  bool currentFilename(string&);
} *diskBrowser;
