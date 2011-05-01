#ifndef _TMS_H_
#define _TMS_H_

namespace MDFN_IEN_SMS
{

extern int text_counter;


void make_tms_tables(void);
void render_bg_tms(int line);
void render_bg_m0(int line);
void render_bg_m1(int line);
void render_bg_m1x(int line);
void render_bg_inv(int line);
void render_bg_m3(int line);
void render_bg_m3x(int line);
void render_bg_m2(int line);
void render_obj_tms(int line);
void parse_line(int line);

}

#endif /* _TMS_H_ */
