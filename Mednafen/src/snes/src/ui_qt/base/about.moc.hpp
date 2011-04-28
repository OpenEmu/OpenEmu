class AboutWindow : public QbWindow {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  struct Logo : public QWidget {
    void paintEvent(QPaintEvent*);
  } *logo;
  QLabel *info;

  AboutWindow();
};

extern AboutWindow *aboutWindow;
