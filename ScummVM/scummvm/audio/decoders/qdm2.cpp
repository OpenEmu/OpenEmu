/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// Based off ffmpeg's QDM2 decoder

#include "common/scummsys.h"
#include "audio/decoders/qdm2.h"

#ifdef AUDIO_QDM2_H

#include "audio/audiostream.h"
#include "audio/decoders/codec.h"
#include "audio/decoders/qdm2data.h"
#include "audio/decoders/raw.h"

#include "common/array.h"
#include "common/debug.h"
#include "common/math.h"
#include "common/rdft.h"
#include "common/stream.h"
#include "common/memstream.h"
#include "common/bitstream.h"
#include "common/textconsole.h"

namespace Audio {

enum {
	SOFTCLIP_THRESHOLD = 27600,
	HARDCLIP_THRESHOLD = 35716,
	MPA_MAX_CHANNELS = 2,
	MPA_FRAME_SIZE = 1152,
	FF_INPUT_BUFFER_PADDING_SIZE = 8
};

typedef int8 sb_int8_array[2][30][64];

struct QDM2SubPacket {
	int type;
	unsigned int size;
	const uint8 *data; // pointer to subpacket data (points to input data buffer, it's not a private copy)
};

struct QDM2SubPNode {
	QDM2SubPacket *packet;
	struct QDM2SubPNode *next; // pointer to next packet in the list, NULL if leaf node
};

struct QDM2Complex {
	float re;
	float im;
};

struct FFTTone {
	float level;
	QDM2Complex *complex;
	const float *table;
	int phase;
	int phase_shift;
	int duration;
	short time_index;
	short cutoff;
};

struct FFTCoefficient {
	int16 sub_packet;
	uint8 channel;
	int16 offset;
	int16 exp;
	uint8 phase;
};

struct VLC {
	int32 bits;
	int16 (*table)[2]; // code, bits
	int32 table_size;
	int32 table_allocated;
};

#include "common/pack-start.h"
struct QDM2FFT {
	QDM2Complex complex[MPA_MAX_CHANNELS][256];
} PACKED_STRUCT;
#include "common/pack-end.h"

class QDM2Stream : public Codec {
public:
	QDM2Stream(Common::SeekableReadStream *extraData, DisposeAfterUse::Flag disposeExtraData);
	~QDM2Stream();

	AudioStream *decodeFrame(Common::SeekableReadStream &stream);

private:
	// Parameters from codec header, do not change during playback
	uint8 _channels;
	uint16 _sampleRate;
	uint16 _bitRate;
	uint16 _blockSize;  // Group
	uint16 _frameSize;  // FFT
	uint16 _packetSize; // Checksum

	// Parameters built from header parameters, do not change during playback
	int _groupOrder;       // order of frame group
	int _fftOrder;         // order of FFT (actually fft order+1)
	int _fftFrameSize;     // size of fft frame, in components (1 comples = re + im)
	int _sFrameSize;        // size of data frame
	int _frequencyRange;
	int _subSampling;      // subsampling: 0=25%, 1=50%, 2=100% */
	int _coeffPerSbSelect; // selector for "num. of coeffs. per subband" tables. Can be 0, 1, 2
	int _cmTableSelect;    // selector for "coding method" tables. Can be 0, 1 (from init: 0-4)

	// Packets and packet lists
	QDM2SubPacket _subPackets[16];    // the packets themselves
	QDM2SubPNode _subPacketListA[16]; // list of all packets
	QDM2SubPNode _subPacketListB[16]; // FFT packets B are on list
	int _subPacketsB;                 // number of packets on 'B' list
	QDM2SubPNode _subPacketListC[16]; // packets with errors?
	QDM2SubPNode _subPacketListD[16]; // DCT packets

	// FFT and tones
	FFTTone _fftTones[1000];
	int _fftToneStart;
	int _fftToneEnd;
	FFTCoefficient _fftCoefs[1000];
	int _fftCoefsIndex;
	int _fftCoefsMinIndex[5];
	int _fftCoefsMaxIndex[5];
	int _fftLevelExp[6];
	Common::RDFT *_rdft;
	QDM2FFT _fft;

	// I/O data
	uint8 *_compressedData;
	float _outputBuffer[1024];

	// Synthesis filter
	int16 ff_mpa_synth_window[512];
	int16 _synthBuf[MPA_MAX_CHANNELS][512*2];
	int _synthBufOffset[MPA_MAX_CHANNELS];
	int32 _sbSamples[MPA_MAX_CHANNELS][128][32];

	// Mixed temporary data used in decoding
	float _toneLevel[MPA_MAX_CHANNELS][30][64];
	int8 _codingMethod[MPA_MAX_CHANNELS][30][64];
	int8 _quantizedCoeffs[MPA_MAX_CHANNELS][10][8];
	int8 _toneLevelIdxBase[MPA_MAX_CHANNELS][30][8];
	int8 _toneLevelIdxHi1[MPA_MAX_CHANNELS][3][8][8];
	int8 _toneLevelIdxMid[MPA_MAX_CHANNELS][26][8];
	int8 _toneLevelIdxHi2[MPA_MAX_CHANNELS][26];
	int8 _toneLevelIdx[MPA_MAX_CHANNELS][30][64];
	int8 _toneLevelIdxTemp[MPA_MAX_CHANNELS][30][64];

	// Flags
	bool _hasErrors;         // packet has errors
	int _superblocktype_2_3; // select fft tables and some algorithm based on superblock type
	int _doSynthFilter;      // used to perform or skip synthesis filter

	uint8 _subPacket; // 0 to 15
	uint32 _superBlockStart;
	int _noiseIdx; // index for dithering noise table

	byte _emptyBuffer[FF_INPUT_BUFFER_PADDING_SIZE];

	VLC _vlcTabLevel;
	VLC _vlcTabDiff;
	VLC _vlcTabRun;
	VLC _fftLevelExpAltVlc;
	VLC _fftLevelExpVlc;
	VLC _fftStereoExpVlc;
	VLC _fftStereoPhaseVlc;
	VLC _vlcTabToneLevelIdxHi1;
	VLC _vlcTabToneLevelIdxMid;
	VLC _vlcTabToneLevelIdxHi2;
	VLC _vlcTabType30;
	VLC _vlcTabType34;
	VLC _vlcTabFftToneOffset[5];
	bool _vlcsInitialized;
	void initVlc(void);

	uint16 _softclipTable[HARDCLIP_THRESHOLD - SOFTCLIP_THRESHOLD + 1];
	void softclipTableInit(void);

	float _noiseTable[4096];
	byte _randomDequantIndex[256][5];
	byte _randomDequantType24[128][3];
	void rndTableInit(void);

	float _noiseSamples[128];
	void initNoiseSamples(void);

	void average_quantized_coeffs(void);
	void build_sb_samples_from_noise(int sb);
	void fix_coding_method_array(int sb, int channels, sb_int8_array coding_method);
	void fill_tone_level_array(int flag);
	void fill_coding_method_array(sb_int8_array tone_level_idx, sb_int8_array tone_level_idx_temp,
	                              sb_int8_array coding_method, int nb_channels,
	                              int c, int superblocktype_2_3, int cm_table_select);
	void synthfilt_build_sb_samples(Common::BitStream *gb, int length, int sb_min, int sb_max);
	void init_quantized_coeffs_elem0(int8 *quantized_coeffs, Common::BitStream *gb, int length);
	void init_tone_level_dequantization(Common::BitStream *gb, int length);
	void process_subpacket_9(QDM2SubPNode *node);
	void process_subpacket_10(QDM2SubPNode *node, int length);
	void process_subpacket_11(QDM2SubPNode *node, int length);
	void process_subpacket_12(QDM2SubPNode *node, int length);
	void process_synthesis_subpackets(QDM2SubPNode *list);
	void qdm2_decode_super_block(void);
	void qdm2_fft_init_coefficient(int sub_packet, int offset, int duration,
	                               int channel, int exp, int phase);
	void qdm2_fft_decode_tones(int duration, Common::BitStream *gb, int b);
	void qdm2_decode_fft_packets(void);
	void qdm2_fft_generate_tone(FFTTone *tone);
	void qdm2_fft_tone_synthesizer(uint8 sub_packet);
	void qdm2_calculate_fft(int channel);
	void qdm2_synthesis_filter(uint8 index);
	bool qdm2_decodeFrame(Common::SeekableReadStream &in, QueuingAudioStream *audioStream);
};

// Fix compilation for non C99-compliant compilers, like MSVC
#ifndef int64_t
typedef signed long long int int64_t;
#endif

#define QDM2_LIST_ADD(list, size, packet) \
	do { \
		if (size > 0) \
			list[size - 1].next = &list[size]; \
		list[size].packet = packet; \
		list[size].next = NULL; \
		size++; \
	} while(0)

// Result is 8, 16 or 30
#define QDM2_SB_USED(subSampling) (((subSampling) >= 2) ? 30 : 8 << (subSampling))

#define FIX_NOISE_IDX(noiseIdx) \
	if ((noiseIdx) >= 3840) \
		(noiseIdx) -= 3840 \

#define SB_DITHERING_NOISE(sb, noiseIdx) (_noiseTable[(noiseIdx)++] * sb_noise_attenuation[(sb)])

// half mpeg encoding window (full precision)
const int32 ff_mpa_enwindow[257] = {
     0,    -1,    -1,    -1,    -1,    -1,    -1,    -2,
    -2,    -2,    -2,    -3,    -3,    -4,    -4,    -5,
    -5,    -6,    -7,    -7,    -8,    -9,   -10,   -11,
   -13,   -14,   -16,   -17,   -19,   -21,   -24,   -26,
   -29,   -31,   -35,   -38,   -41,   -45,   -49,   -53,
   -58,   -63,   -68,   -73,   -79,   -85,   -91,   -97,
  -104,  -111,  -117,  -125,  -132,  -139,  -147,  -154,
  -161,  -169,  -176,  -183,  -190,  -196,  -202,  -208,
   213,   218,   222,   225,   227,   228,   228,   227,
   224,   221,   215,   208,   200,   189,   177,   163,
   146,   127,   106,    83,    57,    29,    -2,   -36,
   -72,  -111,  -153,  -197,  -244,  -294,  -347,  -401,
  -459,  -519,  -581,  -645,  -711,  -779,  -848,  -919,
  -991, -1064, -1137, -1210, -1283, -1356, -1428, -1498,
 -1567, -1634, -1698, -1759, -1817, -1870, -1919, -1962,
 -2001, -2032, -2057, -2075, -2085, -2087, -2080, -2063,
  2037,  2000,  1952,  1893,  1822,  1739,  1644,  1535,
  1414,  1280,  1131,   970,   794,   605,   402,   185,
   -45,  -288,  -545,  -814, -1095, -1388, -1692, -2006,
 -2330, -2663, -3004, -3351, -3705, -4063, -4425, -4788,
 -5153, -5517, -5879, -6237, -6589, -6935, -7271, -7597,
 -7910, -8209, -8491, -8755, -8998, -9219, -9416, -9585,
 -9727, -9838, -9916, -9959, -9966, -9935, -9863, -9750,
 -9592, -9389, -9139, -8840, -8492, -8092, -7640, -7134,
  6574,  5959,  5288,  4561,  3776,  2935,  2037,  1082,
    70,  -998, -2122, -3300, -4533, -5818, -7154, -8540,
 -9975,-11455,-12980,-14548,-16155,-17799,-19478,-21189,
-22929,-24694,-26482,-28289,-30112,-31947,-33791,-35640,
-37489,-39336,-41176,-43006,-44821,-46617,-48390,-50137,
-51853,-53534,-55178,-56778,-58333,-59838,-61289,-62684,
-64019,-65290,-66494,-67629,-68692,-69679,-70590,-71420,
-72169,-72835,-73415,-73908,-74313,-74630,-74856,-74992,
 75038
};

void ff_mpa_synth_init(int16 *window) {
	int i;
	int32 v;

	// max = 18760, max sum over all 16 coefs : 44736
	for(i = 0; i < 257; i++) {
		v = ff_mpa_enwindow[i];
		v = (v + 2) >> 2;
		window[i] = v;

		if ((i & 63) != 0)
			v = -v;

		if (i != 0)
			window[512 - i] = v;
	}
}

static inline uint16 round_sample(int *sum) {
	int sum1;
	sum1 = (*sum) >> 14;
	*sum &= (1 << 14)-1;
	if (sum1 < (-0x7fff - 1))
		sum1 = (-0x7fff - 1);
	if (sum1 > 0x7fff)
		sum1 = 0x7fff;
	return sum1;
}

static inline int MULH(int a, int b) {
	return ((int64_t)(a) * (int64_t)(b))>>32;
}

// signed 16x16 -> 32 multiply add accumulate
#define MACS(rt, ra, rb) rt += (ra) * (rb)

#define MLSS(rt, ra, rb) ((rt) -= (ra) * (rb))

#define SUM8(op, sum, w, p)\
{\
	op(sum, (w)[0 * 64], (p)[0 * 64]);\
	op(sum, (w)[1 * 64], (p)[1 * 64]);\
	op(sum, (w)[2 * 64], (p)[2 * 64]);\
	op(sum, (w)[3 * 64], (p)[3 * 64]);\
	op(sum, (w)[4 * 64], (p)[4 * 64]);\
	op(sum, (w)[5 * 64], (p)[5 * 64]);\
	op(sum, (w)[6 * 64], (p)[6 * 64]);\
	op(sum, (w)[7 * 64], (p)[7 * 64]);\
}

#define SUM8P2(sum1, op1, sum2, op2, w1, w2, p) \
{\
	tmp_s = p[0 * 64];\
	op1(sum1, (w1)[0 * 64], tmp_s);\
	op2(sum2, (w2)[0 * 64], tmp_s);\
	tmp_s = p[1 * 64];\
	op1(sum1, (w1)[1 * 64], tmp_s);\
	op2(sum2, (w2)[1 * 64], tmp_s);\
	tmp_s = p[2 * 64];\
	op1(sum1, (w1)[2 * 64], tmp_s);\
	op2(sum2, (w2)[2 * 64], tmp_s);\
	tmp_s = p[3 * 64];\
	op1(sum1, (w1)[3 * 64], tmp_s);\
	op2(sum2, (w2)[3 * 64], tmp_s);\
	tmp_s = p[4 * 64];\
	op1(sum1, (w1)[4 * 64], tmp_s);\
	op2(sum2, (w2)[4 * 64], tmp_s);\
	tmp_s = p[5 * 64];\
	op1(sum1, (w1)[5 * 64], tmp_s);\
	op2(sum2, (w2)[5 * 64], tmp_s);\
	tmp_s = p[6 * 64];\
	op1(sum1, (w1)[6 * 64], tmp_s);\
	op2(sum2, (w2)[6 * 64], tmp_s);\
	tmp_s = p[7 * 64];\
	op1(sum1, (w1)[7 * 64], tmp_s);\
	op2(sum2, (w2)[7 * 64], tmp_s);\
}

#define FIXHR(a) ((int)((a) * (1LL<<32) + 0.5))

// tab[i][j] = 1.0 / (2.0 * cos(pi*(2*k+1) / 2^(6 - j)))

// cos(i*pi/64)

#define COS0_0  FIXHR(0.50060299823519630134/2)
#define COS0_1  FIXHR(0.50547095989754365998/2)
#define COS0_2  FIXHR(0.51544730992262454697/2)
#define COS0_3  FIXHR(0.53104259108978417447/2)
#define COS0_4  FIXHR(0.55310389603444452782/2)
#define COS0_5  FIXHR(0.58293496820613387367/2)
#define COS0_6  FIXHR(0.62250412303566481615/2)
#define COS0_7  FIXHR(0.67480834145500574602/2)
#define COS0_8  FIXHR(0.74453627100229844977/2)
#define COS0_9  FIXHR(0.83934964541552703873/2)
#define COS0_10 FIXHR(0.97256823786196069369/2)
#define COS0_11 FIXHR(1.16943993343288495515/4)
#define COS0_12 FIXHR(1.48416461631416627724/4)
#define COS0_13 FIXHR(2.05778100995341155085/8)
#define COS0_14 FIXHR(3.40760841846871878570/8)
#define COS0_15 FIXHR(10.19000812354805681150/32)

#define COS1_0 FIXHR(0.50241928618815570551/2)
#define COS1_1 FIXHR(0.52249861493968888062/2)
#define COS1_2 FIXHR(0.56694403481635770368/2)
#define COS1_3 FIXHR(0.64682178335999012954/2)
#define COS1_4 FIXHR(0.78815462345125022473/2)
#define COS1_5 FIXHR(1.06067768599034747134/4)
#define COS1_6 FIXHR(1.72244709823833392782/4)
#define COS1_7 FIXHR(5.10114861868916385802/16)

#define COS2_0 FIXHR(0.50979557910415916894/2)
#define COS2_1 FIXHR(0.60134488693504528054/2)
#define COS2_2 FIXHR(0.89997622313641570463/2)
#define COS2_3 FIXHR(2.56291544774150617881/8)

#define COS3_0 FIXHR(0.54119610014619698439/2)
#define COS3_1 FIXHR(1.30656296487637652785/4)

#define COS4_0 FIXHR(0.70710678118654752439/2)

/* butterfly operator */
#define BF(a, b, c, s)\
{\
	tmp0 = tab[a] + tab[b];\
	tmp1 = tab[a] - tab[b];\
	tab[a] = tmp0;\
	tab[b] = MULH(tmp1<<(s), c);\
}

#define BF1(a, b, c, d)\
{\
	BF(a, b, COS4_0, 1);\
	BF(c, d,-COS4_0, 1);\
	tab[c] += tab[d];\
}

#define BF2(a, b, c, d)\
{\
	BF(a, b, COS4_0, 1);\
	BF(c, d,-COS4_0, 1);\
	tab[c] += tab[d];\
	tab[a] += tab[c];\
	tab[c] += tab[b];\
	tab[b] += tab[d];\
}

#define ADD(a, b) tab[a] += tab[b]

// DCT32 without 1/sqrt(2) coef zero scaling.
static void dct32(int32 *out, int32 *tab) {
	int tmp0, tmp1;

	// pass 1
	BF( 0, 31, COS0_0 , 1);
	BF(15, 16, COS0_15, 5);
	// pass 2
	BF( 0, 15, COS1_0 , 1);
	BF(16, 31,-COS1_0 , 1);
	// pass 1
	BF( 7, 24, COS0_7 , 1);
	BF( 8, 23, COS0_8 , 1);
	// pass 2
	BF( 7,  8, COS1_7 , 4);
	BF(23, 24,-COS1_7 , 4);
	// pass 3
	BF( 0,  7, COS2_0 , 1);
	BF( 8, 15,-COS2_0 , 1);
	BF(16, 23, COS2_0 , 1);
	BF(24, 31,-COS2_0 , 1);
	// pass 1
	BF( 3, 28, COS0_3 , 1);
	BF(12, 19, COS0_12, 2);
	// pass 2
	BF( 3, 12, COS1_3 , 1);
	BF(19, 28,-COS1_3 , 1);
	// pass 1
	BF( 4, 27, COS0_4 , 1);
	BF(11, 20, COS0_11, 2);
	// pass 2
	BF( 4, 11, COS1_4 , 1);
	BF(20, 27,-COS1_4 , 1);
	// pass 3
	BF( 3,  4, COS2_3 , 3);
	BF(11, 12,-COS2_3 , 3);
	BF(19, 20, COS2_3 , 3);
	BF(27, 28,-COS2_3 , 3);
	// pass 4
	BF( 0,  3, COS3_0 , 1);
	BF( 4,  7,-COS3_0 , 1);
	BF( 8, 11, COS3_0 , 1);
	BF(12, 15,-COS3_0 , 1);
	BF(16, 19, COS3_0 , 1);
	BF(20, 23,-COS3_0 , 1);
	BF(24, 27, COS3_0 , 1);
	BF(28, 31,-COS3_0 , 1);

	// pass 1
	BF( 1, 30, COS0_1 , 1);
	BF(14, 17, COS0_14, 3);
	// pass 2
	BF( 1, 14, COS1_1 , 1);
	BF(17, 30,-COS1_1 , 1);
	// pass 1
	BF( 6, 25, COS0_6 , 1);
	BF( 9, 22, COS0_9 , 1);
	// pass 2
	BF( 6,  9, COS1_6 , 2);
	BF(22, 25,-COS1_6 , 2);
	// pass 3
	BF( 1,  6, COS2_1 , 1);
	BF( 9, 14,-COS2_1 , 1);
	BF(17, 22, COS2_1 , 1);
	BF(25, 30,-COS2_1 , 1);

	// pass 1
	BF( 2, 29, COS0_2 , 1);
	BF(13, 18, COS0_13, 3);
	// pass 2
	BF( 2, 13, COS1_2 , 1);
	BF(18, 29,-COS1_2 , 1);
	// pass 1
	BF( 5, 26, COS0_5 , 1);
	BF(10, 21, COS0_10, 1);
	// pass 2
	BF( 5, 10, COS1_5 , 2);
	BF(21, 26,-COS1_5 , 2);
	// pass 3
	BF( 2,  5, COS2_2 , 1);
	BF(10, 13,-COS2_2 , 1);
	BF(18, 21, COS2_2 , 1);
	BF(26, 29,-COS2_2 , 1);
	// pass 4
	BF( 1,  2, COS3_1 , 2);
	BF( 5,  6,-COS3_1 , 2);
	BF( 9, 10, COS3_1 , 2);
	BF(13, 14,-COS3_1 , 2);
	BF(17, 18, COS3_1 , 2);
	BF(21, 22,-COS3_1 , 2);
	BF(25, 26, COS3_1 , 2);
	BF(29, 30,-COS3_1 , 2);

	// pass 5
	BF1( 0,  1,  2,  3);
	BF2( 4,  5,  6,  7);
	BF1( 8,  9, 10, 11);
	BF2(12, 13, 14, 15);
	BF1(16, 17, 18, 19);
	BF2(20, 21, 22, 23);
	BF1(24, 25, 26, 27);
	BF2(28, 29, 30, 31);

	// pass 6
	ADD( 8, 12);
	ADD(12, 10);
	ADD(10, 14);
	ADD(14,  9);
	ADD( 9, 13);
	ADD(13, 11);
	ADD(11, 15);

	out[ 0] = tab[0];
	out[16] = tab[1];
	out[ 8] = tab[2];
	out[24] = tab[3];
	out[ 4] = tab[4];
	out[20] = tab[5];
	out[12] = tab[6];
	out[28] = tab[7];
	out[ 2] = tab[8];
	out[18] = tab[9];
	out[10] = tab[10];
	out[26] = tab[11];
	out[ 6] = tab[12];
	out[22] = tab[13];
	out[14] = tab[14];
	out[30] = tab[15];

	ADD(24, 28);
	ADD(28, 26);
	ADD(26, 30);
	ADD(30, 25);
	ADD(25, 29);
	ADD(29, 27);
	ADD(27, 31);

	out[ 1] = tab[16] + tab[24];
	out[17] = tab[17] + tab[25];
	out[ 9] = tab[18] + tab[26];
	out[25] = tab[19] + tab[27];
	out[ 5] = tab[20] + tab[28];
	out[21] = tab[21] + tab[29];
	out[13] = tab[22] + tab[30];
	out[29] = tab[23] + tab[31];
	out[ 3] = tab[24] + tab[20];
	out[19] = tab[25] + tab[21];
	out[11] = tab[26] + tab[22];
	out[27] = tab[27] + tab[23];
	out[ 7] = tab[28] + tab[18];
	out[23] = tab[29] + tab[19];
	out[15] = tab[30] + tab[17];
	out[31] = tab[31];
}

// 32 sub band synthesis filter. Input: 32 sub band samples, Output:
// 32 samples.
// XXX: optimize by avoiding ring buffer usage
void ff_mpa_synth_filter(int16 *synth_buf_ptr, int *synth_buf_offset,
                         int16 *window, int *dither_state,
                         int16 *samples, int incr,
                         int32 sb_samples[32])
{
	int16 *synth_buf;
	const int16 *w, *w2, *p;
	int j, offset;
	int16 *samples2;
	int32 tmp[32];
	int sum, sum2;
	int tmp_s;

	offset = *synth_buf_offset;
	synth_buf = synth_buf_ptr + offset;

	dct32(tmp, sb_samples);
	for(j = 0; j < 32; j++) {
		// NOTE: can cause a loss in precision if very high amplitude sound
		if (tmp[j] < (-0x7fff - 1))
			synth_buf[j] = (-0x7fff - 1);
		else if (tmp[j] > 0x7fff)
			synth_buf[j] = 0x7fff;
		else
			synth_buf[j] = tmp[j];
	}

	// copy to avoid wrap
	memcpy(synth_buf + 512, synth_buf, 32 * sizeof(int16));

	samples2 = samples + 31 * incr;
	w = window;
	w2 = window + 31;

	sum = *dither_state;
	p = synth_buf + 16;
	SUM8(MACS, sum, w, p);
	p = synth_buf + 48;
	SUM8(MLSS, sum, w + 32, p);
	*samples = round_sample(&sum);
	samples += incr;
	w++;

	// we calculate two samples at the same time to avoid one memory
	// access per two sample
	for(j = 1; j < 16; j++) {
		sum2 = 0;
		p = synth_buf + 16 + j;
		SUM8P2(sum, MACS, sum2, MLSS, w, w2, p);
		p = synth_buf + 48 - j;
		SUM8P2(sum, MLSS, sum2, MLSS, w + 32, w2 + 32, p);

		*samples = round_sample(&sum);
		samples += incr;
		sum += sum2;
		*samples2 = round_sample(&sum);
		samples2 -= incr;
		w++;
		w2--;
	}

	p = synth_buf + 32;
	SUM8(MLSS, sum, w + 32, p);
	*samples = round_sample(&sum);
	*dither_state= sum;

	offset = (offset - 32) & 511;
	*synth_buf_offset = offset;
}

/**
 * parses a vlc code, faster then get_vlc()
 * @param bits is the number of bits which will be read at once, must be
 *             identical to nb_bits in init_vlc()
 * @param max_depth is the number of times bits bits must be read to completely
 *                  read the longest vlc code
 *                  = (max_vlc_length + bits - 1) / bits
 */
static int getVlc2(Common::BitStream *s, int16 (*table)[2], int bits, int maxDepth) {
	int index = s->peekBits(bits);
	int code = table[index][0];
	int n = table[index][1];

	if (maxDepth > 1 && n < 0) {
		s->skip(bits);
		int nbBits = -n;
		index = s->peekBits(-n) + code;
		code = table[index][0];
		n = table[index][1];

		if (maxDepth > 2 && n < 0) {
			s->skip(nbBits);
			index = s->getBits(-n) + code;
			code = table[index][0];
			n = table[index][1];
		}
	}

	s->skip(n);
	return code;
}

static int allocTable(VLC *vlc, int size, int use_static) {
	int index;
	int16 (*temp)[2] = NULL;
	index = vlc->table_size;
	vlc->table_size += size;
	if (vlc->table_size > vlc->table_allocated) {
		if(use_static)
			error("QDM2 cant do anything, init_vlc() is used with too little memory");
		vlc->table_allocated += (1 << vlc->bits);
		temp = (int16 (*)[2])realloc(vlc->table, sizeof(int16 *) * 2 * vlc->table_allocated);
		if (!temp) {
			free(vlc->table);
			vlc->table = NULL;
			return -1;
		}
		vlc->table = temp;
	}
	return index;
}

#define GET_DATA(v, table, i, wrap, size)\
{\
	const uint8 *ptr = (const uint8 *)table + i * wrap;\
	switch(size) {\
		case 1:\
			v = *(const uint8 *)ptr;\
			break;\
		case 2:\
			v = *(const uint16 *)ptr;\
			break;\
		default:\
			v = *(const uint32 *)ptr;\
			break;\
	}\
}

static int build_table(VLC *vlc, int table_nb_bits,
                       int nb_codes,
                       const void *bits, int bits_wrap, int bits_size,
                       const void *codes, int codes_wrap, int codes_size,
                       const void *symbols, int symbols_wrap, int symbols_size,
                       int code_prefix, int n_prefix, int flags)
{
	int i, j, k, n, table_size, table_index, nb, n1, index, code_prefix2, symbol;
	uint32 code;
	int16 (*table)[2];

	table_size = 1 << table_nb_bits;
	table_index = allocTable(vlc, table_size, flags & 4);
	if (table_index < 0)
		return -1;
	table = &vlc->table[table_index];

	for(i = 0; i < table_size; i++) {
		table[i][1] = 0; //bits
		table[i][0] = -1; //codes
	}

	// first pass: map codes and compute auxillary table sizes
	for(i = 0; i < nb_codes; i++) {
		GET_DATA(n, bits, i, bits_wrap, bits_size);
		GET_DATA(code, codes, i, codes_wrap, codes_size);
		// we accept tables with holes
		if (n <= 0)
			continue;
		if (!symbols)
			symbol = i;
		else
			GET_DATA(symbol, symbols, i, symbols_wrap, symbols_size);
		// if code matches the prefix, it is in the table
		n -= n_prefix;
		if(flags & 2)
			code_prefix2= code & (n_prefix>=32 ? 0xffffffff : (1 << n_prefix)-1);
		else
			code_prefix2= code >> n;
		if (n > 0 && code_prefix2 == code_prefix) {
			if (n <= table_nb_bits) {
				// no need to add another table
				j = (code << (table_nb_bits - n)) & (table_size - 1);
				nb = 1 << (table_nb_bits - n);
				for(k = 0; k < nb; k++) {
					if(flags & 2)
						j = (code >> n_prefix) + (k<<n);
					if (table[j][1] /*bits*/ != 0) {
						error("QDM2 incorrect codes");
						return -1;
					}
					table[j][1] = n; //bits
					table[j][0] = symbol;
					j++;
				}
			} else {
				n -= table_nb_bits;
				j = (code >> ((flags & 2) ? n_prefix : n)) & ((1 << table_nb_bits) - 1);
				// compute table size
				n1 = -table[j][1]; //bits
				if (n > n1)
					n1 = n;
				table[j][1] = -n1; //bits
			}
		}
	}

	// second pass : fill auxillary tables recursively
	for(i = 0;i < table_size; i++) {
		n = table[i][1]; //bits
		if (n < 0) {
			n = -n;
			if (n > table_nb_bits) {
				n = table_nb_bits;
				table[i][1] = -n; //bits
			}
			index = build_table(vlc, n, nb_codes,
			                    bits, bits_wrap, bits_size,
			                    codes, codes_wrap, codes_size,
			                    symbols, symbols_wrap, symbols_size,
			                    (flags & 2) ? (code_prefix | (i << n_prefix)) : ((code_prefix << table_nb_bits) | i),
			                    n_prefix + table_nb_bits, flags);
 			if (index < 0)
				return -1;
			// note: realloc has been done, so reload tables
			table = &vlc->table[table_index];
			table[i][0] = index; //code
		}
	}
	return table_index;
}

/* Build VLC decoding tables suitable for use with get_vlc().

   'nb_bits' set thee decoding table size (2^nb_bits) entries. The
   bigger it is, the faster is the decoding. But it should not be too
   big to save memory and L1 cache. '9' is a good compromise.

   'nb_codes' : number of vlcs codes

   'bits' : table which gives the size (in bits) of each vlc code.

   'codes' : table which gives the bit pattern of of each vlc code.

   'symbols' : table which gives the values to be returned from get_vlc().

   'xxx_wrap' : give the number of bytes between each entry of the
   'bits' or 'codes' tables.

   'xxx_size' : gives the number of bytes of each entry of the 'bits'
   or 'codes' tables.

   'wrap' and 'size' allows to use any memory configuration and types
   (byte/word/long) to store the 'bits', 'codes', and 'symbols' tables.

   'use_static' should be set to 1 for tables, which should be freed
   with av_free_static(), 0 if free_vlc() will be used.
*/
void initVlcSparse(VLC *vlc, int nb_bits, int nb_codes,
		const void *bits, int bits_wrap, int bits_size,
		const void *codes, int codes_wrap, int codes_size,
		const void *symbols, int symbols_wrap, int symbols_size) {
	vlc->bits = nb_bits;

	if (vlc->table_size && vlc->table_size == vlc->table_allocated) {
		return;
	} else if (vlc->table_size) {
		error("called on a partially initialized table");
	}

	if (build_table(vlc, nb_bits, nb_codes,
	                bits, bits_wrap, bits_size,
	                codes, codes_wrap, codes_size,
	                symbols, symbols_wrap, symbols_size,
	                0, 0, 4 | 2) < 0) {
		free(&vlc->table);
		return; // Error
	}

	if(vlc->table_size != vlc->table_allocated)
		error("QDM2 needed %d had %d", vlc->table_size, vlc->table_allocated);
}

void QDM2Stream::softclipTableInit(void) {
	uint16 i;
	double dfl = SOFTCLIP_THRESHOLD - 32767;
	float delta = 1.0 / -dfl;

	for (i = 0; i < ARRAYSIZE(_softclipTable); i++)
		_softclipTable[i] = SOFTCLIP_THRESHOLD - ((int)(sin((float)i * delta) * dfl) & 0x0000FFFF);
}

// random generated table
void QDM2Stream::rndTableInit(void) {
	uint16 i;
	uint16 j;
	uint32 ldw, hdw;
	// TODO: Replace Code with uint64 less version...
	int64_t tmp64_1;
	int64_t random_seed = 0;
	float delta = 1.0 / 16384.0;

	for(i = 0; i < ARRAYSIZE(_noiseTable); i++) {
		random_seed = random_seed * 214013 + 2531011;
		_noiseTable[i] = (delta * (float)(((int32)random_seed >> 16) & 0x00007FFF)- 1.0) * 1.3;
	}

	for (i = 0; i < 256; i++) {
		random_seed = 81;
		ldw = i;
		for (j = 0; j < 5; j++) {
			_randomDequantIndex[i][j] = (uint8)((ldw / random_seed) & 0xFF);
			ldw = (uint32)ldw % (uint32)random_seed;
			tmp64_1 = (random_seed * 0x55555556);
			hdw = (uint32)(tmp64_1 >> 32);
			random_seed = (int64_t)(hdw + (ldw >> 31));
		}
	}

	for (i = 0; i < 128; i++) {
		random_seed = 25;
		ldw = i;
		for (j = 0; j < 3; j++) {
			_randomDequantType24[i][j] = (uint8)((ldw / random_seed) & 0xFF);
			ldw = (uint32)ldw % (uint32)random_seed;
			tmp64_1 = (random_seed * 0x66666667);
			hdw = (uint32)(tmp64_1 >> 33);
			random_seed = hdw + (ldw >> 31);
		}
	}
}

void QDM2Stream::initNoiseSamples(void) {
	uint16 i;
	uint32 random_seed = 0;
	float delta = 1.0 / 16384.0;

	for (i = 0; i < ARRAYSIZE(_noiseSamples); i++) {
		random_seed = random_seed * 214013 + 2531011;
		_noiseSamples[i] = (delta * (float)((random_seed >> 16) & 0x00007fff) - 1.0);
	}
}

static const uint16 qdm2_vlc_offs[18] = {
	0, 260, 566, 598, 894, 1166, 1230, 1294, 1678, 1950, 2214, 2278, 2310, 2570, 2834, 3124, 3448, 3838
};

void QDM2Stream::initVlc(void) {
	static int16 qdm2_table[3838][2];

	if (!_vlcsInitialized) {
		_vlcTabLevel.table = &qdm2_table[qdm2_vlc_offs[0]];
		_vlcTabLevel.table_allocated = qdm2_vlc_offs[1] - qdm2_vlc_offs[0];
		_vlcTabLevel.table_size = 0;
		initVlcSparse(&_vlcTabLevel, 8, 24,
			vlc_tab_level_huffbits, 1, 1,
			vlc_tab_level_huffcodes, 2, 2, NULL, 0, 0);

		_vlcTabDiff.table = &qdm2_table[qdm2_vlc_offs[1]];
		_vlcTabDiff.table_allocated = qdm2_vlc_offs[2] - qdm2_vlc_offs[1];
		_vlcTabDiff.table_size = 0;
		initVlcSparse(&_vlcTabDiff, 8, 37,
			vlc_tab_diff_huffbits, 1, 1,
			vlc_tab_diff_huffcodes, 2, 2, NULL, 0, 0);

		_vlcTabRun.table = &qdm2_table[qdm2_vlc_offs[2]];
		_vlcTabRun.table_allocated = qdm2_vlc_offs[3] - qdm2_vlc_offs[2];
		_vlcTabRun.table_size = 0;
		initVlcSparse(&_vlcTabRun, 5, 6,
			vlc_tab_run_huffbits, 1, 1,
			vlc_tab_run_huffcodes, 1, 1, NULL, 0, 0);

		_fftLevelExpAltVlc.table = &qdm2_table[qdm2_vlc_offs[3]];
		_fftLevelExpAltVlc.table_allocated = qdm2_vlc_offs[4] - qdm2_vlc_offs[3];
		_fftLevelExpAltVlc.table_size = 0;
		initVlcSparse(&_fftLevelExpAltVlc, 8, 28,
			fft_level_exp_alt_huffbits, 1, 1,
			fft_level_exp_alt_huffcodes, 2, 2, NULL, 0, 0);

		_fftLevelExpVlc.table = &qdm2_table[qdm2_vlc_offs[4]];
		_fftLevelExpVlc.table_allocated = qdm2_vlc_offs[5] - qdm2_vlc_offs[4];
		_fftLevelExpVlc.table_size = 0;
		initVlcSparse(&_fftLevelExpVlc, 8, 20,
			fft_level_exp_huffbits, 1, 1,
			fft_level_exp_huffcodes, 2, 2, NULL, 0, 0);

		_fftStereoExpVlc.table = &qdm2_table[qdm2_vlc_offs[5]];
		_fftStereoExpVlc.table_allocated = qdm2_vlc_offs[6] - qdm2_vlc_offs[5];
		_fftStereoExpVlc.table_size = 0;
		initVlcSparse(&_fftStereoExpVlc, 6, 7,
			fft_stereo_exp_huffbits, 1, 1,
			fft_stereo_exp_huffcodes, 1, 1, NULL, 0, 0);

		_fftStereoPhaseVlc.table = &qdm2_table[qdm2_vlc_offs[6]];
		_fftStereoPhaseVlc.table_allocated = qdm2_vlc_offs[7] - qdm2_vlc_offs[6];
		_fftStereoPhaseVlc.table_size = 0;
		initVlcSparse(&_fftStereoPhaseVlc, 6, 9,
			fft_stereo_phase_huffbits, 1, 1,
			fft_stereo_phase_huffcodes, 1, 1, NULL, 0, 0);

		_vlcTabToneLevelIdxHi1.table = &qdm2_table[qdm2_vlc_offs[7]];
		_vlcTabToneLevelIdxHi1.table_allocated = qdm2_vlc_offs[8] - qdm2_vlc_offs[7];
		_vlcTabToneLevelIdxHi1.table_size = 0;
		initVlcSparse(&_vlcTabToneLevelIdxHi1, 8, 20,
			vlc_tab_tone_level_idx_hi1_huffbits, 1, 1,
			vlc_tab_tone_level_idx_hi1_huffcodes, 2, 2, NULL, 0, 0);

		_vlcTabToneLevelIdxMid.table = &qdm2_table[qdm2_vlc_offs[8]];
		_vlcTabToneLevelIdxMid.table_allocated = qdm2_vlc_offs[9] - qdm2_vlc_offs[8];
		_vlcTabToneLevelIdxMid.table_size = 0;
		initVlcSparse(&_vlcTabToneLevelIdxMid, 8, 24,
			vlc_tab_tone_level_idx_mid_huffbits, 1, 1,
			vlc_tab_tone_level_idx_mid_huffcodes, 2, 2, NULL, 0, 0);

		_vlcTabToneLevelIdxHi2.table = &qdm2_table[qdm2_vlc_offs[9]];
		_vlcTabToneLevelIdxHi2.table_allocated = qdm2_vlc_offs[10] - qdm2_vlc_offs[9];
		_vlcTabToneLevelIdxHi2.table_size = 0;
		initVlcSparse(&_vlcTabToneLevelIdxHi2, 8, 24,
			vlc_tab_tone_level_idx_hi2_huffbits, 1, 1,
			vlc_tab_tone_level_idx_hi2_huffcodes, 2, 2, NULL, 0, 0);

		_vlcTabType30.table = &qdm2_table[qdm2_vlc_offs[10]];
		_vlcTabType30.table_allocated = qdm2_vlc_offs[11] - qdm2_vlc_offs[10];
		_vlcTabType30.table_size = 0;
		initVlcSparse(&_vlcTabType30, 6, 9,
			vlc_tab_type30_huffbits, 1, 1,
			vlc_tab_type30_huffcodes, 1, 1, NULL, 0, 0);

		_vlcTabType34.table = &qdm2_table[qdm2_vlc_offs[11]];
		_vlcTabType34.table_allocated = qdm2_vlc_offs[12] - qdm2_vlc_offs[11];
		_vlcTabType34.table_size = 0;
		initVlcSparse(&_vlcTabType34, 5, 10,
			vlc_tab_type34_huffbits, 1, 1,
			vlc_tab_type34_huffcodes, 1, 1, NULL, 0, 0);

		_vlcTabFftToneOffset[0].table = &qdm2_table[qdm2_vlc_offs[12]];
		_vlcTabFftToneOffset[0].table_allocated = qdm2_vlc_offs[13] - qdm2_vlc_offs[12];
		_vlcTabFftToneOffset[0].table_size = 0;
		initVlcSparse(&_vlcTabFftToneOffset[0], 8, 23,
			vlc_tab_fft_tone_offset_0_huffbits, 1, 1,
			vlc_tab_fft_tone_offset_0_huffcodes, 2, 2, NULL, 0, 0);

		_vlcTabFftToneOffset[1].table = &qdm2_table[qdm2_vlc_offs[13]];
		_vlcTabFftToneOffset[1].table_allocated = qdm2_vlc_offs[14] - qdm2_vlc_offs[13];
		_vlcTabFftToneOffset[1].table_size = 0;
		initVlcSparse(&_vlcTabFftToneOffset[1], 8, 28,
			vlc_tab_fft_tone_offset_1_huffbits, 1, 1,
			vlc_tab_fft_tone_offset_1_huffcodes, 2, 2, NULL, 0, 0);

		_vlcTabFftToneOffset[2].table = &qdm2_table[qdm2_vlc_offs[14]];
		_vlcTabFftToneOffset[2].table_allocated = qdm2_vlc_offs[15] - qdm2_vlc_offs[14];
		_vlcTabFftToneOffset[2].table_size = 0;
		initVlcSparse(&_vlcTabFftToneOffset[2], 8, 32,
			vlc_tab_fft_tone_offset_2_huffbits, 1, 1,
			vlc_tab_fft_tone_offset_2_huffcodes, 2, 2, NULL, 0, 0);

		_vlcTabFftToneOffset[3].table = &qdm2_table[qdm2_vlc_offs[15]];
		_vlcTabFftToneOffset[3].table_allocated = qdm2_vlc_offs[16] - qdm2_vlc_offs[15];
		_vlcTabFftToneOffset[3].table_size = 0;
		initVlcSparse(&_vlcTabFftToneOffset[3], 8, 35,
			vlc_tab_fft_tone_offset_3_huffbits, 1, 1,
			vlc_tab_fft_tone_offset_3_huffcodes, 2, 2, NULL, 0, 0);

		_vlcTabFftToneOffset[4].table = &qdm2_table[qdm2_vlc_offs[16]];
		_vlcTabFftToneOffset[4].table_allocated = qdm2_vlc_offs[17] - qdm2_vlc_offs[16];
		_vlcTabFftToneOffset[4].table_size = 0;
		initVlcSparse(&_vlcTabFftToneOffset[4], 8, 38,
			vlc_tab_fft_tone_offset_4_huffbits, 1, 1,
			vlc_tab_fft_tone_offset_4_huffcodes, 2, 2, NULL, 0, 0);

		_vlcsInitialized = true;
	}
}

QDM2Stream::QDM2Stream(Common::SeekableReadStream *extraData, DisposeAfterUse::Flag disposeExtraData) {
	uint32 tmp;
	int tmp_val;
	int i;

	debug(1, "QDM2Stream::QDM2Stream() Call");

	_compressedData = NULL;
	_subPacket = 0;
	_superBlockStart = 0;
	memset(_quantizedCoeffs, 0, sizeof(_quantizedCoeffs));
	memset(_fftLevelExp, 0, sizeof(_fftLevelExp));
	_noiseIdx = 0;
	memset(_fftCoefsMinIndex, 0, sizeof(_fftCoefsMinIndex));
	memset(_fftCoefsMaxIndex, 0, sizeof(_fftCoefsMaxIndex));
	_fftToneStart = 0;
	_fftToneEnd = 0;
	for(i = 0; i < ARRAYSIZE(_subPacketListA); i++) {
		_subPacketListA[i].packet = NULL;
		_subPacketListA[i].next = NULL;
	}
	_subPacketsB = 0;
	for(i = 0; i < ARRAYSIZE(_subPacketListB); i++) {
		_subPacketListB[i].packet = NULL;
		_subPacketListB[i].next = NULL;
	}
	for(i = 0; i < ARRAYSIZE(_subPacketListC); i++) {
		_subPacketListC[i].packet = NULL;
		_subPacketListC[i].next = NULL;
	}
	for(i = 0; i < ARRAYSIZE(_subPacketListD); i++) {
		_subPacketListD[i].packet = NULL;
		_subPacketListD[i].next = NULL;
	}
	memset(_synthBuf, 0, sizeof(_synthBuf));
	memset(_synthBufOffset, 0, sizeof(_synthBufOffset));
	memset(_sbSamples, 0, sizeof(_sbSamples));
	memset(_outputBuffer, 0, sizeof(_outputBuffer));
	_vlcsInitialized = false;
	_superblocktype_2_3 = 0;
	_hasErrors = false;

	// The QDM2 "extra data" is really just an amalgam of three QuickTime
	// atoms needed to correctly set up the decoder.

	// Rewind extraData stream from any previous calls
	extraData->seek(0, SEEK_SET);

	// First, the frma atom
	uint32 frmaSize = extraData->readUint32BE();
	if (frmaSize != 12)
		error("Invalid QDM2 frma atom");

	if (extraData->readUint32BE() != MKTAG('f', 'r', 'm', 'a'))
		error("Failed to find frma atom for QDM2");

	uint32 version = extraData->readUint32BE();
	if (version == MKTAG('Q', 'D', 'M', 'C'))
		error("Unhandled QDMC sound");
	else if (version != MKTAG('Q', 'D', 'M', '2'))
		error("Failed to find QDM2 tag in frma atom");

	// Second, the QDCA atom
	uint32 qdcaSize = extraData->readUint32BE();
	if (qdcaSize > (uint32)(extraData->size() - extraData->pos()))
		error("Invalid QDM2 QDCA atom");

	if (extraData->readUint32BE() != MKTAG('Q', 'D', 'C', 'A'))
		error("Failed to find QDCA atom for QDM2");

	extraData->readUint32BE(); // unknown

	_channels = extraData->readUint32BE();
	_sampleRate = extraData->readUint32BE();
	_bitRate = extraData->readUint32BE();
	_blockSize = extraData->readUint32BE();
	_frameSize = extraData->readUint32BE();
	_packetSize = extraData->readUint32BE();

	// Third, we don't care about the QDCP atom

	_fftOrder = Common::intLog2(_frameSize) + 1;
	_fftFrameSize = 2 * _frameSize; // complex has two floats

	// something like max decodable tones
	_groupOrder = Common::intLog2(_blockSize) + 1;
	_sFrameSize = _blockSize / 16; // 16 iterations per super block

	_subSampling = _fftOrder - 7;
	_frequencyRange = 255 / (1 << (2 - _subSampling));

	switch (_subSampling * 2 + _channels - 1) {
		case 0:
			tmp = 40;
			break;
		case 1:
			tmp = 48;
			break;
		case 2:
			tmp = 56;
			break;
		case 3:
			tmp = 72;
			break;
		case 4:
			tmp = 80;
			break;
		case 5:
			tmp = 100;
			break;
		default:
			tmp = _subSampling;
			break;
	}

	tmp_val = 0;
	if ((tmp * 1000) < _bitRate)  tmp_val = 1;
	if ((tmp * 1440) < _bitRate)  tmp_val = 2;
	if ((tmp * 1760) < _bitRate)  tmp_val = 3;
	if ((tmp * 2240) < _bitRate)  tmp_val = 4;
	_cmTableSelect = tmp_val;

	if (_subSampling == 0)
		tmp = 7999;
	else
		tmp = ((-(_subSampling -1)) & 8000) + 20000;

	if (tmp < 8000)
		_coeffPerSbSelect = 0;
	else if (tmp <= 16000)
		_coeffPerSbSelect = 1;
	else
		_coeffPerSbSelect = 2;

	if (_fftOrder < 7 || _fftOrder > 9)
		error("QDM2Stream::QDM2Stream() Unsupported fft_order: %d", _fftOrder);

	_rdft = new Common::RDFT(_fftOrder, Common::RDFT::IDFT_C2R);

	initVlc();
	ff_mpa_synth_init(ff_mpa_synth_window);
	softclipTableInit();
	rndTableInit();
	initNoiseSamples();

	_compressedData = new uint8[_packetSize];

	if (disposeExtraData == DisposeAfterUse::YES)
		delete extraData;
}

QDM2Stream::~QDM2Stream() {
	delete _rdft;
	delete[] _compressedData;
}

static int qdm2_get_vlc(Common::BitStream *gb, VLC *vlc, int flag, int depth) {
	int value = getVlc2(gb, vlc->table, vlc->bits, depth);

	// stage-2, 3 bits exponent escape sequence
	if (value-- == 0)
		value = gb->getBits(gb->getBits(3) + 1);

	// stage-3, optional
	if (flag) {
		int tmp = vlc_stage3_values[value];

		if ((value & ~3) > 0)
			tmp += gb->getBits(value >> 2);
		value = tmp;
	}

	return value;
}

static int qdm2_get_se_vlc(VLC *vlc, Common::BitStream *gb, int depth)
{
	int value = qdm2_get_vlc(gb, vlc, 0, depth);

	return (value & 1) ? ((value + 1) >> 1) : -(value >> 1);
}

/**
 * QDM2 checksum
 *
 * @param data      pointer to data to be checksum'ed
 * @param length    data length
 * @param value     checksum value
 *
 * @return          0 if checksum is OK
 */
static uint16 qdm2_packet_checksum(const uint8 *data, int length, int value) {
	int i;

	for (i = 0; i < length; i++)
		value -= data[i];

	return (uint16)(value & 0xffff);
}

/**
 * Return node pointer to first packet of requested type in list.
 *
 * @param list    list of subpackets to be scanned
 * @param type    type of searched subpacket
 * @return        node pointer for subpacket if found, else NULL
 */
static QDM2SubPNode* qdm2_search_subpacket_type_in_list(QDM2SubPNode *list, int type)
{
	while (list != NULL && list->packet != NULL) {
		if (list->packet->type == type)
			return list;
		list = list->next;
	}
	return NULL;
}

/**
 * Replaces 8 elements with their average value.
 * Called by qdm2_decode_superblock before starting subblock decoding.
 */
void QDM2Stream::average_quantized_coeffs(void) {
	int i, j, n, ch, sum;

	n = coeff_per_sb_for_avg[_coeffPerSbSelect][QDM2_SB_USED(_subSampling) - 1] + 1;

	for (ch = 0; ch < _channels; ch++) {
		for (i = 0; i < n; i++) {
			sum = 0;

			for (j = 0; j < 8; j++)
				sum += _quantizedCoeffs[ch][i][j];

			sum /= 8;
			if (sum > 0)
				sum--;

			for (j = 0; j < 8; j++)
				_quantizedCoeffs[ch][i][j] = sum;
		}
	}
}

/**
 * Build subband samples with noise weighted by q->tone_level.
 * Called by synthfilt_build_sb_samples.
 *
 * @param sb    subband index
 */
void QDM2Stream::build_sb_samples_from_noise(int sb) {
	int ch, j;

	FIX_NOISE_IDX(_noiseIdx);

	if (!_channels)
		return;

	for (ch = 0; ch < _channels; ch++) {
		for (j = 0; j < 64; j++) {
			_sbSamples[ch][j * 2][sb] = (int32)(SB_DITHERING_NOISE(sb, _noiseIdx) * _toneLevel[ch][sb][j] + .5);
			_sbSamples[ch][j * 2 + 1][sb] = (int32)(SB_DITHERING_NOISE(sb, _noiseIdx) * _toneLevel[ch][sb][j] + .5);
		}
	}
}

/**
 * Called while processing data from subpackets 11 and 12.
 * Used after making changes to coding_method array.
 *
 * @param sb               subband index
 * @param channels         number of channels
 * @param coding_method    q->coding_method[0][0][0]
 */
void QDM2Stream::fix_coding_method_array(int sb, int channels, sb_int8_array coding_method)
{
	int j, k;
	int ch;
	int run, case_val;
	int switchtable[23] = {0,5,1,5,5,5,5,5,2,5,5,5,5,5,5,5,3,5,5,5,5,5,4};

	for (ch = 0; ch < channels; ch++) {
		for (j = 0; j < 64; ) {
			if ((coding_method[ch][sb][j] - 8) > 22) {
				run = 1;
				case_val = 8;
			} else {
				switch (switchtable[coding_method[ch][sb][j]-8]) {
					case 0: run = 10; case_val = 10; break;
					case 1: run = 1; case_val = 16; break;
					case 2: run = 5; case_val = 24; break;
					case 3: run = 3; case_val = 30; break;
					case 4: run = 1; case_val = 30; break;
					case 5: run = 1; case_val = 8; break;
					default: run = 1; case_val = 8; break;
				}
			}
			for (k = 0; k < run; k++)
				if (j + k < 128)
					if (coding_method[ch][sb + (j + k) / 64][(j + k) % 64] > coding_method[ch][sb][j])
						if (k > 0) {
							warning("QDM2 Untested Code: not debugged, almost never used");
							memset(&coding_method[ch][sb][j + k], case_val, k * sizeof(int8));
							memset(&coding_method[ch][sb][j + k], case_val, 3 * sizeof(int8));
						}
			j += run;
		}
	}
}

/**
 * Related to synthesis filter
 * Called by process_subpacket_10
 *
 * @param flag    1 if called after getting data from subpacket 10, 0 if no subpacket 10
 */
void QDM2Stream::fill_tone_level_array(int flag) {
	int i, sb, ch, sb_used;
	int tmp, tab;

	// This should never happen
	if (_channels <= 0)
		return;

	for (ch = 0; ch < _channels; ch++) {
		for (sb = 0; sb < 30; sb++) {
			for (i = 0; i < 8; i++) {
				if ((tab=coeff_per_sb_for_dequant[_coeffPerSbSelect][sb]) < (last_coeff[_coeffPerSbSelect] - 1))
					tmp = _quantizedCoeffs[ch][tab + 1][i] * dequant_table[_coeffPerSbSelect][tab + 1][sb]+
					      _quantizedCoeffs[ch][tab][i] * dequant_table[_coeffPerSbSelect][tab][sb];
				else
					tmp = _quantizedCoeffs[ch][tab][i] * dequant_table[_coeffPerSbSelect][tab][sb];
				if(tmp < 0)
					tmp += 0xff;
				_toneLevelIdxBase[ch][sb][i] = (tmp / 256) & 0xff;
			}
		}
	}

	sb_used = QDM2_SB_USED(_subSampling);

	if ((_superblocktype_2_3 != 0) && !flag) {
		for (sb = 0; sb < sb_used; sb++) {
			for (ch = 0; ch < _channels; ch++) {
				for (i = 0; i < 64; i++) {
					_toneLevelIdx[ch][sb][i] = _toneLevelIdxBase[ch][sb][i / 8];
					if (_toneLevelIdx[ch][sb][i] < 0)
						_toneLevel[ch][sb][i] = 0;
					else
						_toneLevel[ch][sb][i] = fft_tone_level_table[0][_toneLevelIdx[ch][sb][i] & 0x3f];
				}
			}
		}
	} else {
		tab = _superblocktype_2_3 ? 0 : 1;
		for (sb = 0; sb < sb_used; sb++) {
			if ((sb >= 4) && (sb <= 23)) {
				for (ch = 0; ch < _channels; ch++) {
					for (i = 0; i < 64; i++) {
						tmp = _toneLevelIdxBase[ch][sb][i / 8] -
						      _toneLevelIdxHi1[ch][sb / 8][i / 8][i % 8] -
						      _toneLevelIdxMid[ch][sb - 4][i / 8] -
						      _toneLevelIdxHi2[ch][sb - 4];
						_toneLevelIdx[ch][sb][i] = tmp & 0xff;
						if ((tmp < 0) || (!_superblocktype_2_3 && !tmp))
							_toneLevel[ch][sb][i] = 0;
						else
							_toneLevel[ch][sb][i] = fft_tone_level_table[tab][tmp & 0x3f];
					}
				}
			} else {
				if (sb > 4) {
					for (ch = 0; ch < _channels; ch++) {
						for (i = 0; i < 64; i++) {
							tmp = _toneLevelIdxBase[ch][sb][i / 8] -
							      _toneLevelIdxHi1[ch][2][i / 8][i % 8] -
							      _toneLevelIdxHi2[ch][sb - 4];
							_toneLevelIdx[ch][sb][i] = tmp & 0xff;
							if ((tmp < 0) || (!_superblocktype_2_3 && !tmp))
								_toneLevel[ch][sb][i] = 0;
							else
								_toneLevel[ch][sb][i] = fft_tone_level_table[tab][tmp & 0x3f];
						}
					}
				} else {
					for (ch = 0; ch < _channels; ch++) {
						for (i = 0; i < 64; i++) {
							tmp = _toneLevelIdx[ch][sb][i] = _toneLevelIdxBase[ch][sb][i / 8];
							if ((tmp < 0) || (!_superblocktype_2_3 && !tmp))
								_toneLevel[ch][sb][i] = 0;
							else
								_toneLevel[ch][sb][i] = fft_tone_level_table[tab][tmp & 0x3f];
						}
					}
				}
			}
		}
	}
}

/**
 * Related to synthesis filter
 * Called by process_subpacket_11
 * c is built with data from subpacket 11
 * Most of this function is used only if superblock_type_2_3 == 0, never seen it in samples
 *
 * @param tone_level_idx
 * @param tone_level_idx_temp
 * @param coding_method        q->coding_method[0][0][0]
 * @param nb_channels          number of channels
 * @param c                    coming from subpacket 11, passed as 8*c
 * @param superblocktype_2_3   flag based on superblock packet type
 * @param cm_table_select      q->cm_table_select
 */
void QDM2Stream::fill_coding_method_array(sb_int8_array tone_level_idx, sb_int8_array tone_level_idx_temp,
                sb_int8_array coding_method, int nb_channels,
                int c, int superblocktype_2_3, int cm_table_select) {
	int ch, sb, j;
	int tmp, acc, esp_40, comp;
	int add1, add2, add3, add4;
	// TODO : Remove multres 64 bit variable necessity...
	int64_t multres;

	// This should never happen
	if (nb_channels <= 0)
		return;
	if (!superblocktype_2_3) {
		warning("QDM2 This case is untested, no samples available");
		for (ch = 0; ch < nb_channels; ch++)
			for (sb = 0; sb < 30; sb++) {
				for (j = 1; j < 63; j++) {  // The loop only iterates to 63 so the code doesn't overflow the buffer
					add1 = tone_level_idx[ch][sb][j] - 10;
					if (add1 < 0)
						add1 = 0;
					add2 = add3 = add4 = 0;
					if (sb > 1) {
						add2 = tone_level_idx[ch][sb - 2][j] + tone_level_idx_offset_table[sb][0] - 6;
						if (add2 < 0)
							add2 = 0;
					}
					if (sb > 0) {
						add3 = tone_level_idx[ch][sb - 1][j] + tone_level_idx_offset_table[sb][1] - 6;
						if (add3 < 0)
							add3 = 0;
					}
					if (sb < 29) {
						add4 = tone_level_idx[ch][sb + 1][j] + tone_level_idx_offset_table[sb][3] - 6;
						if (add4 < 0)
							add4 = 0;
					}
					tmp = tone_level_idx[ch][sb][j + 1] * 2 - add4 - add3 - add2 - add1;
					if (tmp < 0)
						tmp = 0;
					tone_level_idx_temp[ch][sb][j + 1] = tmp & 0xff;
				}
				tone_level_idx_temp[ch][sb][0] = tone_level_idx_temp[ch][sb][1];
			}
			acc = 0;
			for (ch = 0; ch < nb_channels; ch++)
				for (sb = 0; sb < 30; sb++)
					for (j = 0; j < 64; j++)
						acc += tone_level_idx_temp[ch][sb][j];

			multres = 0x66666667 * (acc * 10);
			esp_40 = (multres >> 32) / 8 + ((multres & 0xffffffff) >> 31);
			for (ch = 0;  ch < nb_channels; ch++)
				for (sb = 0; sb < 30; sb++)
					for (j = 0; j < 64; j++) {
						comp = tone_level_idx_temp[ch][sb][j]* esp_40 * 10;
						if (comp < 0)
							comp += 0xff;
						comp /= 256; // signed shift
						switch(sb) {
							case 0:
								if (comp < 30)
									comp = 30;
								comp += 15;
								break;
							case 1:
								if (comp < 24)
									comp = 24;
								comp += 10;
								break;
							case 2:
							case 3:
							case 4:
								if (comp < 16)
									comp = 16;
						}
						if (comp <= 5)
							tmp = 0;
						else if (comp <= 10)
							tmp = 10;
						else if (comp <= 16)
							tmp = 16;
						else if (comp <= 24)
							tmp = -1;
						else
							tmp = 0;
						coding_method[ch][sb][j] = ((tmp & 0xfffa) + 30 )& 0xff;
					}
			for (sb = 0; sb < 30; sb++)
				fix_coding_method_array(sb, nb_channels, coding_method);
			for (ch = 0; ch < nb_channels; ch++)
				for (sb = 0; sb < 30; sb++)
					for (j = 0; j < 64; j++)
						if (sb >= 10) {
							if (coding_method[ch][sb][j] < 10)
								coding_method[ch][sb][j] = 10;
						} else {
							if (sb >= 2) {
								if (coding_method[ch][sb][j] < 16)
									coding_method[ch][sb][j] = 16;
							} else {
								if (coding_method[ch][sb][j] < 30)
									coding_method[ch][sb][j] = 30;
							}
						}
	} else { // superblocktype_2_3 != 0
		for (ch = 0; ch < nb_channels; ch++)
			for (sb = 0; sb < 30; sb++)
				for (j = 0; j < 64; j++)
					coding_method[ch][sb][j] = coding_method_table[cm_table_select][sb];
	}
}

/**
 *
 * Called by process_subpacket_11 to process more data from subpacket 11 with sb 0-8
 * Called by process_subpacket_12 to process data from subpacket 12 with sb 8-sb_used
 *
 * @param gb        bitreader context
 * @param length    packet length in bits
 * @param sb_min    lower subband processed (sb_min included)
 * @param sb_max    higher subband processed (sb_max excluded)
 */
void QDM2Stream::synthfilt_build_sb_samples(Common::BitStream *gb, int length, int sb_min, int sb_max) {
	int sb, j, k, n, ch, run, channels;
	int joined_stereo, zero_encoding, chs;
	int type34_first;
	float type34_div = 0;
	float type34_predictor;
	float samples[10], sign_bits[16];

	if (length == 0) {
		// If no data use noise
		for (sb = sb_min; sb < sb_max; sb++)
			build_sb_samples_from_noise(sb);

		return;
	}

	for (sb = sb_min; sb < sb_max; sb++) {
		FIX_NOISE_IDX(_noiseIdx);

		channels = _channels;

		if (_channels <= 1 || sb < 12)
			joined_stereo = 0;
		else if (sb >= 24)
			joined_stereo = 1;
		else
			joined_stereo = ((length - gb->pos()) >= 1) ? gb->getBit() : 0;

		if (joined_stereo) {
			if ((length - gb->pos()) >= 16)
				for (j = 0; j < 16; j++)
					sign_bits[j] = gb->getBit();

			for (j = 0; j < 64; j++)
				if (_codingMethod[1][sb][j] > _codingMethod[0][sb][j])
					_codingMethod[0][sb][j] = _codingMethod[1][sb][j];

			fix_coding_method_array(sb, _channels, _codingMethod);
			channels = 1;
		}

		for (ch = 0; ch < channels; ch++) {
			zero_encoding = ((length - gb->pos()) >= 1) ? gb->getBit() : 0;
			type34_predictor = 0.0;
			type34_first = 1;

			for (j = 0; j < 128; ) {
				switch (_codingMethod[ch][sb][j / 2]) {
					case 8:
						if ((length - gb->pos()) >= 10) {
							if (zero_encoding) {
								for (k = 0; k < 5; k++) {
								if ((j + 2 * k) >= 128)
									break;
									samples[2 * k] = gb->getBit() ? dequant_1bit[joined_stereo][2 * gb->getBit()] : 0;
								}
							} else {
								n = gb->getBits(8);
								for (k = 0; k < 5; k++)
									samples[2 * k] = dequant_1bit[joined_stereo][_randomDequantIndex[n][k]];
							}
							for (k = 0; k < 5; k++)
								samples[2 * k + 1] = SB_DITHERING_NOISE(sb, _noiseIdx);
						} else {
							for (k = 0; k < 10; k++)
								samples[k] = SB_DITHERING_NOISE(sb, _noiseIdx);
						}
						run = 10;
						break;

					case 10:
						if ((length - gb->pos()) >= 1) {
							double f = 0.81;

							if (gb->getBit())
								f = -f;
							f -= _noiseSamples[((sb + 1) * (j +5 * ch + 1)) & 127] * 9.0 / 40.0;
							samples[0] = f;
						} else {
							samples[0] = SB_DITHERING_NOISE(sb, _noiseIdx);
						}
						run = 1;
						break;

					case 16:
						if ((length - gb->pos()) >= 10) {
							if (zero_encoding) {
								for (k = 0; k < 5; k++) {
									if ((j + k) >= 128)
										break;
									samples[k] = (gb->getBit() == 0) ? 0 : dequant_1bit[joined_stereo][2 * gb->getBit()];
								}
							} else {
								n = gb->getBits(8);
								for (k = 0; k < 5; k++)
									samples[k] = dequant_1bit[joined_stereo][_randomDequantIndex[n][k]];
							}
						} else {
							for (k = 0; k < 5; k++)
								samples[k] = SB_DITHERING_NOISE(sb, _noiseIdx);
						}
						run = 5;
						break;

					case 24:
						if ((length - gb->pos()) >= 7) {
							n = gb->getBits(7);
							for (k = 0; k < 3; k++)
								samples[k] = (_randomDequantType24[n][k] - 2.0) * 0.5;
						} else {
							for (k = 0; k < 3; k++)
								samples[k] = SB_DITHERING_NOISE(sb, _noiseIdx);
						}
						run = 3;
						break;

					case 30:
						if ((length - gb->pos()) >= 4)
							samples[0] = type30_dequant[qdm2_get_vlc(gb, &_vlcTabType30, 0, 1)];
						else
							samples[0] = SB_DITHERING_NOISE(sb, _noiseIdx);

						run = 1;
						break;

					case 34:
						if ((length - gb->pos()) >= 7) {
							if (type34_first) {
								type34_div = (float)(1 << gb->getBits(2));
								samples[0] = ((float)gb->getBits(5) - 16.0) / 15.0;
								type34_predictor = samples[0];
								type34_first = 0;
							} else {
								samples[0] = type34_delta[qdm2_get_vlc(gb, &_vlcTabType34, 0, 1)] / type34_div + type34_predictor;
								type34_predictor = samples[0];
							}
						} else {
							samples[0] = SB_DITHERING_NOISE(sb, _noiseIdx);
						}
						run = 1;
						break;

					default:
						samples[0] = SB_DITHERING_NOISE(sb, _noiseIdx);
						run = 1;
						break;
				}

				if (joined_stereo) {
					float tmp[10][MPA_MAX_CHANNELS];

					for (k = 0; k < run; k++) {
						tmp[k][0] = samples[k];
						tmp[k][1] = (sign_bits[(j + k) / 8]) ? -samples[k] : samples[k];
					}
					for (chs = 0; chs < _channels; chs++)
						for (k = 0; k < run; k++)
							if ((j + k) < 128)
								_sbSamples[chs][j + k][sb] = (int32)(_toneLevel[chs][sb][((j + k)/2)] * tmp[k][chs] + .5);
				} else {
					for (k = 0; k < run; k++)
						if ((j + k) < 128)
							_sbSamples[ch][j + k][sb] = (int32)(_toneLevel[ch][sb][(j + k)/2] * samples[k] + .5);
				}

				j += run;
			} // j loop
		} // channel loop
	} // subband loop
}

/**
 * Init the first element of a channel in quantized_coeffs with data from packet 10 (quantized_coeffs[ch][0]).
 * This is similar to process_subpacket_9, but for a single channel and for element [0]
 * same VLC tables as process_subpacket_9 are used.
 *
 * @param quantized_coeffs    pointer to quantized_coeffs[ch][0]
 * @param gb        bitreader context
 * @param length    packet length in bits
 */
void QDM2Stream::init_quantized_coeffs_elem0(int8 *quantized_coeffs, Common::BitStream *gb, int length) {
	int i, k, run, level, diff;

	if ((length - gb->pos()) < 16)
		return;
	level = qdm2_get_vlc(gb, &_vlcTabLevel, 0, 2);

	quantized_coeffs[0] = level;

	for (i = 0; i < 7; ) {
		if ((length - gb->pos()) < 16)
			break;
		run = qdm2_get_vlc(gb, &_vlcTabRun, 0, 1) + 1;

		if ((length - gb->pos()) < 16)
			break;
		diff = qdm2_get_se_vlc(&_vlcTabDiff, gb, 2);

		for (k = 1; k <= run; k++)
			quantized_coeffs[i + k] = (level + ((k * diff) / run));

		level += diff;
		i += run;
	}
}

/**
 * Related to synthesis filter, process data from packet 10
 * Init part of quantized_coeffs via function init_quantized_coeffs_elem0
 * Init tone_level_idx_hi1, tone_level_idx_hi2, tone_level_idx_mid with data from packet 10
 *
 * @param gb        bitreader context
 * @param length    packet length in bits
 */
void QDM2Stream::init_tone_level_dequantization(Common::BitStream *gb, int length) {
	int sb, j, k, n, ch;

	for (ch = 0; ch < _channels; ch++) {
		init_quantized_coeffs_elem0(_quantizedCoeffs[ch][0], gb, length);

		if ((length - gb->pos()) < 16) {
			memset(_quantizedCoeffs[ch][0], 0, 8);
			break;
		}
	}

	n = _subSampling + 1;

	for (sb = 0; sb < n; sb++)
		for (ch = 0; ch < _channels; ch++)
			for (j = 0; j < 8; j++) {
				if ((length - gb->pos()) < 1)
					break;
				if (gb->getBit()) {
					for (k=0; k < 8; k++) {
						if ((length - gb->pos()) < 16)
							break;
						_toneLevelIdxHi1[ch][sb][j][k] = qdm2_get_vlc(gb, &_vlcTabToneLevelIdxHi1, 0, 2);
					}
				} else {
					for (k=0; k < 8; k++)
						_toneLevelIdxHi1[ch][sb][j][k] = 0;
				}
			}

	n = QDM2_SB_USED(_subSampling) - 4;

	for (sb = 0; sb < n; sb++)
		for (ch = 0; ch < _channels; ch++) {
			if ((length - gb->pos()) < 16)
				break;
			_toneLevelIdxHi2[ch][sb] = qdm2_get_vlc(gb, &_vlcTabToneLevelIdxHi2, 0, 2);
			if (sb > 19)
				_toneLevelIdxHi2[ch][sb] -= 16;
			else
				for (j = 0; j < 8; j++)
					_toneLevelIdxMid[ch][sb][j] = -16;
		}

	n = QDM2_SB_USED(_subSampling) - 5;

	for (sb = 0; sb < n; sb++) {
		for (ch = 0; ch < _channels; ch++) {
			for (j = 0; j < 8; j++) {
				if ((length - gb->pos()) < 16)
					break;
				_toneLevelIdxMid[ch][sb][j] = qdm2_get_vlc(gb, &_vlcTabToneLevelIdxMid, 0, 2) - 32;
			}
		}
	}
}

/**
 * Process subpacket 9, init quantized_coeffs with data from it
 *
 * @param node    pointer to node with packet
 */
void QDM2Stream::process_subpacket_9(QDM2SubPNode *node) {
	int i, j, k, n, ch, run, level, diff;

	Common::MemoryReadStream d(node->packet->data, node->packet->size*8);
	Common::BitStream32LELSB gb(&d);

	n = coeff_per_sb_for_avg[_coeffPerSbSelect][QDM2_SB_USED(_subSampling) - 1] + 1; // same as averagesomething function

	for (i = 1; i < n; i++)
		for (ch = 0; ch < _channels; ch++) {
			level = qdm2_get_vlc(&gb, &_vlcTabLevel, 0, 2);
			_quantizedCoeffs[ch][i][0] = level;

			for (j = 0; j < (8 - 1); ) {
				run = qdm2_get_vlc(&gb, &_vlcTabRun, 0, 1) + 1;
				diff = qdm2_get_se_vlc(&_vlcTabDiff, &gb, 2);

				for (k = 1; k <= run; k++)
					_quantizedCoeffs[ch][i][j + k] = (level + ((k*diff) / run));

				level += diff;
				j += run;
			}
		}

	for (ch = 0; ch < _channels; ch++)
		for (i = 0; i < 8; i++)
			_quantizedCoeffs[ch][0][i] = 0;
}

/**
 * Process subpacket 10 if not null, else
 *
 * @param node      pointer to node with packet
 * @param length    packet length in bits
 */
void QDM2Stream::process_subpacket_10(QDM2SubPNode *node, int length) {
	Common::MemoryReadStream d(((node == NULL) ? _emptyBuffer : node->packet->data), ((node == NULL) ? 0 : node->packet->size*8));
	Common::BitStream32LELSB gb(&d);

	if (length != 0) {
		init_tone_level_dequantization(&gb, length);
		fill_tone_level_array(1);
	} else {
		fill_tone_level_array(0);
	}
}

/**
 * Process subpacket 11
 *
 * @param node      pointer to node with packet
 * @param length    packet length in bit
 */
void QDM2Stream::process_subpacket_11(QDM2SubPNode *node, int length) {
	Common::MemoryReadStream d(((node == NULL) ? _emptyBuffer : node->packet->data), ((node == NULL) ? 0 : node->packet->size*8));
	Common::BitStream32LELSB gb(&d);

	if (length >= 32) {
		int c = gb.getBits(13);

		if (c > 3)
			fill_coding_method_array(_toneLevelIdx, _toneLevelIdxTemp, _codingMethod,
			                         _channels, 8*c, _superblocktype_2_3, _cmTableSelect);
	}

	synthfilt_build_sb_samples(&gb, length, 0, 8);
}

/**
 * Process subpacket 12
 *
 * @param node      pointer to node with packet
 * @param length    packet length in bits
 */
void QDM2Stream::process_subpacket_12(QDM2SubPNode *node, int length) {
	Common::MemoryReadStream d(((node == NULL) ? _emptyBuffer : node->packet->data), ((node == NULL) ? 0 : node->packet->size*8));
	Common::BitStream32LELSB gb(&d);

	synthfilt_build_sb_samples(&gb, length, 8, QDM2_SB_USED(_subSampling));
}

/*
 * Process new subpackets for synthesis filter
 *
 * @param list    list with synthesis filter packets (list D)
 */
void QDM2Stream::process_synthesis_subpackets(QDM2SubPNode *list) {
	struct QDM2SubPNode *nodes[4];

	nodes[0] = qdm2_search_subpacket_type_in_list(list, 9);
	if (nodes[0] != NULL)
		process_subpacket_9(nodes[0]);

	nodes[1] = qdm2_search_subpacket_type_in_list(list, 10);
	if (nodes[1] != NULL)
		process_subpacket_10(nodes[1], nodes[1]->packet->size << 3);
	else
		process_subpacket_10(NULL, 0);

	nodes[2] = qdm2_search_subpacket_type_in_list(list, 11);
	if (nodes[0] != NULL && nodes[1] != NULL && nodes[2] != NULL)
		process_subpacket_11(nodes[2], (nodes[2]->packet->size << 3));
	else
		process_subpacket_11(NULL, 0);

	nodes[3] = qdm2_search_subpacket_type_in_list(list, 12);
	if (nodes[0] != NULL && nodes[1] != NULL && nodes[3] != NULL)
		process_subpacket_12(nodes[3], (nodes[3]->packet->size << 3));
	else
		process_subpacket_12(NULL, 0);
}

/*
 * Decode superblock, fill packet lists.
 *
 */
void QDM2Stream::qdm2_decode_super_block(void) {
	struct QDM2SubPacket header, *packet;
	int i, packet_bytes, sub_packet_size, subPacketsD;
	unsigned int next_index = 0;

	memset(_toneLevelIdxHi1, 0, sizeof(_toneLevelIdxHi1));
	memset(_toneLevelIdxMid, 0, sizeof(_toneLevelIdxMid));
	memset(_toneLevelIdxHi2, 0, sizeof(_toneLevelIdxHi2));

	_subPacketsB = 0;
	subPacketsD = 0;

	average_quantized_coeffs(); // average elements in quantized_coeffs[max_ch][10][8]

	Common::MemoryReadStream *d = new Common::MemoryReadStream(_compressedData, _packetSize*8);
	Common::BitStream *gb = new Common::BitStream32LELSB(d);
	//qdm2_decode_sub_packet_header
	header.type = gb->getBits(8);

	if (header.type == 0) {
		header.size = 0;
		header.data = NULL;
	} else {
		header.size = gb->getBits(8);

		if (header.type & 0x80) {
			header.size <<= 8;
			header.size |= gb->getBits(8);
			header.type &= 0x7f;
		}

		if (header.type == 0x7f)
			header.type |= (gb->getBits(8) << 8);

		header.data = &_compressedData[gb->pos() / 8];
	}

	if (header.type < 2 || header.type >= 8) {
		_hasErrors = true;
		error("QDM2 : bad superblock type");
		return;
	}

	_superblocktype_2_3 = (header.type == 2 || header.type == 3);
	packet_bytes = (_packetSize - gb->pos() / 8);

	delete gb;
	delete d;
	d = new Common::MemoryReadStream(header.data, header.size*8);
	gb = new Common::BitStream32LELSB(d);

	if (header.type == 2 || header.type == 4 || header.type == 5) {
		int csum = 257 * gb->getBits(8) + 2 * gb->getBits(8);

		csum = qdm2_packet_checksum(_compressedData, _packetSize, csum);

		if (csum != 0) {
			_hasErrors = true;
			error("QDM2 : bad packet checksum");
			return;
		}
	}

	_subPacketListB[0].packet = NULL;
	_subPacketListD[0].packet = NULL;

	for (i = 0; i < 6; i++)
		if (--_fftLevelExp[i] < 0)
			_fftLevelExp[i] = 0;

	for (i = 0; packet_bytes > 0; i++) {
		int j;

		_subPacketListA[i].next = NULL;

		if (i > 0) {
			_subPacketListA[i - 1].next = &_subPacketListA[i];

			// seek to next block
			delete gb;
			delete d;
			d = new Common::MemoryReadStream(header.data, header.size*8);
			gb = new Common::BitStream32LELSB(d);
			gb->skip(next_index*8);

			if (next_index >= header.size)
				break;
		}

		// decode subpacket
		packet = &_subPackets[i];
		//qdm2_decode_sub_packet_header
		packet->type = gb->getBits(8);

		if (packet->type == 0) {
			packet->size = 0;
			packet->data = NULL;
		} else {
			packet->size = gb->getBits(8);

			if (packet->type & 0x80) {
				packet->size <<= 8;
				packet->size |= gb->getBits(8);
				packet->type &= 0x7f;
			}

			if (packet->type == 0x7f)
				packet->type |= (gb->getBits(8) << 8);

			packet->data = &header.data[gb->pos() / 8];
		}

		next_index = packet->size + gb->pos() / 8;
		sub_packet_size = ((packet->size > 0xff) ? 1 : 0) + packet->size + 2;

		if (packet->type == 0)
			break;

		if (sub_packet_size > packet_bytes) {
			if (packet->type != 10 && packet->type != 11 && packet->type != 12)
				break;
			packet->size += packet_bytes - sub_packet_size;
		}

		packet_bytes -= sub_packet_size;

		// add subpacket to 'all subpackets' list
		_subPacketListA[i].packet = packet;

		// add subpacket to related list
		if (packet->type == 8) {
			error("Unsupported packet type 8");
			delete gb;
			delete d;
			return;
		} else if (packet->type >= 9 && packet->type <= 12) {
			// packets for MPEG Audio like Synthesis Filter
			QDM2_LIST_ADD(_subPacketListD, subPacketsD, packet);
		} else if (packet->type == 13) {
			for (j = 0; j < 6; j++)
				_fftLevelExp[j] = gb->getBits(6);
		} else if (packet->type == 14) {
			for (j = 0; j < 6; j++)
				_fftLevelExp[j] = qdm2_get_vlc(gb, &_fftLevelExpVlc, 0, 2);
		} else if (packet->type == 15) {
			error("Unsupported packet type 15");
			delete gb;
			delete d;
			return;
		} else if (packet->type >= 16 && packet->type < 48 && !fft_subpackets[packet->type - 16]) {
			// packets for FFT
			QDM2_LIST_ADD(_subPacketListB, _subPacketsB, packet);
		}
	} // Packet bytes loop

// ****************************************************************
	if (_subPacketListD[0].packet != NULL) {
		process_synthesis_subpackets(_subPacketListD);
		_doSynthFilter = 1;
	} else if (_doSynthFilter) {
		process_subpacket_10(NULL, 0);
		process_subpacket_11(NULL, 0);
		process_subpacket_12(NULL, 0);
	}
// ****************************************************************
	delete gb;
	delete d;
}

void QDM2Stream::qdm2_fft_init_coefficient(int sub_packet, int offset, int duration,
                                           int channel, int exp, int phase) {
	if (_fftCoefsMinIndex[duration] < 0)
	    _fftCoefsMinIndex[duration] = _fftCoefsIndex;

	_fftCoefs[_fftCoefsIndex].sub_packet = ((sub_packet >= 16) ? (sub_packet - 16) : sub_packet);
	_fftCoefs[_fftCoefsIndex].channel = channel;
	_fftCoefs[_fftCoefsIndex].offset = offset;
	_fftCoefs[_fftCoefsIndex].exp = exp;
	_fftCoefs[_fftCoefsIndex].phase = phase;
	_fftCoefsIndex++;
}

void QDM2Stream::qdm2_fft_decode_tones(int duration, Common::BitStream *gb, int b) {
	int channel, stereo, phase, exp;
	int local_int_4,  local_int_8,  stereo_phase,  local_int_10;
	int local_int_14, stereo_exp, local_int_20, local_int_28;
	int n, offset;

	local_int_4 = 0;
	local_int_28 = 0;
	local_int_20 = 2;
	local_int_8 = (4 - duration);
	local_int_10 = 1 << (_groupOrder - duration - 1);
	offset = 1;

	while (1) {
		if (_superblocktype_2_3) {
			while ((n = qdm2_get_vlc(gb, &_vlcTabFftToneOffset[local_int_8], 1, 2)) < 2) {
				offset = 1;
				if (n == 0) {
					local_int_4 += local_int_10;
					local_int_28 += (1 << local_int_8);
				} else {
					local_int_4 += 8*local_int_10;
					local_int_28 += (8 << local_int_8);
				}
			}
			offset += (n - 2);
		} else {
			offset += qdm2_get_vlc(gb, &_vlcTabFftToneOffset[local_int_8], 1, 2);
			while (offset >= (local_int_10 - 1)) {
				offset += (1 - (local_int_10 - 1));
				local_int_4  += local_int_10;
				local_int_28 += (1 << local_int_8);
			}
		}

		if (local_int_4 >= _blockSize)
			return;

		local_int_14 = (offset >> local_int_8);

		if (_channels > 1) {
			channel = gb->getBit();
			stereo = gb->getBit();
		} else {
			channel = 0;
			stereo = 0;
		}

		exp = qdm2_get_vlc(gb, (b ? &_fftLevelExpVlc : &_fftLevelExpAltVlc), 0, 2);
		exp += _fftLevelExp[fft_level_index_table[local_int_14]];
		exp = (exp < 0) ? 0 : exp;

		phase = gb->getBits(3);
		stereo_exp = 0;
		stereo_phase = 0;

		if (stereo) {
			stereo_exp = (exp - qdm2_get_vlc(gb, &_fftStereoExpVlc, 0, 1));
			stereo_phase = (phase - qdm2_get_vlc(gb, &_fftStereoPhaseVlc, 0, 1));
			if (stereo_phase < 0)
				stereo_phase += 8;
		}

		if (_frequencyRange > (local_int_14 + 1)) {
			int sub_packet = (local_int_20 + local_int_28);

			qdm2_fft_init_coefficient(sub_packet, offset, duration, channel, exp, phase);
			if (stereo)
				qdm2_fft_init_coefficient(sub_packet, offset, duration, (1 - channel), stereo_exp, stereo_phase);
		}

		offset++;
	}
}

void QDM2Stream::qdm2_decode_fft_packets(void) {
	int i, j, min, max, value, type, unknown_flag;

	if (_subPacketListB[0].packet == NULL)
		return;

	// reset minimum indexes for FFT coefficients
	_fftCoefsIndex = 0;
	for (i=0; i < 5; i++)
		_fftCoefsMinIndex[i] = -1;

	// process subpackets ordered by type, largest type first
	for (i = 0, max = 256; i < _subPacketsB; i++) {
		QDM2SubPacket *packet= NULL;

		// find subpacket with largest type less than max
		for (j = 0, min = 0; j < _subPacketsB; j++) {
			value = _subPacketListB[j].packet->type;
			if (value > min && value < max) {
				min = value;
				packet = _subPacketListB[j].packet;
			}
		}

		max = min;

		// check for errors (?)
		if (!packet)
			return;

		if (i == 0 && (packet->type < 16 || packet->type >= 48 || fft_subpackets[packet->type - 16]))
			return;

		// decode FFT tones
		Common::MemoryReadStream d(packet->data, packet->size*8);
		Common::BitStream32LELSB gb(&d);

		if (packet->type >= 32 && packet->type < 48 && !fft_subpackets[packet->type - 16])
			unknown_flag = 1;
		else
			unknown_flag = 0;

		type = packet->type;

		if ((type >= 17 && type < 24) || (type >= 33 && type < 40)) {
			int duration = _subSampling + 5 - (type & 15);

			if (duration >= 0 && duration < 4) { // TODO: Should be <= 4?
				qdm2_fft_decode_tones(duration, &gb, unknown_flag);
			}
		} else if (type == 31) {
			for (j=0; j < 4; j++) {
				qdm2_fft_decode_tones(j, &gb, unknown_flag);
			}
		} else if (type == 46) {
			for (j=0; j < 6; j++)
				_fftLevelExp[j] = gb.getBits(6);
			for (j=0; j < 4; j++) {
				qdm2_fft_decode_tones(j, &gb, unknown_flag);
			}
		}
	} // Loop on B packets

	// calculate maximum indexes for FFT coefficients
	for (i = 0, j = -1; i < 5; i++)
		if (_fftCoefsMinIndex[i] >= 0) {
			if (j >= 0)
				_fftCoefsMaxIndex[j] = _fftCoefsMinIndex[i];
			j = i;
		}
	if (j >= 0)
		_fftCoefsMaxIndex[j] = _fftCoefsIndex;
}

void QDM2Stream::qdm2_fft_generate_tone(FFTTone *tone)
{
	float level, f[6];
	int i;
	QDM2Complex c;
	const double iscale = 2.0 * M_PI / 512.0;

	tone->phase += tone->phase_shift;

	// calculate current level (maximum amplitude) of tone
	level = fft_tone_envelope_table[tone->duration][tone->time_index] * tone->level;
	c.im = level * sin(tone->phase*iscale);
	c.re = level * cos(tone->phase*iscale);

	// generate FFT coefficients for tone
	if (tone->duration >= 3 || tone->cutoff >= 3) {
	    tone->complex[0].im += c.im;
	    tone->complex[0].re += c.re;
	    tone->complex[1].im -= c.im;
	    tone->complex[1].re -= c.re;
	} else {
		f[1] = -tone->table[4];
		f[0] =  tone->table[3] - tone->table[0];
		f[2] =  1.0 - tone->table[2] - tone->table[3];
		f[3] =  tone->table[1] + tone->table[4] - 1.0;
		f[4] =  tone->table[0] - tone->table[1];
		f[5] =  tone->table[2];
		for (i = 0; i < 2; i++) {
			tone->complex[fft_cutoff_index_table[tone->cutoff][i]].re += c.re * f[i];
			tone->complex[fft_cutoff_index_table[tone->cutoff][i]].im += c.im *((tone->cutoff <= i) ? -f[i] : f[i]);
		}
		for (i = 0; i < 4; i++) {
			tone->complex[i].re += c.re * f[i+2];
			tone->complex[i].im += c.im * f[i+2];
		}
	}

	// copy the tone if it has not yet died out
	if (++tone->time_index < ((1 << (5 - tone->duration)) - 1)) {
		memcpy(&_fftTones[_fftToneEnd], tone, sizeof(FFTTone));
		_fftToneEnd = (_fftToneEnd + 1) % 1000;
	}
}

void QDM2Stream::qdm2_fft_tone_synthesizer(uint8 sub_packet) {
	int i, j, ch;
	const double iscale = 0.25 * M_PI;

	for (ch = 0; ch < _channels; ch++) {
		memset(_fft.complex[ch], 0, _frameSize * sizeof(QDM2Complex));
	}

	// apply FFT tones with duration 4 (1 FFT period)
	if (_fftCoefsMinIndex[4] >= 0)
		for (i = _fftCoefsMinIndex[4]; i < _fftCoefsMaxIndex[4]; i++) {
			float level;
			QDM2Complex c;

			if (_fftCoefs[i].sub_packet != sub_packet)
				break;

			ch = (_channels == 1) ? 0 : _fftCoefs[i].channel;
			level = (_fftCoefs[i].exp < 0) ? 0.0 : fft_tone_level_table[_superblocktype_2_3 ? 0 : 1][_fftCoefs[i].exp & 63];

			c.re = level * cos(_fftCoefs[i].phase * iscale);
			c.im = level * sin(_fftCoefs[i].phase * iscale);
			_fft.complex[ch][_fftCoefs[i].offset + 0].re += c.re;
			_fft.complex[ch][_fftCoefs[i].offset + 0].im += c.im;
			_fft.complex[ch][_fftCoefs[i].offset + 1].re -= c.re;
			_fft.complex[ch][_fftCoefs[i].offset + 1].im -= c.im;
		}

	// generate existing FFT tones
	for (i = _fftToneEnd; i != _fftToneStart; ) {
		qdm2_fft_generate_tone(&_fftTones[_fftToneStart]);
		_fftToneStart = (_fftToneStart + 1) % 1000;
	}

	// create and generate new FFT tones with duration 0 (long) to 3 (short)
	for (i = 0; i < 4; i++)
		if (_fftCoefsMinIndex[i] >= 0) {
			for (j = _fftCoefsMinIndex[i]; j < _fftCoefsMaxIndex[i]; j++) {
				int offset, four_i;
				FFTTone tone;

				if (_fftCoefs[j].sub_packet != sub_packet)
					break;

				four_i = (4 - i);
				offset = _fftCoefs[j].offset >> four_i;
				ch = (_channels == 1) ? 0 : _fftCoefs[j].channel;

				if (offset < _frequencyRange) {
					if (offset < 2)
						tone.cutoff = offset;
					else
						tone.cutoff = (offset >= 60) ? 3 : 2;

					tone.level = (_fftCoefs[j].exp < 0) ? 0.0 : fft_tone_level_table[_superblocktype_2_3 ? 0 : 1][_fftCoefs[j].exp & 63];
					tone.complex = &_fft.complex[ch][offset];
					tone.table = fft_tone_sample_table[i][_fftCoefs[j].offset - (offset << four_i)];
					tone.phase = 64 * _fftCoefs[j].phase - (offset << 8) - 128;
					tone.phase_shift = (2 * _fftCoefs[j].offset + 1) << (7 - four_i);
					tone.duration = i;
					tone.time_index = 0;

					qdm2_fft_generate_tone(&tone);
				}
			}
			_fftCoefsMinIndex[i] = j;
		}
}

void QDM2Stream::qdm2_calculate_fft(int channel) {
	_fft.complex[channel][0].re *= 2.0f;
	_fft.complex[channel][0].im = 0.0f;

	_rdft->calc((float *)_fft.complex[channel]);

	// add samples to output buffer
	for (int i = 0; i < ((_fftFrameSize + 15) & ~15); i++)
		_outputBuffer[_channels * i + channel] += ((float *) _fft.complex[channel])[i];
}

/**
 * @param index    subpacket number
 */
void QDM2Stream::qdm2_synthesis_filter(uint8 index)
{
	int16 samples[MPA_MAX_CHANNELS * MPA_FRAME_SIZE];
	int i, k, ch, sb_used, sub_sampling, dither_state = 0;

	// copy sb_samples
	sb_used = QDM2_SB_USED(_subSampling);

	for (ch = 0; ch < _channels; ch++)
		for (i = 0; i < 8; i++)
			for (k = sb_used; k < 32; k++)
				_sbSamples[ch][(8 * index) + i][k] = 0;

	for (ch = 0; ch < _channels; ch++) {
		int16 *samples_ptr = samples + ch;

		for (i = 0; i < 8; i++) {
			ff_mpa_synth_filter(_synthBuf[ch], &(_synthBufOffset[ch]),
			                    ff_mpa_synth_window, &dither_state,
			                    samples_ptr, _channels,
			                    _sbSamples[ch][(8 * index) + i]);
			samples_ptr += 32 * _channels;
		}
	}

	// add samples to output buffer
	sub_sampling = (4 >> _subSampling);

	for (ch = 0; ch < _channels; ch++)
		for (i = 0; i < _sFrameSize; i++)
			_outputBuffer[_channels * i + ch] += (float)(samples[_channels * sub_sampling * i + ch] >> (sizeof(int16)*8-16));
}

bool QDM2Stream::qdm2_decodeFrame(Common::SeekableReadStream &in, QueuingAudioStream *audioStream) {
	debug(1, "QDM2Stream::qdm2_decodeFrame in.pos(): %d in.size(): %d", in.pos(), in.size());
	int ch, i;
	const int frame_size = (_sFrameSize * _channels);

	// If we're in any packet but the first, seek back to the first
	if (_subPacket == 0)
		_superBlockStart = in.pos();
	else
		in.seek(_superBlockStart);

	// select input buffer
	if (in.eos() || in.pos() >= in.size()) {
		debug(1, "QDM2Stream::qdm2_decodeFrame End of Input Stream");
		return false;
	}

	if ((in.size() - in.pos()) < _packetSize) {
		debug(1, "QDM2Stream::qdm2_decodeFrame Insufficient Packet Data in Input Stream Found: %d Need: %d", in.size() - in.pos(), _packetSize);
		return false;
	}

	if (!in.eos()) {
		in.read(_compressedData, _packetSize);
		debug(1, "QDM2Stream::qdm2_decodeFrame constructed input data");
	}

	// copy old block, clear new block of output samples
	memmove(_outputBuffer, &_outputBuffer[frame_size], frame_size * sizeof(float));
	memset(&_outputBuffer[frame_size], 0, frame_size * sizeof(float));
	debug(1, "QDM2Stream::qdm2_decodeFrame cleared outputBuffer");

	if (!in.eos()) {
		// decode block of QDM2 compressed data
		debug(1, "QDM2Stream::qdm2_decodeFrame decode block of QDM2 compressed data");
		if (_subPacket == 0) {
			_hasErrors = false; // reset it for a new super block
			debug(1, "QDM2 : Superblock follows");
			qdm2_decode_super_block();
		}

		// parse subpackets
		debug(1, "QDM2Stream::qdm2_decodeFrame parse subpackets");
		if (!_hasErrors) {
			if (_subPacket == 2) {
				debug(1, "QDM2Stream::qdm2_decodeFrame qdm2_decode_fft_packets()");
				qdm2_decode_fft_packets();
			}

			debug(1, "QDM2Stream::qdm2_decodeFrame qdm2_fft_tone_synthesizer(%d)", _subPacket);
			qdm2_fft_tone_synthesizer(_subPacket);
		}

		// sound synthesis stage 1 (FFT)
		debug(1, "QDM2Stream::qdm2_decodeFrame sound synthesis stage 1 (FFT)");
		for (ch = 0; ch < _channels; ch++) {
			qdm2_calculate_fft(ch);

			if (!_hasErrors && _subPacketListC[0].packet != NULL) {
				error("QDM2 : has errors, and C list is not empty");
				return false;
			}
		}

		// sound synthesis stage 2 (MPEG audio like synthesis filter)
		debug(1, "QDM2Stream::qdm2_decodeFrame sound synthesis stage 2 (MPEG audio like synthesis filter)");
		if (!_hasErrors && _doSynthFilter)
			qdm2_synthesis_filter(_subPacket);

		_subPacket = (_subPacket + 1) % 16;

		if(_hasErrors)
			warning("QDM2 Packet error...");

		// clip and convert output float[] to 16bit signed samples
		debug(1, "QDM2Stream::qdm2_decodeFrame clip and convert output float[] to 16bit signed samples");
	}

	if (frame_size == 0)
		return false;

	// Prepare a buffer for queuing
	uint16 *outputBuffer = (uint16 *)malloc(frame_size * 2);

	for (i = 0; i < frame_size; i++) {
		int value = (int)_outputBuffer[i];

		if (value > SOFTCLIP_THRESHOLD)
			value = (value >  HARDCLIP_THRESHOLD) ?  32767 :  _softclipTable[ value - SOFTCLIP_THRESHOLD];
		else if (value < -SOFTCLIP_THRESHOLD)
			value = (value < -HARDCLIP_THRESHOLD) ? -32767 : -_softclipTable[-value - SOFTCLIP_THRESHOLD];

		outputBuffer[i] = value;
	}

	// Queue the translated buffer to our stream
	byte flags = FLAG_16BITS;

	if (_channels == 2)
		flags |= FLAG_STEREO;

#ifdef SCUMM_LITTLE_ENDIAN
	flags |= FLAG_LITTLE_ENDIAN;
#endif

	audioStream->queueBuffer((byte *)outputBuffer, frame_size * 2, DisposeAfterUse::YES, flags);

	return true;
}

AudioStream *QDM2Stream::decodeFrame(Common::SeekableReadStream &stream) {
	QueuingAudioStream *audioStream = makeQueuingAudioStream(_sampleRate, _channels == 2);

	while (qdm2_decodeFrame(stream, audioStream))
		;

	return audioStream;
}

Codec *makeQDM2Decoder(Common::SeekableReadStream *extraData, DisposeAfterUse::Flag disposeExtraData) {
	return new QDM2Stream(extraData, disposeExtraData);
}

} // End of namespace Audio

#endif
