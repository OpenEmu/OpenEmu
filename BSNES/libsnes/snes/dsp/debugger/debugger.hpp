class DSPDebugger : public DSP, public ChipDebugger {
public:
  bool property(unsigned id, string &name, string &value);
};
