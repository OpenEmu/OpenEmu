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

#if !defined(SCUMM_HE_SPRITE_HE_H) && defined(ENABLE_HE)
#define SCUMM_HE_SPRITE_HE_H

namespace Scumm {

enum SpriteFlags {
	kSFChanged           = 0x1,
	kSFNeedRedraw        = 0x2,
	kSFScaled            = 0x10,
	kSFRotated           = 0x20,
	kSFDoubleBuffered    = 0x1000,
	kSFYFlipped          = 0x2000,
	kSFXFlipped          = 0x4000,
	kSFActive            = 0x8000,
	kSFRemapPalette      = 0x80000,
	kSFAutoAnim          = 0x200000,
	kSFMarkDirty         = 0x400000,
	kSFBlitDirectly      = 0x2000000,
	kSFImageless         = 0x40000000
};

enum SpriteGroupFlags {
	kSGFClipBox     = (1 << 0)
};

struct SpriteInfo {
	int32 id;
	int32 zorder;
	int32 flags;
	int32 image;
	int32 imageState;
	int32 group;
	int32 palette;
	int32 priority;
	Common::Rect bbox;
	int32 dx;
	int32 dy;
	Common::Point pos;
	int32 tx;
	int32 ty;
	int32 userValue;
	int32 curImageState;
	int32 curImage;
	int32 imglistNum;
	int32 shadow;
	int32 imageStateCount;
	int32 angle;
	int32 scale;
	int32 animProgress;
	int32 curAngle;
	int32 curScale;
	int32 curImgFlags;
	int32 field_74;
	int32 animSpeed;
	int32 sourceImage;
	int32 maskImage;
	int32 field_84;
	int32 classFlags;
	int32 imgFlags;
	int32 field_90;
};

struct SpriteGroup {
	Common::Rect bbox;
	int32 priority;
	int32 flags;
	int32 tx;
	int32 ty;
	int32 image;
	int32 scaling;
	int32 scale_x_ratio_mul;
	int32 scale_x_ratio_div;
	int32 scale_y_ratio_mul;
	int32 scale_y_ratio_div;
};

class ScummEngine_v90he;

class Sprite {
public:
	Sprite(ScummEngine_v90he *vm);
	virtual ~Sprite();

	SpriteInfo *_spriteTable;
	SpriteGroup *_spriteGroups;
	SpriteInfo **_activeSpritesTable;

	int32 _numSpritesToProcess;
	int32 _varNumSpriteGroups;
	int32 _varNumSprites;
	int32 _varMaxSprites;

	void saveOrLoadSpriteData(Serializer *s);
	void resetBackground();
	void setRedrawFlags(bool checkZOrder);
	void sortActiveSprites();
	void processImages(bool arg);
	void updateImages();

	int findSpriteWithClassOf(int x, int y, int spriteGroupId, int d, int num, int *args);
	int getSpriteClass(int spriteId, int num, int *args);
	int getSpriteFlagDoubleBuffered(int spriteId);
	int getSpriteFlagYFlipped(int spriteId);
	int getSpriteFlagXFlipped(int spriteId);
	int getSpriteFlagActive(int spriteId);
	int getSpriteFlagRemapPalette(int spriteId);
	int getSpriteFlagAutoAnim(int spriteId);
	int getSpriteFlagUpdateType(int spriteId);
	int getSpriteFlagEraseType(int spriteId);
	int getSpriteImage(int spriteId);
	int getSpriteImageState(int spriteId);
	int getSpriteGroup(int spriteId);
	int getSpritePalette(int spriteId);
	int getSpritePriority(int spriteId);
	int getSpriteDisplayX(int spriteId);
	int getSpriteDisplayY(int spriteId);
	int getSpriteUserValue(int spriteId);
	int getSpriteShadow(int spriteId);
	int getSpriteImageStateCount(int spriteId);
	int getSpriteScale(int spriteId);
	int getSpriteAnimSpeed(int spriteId);
	int getSpriteSourceImage(int spriteId);
	int getSpriteMaskImage(int spriteId);
	int getSpriteGeneralProperty(int spriteId, int type);
	void getSpriteBounds(int spriteId, bool checkGroup, Common::Rect &bound);
	void getSpriteImageDim(int spriteId, int32 &w, int32 &h);
	void getSpritePosition(int spriteId, int32 &tx, int32 &ty);
	void getSpriteDist(int spriteId, int32 &dx, int32 &dy);

	int getGroupPriority(int spriteGroupId);
	int getGroupDstResNum(int spriteGroupId);
	int getGroupXMul(int spriteGroupId);
	int getGroupXDiv(int spriteGroupId);
	int getGroupYMul(int spriteGroupId);
	int getGroupYDiv(int spriteGroupId);
	void getGroupPosition(int spriteGroupId, int32 &tx, int32 &ty);

	void setSpritePalette(int spriteId, int value);
	void setSpriteSourceImage(int spriteId, int value);
	void setSpriteMaskImage(int spriteId, int value);
	void resetSprite(int spriteId);
	void setSpriteImageState(int spriteId, int value);
	void setSpritePosition(int spriteId, int value1, int value2);
	void setSpriteGroup(int spriteId, int value);
	void setSpriteDist(int spriteId, int value1, int value2);
	void setSpriteShadow(int spriteId, int value);
	void setSpriteUserValue(int spriteId, int value1, int value2);
	void setSpritePriority(int spriteId, int value);
	void moveSprite(int spriteId, int value1, int value2);
	void setSpriteScale(int spriteId, int value);
	void setSpriteAngle(int spriteId, int value);
	void setSpriteFlagDoubleBuffered(int spriteId, int value);
	void setSpriteFlagYFlipped(int spriteId, int value);
	void setSpriteFlagXFlipped(int spriteId, int value);
	void setSpriteFlagActive(int spriteId, int value);
	void setSpriteFlagRemapPalette(int spriteId, int value);
	void setSpriteFlagAutoAnim(int spriteId, int value);
	void setSpriteFlagUpdateType(int spriteId, int value);
	void setSpriteFlagEraseType(int spriteId, int value);
	void setSpriteAnimSpeed(int spriteId, int value);
	void setSpriteSetClass(int spriteId, int classId, int toggle);
	void setSpriteResetClass(int spriteId);
	void setSpriteField84(int spriteId, int value);
	void setSpriteGeneralProperty(int spriteId, int type, int value);

	void moveGroupMembers(int spriteGroupId, int value1, int value2);
	void redrawSpriteGroup(int spriteGroupId);
	void setGroupMembersPriority(int spriteGroupId, int value);
	void setGroupMembersGroup(int spriteGroupId, int value);
	void setGroupMembersUpdateType(int spriteGroupId, int value);
	void setGroupMembersResetSprite(int spriteGroupId);
	void setGroupMembersAnimationSpeed(int spriteGroupId, int value);
	void setGroupMembersAutoAnimFlag(int spriteGroupId, int value);
	void setGroupMembersShadow(int spriteGroupId, int value);

	void moveGroup(int spriteGroupId, int value1, int value2);
	void setGroupBounds(int spriteGroupId, int x1, int y1, int x2, int y2);
	void setGroupPriority(int spriteGroupId, int value);
	void setGroupPosition(int spriteGroupId, int value1, int value2);
	void setGroupImage(int spriteGroupId, int value);
	void setGroupScaling(int spriteGroupId);
	void setGroupXMul(int spriteGroupId, int value);
	void setGroupXDiv(int spriteGroupId, int value);
	void setGroupYMul(int spriteGroupId, int value);
	void setGroupYDiv(int spriteGroupId, int value);
	void resetGroupBounds(int spriteGroupId);

	void allocTables(int numSprites, int numGroups, int numMaxSprites);
	void resetGroup(int spriteGroupId);
	void resetTables(bool refreshScreen);
	void setSpriteImage(int spriteId, int imageNum);
private:
	ScummEngine_v90he *_vm;
};

} // End of namespace Scumm

#endif
