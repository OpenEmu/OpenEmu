InputCaptureWindow::InputCaptureWindow() : QbWindow(config.geometry.inputCaptureWindow) {
  activeObject = 0;
  activeGroup = 0;
  groupIndex = 0;
  inputLock = false;

  setObjectName("input-capture-window");
  setWindowTitle("Input Capture");

  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(0);
  layout->setAlignment(Qt::AlignTop);
  setLayout(layout);

  hlayout = new QHBoxLayout;
  hlayout->setSpacing(Style::WidgetSpacing); {
    title = new QLabel;
    hlayout->addWidget(title, 0, Qt::AlignTop);

    mouseAxes = new QPushButton("  Assign Mouse Axis  ");
    mouseAxes->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    hlayout->addWidget(mouseAxes, 0, Qt::AlignTop);

    mouseButtons = new QPushButton("  Assign Mouse Button  ");
    mouseButtons->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    hlayout->addWidget(mouseButtons, 0, Qt::AlignTop);
  }
  layout->addLayout(hlayout);

  imageSpacer = new QWidget;
  imageSpacer->setFixedSize(Style::WidgetSpacing, Style::WidgetSpacing);
  layout->addWidget(imageSpacer);

  imageWidget = new ImageWidget;
  layout->addWidget(imageWidget, 0, Qt::AlignHCenter);

  connect(mouseAxes, SIGNAL(released()), this, SLOT(assignMouseAxis()));
  connect(mouseButtons, SIGNAL(released()), this, SLOT(assignMouseButton()));

  inputMouseCaptureWindow = new InputMouseCaptureWindow;
  inputCalibrationWindow = new InputCalibrationWindow;
}

void InputCaptureWindow::activate(InputObject *object) {
  if(!activeGroup) hide();

  utf8 info;
  info << "<b>ID:</b> ";
  if(object->parent) {
    InputDevice *device = dynamic_cast<InputDevice*>(object->parent);
    if(device) info << "Controller port " << (int)(device->port + 1) << " <b>::</b> ";
    else info << "User interface <b>::</b> ";
    info << object->parent->name << " <b>::</b> ";
  }
  info << object->name << "<br>";

  activeObject = object;
  if(activeObject->type == InputObject::Button) {
    mouseAxes->hide();
    mouseButtons->show();

    info << "Press any key or button to assign to this ID.";
  } else /*(activeObject->type == InputObject::Axis)*/ {
    mouseAxes->show();
    mouseButtons->hide();

    info << "Move any axis to assign to this ID.";
  }

  if(dynamic_cast<Joypad*>(activeObject->parent)) {
    imageSpacer->show();
    imageWidget->setFixedSize(480, 210);
    imageWidget->show();
  } else {
    imageSpacer->hide();
    imageWidget->hide();
  }

  title->setText(info);
  show();
  shrink();
}

void InputCaptureWindow::activate(InputGroup *group) {
  activeGroup = group;
  groupIndex = 0;
  hide();
  activate((*activeGroup)[groupIndex]);
}

void InputCaptureWindow::inputEvent(uint16_t code, bool forceAssign /* = false */) {
  if(!activeObject || inputLock == true) return;

  //input polling is global, need to block mouse actions that may be UI interactions.
  //custom controls on window allow mouse assignment instead.
  if(forceAssign == false) {
    if(inputMouseCaptureWindow->isActiveWindow()) {
      inputMouseCaptureWindow->inputEvent(code);
      return;
    }
    if(!isActiveWindow()) return;

    //get as much info as possible about this code
    InputCode::type_t type = InputCode::type(code);
    InputCode::axistype_t axisType = InputCode::axisType(code);
    int joypadNumber = InputCode::joypadNumber(code);
    int16_t state = inputManager.state(code);
    unsigned distance = abs(state - inputManager.lastState(code));

    if(type == InputCode::JoypadHat) {
      //hats can be in any of nine clock-wise states (4x direct, 4x angled and centered.)
      //only map when hat is moved to an explicit direction.
      if(state != joypad<>::hat_up   && state != joypad<>::hat_down
      && state != joypad<>::hat_left && state != joypad<>::hat_right) return;
    }

    if(type == InputCode::JoypadAxis) {
      //require a degree of resistance to prevent accidental mapping
      if(axisType == InputCode::Stick) {
        //require axis to be pressed almost completely toward a specific direction
        if(state > -28672 && state < +28672) return;
      } else if(axisType == InputCode::Trigger) {
        //require trigger to be at least 75% pressed down
        if(state > -16384) return;
      } else {
        //invalid axis type: most likely the controller has yet to be calibrated
        if(joypadNumber < 0) return;  //should never occur

        //some analog triggers report phantom motion even when user does not touch gamepad
        if(distance < 64) return;  //require some degree of force to trigger calibration
        if(state > -8192 && state < +8192) return;  //ignore center motion

        if(inputManager.calibrated(joypadNumber) == false) {
          inputCalibrationWindow->activate(joypadNumber);
        }

        //block assignment until controller is fully calibrated
        return;
      }
    }

    if(activeObject->type == InputObject::Axis) {
      if(type == InputCode::KeyboardButton
      || type == InputCode::MouseAxis
      || type == InputCode::MouseButton
      || type == InputCode::JoypadHat
      || type == InputCode::JoypadButton
      ) return;

      //uni-directional trigger cannot map to bi-directional axis
      if(type == InputCode::JoypadAxis && axisType == InputCode::Trigger) return;
    }

    if(activeObject->type == InputObject::Button) {
      if(type == InputCode::MouseAxis
      || type == InputCode::MouseButton
      ) return;

      //only capture on button press, not release
      if(type != InputCode::JoypadAxis && state == false) return;
    }

    //if assigning a complete controller set, ensure requested key has not been assigned
    //to a previous entry in the group already. this prevents slow motion of joypad axes
    //from assigning the same value to multiple entries in rapid succession.
    if(activeGroup && groupIndex > 0) {
      for(unsigned i = 0; i < groupIndex; i++) {
        if(code == (*activeGroup)[i]->code) {
          //joypad hats and axes have multiple states, and are differentiated by modifier.
          //allow mapping only if requested modifier is unique.
          if(type == InputCode::JoypadHat) {
            if(state == joypad<>::hat_up    && (*activeGroup)[i]->modifier == InputObject::Up   ) return;
            if(state == joypad<>::hat_down  && (*activeGroup)[i]->modifier == InputObject::Down ) return;
            if(state == joypad<>::hat_left  && (*activeGroup)[i]->modifier == InputObject::Left ) return;
            if(state == joypad<>::hat_right && (*activeGroup)[i]->modifier == InputObject::Right) return;
          } else if(type == InputCode::JoypadAxis) {
            if(axisType == InputCode::Stick) {
              if(state <  0 && (*activeGroup)[i]->modifier == InputObject::Lo) return;
              if(state >= 0 && (*activeGroup)[i]->modifier == InputObject::Hi) return;
            } else if(axisType == InputCode::Trigger) {
              if((*activeGroup)[i]->modifier == InputObject::Trigger) return;
            }
          } else {
            //this code has already been used, do not map it
            return;
          }
        }
      }
    }
  }

  //bind code and update GUI input assignment list
  activeObject->bind(code);
  inputSettingsWindow->updateList();
  activeObject = 0;

  //ignore multiple simultaneous state changes.
  //this helps with joypads that only activate
  //analog inputs after the first button press.
  inputLock = true;
  for(unsigned i = 0; i < 2; i++) inputManager.refresh();
  inputLock = false;

  if(!activeGroup) {
    hide();
    inputMouseCaptureWindow->hide();
  } else {
    //try and map the next code in this input group
    groupIndex++;
    if(groupIndex < activeGroup->size()) {
      activate((*activeGroup)[groupIndex]);
    } else {
      //all group codes mapped
      hide();
      inputMouseCaptureWindow->hide();
      activeGroup = 0;
    }
  }
}

void InputCaptureWindow::closeEvent(QCloseEvent *event) {
  QbWindow::closeEvent(event);

  //window closed by user, cancel key assignment
  activeObject = 0;
  activeGroup = 0;

  inputMouseCaptureWindow->hide();
  inputCalibrationWindow->dismiss();
}

void InputCaptureWindow::assignMouseAxis() {
  //refresh input state so that mouse release event (from SIGNAL(released())
  //is not sent immediately after window is visible.
  inputManager.refresh();
  inputMouseCaptureWindow->activate(InputMouseCaptureWindow::AxisMode);
}

void InputCaptureWindow::assignMouseButton() {
  inputManager.refresh();
  inputMouseCaptureWindow->activate(InputMouseCaptureWindow::ButtonMode);
}

void InputCaptureWindow::ImageWidget::paintEvent(QPaintEvent*) {
  //currently, there is only an image available for the joypad.
  //in the future, this routine should determine which type of
  //image to draw via activeObject->parent's derived class type.
  QPainter painter(this);
  QPixmap pixmap(":/joypad.png");
  painter.drawPixmap(0, 0, pixmap);
}

//=======================
//InputMouseCaptureWindow
//=======================

InputMouseCaptureWindow::InputMouseCaptureWindow() : QbWindow(config.geometry.inputMouseCaptureWindow) {
  setObjectName("input-mouse-capture-window");
  setWindowTitle("Mouse Input Capture");

  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(0);
  layout->setAlignment(Qt::AlignTop);
  setLayout(layout);

  info = new QLabel;
  layout->addWidget(info);
  layout->addSpacing(Style::WidgetSpacing);

  captureBox = new QLabel("[ capture box ]");
  captureBox->setObjectName("mouse-capture-box");
  captureBox->setAlignment(Qt::AlignCenter);
  captureBox->setFixedHeight(120);
  layout->addWidget(captureBox);

  buttonLayout = new QHBoxLayout;
  buttonLayout->setSpacing(Style::WidgetSpacing);
  buttonLayout->setAlignment(Qt::AlignRight);
  layout->addLayout(buttonLayout);

  xAxis = new QPushButton("X-axis");
  buttonLayout->addWidget(xAxis);

  yAxis = new QPushButton("Y-axis");
  buttonLayout->addWidget(yAxis);

  connect(xAxis, SIGNAL(released()), this, SLOT(assignAxisX()));
  connect(yAxis, SIGNAL(released()), this, SLOT(assignAxisY()));
}

void InputMouseCaptureWindow::activate(InputMouseCaptureWindow::Mode mode_) {
  hide();
  activeMode = mode_;

  if(activeMode == AxisMode) {
    captureBox->hide();
    xAxis->show();
    yAxis->show();

    info->setText(utf8()
    << "To assign a mouse axis to this ID, please click the desired axis button<br>"
    << "below, using the mouse that you want the axis to be assigned to."
    );

    activeMouse = -1;
  } else /*(activeMode == ButtonMode) */ {
    captureBox->show();
    xAxis->hide();
    yAxis->hide();

    info->setText(utf8()
    << "To assign a mouse button to this ID, please place the mouse you wish to map<br>"
    << "over the capture box below, and then click any mouse button to set assignment."
    );
  }

  show();
}

//this is only called when isActiveWindow() == true
void InputMouseCaptureWindow::inputEvent(uint16_t code) {
  InputCode::type_t type = InputCode::type(code);
  int16_t state = inputManager.state(code);

  if(activeMode == AxisMode) {
    //when pressing down mouse button (eg to select "X-axis" or "Y-axis"),
    //record mouse index for later assignment
    if(type == InputCode::MouseButton && state == true) {
      activeMouse = InputCode::mouseNumber(code);
      return;
    }
  } else if(activeMode == ButtonMode) {
    //if this is a mouse button that is being released ...
    if(type == InputCode::MouseButton && state == false) {
      //ensure button was clicked inside active capture box
      QRect windowRect = geometry();
      QRect widgetRect = captureBox->geometry();
      unsigned wx = windowRect.left() + widgetRect.left();
      unsigned wy = windowRect.top() + widgetRect.top();
      unsigned px = QCursor::pos().x();
      unsigned py = QCursor::pos().y();

      if(px < wx || px >= wx + widgetRect.size().width() ) return;
      if(py < wy || py >= wy + widgetRect.size().height()) return;

      inputCaptureWindow->inputEvent(code, true);
      return;
    }
  }
}

void InputMouseCaptureWindow::assignAxisX() {
  if(activeMouse >= 0) {
    inputCaptureWindow->inputEvent(mouse<>::index(activeMouse, mouse<>::x), true);
  }
}

void InputMouseCaptureWindow::assignAxisY() {
  if(activeMouse >= 0) {
    inputCaptureWindow->inputEvent(mouse<>::index(activeMouse, mouse<>::y), true);
  }
}

//====================
//InputCalibrateWindow
//====================

//background:
//===========
//HID devices work by sending device state *changes* only. what this means is that when an application is started,
//it does not know the current state of said device. the keyboard and mouse are exceptions, as the OS globally
//tracks their states. but this does apply to joypads. once a button is pressed, or an axis is moved, the entire
//joypad state will be sent in a message, that APIs such as DirectInput and SDL will buffer.
//
//to complicate matters, recent joypads have added pressure-sensitive buttons (triggers), in addition to the
//existing analog sticks. but this functionality was not extended to the USB HID state or to various platform
//input APIs. instead, they are treated exactly like axes.
//
//however, an application needs to treat these two input types distinctly:
//a stick is a bi-directional input. the stick starts off centered, and can be moved left or right, or up or down.
//a trigger is a uni-directional input. it can only be pushed down.
//
//a stick's default, unpressed state is centered (0), whereas a trigger's default state is fully depressed (+32767.)
//but because the default state is not available until the user presses a button on a joypad, it is not possible to
//calibrate a joypad on startup. all axes will report a value of 0, even if buttons are depressed.
//
//thusly, this class is needed. it will spawn a window upon the first attempt to map a joypad axis after startup.
//by the point this window appears, an axis must have been moved, so the joypad state is now valid. but since it's
//not possible to tell which button was pressed or which axis was moved, it's possible the axis that we're trying to
//map was moved. so querying its state now might result in improper mapping. so instead, this window is shown, and
//the user is asked not to press any buttons or move any axes. after hitting okay to confirm the joypad is idle,
//the joypad can finally be calibrated properly.
//
//upon assignment, the calibration data is appended to the input assignment value (eg "joypad00.axis00::trigger"),
//so that calibration is not necessary every time the emulator is run -- only when modifying input mapping on an axis.

void InputCalibrationWindow::activate(unsigned joy) {
  //do not override an already active calibration window
  if(isVisible()) return;

  activeJoypad = joy;
  info->setText(utf8()
  << "Joypad #" << joy << " needs to be calibrated before it can be mapped. "
  << "Please ensure<br>that no buttons are pressed, "
  << "and all axes are centered before pressing ok."
  );

  show();
  ok->setFocus();
}

InputCalibrationWindow::InputCalibrationWindow() : QbWindow(config.geometry.inputCalibrationWindow) {
  activeJoypad = -1;

  setObjectName("input-calibrate-window");
  setWindowTitle("Joypad Calibration");

  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(0);
  layout->setAlignment(Qt::AlignTop);
  setLayout(layout);

  info = new QLabel;
  layout->addWidget(info);
  layout->addSpacing(Style::WidgetSpacing);

  controlLayout = new QHBoxLayout;
  controlLayout->setAlignment(Qt::AlignRight);
  layout->addLayout(controlLayout);

  ok = new QPushButton("Ok");
  controlLayout->addWidget(ok);

  connect(ok, SIGNAL(released()), this, SLOT(dismiss()));
}

void InputCalibrationWindow::dismiss() {
  hide();
  if(activeJoypad != -1) {
    inputManager.calibrate(activeJoypad);
    activeJoypad = -1;
  }
}

void InputCalibrationWindow::closeEvent(QCloseEvent *event) {
  QbWindow::closeEvent(event);
  dismiss();
}
