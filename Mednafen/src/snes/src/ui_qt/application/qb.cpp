void QbWindow::setCloseOnEscape(bool state) {
  closeOnEscape = state;
}

void QbWindow::shrink() {
  if(config().video.isFullscreen == false) {
    for(unsigned i = 0; i < 2; i++) {
      resize(0, 0);
      usleep(2000);
      QApplication::processEvents();
    }
  }
}

void QbWindow::show() {
  if(isVisible() == false) {
    uint8_t *data;
    unsigned length;
    base64::decode(data, length, geometryString);
    QByteArray array((const char*)data, length);
    delete[] data;
    restoreGeometry(array);
    QWidget::show();
  }

  QApplication::processEvents();
  activateWindow();
  raise();
}

void QbWindow::hide() {
  if(isVisible() == true) {
    char *data;
    QByteArray geometry = saveGeometry();
    base64::encode(data, (const uint8_t*)geometry.data(), geometry.length());
    geometryString = data;
    delete[] data;
    QWidget::hide();
  }
}

void QbWindow::closeEvent(QCloseEvent *event) {
  char *data;
  QByteArray geometry = saveGeometry();
  base64::encode(data, (const uint8_t*)geometry.data(), geometry.length());
  geometryString = data;
  delete[] data;
  QWidget::hide();
}

void QbWindow::keyReleaseEvent(QKeyEvent *event) {
  if((closeOnEscape == true) && (event->key() == Qt::Key_Escape)) close();
  QWidget::keyReleaseEvent(event);
}

QbWindow::QbWindow(string &geometryString_) : geometryString(geometryString_) {
  closeOnEscape = true;

  //keep track of all created windows (for geometry save on exit, always-on-top control, etc)
  application.windowList.add(this);
}

//

bool QbCheckAction::isChecked() const {
  return checked;
}

void QbCheckAction::setChecked(bool checked_) {
  checked = checked_;
  if(checked) setIcon(QIcon(":/16x16/item-check-on.png"));
  else setIcon(QIcon(":/16x16/item-check-off.png"));
}

void QbCheckAction::toggleChecked() {
  setChecked(!isChecked());
}

QbCheckAction::QbCheckAction(const QString &text, QObject *parent) : QAction(text, parent) {
  setChecked(false);
}

//

bool QbRadioAction::isChecked() const {
  return checked;
}

void QbRadioAction::setChecked(bool checked_) {
  checked = checked_;
  if(checked) setIcon(QIcon(":/16x16/item-radio-on.png"));
  else setIcon(QIcon(":/16x16/item-radio-off.png"));
}

void QbRadioAction::toggleChecked() {
  setChecked(!isChecked());
}

QbRadioAction::QbRadioAction(const QString &text, QObject *parent) : QAction(text, parent) {
  setChecked(false);
}
