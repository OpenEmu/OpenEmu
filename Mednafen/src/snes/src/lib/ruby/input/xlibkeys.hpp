uint8_t scancode[256];

enum XScancode {
  Escape, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
  ScrollLock, Pause, Tilde,
  Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, Num0,
  Dash, Equal, Backspace,
  Insert, Delete, Home, End, PageUp, PageDown,
  A, B, C, D, E, F, G, H, I, J, K, L, M,
  N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
  LeftBracket, RightBracket, Backslash, Semicolon, Apostrophe, Comma, Period, Slash,
  Keypad1, Keypad2, Keypad3, Keypad4, Keypad5, Keypad6, Keypad7, Keypad8, Keypad9, Keypad0,
  Point, Enter, Add, Subtract, Multiply, Divide,
  Up, Down, Left, Right,
  Tab, Return, Spacebar, Menu,
  LeftShift, RightShift, LeftControl, RightControl, LeftAlt, RightAlt, LeftSuper, RightSuper,
};

void x_poll(int16_t *table) {
  char state[32];
  Display *display = XOpenDisplay(0);
  XQueryKeymap(display, state);
  XCloseDisplay(display);

  #define key(id) table[keyboard(0)[id]]
  #define pressed(id) (bool)(state[scancode[id] >> 3] & (1 << (scancode[id] & 7)))

  key(Keyboard::Escape) = pressed(Escape);

  key(Keyboard::F1) = pressed(F1);
  key(Keyboard::F2) = pressed(F2);
  key(Keyboard::F3) = pressed(F3);
  key(Keyboard::F4) = pressed(F4);
  key(Keyboard::F5) = pressed(F5);
  key(Keyboard::F6) = pressed(F6);
  key(Keyboard::F7) = pressed(F7);
  key(Keyboard::F8) = pressed(F8);
  key(Keyboard::F9) = pressed(F9);
  key(Keyboard::F10) = pressed(F10);
  key(Keyboard::F11) = pressed(F11);
  key(Keyboard::F12) = pressed(F12);

  key(Keyboard::ScrollLock) = pressed(ScrollLock);
  key(Keyboard::Pause) = pressed(Pause);
  key(Keyboard::Tilde) = pressed(Tilde);

  key(Keyboard::Num1) = pressed(Num1);
  key(Keyboard::Num2) = pressed(Num2);
  key(Keyboard::Num3) = pressed(Num3);
  key(Keyboard::Num4) = pressed(Num4);
  key(Keyboard::Num5) = pressed(Num5);
  key(Keyboard::Num6) = pressed(Num6);
  key(Keyboard::Num7) = pressed(Num7);
  key(Keyboard::Num8) = pressed(Num8);
  key(Keyboard::Num9) = pressed(Num9);
  key(Keyboard::Num0) = pressed(Num0);

  key(Keyboard::Dash) = pressed(Dash);
  key(Keyboard::Equal) = pressed(Equal);
  key(Keyboard::Backspace) = pressed(Backspace);

  key(Keyboard::Insert) = pressed(Insert);
  key(Keyboard::Delete) = pressed(Delete);
  key(Keyboard::Home) = pressed(Home);
  key(Keyboard::End) = pressed(End);
  key(Keyboard::PageUp) = pressed(PageUp);
  key(Keyboard::PageDown) = pressed(PageDown);

  key(Keyboard::A) = pressed(A);
  key(Keyboard::B) = pressed(B);
  key(Keyboard::C) = pressed(C);
  key(Keyboard::D) = pressed(D);
  key(Keyboard::E) = pressed(E);
  key(Keyboard::F) = pressed(F);
  key(Keyboard::G) = pressed(G);
  key(Keyboard::H) = pressed(H);
  key(Keyboard::I) = pressed(I);
  key(Keyboard::J) = pressed(J);
  key(Keyboard::K) = pressed(K);
  key(Keyboard::L) = pressed(L);
  key(Keyboard::M) = pressed(M);
  key(Keyboard::N) = pressed(N);
  key(Keyboard::O) = pressed(O);
  key(Keyboard::P) = pressed(P);
  key(Keyboard::Q) = pressed(Q);
  key(Keyboard::R) = pressed(R);
  key(Keyboard::S) = pressed(S);
  key(Keyboard::T) = pressed(T);
  key(Keyboard::U) = pressed(U);
  key(Keyboard::V) = pressed(V);
  key(Keyboard::W) = pressed(W);
  key(Keyboard::X) = pressed(X);
  key(Keyboard::Y) = pressed(Y);
  key(Keyboard::Z) = pressed(Z);

  key(Keyboard::LeftBracket) = pressed(LeftBracket);
  key(Keyboard::RightBracket) = pressed(RightBracket);
  key(Keyboard::Backslash) = pressed(Backslash);
  key(Keyboard::Semicolon) = pressed(Semicolon);
  key(Keyboard::Apostrophe) = pressed(Apostrophe);
  key(Keyboard::Comma) = pressed(Comma);
  key(Keyboard::Period) = pressed(Period);
  key(Keyboard::Slash) = pressed(Slash);

  key(Keyboard::Keypad1) = pressed(Keypad1);
  key(Keyboard::Keypad2) = pressed(Keypad2);
  key(Keyboard::Keypad3) = pressed(Keypad3);
  key(Keyboard::Keypad4) = pressed(Keypad4);
  key(Keyboard::Keypad5) = pressed(Keypad5);
  key(Keyboard::Keypad6) = pressed(Keypad6);
  key(Keyboard::Keypad7) = pressed(Keypad7);
  key(Keyboard::Keypad8) = pressed(Keypad8);
  key(Keyboard::Keypad9) = pressed(Keypad9);
  key(Keyboard::Keypad0) = pressed(Keypad0);

  key(Keyboard::Point) = pressed(Point);
  key(Keyboard::Enter) = pressed(Enter);
  key(Keyboard::Add) = pressed(Add);
  key(Keyboard::Subtract) = pressed(Subtract);
  key(Keyboard::Multiply) = pressed(Multiply);
  key(Keyboard::Divide) = pressed(Divide);

  key(Keyboard::Up) = pressed(Up);
  key(Keyboard::Down) = pressed(Down);
  key(Keyboard::Left) = pressed(Left);
  key(Keyboard::Right) = pressed(Right);

  key(Keyboard::Tab) = pressed(Tab);
  key(Keyboard::Return) = pressed(Return);
  key(Keyboard::Spacebar) = pressed(Spacebar);
  key(Keyboard::Menu) = pressed(Menu);

  key(Keyboard::Shift) = pressed(LeftShift) || pressed(RightShift);
  key(Keyboard::Control) = pressed(LeftControl) || pressed(RightControl);
  key(Keyboard::Alt) = pressed(LeftAlt) || pressed(RightAlt);
  key(Keyboard::Super) = pressed(LeftSuper) || pressed(RightSuper);

  #undef key
  #undef pressed
}

void x_init() {
  Display *display = XOpenDisplay(0);
  memset(&scancode, 0, sizeof scancode);

  #define assign(x, y) scancode[x] = XKeysymToKeycode(display, y)
  assign(Escape, XK_Escape);

  assign(F1, XK_F1);
  assign(F2, XK_F2);
  assign(F3, XK_F3);
  assign(F4, XK_F4);
  assign(F5, XK_F5);
  assign(F6, XK_F6);
  assign(F7, XK_F7);
  assign(F8, XK_F8);
  assign(F9, XK_F9);
  assign(F10, XK_F10);
  assign(F11, XK_F11);
  assign(F12, XK_F12);

  assign(ScrollLock, XK_Scroll_Lock);
  assign(Pause, XK_Pause);

  assign(Tilde, XK_asciitilde);

  assign(Num0, XK_0);
  assign(Num1, XK_1);
  assign(Num2, XK_2);
  assign(Num3, XK_3);
  assign(Num4, XK_4);
  assign(Num5, XK_5);
  assign(Num6, XK_6);
  assign(Num7, XK_7);
  assign(Num8, XK_8);
  assign(Num9, XK_9);

  assign(Dash, XK_minus);
  assign(Equal, XK_equal);
  assign(Backspace, XK_BackSpace);

  assign(Insert, XK_Insert);
  assign(Delete, XK_Delete);
  assign(Home, XK_Home);
  assign(End, XK_End);
  assign(PageUp, XK_Prior);
  assign(PageDown, XK_Next);

  assign(A, XK_A);
  assign(B, XK_B);
  assign(C, XK_C);
  assign(D, XK_D);
  assign(E, XK_E);
  assign(F, XK_F);
  assign(G, XK_G);
  assign(H, XK_H);
  assign(I, XK_I);
  assign(J, XK_J);
  assign(K, XK_K);
  assign(L, XK_L);
  assign(M, XK_M);
  assign(N, XK_N);
  assign(O, XK_O);
  assign(P, XK_P);
  assign(Q, XK_Q);
  assign(R, XK_R);
  assign(S, XK_S);
  assign(T, XK_T);
  assign(U, XK_U);
  assign(V, XK_V);
  assign(W, XK_W);
  assign(X, XK_X);
  assign(Y, XK_Y);
  assign(Z, XK_Z);

  assign(LeftBracket, XK_bracketleft);
  assign(RightBracket, XK_bracketright);
  assign(Backslash, XK_backslash);
  assign(Semicolon, XK_semicolon);
  assign(Apostrophe, XK_apostrophe);
  assign(Comma, XK_comma);
  assign(Period, XK_period);
  assign(Slash, XK_slash);

  assign(Keypad0, XK_KP_0);
  assign(Keypad1, XK_KP_1);
  assign(Keypad2, XK_KP_2);
  assign(Keypad3, XK_KP_3);
  assign(Keypad4, XK_KP_4);
  assign(Keypad5, XK_KP_5);
  assign(Keypad6, XK_KP_6);
  assign(Keypad7, XK_KP_7);
  assign(Keypad8, XK_KP_8);
  assign(Keypad9, XK_KP_9);

  assign(Add, XK_KP_Add);
  assign(Subtract, XK_KP_Subtract);
  assign(Multiply, XK_KP_Multiply);
  assign(Divide, XK_KP_Divide);
  assign(Enter, XK_KP_Enter);

  assign(Up, XK_Up);
  assign(Down, XK_Down);
  assign(Left, XK_Left);
  assign(Right, XK_Right);

  assign(Tab, XK_Tab);
  assign(Return, XK_Return);
  assign(Spacebar, XK_space);

  assign(LeftControl, XK_Control_L);
  assign(RightControl, XK_Control_R);
  assign(LeftAlt, XK_Alt_L);
  assign(RightAlt, XK_Alt_R);
  assign(LeftShift, XK_Shift_L);
  assign(RightShift, XK_Shift_R);
  assign(LeftSuper, XK_Super_L);
  assign(RightSuper, XK_Super_R);
  assign(Menu, XK_Menu);

  #undef assign

  XCloseDisplay(display);
}
