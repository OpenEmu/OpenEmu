class Movie {
public:
  enum State { Inactive, Playback, Record } state;

  void chooseFile();
  void play(string filename);
  void record();
  void stop();

  Movie();

//private:
  file fp;

  string makeFilename() const;
  int16_t read();
  void write(int16_t value);
};

extern Movie movie;
