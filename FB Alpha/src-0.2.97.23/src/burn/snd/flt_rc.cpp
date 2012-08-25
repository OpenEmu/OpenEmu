#include "burnint.h"
#include "burn_sound.h"
#include "flt_rc.h"

#include <math.h>

#define FLT_RC_NUM		6

struct flt_rc_info
{
	INT32 type;
	double R1;
	double R2;
	double R3;
	double C;
	
	struct {
		INT32 k;
		INT32 memory;
		INT32 type;
	} state;
	
	double src_gain;
	double gain;
	INT32 output_dir;
	INT32 add_signal;
};

static struct flt_rc_info flt_rc_table[FLT_RC_NUM];

static INT32 num_filters;

void filter_rc_update(INT32 num, INT16 *src, INT16 *pSoundBuf, INT32 length)
{
#if defined FBA_DEBUG
	if (!DebugSnd_FilterRCInitted) bprintf(PRINT_ERROR, _T("filter_rc_update called without init\n"));
	if (num > num_filters) bprintf(PRINT_ERROR, _T("filter_rc_update called with invalid num %i\n"), num);
#endif

	struct flt_rc_info *ptr;

	ptr = &flt_rc_table[num];
		
	INT32 memory = ptr->state.memory;
		
	switch (ptr->state.type) {
		case FLT_RC_LOWPASS: {
			while (length--) {
				memory += (((INT32)((*src++ * ptr->src_gain)) - memory) * ptr->state.k) / 0x10000;
				
				INT32 nLeftSample = 0, nRightSample = 0;
				
				if ((ptr->output_dir & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
					nLeftSample += (INT32)(memory * ptr->gain);
				}
				
				if ((ptr->output_dir & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
					nRightSample += (INT32)(memory * ptr->gain);
				}
				
				nLeftSample = BURN_SND_CLIP(nLeftSample);
				nRightSample = BURN_SND_CLIP(nRightSample);
				
				if (ptr->add_signal) {
					pSoundBuf[0] += nLeftSample;
					pSoundBuf[1] += nRightSample;
				} else {
					pSoundBuf[0] = nLeftSample;
					pSoundBuf[1] = nRightSample;
				}
				pSoundBuf += 2;
			}
			break;
		}
			
		case FLT_RC_HIGHPASS:
		case FLT_RC_AC: {
			while (length--) {
				INT16 value = (INT32)(*src * ptr->src_gain) - memory;
				
				INT32 nLeftSample = 0, nRightSample = 0;
				
				if ((ptr->output_dir & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
					nLeftSample += (INT32)(value * ptr->gain);
				}
				
				if ((ptr->output_dir & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
					nRightSample += (INT32)(value * ptr->gain);
				}
				
				nLeftSample = BURN_SND_CLIP(nLeftSample);
				nRightSample = BURN_SND_CLIP(nRightSample);
				
				if (ptr->add_signal) {
					pSoundBuf[0] += nLeftSample;
					pSoundBuf[1] += nRightSample;
				} else {
					pSoundBuf[0] = nLeftSample;
					pSoundBuf[1] = nRightSample;
				}
				pSoundBuf += 2;
				memory += (((INT32)(*src++ * ptr->src_gain) - memory) * ptr->state.k) / 0x10000;
			}
			break;
		}
	}
		
	ptr->state.memory = memory;
}

static void set_RC_info(INT32 num, INT32 type, double R1, double R2, double R3, double C)
{
	double Req = 0.00;
	
	struct flt_rc_info *ptr;

	ptr = &flt_rc_table[num];

	ptr->state.type = type;

	switch (ptr->state.type) {
		case FLT_RC_LOWPASS: {
			if (C == 0.0) {
				/* filter disabled */
				ptr->state.k = 0x10000;
				return;
			}
			Req = (R1 * (R2 + R3)) / (R1 + R2 + R3);
			break;
		}
		
		case FLT_RC_HIGHPASS:
		case FLT_RC_AC: {
			if (C == 0.0) {
				/* filter disabled */
				ptr->state.k = 0x0;
				ptr->state.memory = 0x0;
				return;
			}
			Req = R1;
			break;
		}
		
		default:
			bprintf(PRINT_IMPORTANT, _T("filter_rc_setRC: Wrong filter type %d\n"), ptr->state.type);
	}

	/* Cut Frequency = 1/(2*Pi*Req*C) */
	/* k = (1-(EXP(-TIMEDELTA/RC)))    */
	ptr->state.k = (INT32)(0x10000 - 0x10000 * (exp(-1 / (Req * C) / nBurnSoundRate)));
}

void filter_rc_set_RC(INT32 num, INT32 type, double R1, double R2, double R3, double C)
{
#if defined FBA_DEBUG
	if (!DebugSnd_FilterRCInitted) bprintf(PRINT_ERROR, _T("filter_rc_set_RC called without init\n"));
	if (num > num_filters) bprintf(PRINT_ERROR, _T("filter_rc_set_RC called with invalid num %i\n"), num);
#endif

	set_RC_info(num, type, R1, R2, R3, C);
}

void filter_rc_init(INT32 num, INT32 type, double R1, double R2, double R3, double C, INT32 add_signal)
{
#if defined FBA_DEBUG
	if (num >= FLT_RC_NUM) bprintf (PRINT_ERROR, _T("filter_rc_init called for too many chips (%d)! Change FLT_RC_NUM (%d)!\n"), num, FLT_RC_NUM);
#endif

	DebugSnd_FilterRCInitted = 1;
	
	num_filters = num + 1;
	
	set_RC_info(num, type, R1, R2, R3, C);
	
	struct flt_rc_info *ptr;

	ptr = &flt_rc_table[num];
	
	ptr->src_gain = 1.00;
	ptr->gain = 1.00;
	ptr->output_dir = BURN_SND_ROUTE_BOTH;
	ptr->add_signal = add_signal;
}

void filter_rc_set_src_gain(INT32 num, double gain)
{
#if defined FBA_DEBUG
	if (!DebugSnd_FilterRCInitted) bprintf(PRINT_ERROR, _T("filter_rc_set_src_gain called without init\n"));
	if (num > num_filters) bprintf(PRINT_ERROR, _T("filter_rc_set_src_gain called with invalid num %i\n"), num);
#endif

	struct flt_rc_info *ptr;

	ptr = &flt_rc_table[num];
	
	ptr->src_gain = gain;
}

void filter_rc_set_route(INT32 num, double nVolume, INT32 nRouteDir)
{
	#if defined FBA_DEBUG
	if (!DebugSnd_FilterRCInitted) bprintf(PRINT_ERROR, _T("filter_rc_set_route called without init\n"));
	if (num > num_filters) bprintf(PRINT_ERROR, _T("filter_rc_set_route called with invalid num %i\n"), num);
#endif

	struct flt_rc_info *ptr;

	ptr = &flt_rc_table[num];
	
	ptr->gain = nVolume;
	ptr->output_dir = nRouteDir;
}

void filter_rc_exit()
{
#if defined FBA_DEBUG
	if (!DebugSnd_FilterRCInitted) bprintf(PRINT_ERROR, _T("filter_rc_exit called without init\n"));
#endif

	for (INT32 i = 0; i < FLT_RC_NUM; i++) {
		struct flt_rc_info *ptr;
		
		ptr = &flt_rc_table[i];

		memset(ptr, 0, sizeof(flt_rc_info));
	}

	num_filters = 0;
	
	DebugSnd_FilterRCInitted = 0;
}
