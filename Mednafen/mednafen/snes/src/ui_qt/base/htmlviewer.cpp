#include "htmlviewer.moc"
HtmlViewerWindow *htmlViewerWindow;

HtmlViewerWindow::HtmlViewerWindow() : QbWindow(config().geometry.htmlViewerWindow) {
  setObjectName("html-window");
  resize(560, 480);

  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(0);
  setLayout(layout);

  document = new QTextBrowser;
  layout->addWidget(document);
}

void HtmlViewerWindow::show(const char *title, const char *htmlData) {
  document->setHtml(string() << htmlData);
  setWindowTitle(title);
  QbWindow::show();
}
