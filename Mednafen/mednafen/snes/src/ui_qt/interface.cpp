Interface interface;

void Interface::video_refresh(uint16_t *data, unsigned pitch, unsigned *line, unsigned width, unsigned height) {
  uint32_t *output;
  unsigned outwidth, outheight, outpitch;
  filter.size(outwidth, outheight, width, height);

  if(video.lock(output, outpitch, outwidth, outheight) == true) {
    filter.render(output, outpitch, data, pitch, line, width, height);
    video.unlock();
    video.refresh();
    if(saveScreenshot == true) captureScreenshot(output, outpitch, outwidth, outheight);
  }

  state.frame();
  #if defined(DEBUGGER)
  debugger->frameTick();
  #endif
}

void Interface::audio_sample(uint16_t left, uint16_t right) {
  if(config().audio.mute) left = right = 0;
  audio.sample(left, right);
}

void Interface::input_poll() {
  mapper().cache();
}

int16_t Interface::input_poll(bool port, unsigned device, unsigned index, unsigned id) {
  return mapper().status(port, device, index, id);
}

void Interface::captureScreenshot(uint32_t *data, unsigned pitch, unsigned width, unsigned height) {
  saveScreenshot = false;
  QImage image((const unsigned char*)data, width, height, pitch, QImage::Format_RGB32);

  string filename = nall::basename(cartridge.fileName);
  time_t systemTime = time(0);
  tm *currentTime = localtime(&systemTime);
  char t[512];
  sprintf(t, "%.4u%.2u%.2u-%.2u%.2u%.2u",
    1900 + currentTime->tm_year, 1 + currentTime->tm_mon, currentTime->tm_mday,
    currentTime->tm_hour, currentTime->tm_min, currentTime->tm_sec
  );
  filename << "-" << t << ".png";

  image.save(filepath(filename, config().path.data));
  utility.showMessage("Screenshot saved.");
}

Interface::Interface() {
  saveScreenshot = false;
}
