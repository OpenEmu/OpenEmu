//shared keycode lookup table + initialization routine:
//#include inside a class interface to use

//Xlib keycodes for each key can vary between platforms, so this header file
//will lookup keycodes from static keysyms, and map them to nall/input.hpp's
//keyboard identifiers.
//
//this allows input capture routine to iterate quickly over all keycodes and
//map their states to ruby's input state table.

uint8_t keycode[256];

void init_keycodes() {
  Display *display = XOpenDisplay(0);
  memset(&keycode, 0, sizeof keycode);

  #define assign(x, y) keycode[x] = XKeysymToKeycode(display, y)
  assign(keyboard<0>::escape, XK_Escape);

  assign(keyboard<0>::f1, XK_F1);
  assign(keyboard<0>::f2, XK_F2);
  assign(keyboard<0>::f3, XK_F3);
  assign(keyboard<0>::f4, XK_F4);
  assign(keyboard<0>::f5, XK_F5);
  assign(keyboard<0>::f6, XK_F6);
  assign(keyboard<0>::f7, XK_F7);
  assign(keyboard<0>::f8, XK_F8);
  assign(keyboard<0>::f9, XK_F9);
  assign(keyboard<0>::f10, XK_F10);
  assign(keyboard<0>::f11, XK_F11);
  assign(keyboard<0>::f12, XK_F12);

  //assign(keyboard<0>::print_screen, XK_???);
  assign(keyboard<0>::scroll_lock, XK_Scroll_Lock);
  assign(keyboard<0>::pause, XK_Pause);

  assign(keyboard<0>::tilde, XK_asciitilde);

  assign(keyboard<0>::num_0, XK_0);
  assign(keyboard<0>::num_1, XK_1);
  assign(keyboard<0>::num_2, XK_2);
  assign(keyboard<0>::num_3, XK_3);
  assign(keyboard<0>::num_4, XK_4);
  assign(keyboard<0>::num_5, XK_5);
  assign(keyboard<0>::num_6, XK_6);
  assign(keyboard<0>::num_7, XK_7);
  assign(keyboard<0>::num_8, XK_8);
  assign(keyboard<0>::num_9, XK_9);

  assign(keyboard<0>::dash, XK_minus);
  assign(keyboard<0>::equal, XK_equal);
  assign(keyboard<0>::backspace, XK_BackSpace);

  assign(keyboard<0>::insert, XK_Insert);
  assign(keyboard<0>::delete_, XK_Delete);
  assign(keyboard<0>::home, XK_Home);
  assign(keyboard<0>::end, XK_End);
  assign(keyboard<0>::page_up, XK_Prior);
  assign(keyboard<0>::page_down, XK_Next);

  assign(keyboard<0>::a, XK_A);
  assign(keyboard<0>::b, XK_B);
  assign(keyboard<0>::c, XK_C);
  assign(keyboard<0>::d, XK_D);
  assign(keyboard<0>::e, XK_E);
  assign(keyboard<0>::f, XK_F);
  assign(keyboard<0>::g, XK_G);
  assign(keyboard<0>::h, XK_H);
  assign(keyboard<0>::i, XK_I);
  assign(keyboard<0>::j, XK_J);
  assign(keyboard<0>::k, XK_K);
  assign(keyboard<0>::l, XK_L);
  assign(keyboard<0>::m, XK_M);
  assign(keyboard<0>::n, XK_N);
  assign(keyboard<0>::o, XK_O);
  assign(keyboard<0>::p, XK_P);
  assign(keyboard<0>::q, XK_Q);
  assign(keyboard<0>::r, XK_R);
  assign(keyboard<0>::s, XK_S);
  assign(keyboard<0>::t, XK_T);
  assign(keyboard<0>::u, XK_U);
  assign(keyboard<0>::v, XK_V);
  assign(keyboard<0>::w, XK_W);
  assign(keyboard<0>::x, XK_X);
  assign(keyboard<0>::y, XK_Y);
  assign(keyboard<0>::z, XK_Z);

  assign(keyboard<0>::lbracket, XK_bracketleft);
  assign(keyboard<0>::rbracket, XK_bracketright);
  assign(keyboard<0>::backslash, XK_backslash);
  assign(keyboard<0>::semicolon, XK_semicolon);
  assign(keyboard<0>::apostrophe, XK_apostrophe);
  assign(keyboard<0>::comma, XK_comma);
  assign(keyboard<0>::period, XK_period);
  assign(keyboard<0>::slash, XK_slash);

  assign(keyboard<0>::pad_0, XK_KP_0);
  assign(keyboard<0>::pad_1, XK_KP_1);
  assign(keyboard<0>::pad_2, XK_KP_2);
  assign(keyboard<0>::pad_3, XK_KP_3);
  assign(keyboard<0>::pad_4, XK_KP_4);
  assign(keyboard<0>::pad_5, XK_KP_5);
  assign(keyboard<0>::pad_6, XK_KP_6);
  assign(keyboard<0>::pad_7, XK_KP_7);
  assign(keyboard<0>::pad_8, XK_KP_8);
  assign(keyboard<0>::pad_9, XK_KP_9);

  assign(keyboard<0>::add, XK_KP_Add);
  assign(keyboard<0>::subtract, XK_KP_Subtract);
  assign(keyboard<0>::multiply, XK_KP_Multiply);
  assign(keyboard<0>::divide, XK_KP_Divide);
  assign(keyboard<0>::enter, XK_KP_Enter);

  //assign(keyboard<0>::num_lock, XK_???);
  //assign(keyboard<0>::caps_lock, XK_???);

  assign(keyboard<0>::up, XK_Up);
  assign(keyboard<0>::down, XK_Down);
  assign(keyboard<0>::left, XK_Left);
  assign(keyboard<0>::right, XK_Right);

  assign(keyboard<0>::tab, XK_Tab);
  assign(keyboard<0>::return_, XK_Return);
  assign(keyboard<0>::spacebar, XK_space);

  assign(keyboard<0>::lctrl, XK_Control_L);
  assign(keyboard<0>::rctrl, XK_Control_R);
  assign(keyboard<0>::lalt, XK_Alt_L);
  assign(keyboard<0>::ralt, XK_Alt_R);
  assign(keyboard<0>::lshift, XK_Shift_L);
  assign(keyboard<0>::rshift, XK_Shift_R);
  assign(keyboard<0>::lsuper, XK_Super_L);
  assign(keyboard<0>::rsuper, XK_Super_R);
  assign(keyboard<0>::menu, XK_Menu);
  #undef assign

  XCloseDisplay(display);
}
