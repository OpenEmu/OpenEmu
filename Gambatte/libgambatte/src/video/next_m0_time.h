#ifndef NEXT_M0_TIME_H_
#define NEXT_M0_TIME_H_

namespace gambatte {

class NextM0Time {
	unsigned predictedNextM0Time_;
	
public:
	NextM0Time() : predictedNextM0Time_(0) {}
	void predictNextM0Time(const class PPU &v);
	void invalidatePredictedNextM0Time() { predictedNextM0Time_ = 0; }
	unsigned predictedNextM0Time() const { return predictedNextM0Time_; }
};

}

#endif
