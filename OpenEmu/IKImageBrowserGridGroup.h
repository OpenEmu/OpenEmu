#import <Foundation/Foundation.h>

@interface IKImageBrowserGridGroup : NSObject
{
    struct _NSRange _range;
    struct _NSRange _rangeToParent;
    struct _NSRange _transformedRange;
    BOOL _expanded;
    int _ghostCellCountOnTheLeft;
    int _ghostCellCountOnTheRight;
    int _groupStyle;
    NSColor *_bgColor;
    id _title;
    id _headLayer;
    id _tailLayer;
    BOOL _selected;
    BOOL _highlighted;
    BOOL _isAnimating;
    float _alpha;
    // IKImageBrowserSubsetLayoutManager *_grid;
    NSMutableDictionary *_attributes;
    id _sourceAttributes;
}

@property(retain) id sourceAttributes; // @synthesize sourceAttributes=_sourceAttributes;
// @property IKImageBrowserSubsetLayoutManager *grid; // @synthesize grid=_grid;
- (id)copyWithZone:(struct _NSZone *)arg1;
- (id)collapsedIndexesWithColumnCount:(unsigned long long)arg1;
- (BOOL)hasCoverflow;
- (BOOL)hasSubLayoutWhenCollapsed;
- (BOOL)drawBezelOnDragOver;
- (BOOL)hasFloatingHeader;
- (BOOL)hasHeader;
- (BOOL)hasSubLayout;
- (BOOL)supportsHorizontalScrolling;
- (id)objectForKey:(id)arg1;
- (void)setObject:(id)arg1 forKey:(id)arg2;
- (void)setAlpha:(float)arg1;
- (float)alpha;
- (void)setHighlighted:(BOOL)arg1;
- (BOOL)highlighted;
- (void)setSelected:(BOOL)arg1;
- (BOOL)selected;
- (void)setGhostCellCountOnTheRight:(int)arg1;
- (int)ghostCellCountOnTheRight;
- (void)setGhostCellCountOnTheLeft:(int)arg1;
- (int)ghostCellCountOnTheLeft;
- (void)setRangeToParent:(struct _NSRange)arg1;
- (struct _NSRange)rangeToParent;
- (void)setTransformedRange:(struct _NSRange)arg1;
- (struct _NSRange)transformedRange;
- (void)setAnimating:(BOOL)arg1;
- (BOOL)animating;
- (void)setTailLayer:(id)arg1;
- (id)tailLayer;
- (void)setHeadLayer:(id)arg1;
- (id)headLayer;
- (void)setTitle:(id)arg1;
- (id)title;
- (void)setBgColor:(id)arg1;
- (id)bgColor;
- (int)style;
- (void)setStyle:(int)arg1;
- (void)setRange:(struct _NSRange)arg1;
- (struct _NSRange)range;
- (void)setExpanded:(BOOL)arg1;
- (BOOL)expanded;
- (void)dealloc;
- (id)init;

@end

