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

#ifndef AGI_SPRITE_H
#define AGI_SPRITE_H

namespace Agi {


struct Sprite;
typedef Common::List<Sprite *> SpriteList;

class AgiEngine;
class GfxMgr;
class Obejcts;

class SpritesMgr {
private:
	GfxMgr *_gfx;
	AgiEngine *_vm;

	uint8 *_spritePool;
	uint8 *_poolTop;

	//
	// Sprite management functions
	//

	SpriteList _sprUpd;
	SpriteList _sprNonupd;

	void *poolAlloc(int size);
	void poolRelease(void *s);
	void blitPixel(uint8 *p, uint8 *end, uint8 col, int spr, int width, int *hidden);
	int blitCel(int x, int y, int spr, ViewCel *c, bool agi256_2);
	void objsSaveArea(Sprite *s);
	void objsRestoreArea(Sprite *s);

	int prioToY(int p);
	Sprite *newSprite(VtEntry *v);
	void sprAddlist(SpriteList &l, VtEntry *v);
	void buildList(SpriteList &l, bool (*test)(VtEntry *, AgiEngine *));
	void buildUpdBlitlist();
	void buildNonupdBlitlist();
	void freeList(SpriteList &l);
	void commitSprites(SpriteList &l, bool immediate = false);
	void eraseSprites(SpriteList &l);
	void blitSprites(SpriteList &l);
	static bool testUpdating(VtEntry *v, AgiEngine *);
	static bool testNotUpdating(VtEntry *v, AgiEngine *);

public:
	SpritesMgr(AgiEngine *agi, GfxMgr *gfx);
	~SpritesMgr();

	int initSprites();
	void deinitSprites();
	void eraseUpdSprites();
	void eraseNonupdSprites();
	void eraseBoth();
	void blitUpdSprites();
	void blitNonupdSprites();
	void blitBoth();
	void commitUpdSprites();
	void commitNonupdSprites();
	void commitBoth();
	void addToPic(int, int, int, int, int, int, int);
	void showObj(int);
	void commitBlock(int x1, int y1, int x2, int y2, bool immediate = false);
};

} // End of namespace Agi

#endif /* AGI_SPRITE_H */
