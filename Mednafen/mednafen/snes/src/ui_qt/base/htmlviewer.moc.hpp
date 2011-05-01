class HtmlViewerWindow : public QbWindow {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QTextBrowser *document;

  void show(const char *title, const char *htmlData);
  HtmlViewerWindow();

public slots:
};

extern HtmlViewerWindow *htmlViewerWindow;
