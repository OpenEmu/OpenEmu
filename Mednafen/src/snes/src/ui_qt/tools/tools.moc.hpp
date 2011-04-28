class ToolsWindow : public QbWindow {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QTabWidget *tab;

  ToolsWindow();

public slots:
};

extern ToolsWindow *toolsWindow;
