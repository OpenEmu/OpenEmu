class SettingsWindow : public QbWindow {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QTabWidget *tab;

  SettingsWindow();

public slots:
};

extern SettingsWindow *settingsWindow;
