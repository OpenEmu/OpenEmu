#ifndef __MDFN_DRIVERS_OVERLAY_H
#define __MDFN_DRIVERS_OVERLAY_H

void OV_Blit(SDL_Surface *src_surface, const SDL_Rect *src_rect, const SDL_Rect *original_src_rect,
        const SDL_Rect *dest_rect, SDL_Surface *dest_surface, int softscale, int scanlines, int rotated);


void OV_Kill(void);

#endif
