AboutWindow::AboutWindow() : QbWindow(config.geometry.aboutWindow) {
  setObjectName("about-window");
  setWindowTitle("About bsnes ...");

  layout = new QVBoxLayout;
  layout->setSizeConstraint(QLayout::SetFixedSize);
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  logo = new Logo;
  logo->setFixedSize(600, 106);
  layout->addWidget(logo);

  info = new QLabel(utf8() <<
    "<table>"
    "<tr><td align='right'><b>Version: </b></td><td>" << bsnesVersion << "</td></tr>"
    "<tr><td align='right'><b>Author: </b></td><td>byuu</td></tr>"
    "<tr><td align='right'><b>Homepage: </b></td><td>http://byuu.org/</td></tr>"
    "</table>"
  );
  layout->addWidget(info);
}

void AboutWindow::Logo::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  QPixmap pixmap(":/logo.png");
  painter.drawPixmap(0, 0, pixmap);
}
