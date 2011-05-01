class EmuRealSyncher
{
 public:
 EmuRealSyncher();
 ~EmuRealSyncher()
 {

 }

 void SetEmuClock(int64 EmuClock_arg);

 bool NeedFrameSkip(void);
 void Sync();
 void AddEmuTime(const int64 zetime, bool frame_end = true);

 void SetETtoRT(void);

 private:
 int64 EmuClock;

 int64 EmuTime;

 int64 ForcedCatchupTH;
 int64 FrameSkipTH;
 int64 NoFrameSkipTH;

 int64 LastNoFrameSkipTime;

 int64 AvgTimePerFrame; // 56.8

 int64 tmp_frame_accumtime;
};

