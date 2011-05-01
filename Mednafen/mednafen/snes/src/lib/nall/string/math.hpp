#ifndef NALL_STRING_MATH_HPP
#define NALL_STRING_MATH_HPP

static int eval_integer(const char *&s) {
  if(!*s) throw "unrecognized_integer";
  int value = 0, x = *s, y = *(s + 1);

  //hexadecimal
  if(x == '0' && (y == 'X' || y == 'x')) {
    s += 2;
    while(true) {
      if(*s >= '0' && *s <= '9') { value = value * 16 + (*s++ - '0');      continue; }
      if(*s >= 'A' && *s <= 'F') { value = value * 16 + (*s++ - 'A' + 10); continue; }
      if(*s >= 'a' && *s <= 'f') { value = value * 16 + (*s++ - 'a' + 10); continue; }
      return value;
    }
  }

  //binary
  if(x == '0' && (y == 'B' || y == 'b')) {
    s += 2;
    while(true) {
      if(*s == '0' || *s == '1') { value = value * 2 + (*s++ - '0'); continue; }
      return value;
    }
  }

  //octal (or decimal '0')
  if(x == '0') {
    s += 1;
    while(true) {
      if(*s >= '0' && *s <= '7') { value = value * 8 + (*s++ - '0'); continue; }
      return value;
    }
  }

  //decimal
  if(x >= '0' && x <= '9') {
    while(true) {
      if(*s >= '0' && *s <= '9') { value = value * 10 + (*s++ - '0'); continue; }
      return value;
    }
  }

  //char
  if(x == '\'' && y != '\'') {
    s += 1;
    while(true) {
      value = value * 256 + *s++;
      if(*s == '\'') { s += 1; return value; }
      if(!*s) throw "mismatched_char";
    }
  }

  throw "unrecognized_integer";
}

static int eval(const char *&s, int depth = 0) {
  while(*s == ' ' || *s == '\t') s++; //trim whitespace
  if(!*s) throw "unrecognized_token";
  int value = 0, x = *s, y = *(s + 1);

  if(*s == '(') {
    value = eval(++s, 1);
    if(*s++ != ')') throw "mismatched_group";
  }

  else if(x == '!') value = !eval(++s, 13);
  else if(x == '~') value = ~eval(++s, 13);
  else if(x == '+') value = +eval(++s, 13);
  else if(x == '-') value = -eval(++s, 13);

  else if((x >= '0' && x <= '9') || x == '\'') value = eval_integer(s);

  else throw "unrecognized_token";

  while(true) {
    while(*s == ' ' || *s == '\t') s++; //trim whitespace
    if(!*s) break;
    x = *s, y = *(s + 1);

    if(depth >= 13) break;
    if(x == '*') { value *= eval(++s, 13); continue; }
    if(x == '/') { value /= eval(++s, 13); continue; }
    if(x == '%') { value %= eval(++s, 13); continue; }

    if(depth >= 12) break;
    if(x == '+') { value += eval(++s, 12); continue; }
    if(x == '-') { value -= eval(++s, 12); continue; }

    if(depth >= 11) break;
    if(x == '<' && y == '<') { value <<= eval(++++s, 11); continue; }
    if(x == '>' && y == '>') { value >>= eval(++++s, 11); continue; }

    if(depth >= 10) break;
    if(x == '<' && y == '=') { value = value <= eval(++++s, 10); continue; }
    if(x == '>' && y == '=') { value = value >= eval(++++s, 10); continue; }
    if(x == '<') { value = value < eval(++s, 10); continue; }
    if(x == '>') { value = value > eval(++s, 10); continue; }

    if(depth >= 9) break;
    if(x == '=' && y == '=') { value = value == eval(++++s, 9); continue; }
    if(x == '!' && y == '=') { value = value != eval(++++s, 9); continue; }

    if(depth >= 8) break;
    if(x == '&' && y != '&') { value = value & eval(++s, 8); continue; }

    if(depth >= 7) break;
    if(x == '^' && y != '^') { value = value ^ eval(++s, 7); continue; }

    if(depth >= 6) break;
    if(x == '|' && y != '|') { value = value | eval(++s, 6); continue; }

    if(depth >= 5) break;
    if(x == '&' && y == '&') { value = eval(++++s, 5) && value; continue; }

    if(depth >= 4) break;
    if(x == '^' && y == '^') { value = (!eval(++++s, 4) != !value); continue; }

    if(depth >= 3) break;
    if(x == '|' && y == '|') { value = eval(++++s, 3) || value; continue; }

    if(x == '?') {
      int lhs = eval(++s, 2);
      if(*s != ':') throw "mismatched_ternary";
      int rhs = eval(++s, 2);
      value = value ? lhs : rhs;
      continue;
    }
    if(depth >= 2) break;

    if(depth > 0 && x == ')') break;

    throw "unrecognized_token";
  }

  return value;
}

bool strint(const char *s, int &result) {
  try {
    result = eval_integer(s);
    return true;
  } catch(const char*) {
    result = 0;
    return false;
  }
}

bool strmath(const char *s, int &result) {
  try {
    result = eval(s);
    return true;
  } catch(const char*) {
    result = 0;
    return false;
  }
}

#endif
