#include "input.moc"
InputSettingsWindow *inputSettingsWindow;

InputSettingsWindow::InputSettingsWindow() {
  activeInput = 0;

  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  list = new QTreeWidget;
  list->setColumnCount(2);
  list->setAllColumnsShowFocus(true);
  list->setSortingEnabled(false);
  list->header()->hide();
  list->header()->setResizeMode(QHeaderView::ResizeToContents);
  layout->addWidget(list);

  controlLayout = new QHBoxLayout;
  layout->addLayout(controlLayout);

  message = new QLabel;
  message->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  controlLayout->addWidget(message);

  optionButton = new QPushButton("Options");
  controlLayout->addWidget(optionButton);

  optionMenu = new QMenu;
  optionButton->setMenu(optionMenu);

  optionAssignModifiers = new QbCheckAction("Assign Modifiers as Keys", 0);
  optionMenu->addAction(optionAssignModifiers);

  assignButton = new QPushButton("Assign");
  controlLayout->addWidget(assignButton);

  unassignButton = new QPushButton("Unassign");
  controlLayout->addWidget(unassignButton);

  connect(list, SIGNAL(itemSelectionChanged()), this, SLOT(synchronize()));
  connect(list, SIGNAL(itemActivated(QTreeWidgetItem*, int)), this, SLOT(assign()));
  connect(assignButton, SIGNAL(released()), this, SLOT(assign()));
  connect(unassignButton, SIGNAL(released()), this, SLOT(unassign()));

  connect(optionAssignModifiers, SIGNAL(triggered()), this, SLOT(toggleAssignModifiers()));

  //initialize list

  port1 = new QTreeWidgetItem(list);
  port1->setData(0, Qt::UserRole, QVariant(-1));
  port1->setText(0, "Controller Port 1");

  port2 = new QTreeWidgetItem(list);
  port2->setData(0, Qt::UserRole, QVariant(-1));
  port2->setText(0, "Controller Port 2");

  userInterface = new QTreeWidgetItem(list);
  userInterface->setData(0, Qt::UserRole, QVariant(-1));
  userInterface->setText(0, "User Interface");

  for(unsigned i = 0; i < mapper().size(); i++) {
    InputGroup &group = *(mapper()[i]);

    QTreeWidgetItem *grandparent = 0;
    if(group.category == InputCategory::Port1) { grandparent = port1; }
    if(group.category == InputCategory::Port2) { grandparent = port2; }
    if(group.category == InputCategory::UserInterface) { grandparent = userInterface; }
    if(!grandparent) continue;

    QTreeWidgetItem *parent = new QTreeWidgetItem(grandparent);
    parent->setData(0, Qt::UserRole, QVariant(-1));
    parent->setText(0, group.label);

    for(unsigned i = 0; i < group.size(); i++) {
      QTreeWidgetItem *child = new QTreeWidgetItem(parent);
      child->setData(0, Qt::UserRole, QVariant(inputTable.size()));
      inputTable.add(group[i]);
    }
  }

  updateList();
  synchronize();
}

void InputSettingsWindow::synchronize() {
  bool enable = false;
  QList<QTreeWidgetItem*> items = list->selectedItems();
  if(items.count() > 0) {
    QTreeWidgetItem *item = items[0];
    signed index = item->data(0, Qt::UserRole).toInt();
    enable = (index != -1);
  }
  assignButton->setEnabled(enable);
  unassignButton->setEnabled(enable);
}

void InputSettingsWindow::updateList() {
  QList<QTreeWidgetItem*> all = list->findItems("", Qt::MatchContains);
  for(unsigned i = 0; i < all.size(); i++) {
    QTreeWidgetItem *grandparent = all[i];
    for(unsigned j = 0; j < grandparent->childCount(); j++) {
      QTreeWidgetItem *parent = grandparent->child(j);
      for(unsigned k = 0; k < parent->childCount(); k++) {
        QTreeWidgetItem *child = parent->child(k);
        signed index = child->data(0, Qt::UserRole).toInt();
        if(index == -1) continue;
        MappedInput *input = inputTable[index];
        child->setText(0, input->label);
        child->setText(1, string() << "= " << input->name);
        child->setForeground(1, QBrush(QColor(128, 128, 128)));
      }
    }
  }
}

void InputSettingsWindow::setAssignment(string name) {
  activeInput->name = name;
  activeInput = 0;
  list->setFocus();
  message->setText("");
  mapper().bind();
  updateList();
}

void InputSettingsWindow::inputEvent(uint16_t scancode) {
  if(!activeInput) return;
  if(!isActiveWindow() || isMinimized()) return;
  int16_t state = mapper().state(scancode);

  if(dynamic_cast<DigitalInput*>(activeInput)) {
    if(Keyboard::isAnyKey(scancode) && mapper().state(scancode)) {
      for(unsigned i = 0; i < Keyboard::Count; i++) {
        //don't map escape key, as it is reserved by the user interface
        if(scancode == keyboard(i)[Keyboard::Escape]) return;
      }

      setAssignment(string() << mapper().modifierString() << Scancode::encode(scancode));
    } else if(Keyboard::isAnyModifier(scancode) && optionAssignModifiers->isChecked()) {
      setAssignment(string() << Scancode::encode(scancode));
    } else if(Mouse::isAnyButton(scancode) && mapper().state(scancode)) {
      //ensure button was clicked inside list box
      unsigned wx = 0, wy = 0;
      QWidget *widget = message;
      while(widget) {
        wx += widget->geometry().x();
        wy += widget->geometry().y();
        widget = widget->parentWidget();
      }
      unsigned px = QCursor::pos().x();
      unsigned py = QCursor::pos().y();
      if(px < wx || px >= wx + message->size().width()) return;
      if(py < wy || py >= wy + message->size().height()) return;

      setAssignment(string() << mapper().modifierString() << Scancode::encode(scancode));
    } else if(Joypad::isAnyHat(scancode)) {
      string position;
      if(state == Joypad::HatUp) position = ".Up";
      else if(state == Joypad::HatDown) position = ".Down";
      else if(state == Joypad::HatLeft) position = ".Left";
      else if(state == Joypad::HatRight) position = ".Right";
      else return;

      setAssignment(string() << mapper().modifierString() << Scancode::encode(scancode) << position);
    } else if(Joypad::isAnyAxis(scancode) && mapper().distance(scancode) > 64) {
      if(mapper().calibrated == false) {
        MappedInput *temp = activeInput;
        activeInput = 0;
        mapper().calibrate();
        activeInput = temp;
      }

      if(mapper().isTrigger[Joypad::numberDecode(scancode)][Joypad::axisDecode(scancode)] == false) {
        string position;
        if(state < -24576) position = ".Lo";
        else if(state > +24576) position = ".Hi";
        else return;

        setAssignment(string() << mapper().modifierString() << Scancode::encode(scancode) << position);
      } else {
        if(state >= 0) return;

        setAssignment(string() << mapper().modifierString() << Scancode::encode(scancode) << ".Trigger");
      }
    } else if(Joypad::isAnyButton(scancode) && mapper().state(scancode)) {
      setAssignment(string() << mapper().modifierString() << Scancode::encode(scancode));
    }
  } else if(dynamic_cast<AnalogInput*>(activeInput)) {
    if(Mouse::isAnyButton(scancode)) {
      //ensure button was clicked inside list box
      unsigned wx = 0, wy = 0;
      QWidget *widget = message;
      while(widget) {
        wx += widget->geometry().x();
        wy += widget->geometry().y();
        widget = widget->parentWidget();
      }
      unsigned px = QCursor::pos().x();
      unsigned py = QCursor::pos().y();
      if(px < wx || px >= wx + message->size().width()) return;
      if(py < wy || py >= wy + message->size().height()) return;

      unsigned number = Mouse::numberDecode(scancode);
      unsigned button = Mouse::buttonDecode(scancode);
      if(button == 0) setAssignment(string() << Scancode::encode(mouse(number).axis(0)));
      if(button == 2) setAssignment(string() << Scancode::encode(mouse(number).axis(1)));
    } else if(Joypad::isAnyAxis(scancode) && mapper().distance(scancode) > 64) {
      if(mapper().calibrated == false) {
        MappedInput *temp = activeInput;
        activeInput = 0;
        mapper().calibrate();
        activeInput = temp;
      }

      if(mapper().isTrigger[Joypad::numberDecode(scancode)][Joypad::axisDecode(scancode)] == false) {
        if(state < -24576 || state > +24576) {
          setAssignment(string() << Scancode::encode(scancode));
        }
      }
    }
  }
}

void InputSettingsWindow::assign() {
  QTreeWidgetItem *item = list->currentItem();
  if(!item) return;
  signed index = item->data(0, Qt::UserRole).toInt();
  if(index == -1) return;

  //flush any pending events to prevent instantaneous assignment of scancodes
  mapper().poll();

  activeInput = inputTable[index];
  message->setFocus();
  message->setText(string() << "Set assignment for: " << activeInput->label);
}

void InputSettingsWindow::unassign() {
  QTreeWidgetItem *item = list->currentItem();
  if(!item) return;
  signed index = item->data(0, Qt::UserRole).toInt();
  if(index == -1) return;

  MappedInput *input = inputTable[index];
  input->name = "None";
  mapper().bind();
  updateList();
}

void InputSettingsWindow::toggleAssignModifiers() {
  optionAssignModifiers->setChecked(!optionAssignModifiers->isChecked());
}
