class InputSettingsWindow : public QWidget {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QTreeWidget *list;
  QHBoxLayout *controlLayout;
  QLabel *message;
  QPushButton *optionButton;
  QMenu *optionMenu;
  QbCheckAction *optionAssignModifiers;
  QPushButton *assignButton;
  QPushButton *unassignButton;

  void inputEvent(uint16_t scancode);
  InputSettingsWindow();

private slots:
  void synchronize();
  void assign();
  void unassign();
  void toggleAssignModifiers();

private:
  QTreeWidgetItem *port1, *port2, *userInterface;
  array<MappedInput*> inputTable;
  MappedInput *activeInput;

  void updateList();
  void setAssignment(string);
};

extern InputSettingsWindow *inputSettingsWindow;
