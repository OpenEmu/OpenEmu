class QbWindow : public QWidget {
public:
  void setCloseOnEscape(bool);
  void shrink();
  void show();
  void hide();
  void closeEvent(QCloseEvent*);
  void keyReleaseEvent(QKeyEvent*);
  QbWindow(string&);

private:
  string &geometryString;
  bool closeOnEscape;
};

class QbCheckAction : public QAction {
public:
  bool isChecked() const;
  void setChecked(bool);
  void toggleChecked();
  QbCheckAction(const QString&, QObject*);

private:
  bool checked;
};

class QbRadioAction : public QAction {
public:
  bool isChecked() const;
  void setChecked(bool);
  void toggleChecked();
  QbRadioAction(const QString&, QObject*);

private:
  bool checked;
};
