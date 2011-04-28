char* userpath(char *path) {
  *path = 0;
  struct passwd *userinfo = getpwuid(getuid());
  if(userinfo) strcpy(path, userinfo->pw_dir);
  return path;
}

char *getcwd(char *path) {
  return getcwd(path, PATH_MAX);
}

void initargs(int &argc, char **&argv) {
}

void supressScreenSaver() {
}

