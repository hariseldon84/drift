#import <Cocoa/Cocoa.h>
#include "browser_window.h"
#include "drift_client.h"
#include "include/cef_browser.h"
#include "include/cef_app.h"
#include "engine/cef/cef_engine_adapter.h"
#include "bookmarks/bookmark_state.h"
#include "history/history_state.h"
#include "browser-state/browser_state.h"
#include "storage/sqlite_bookmark_repo.h"
#include "storage/sqlite_history_repo.h"

#include <algorithm>
#include <atomic>

// ─────────────────────────────────────────────────────────────────────────────
// Constants
// ─────────────────────────────────────────────────────────────────────────────
static const CGFloat kStripHeight    = 32.0;
static const CGFloat kNavBarHeight   = 44.0;
static const CGFloat kNavPillH       = 31.0;
static const CGFloat kTabMaxWidth    = 200.0;
static const CGFloat kTabMinWidth    = 80.0;
static const CGFloat kPinnedTabWidth = 32.0;
static const CGFloat kNewTabBtnWidth = 28.0;
static const CGFloat kFaviconSize    = 14.0;
static const CGFloat kCloseBtnSize   = 14.0;
static const CGFloat kBookmarksBarH  = 26.0;
static const CGFloat kDownloadBarH   = 48.0;

// Maps download URL → NSSavePanel-chosen path; consumed once in OnBeforeDownload.
static NSMutableDictionary<NSString*, NSString*>* gPendingSavePaths = nil;

// ─────────────────────────────────────────────────────────────────────────────

// ─────────────────────────────────────────────────────────────────────────────
// DriftTabInfo — immutable display snapshot for one tab in the strip
// ─────────────────────────────────────────────────────────────────────────────
@interface DriftTabInfo : NSObject
@property NSInteger    index;
@property (copy) NSString*  title;
@property (copy) NSString*  faviconLetter;
@property (strong) NSImage* favicon;
@property BOOL isPinned;
@property BOOL isActive;
@property BOOL isLoading;
@end
@implementation DriftTabInfo @end

// ─────────────────────────────────────────────────────────────────────────────
// DriftTitleBarView — title bar background that respects system double-click pref
// ─────────────────────────────────────────────────────────────────────────────
@interface DriftTitleBarView : NSView @end
@implementation DriftTitleBarView
- (BOOL)mouseDownCanMoveWindow { return YES; }
- (void)mouseDown:(NSEvent*)e {
  if (e.clickCount == 2) {
    NSString* pref = [NSUserDefaults.standardUserDefaults
                      stringForKey:@"AppleActionOnDoubleClick"];
    if (!pref || [pref isEqualToString:@"Maximize"])
      [self.window zoom:nil];
    else if ([pref isEqualToString:@"Minimize"])
      [self.window miniaturize:nil];
    return;
  }
  [super mouseDown:e];
}
@end

// ─────────────────────────────────────────────────────────────────────────────
// DriftNavButton — SF Symbol nav button with hover highlight
// ─────────────────────────────────────────────────────────────────────────────
@interface DriftNavButton : NSButton @end
@implementation DriftNavButton {
  NSTrackingArea* _ta;
  BOOL _hovered;
}
- (void)updateTrackingAreas {
  [super updateTrackingAreas];
  if (_ta) [self removeTrackingArea:_ta];
  _ta = [[NSTrackingArea alloc] initWithRect:self.bounds
    options:NSTrackingMouseEnteredAndExited | NSTrackingActiveInKeyWindow
    owner:self userInfo:nil];
  [self addTrackingArea:_ta];
}
- (void)mouseEntered:(NSEvent*)__unused e { _hovered = YES;  [self setNeedsDisplay:YES]; }
- (void)mouseExited:(NSEvent*)__unused e  { _hovered = NO;   [self setNeedsDisplay:YES]; }
- (void)drawRect:(NSRect)dirty {
  if (_hovered && self.enabled) {
    [[NSColor colorWithSRGBRed:0.110 green:0.078 blue:0.063 alpha:0.06] setFill]; // --drift-text 6%
    [[NSBezierPath bezierPathWithRoundedRect:self.bounds xRadius:5 yRadius:5] fill];
  }
  [super drawRect:dirty];
}
@end

// ─────────────────────────────────────────────────────────────────────────────
// DriftURLDisplayLabel — non-interactive host/path attributed overlay
// ─────────────────────────────────────────────────────────────────────────────
@interface DriftURLDisplayLabel : NSTextField @end
@implementation DriftURLDisplayLabel
- (NSView*)hitTest:(NSPoint)__unused p { return nil; }  // pass all clicks to URL field below
@end

// ─────────────────────────────────────────────────────────────────────────────
// DriftURLField — selects all text on first click into the field
// ─────────────────────────────────────────────────────────────────────────────
@interface DriftURLField : NSTextField @end
@implementation DriftURLField
- (void)mouseDown:(NSEvent*)e {
  BOOL wasEditing = (self.currentEditor != nil);
  [super mouseDown:e];  // positions cursor at click point
  // Override cursor position with selectAll only on the activation click
  if (!wasEditing && self.currentEditor)
    [self.currentEditor setSelectedRange:NSMakeRange(0, self.currentEditor.string.length)];
}
@end

// ─────────────────────────────────────────────────────────────────────────────
// DriftTabStripDelegate
// ─────────────────────────────────────────────────────────────────────────────
@class DriftTabStripView;
@protocol DriftTabStripDelegate <NSObject>
- (void)tabStripDidRequestNewTab:(DriftTabStripView*)strip;
- (void)tabStrip:(DriftTabStripView*)strip didSelectTabAtIndex:(NSInteger)i;
- (void)tabStrip:(DriftTabStripView*)strip didCloseTabAtIndex:(NSInteger)i;
- (void)tabStrip:(DriftTabStripView*)strip didMoveTabFrom:(NSInteger)from to:(NSInteger)to;
- (void)tabStrip:(DriftTabStripView*)strip didTogglePinAtIndex:(NSInteger)i;
- (void)tabStrip:(DriftTabStripView*)strip didRequestDuplicateTabAtIndex:(NSInteger)i;
- (void)tabStrip:(DriftTabStripView*)strip didCloseOtherTabsExceptIndex:(NSInteger)i;
@end

// ─────────────────────────────────────────────────────────────────────────────
// DriftTabStripView
// ─────────────────────────────────────────────────────────────────────────────
@interface DriftTabStripView : NSView
@property (assign) id<DriftTabStripDelegate> delegate;  // MRC — controller outlives view
@property (copy) NSArray<DriftTabInfo*>*   tabs;
@property NSInteger activeIndex;
@property NSInteger hoverIndex;
@property NSInteger dragIndex;
@property CGFloat   dragX;
@end

@implementation DriftTabStripView {
  NSTrackingArea* _trackingArea;
}

- (instancetype)initWithFrame:(NSRect)frame {
  if (!(self = [super initWithFrame:frame])) return nil;
  _tabs        = @[];
  _activeIndex = 0;
  _hoverIndex  = -1;
  _dragIndex   = -1;
  return self;
}

- (BOOL)isFlipped { return NO; }

// ── Layout ────────────────────────────────────────────────────────────────────

- (CGFloat)tabWidthForArray:(NSArray<DriftTabInfo*>*)tabs {
  NSInteger pinned   = 0;
  for (DriftTabInfo* t in tabs) if (t.isPinned) pinned++;
  NSInteger unpinned = (NSInteger)tabs.count - pinned;
  CGFloat avail      = self.bounds.size.width - kNewTabBtnWidth - 6
                       - pinned * kPinnedTabWidth;
  return unpinned > 0
    ? MIN(kTabMaxWidth, MAX(kTabMinWidth, avail / unpinned))
    : kTabMinWidth;
}

- (NSRect)tabRectAtIndex:(NSInteger)i inArray:(NSArray<DriftTabInfo*>*)tabs {
  CGFloat tabW = [self tabWidthForArray:tabs];
  CGFloat x    = 0;
  for (NSInteger j = 0; j < (NSInteger)tabs.count; j++) {
    CGFloat w = tabs[j].isPinned ? kPinnedTabWidth : tabW;
    if (j == i) return NSMakeRect(x, 0, w, self.bounds.size.height);
    x += w;
  }
  return NSZeroRect;
}

- (NSRect)newTabButtonRect {
  CGFloat tabW = [self tabWidthForArray:self.tabs];
  CGFloat x    = 0;
  for (DriftTabInfo* t in self.tabs) x += t.isPinned ? kPinnedTabWidth : tabW;
  CGFloat by = (self.bounds.size.height - kNewTabBtnWidth) / 2.0;
  return NSMakeRect(x + 4, by, kNewTabBtnWidth, kNewTabBtnWidth);
}

- (NSRect)closeBtnRectForTab:(NSRect)tabRect {
  CGFloat size = kCloseBtnSize;
  return NSMakeRect(NSMaxX(tabRect) - size - 4,
                    NSMidY(tabRect) - size / 2.0, size, size);
}

- (NSRect)faviconRectForTab:(NSRect)tabRect {
  return NSMakeRect(NSMinX(tabRect) + 8,
                    NSMidY(tabRect) - kFaviconSize / 2.0,
                    kFaviconSize, kFaviconSize);
}

- (NSInteger)tabIndexAtPoint:(NSPoint)p {
  for (NSInteger i = 0; i < (NSInteger)self.tabs.count; i++)
    if (NSPointInRect(p, [self tabRectAtIndex:i inArray:self.tabs])) return i;
  return -1;
}

// ── Drawing ───────────────────────────────────────────────────────────────────

- (void)drawRect:(NSRect)__unused dirty {
  // Strip background
  [[NSColor colorWithSRGBRed:0.949 green:0.929 blue:0.898 alpha:1] setFill];
  NSRectFill(self.bounds);
  // Bottom hairline
  [[NSColor colorWithSRGBRed:0.776 green:0.733 blue:0.678 alpha:1] setFill];
  NSRectFill(NSMakeRect(0, 0, self.bounds.size.width, 0.5));

  NSDictionary* attrs = @{
    NSFontAttributeName: [NSFont fontWithName:@"Inter" size:11]
                           ?: [NSFont systemFontOfSize:11 weight:NSFontWeightRegular],
    NSForegroundColorAttributeName:
      [NSColor colorWithSRGBRed:0.11 green:0.078 blue:0.063 alpha:1],
    NSParagraphStyleAttributeName: ({
      NSMutableParagraphStyle* ps = [NSMutableParagraphStyle new];
      ps.lineBreakMode = NSLineBreakByTruncatingTail;
      ps;
    })
  };

  for (NSInteger i = 0; i < (NSInteger)self.tabs.count; i++) {
    DriftTabInfo* tab = self.tabs[i];
    NSRect tr = [self tabRectAtIndex:i inArray:self.tabs];
    if (i == _dragIndex) tr.origin.x = _dragX - tr.size.width / 2.0;

    BOOL active = (i == _activeIndex);
    BOOL hover  = (i == _hoverIndex && _dragIndex < 0);

    // Tab fill
    NSColor* fill = active
      ? [NSColor colorWithSRGBRed:0.980 green:0.969 blue:0.949 alpha:1]
      : hover
        ? [NSColor colorWithSRGBRed:0.965 green:0.953 blue:0.933 alpha:0.8]
        : [NSColor clearColor];
    [fill setFill];

    CGFloat r = 6, x = tr.origin.x, y = tr.origin.y, w = tr.size.width, h = tr.size.height;
    NSBezierPath* path = [NSBezierPath bezierPath];
    [path moveToPoint:NSMakePoint(x, y)];
    [path lineToPoint:NSMakePoint(x, y + h - r)];
    [path appendBezierPathWithArcFromPoint:NSMakePoint(x, y + h)
                                   toPoint:NSMakePoint(x + r, y + h) radius:r];
    [path lineToPoint:NSMakePoint(x + w - r, y + h)];
    [path appendBezierPathWithArcFromPoint:NSMakePoint(x + w, y + h)
                                   toPoint:NSMakePoint(x + w, y + h - r) radius:r];
    [path lineToPoint:NSMakePoint(x + w, y)];
    [path closePath];
    [path fill];

    if (active) {
      [[NSColor colorWithSRGBRed:0.776 green:0.733 blue:0.678 alpha:0.6] setStroke];
      path.lineWidth = 0.5;
      [path stroke];
    }

    // Favicon
    NSRect fr = [self faviconRectForTab:tr];
    if (tab.favicon) {
      [tab.favicon drawInRect:fr fromRect:NSZeroRect
                    operation:NSCompositingOperationSourceOver fraction:1];
    } else {
      NSString* letter = tab.faviconLetter.length ? tab.faviconLetter : @"?";
      NSColor* amber = [NSColor colorWithSRGBRed:0.831 green:0.510 blue:0.039 alpha:1];
      [[amber colorWithAlphaComponent:0.15] setFill];
      [[NSBezierPath bezierPathWithOvalInRect:fr] fill];
      NSDictionary* la = @{
        NSFontAttributeName: [NSFont fontWithName:@"Inter" size:8] ?: [NSFont boldSystemFontOfSize:8],
        NSForegroundColorAttributeName: amber
      };
      NSSize ls = [letter sizeWithAttributes:la];
      [letter drawAtPoint:NSMakePoint(NSMidX(fr) - ls.width/2, NSMidY(fr) - ls.height/2)
           withAttributes:la];
    }

    // Title (non-pinned only)
    if (!tab.isPinned) {
      NSRect titleRect = NSMakeRect(
        NSMaxX(fr) + 4, y + (h - 13) / 2.0,
        w - kFaviconSize - kCloseBtnSize - 24, 13);
      [tab.title drawInRect:titleRect withAttributes:attrs];
    }

    // Close button (non-pinned, on hover or active)
    if (!tab.isPinned && (hover || active)) {
      NSRect cr = [self closeBtnRectForTab:tr];
      [[NSColor colorWithSRGBRed:0.776 green:0.733 blue:0.678 alpha:0.6] setFill];
      [[NSBezierPath bezierPathWithOvalInRect:cr] fill];
      NSBezierPath* xp = [NSBezierPath bezierPath];
      CGFloat pad = 3.5;
      [xp moveToPoint:NSMakePoint(NSMinX(cr)+pad, NSMinY(cr)+pad)];
      [xp lineToPoint:NSMakePoint(NSMaxX(cr)-pad, NSMaxY(cr)-pad)];
      [xp moveToPoint:NSMakePoint(NSMaxX(cr)-pad, NSMinY(cr)+pad)];
      [xp lineToPoint:NSMakePoint(NSMinX(cr)+pad, NSMaxY(cr)-pad)];
      xp.lineWidth = 1.5;
      xp.lineCapStyle = NSLineCapStyleRound;
      [[NSColor colorWithSRGBRed:0.11 green:0.078 blue:0.063 alpha:0.7] setStroke];
      [xp stroke];
    }

    // Loading indicator: amber bar at tab bottom
    if (tab.isLoading) {
      [[NSColor colorWithSRGBRed:0.831 green:0.510 blue:0.039 alpha:0.8] setFill];
      NSRect dot = NSMakeRect(NSMidX(tr) - 6, y + 1.5, 12, 2);
      [[NSBezierPath bezierPathWithRoundedRect:dot xRadius:1 yRadius:1] fill];
    }
  }

  // + button
  NSRect nb = [self newTabButtonRect];
  NSBezierPath* plus = [NSBezierPath bezierPath];
  CGFloat cx = NSMidX(nb), cy = NSMidY(nb);
  [plus moveToPoint:NSMakePoint(cx-5, cy)]; [plus lineToPoint:NSMakePoint(cx+5, cy)];
  [plus moveToPoint:NSMakePoint(cx, cy-5)]; [plus lineToPoint:NSMakePoint(cx, cy+5)];
  plus.lineWidth = 1.5;
  plus.lineCapStyle = NSLineCapStyleRound;
  [[NSColor colorWithSRGBRed:0.476 green:0.431 blue:0.396 alpha:1] setStroke];
  [plus stroke];
}

// ── Mouse ─────────────────────────────────────────────────────────────────────

- (void)updateTrackingAreas {
  [super updateTrackingAreas];
  if (_trackingArea) [self removeTrackingArea:_trackingArea];
  _trackingArea = [[NSTrackingArea alloc]
    initWithRect:self.bounds
         options:NSTrackingMouseMoved | NSTrackingMouseEnteredAndExited |
                 NSTrackingActiveInKeyWindow
           owner:self userInfo:nil];
  [self addTrackingArea:_trackingArea];
}

- (void)mouseMoved:(NSEvent*)e {
  NSPoint p = [self convertPoint:e.locationInWindow fromView:nil];
  NSInteger idx = [self tabIndexAtPoint:p];
  if (idx != _hoverIndex) { _hoverIndex = idx; [self setNeedsDisplay:YES]; }
}

- (void)mouseExited:(NSEvent*)__unused e {
  _hoverIndex = -1; [self setNeedsDisplay:YES];
}

- (void)mouseDown:(NSEvent*)e {
  NSPoint p = [self convertPoint:e.locationInWindow fromView:nil];

  if (NSPointInRect(p, [self newTabButtonRect])) {
    [self.delegate tabStripDidRequestNewTab:self];
    return;
  }

  NSInteger idx = [self tabIndexAtPoint:p];
  if (idx < 0) {
    if (e.clickCount == 2) {
      NSString* pref = [NSUserDefaults.standardUserDefaults
                        stringForKey:@"AppleActionOnDoubleClick"];
      if (!pref || [pref isEqualToString:@"Maximize"])
        [self.window zoom:nil];
      else if ([pref isEqualToString:@"Minimize"])
        [self.window miniaturize:nil];
    }
    return;
  }

  // Close button?
  if (!self.tabs[idx].isPinned) {
    NSRect tr = [self tabRectAtIndex:idx inArray:self.tabs];
    if (NSPointInRect(p, [self closeBtnRectForTab:tr])) {
      [self.delegate tabStrip:self didCloseTabAtIndex:idx];
      return;
    }
  }

  _dragIndex = idx;
  NSRect tr  = [self tabRectAtIndex:idx inArray:self.tabs];
  _dragX     = NSMidX(tr);
  [self.delegate tabStrip:self didSelectTabAtIndex:idx];
  [self setNeedsDisplay:YES];
}

- (void)mouseDragged:(NSEvent*)e {
  if (_dragIndex < 0) return;
  NSPoint p = [self convertPoint:e.locationInWindow fromView:nil];
  _dragX = p.x;
  [self setNeedsDisplay:YES];

  NSInteger newIdx = _dragIndex;
  for (NSInteger i = 0; i < (NSInteger)self.tabs.count; i++) {
    NSRect tr = [self tabRectAtIndex:i inArray:self.tabs];
    if (p.x < NSMidX(tr) && i < _dragIndex)  { newIdx = i; break; }
    if (p.x > NSMidX(tr) && i > _dragIndex)    newIdx = i;
  }
  if (newIdx != _dragIndex) {
    [self.delegate tabStrip:self didMoveTabFrom:_dragIndex to:newIdx];
    _dragIndex = newIdx;
  }
}

- (void)mouseUp:(NSEvent*)__unused e {
  _dragIndex = -1; [self setNeedsDisplay:YES];
}

- (void)rightMouseDown:(NSEvent*)e {
  NSPoint p   = [self convertPoint:e.locationInWindow fromView:nil];
  NSInteger i = [self tabIndexAtPoint:p];
  NSMenu* menu = [[NSMenu alloc] initWithTitle:@""];

  NSMenuItem* newItem = [[NSMenuItem alloc]
    initWithTitle:@"New Tab" action:@selector(newTabAction:) keyEquivalent:@""];
  newItem.target = self;
  [menu addItem:newItem];

  if (i >= 0) {
    NSMenuItem* dupItem = [[NSMenuItem alloc]
      initWithTitle:@"Duplicate Tab" action:@selector(duplicateTab:) keyEquivalent:@""];
    dupItem.tag = i; dupItem.target = self;
    [menu addItem:dupItem];

    [menu addItem:[NSMenuItem separatorItem]];

    NSMenuItem* closeItem = [[NSMenuItem alloc]
      initWithTitle:@"Close Tab" action:@selector(closeTab:) keyEquivalent:@""];
    closeItem.tag = i; closeItem.target = self;
    [menu addItem:closeItem];

    NSMenuItem* closeOthersItem = [[NSMenuItem alloc]
      initWithTitle:@"Close Other Tabs" action:@selector(closeOtherTabs:) keyEquivalent:@""];
    closeOthersItem.tag = i; closeOthersItem.target = self;
    closeOthersItem.enabled = (self.tabs.count > 1);
    [menu addItem:closeOthersItem];

    [menu addItem:[NSMenuItem separatorItem]];

    BOOL pinned = self.tabs[i].isPinned;
    NSMenuItem* pinItem = [[NSMenuItem alloc]
      initWithTitle:pinned ? @"Unpin Tab" : @"Pin Tab"
             action:@selector(togglePin:) keyEquivalent:@""];
    pinItem.tag = i; pinItem.target = self;
    [menu addItem:pinItem];
  }

  [NSMenu popUpContextMenu:menu withEvent:e forView:self];
}

- (void)newTabAction:(id)__unused sender {
  [self.delegate tabStripDidRequestNewTab:self];
}
- (void)duplicateTab:(NSMenuItem*)item {
  [self.delegate tabStrip:self didRequestDuplicateTabAtIndex:item.tag];
}
- (void)closeTab:(NSMenuItem*)item {
  [self.delegate tabStrip:self didCloseTabAtIndex:item.tag];
}
- (void)closeOtherTabs:(NSMenuItem*)item {
  [self.delegate tabStrip:self didCloseOtherTabsExceptIndex:item.tag];
}
- (void)togglePin:(NSMenuItem*)item {
  [self.delegate tabStrip:self didTogglePinAtIndex:item.tag];
}

@end

// ─────────────────────────────────────────────────────────────────────────────
// DriftTabModel — CEF-specific fields only; canonical tab state lives in _browserState
// ─────────────────────────────────────────────────────────────────────────────
@interface DriftTabModel : NSObject
@property uint64_t                       tabId;   // drift::browser::TabId value
@property int                            browserId;
@property (assign) drift::engine::IBrowserEngine* engine;
@property (copy) NSString*               faviconURL;
@property (strong) NSImage*              favicon;
@end
@implementation DriftTabModel
- (instancetype)init {
  if (!(self = [super init])) return nil;
  _browserId = -1;
  return self;
}
@end

// ─────────────────────────────────────────────────────────────────────────────
// DriftBrowserWindowController
// ─────────────────────────────────────────────────────────────────────────────
@interface DriftBrowserWindowController
    : NSObject <NSWindowDelegate, DriftTabStripDelegate, NSTextFieldDelegate,
                NSTableViewDataSource, NSTableViewDelegate>
@property (strong) NSWindow*           window;
@property (strong) DriftTabStripView*  tabStrip;
@property (strong) NSView*             contentContainer;
@property (strong) NSMutableArray<DriftTabModel*>* tabs;
@property (strong) NSMutableArray<DriftTabModel*>* closingTabs; // keeps CefRefPtr alive until OnBeforeClose
@property (strong) NSView*             urlPill;
@property (strong) NSImageView*        lockIcon;
@property (strong) NSTextField*        urlField;
@property (strong) DriftURLDisplayLabel* urlDisplayLabel;
@property (strong) id                  cmdAMonitor;  // NSEvent local monitor for ⌘A
@property (strong) NSButton*           backButton;
@property (strong) NSButton*           forwardButton;
@property (strong) NSButton*           reloadStopButton;
@property (strong) NSView*             progressBar;
@property BOOL urlBarFocusPending;  // set when blank tab opens; cleared in DriftOnAddressChange
// Bookmarks — state lives in _bookmarkState C++ ivar (see @implementation)
@property (strong) NSView*    bookmarksBar;
@property (strong) NSButton*  starButton;
@property BOOL bookmarksBarVisible;
// History
@property (strong) NSPanel*       historyPanel;
@property (strong) NSTableView*   historyTableView;
// Downloads (display-only macOS metadata; canonical state is in _browserState.downloads)
@property (strong) NSView*    downloadBar;
@property (strong) NSView*    downloadItemsView;  // cleared on each refresh
@end

@implementation DriftBrowserWindowController {
  drift::bookmarks::BookmarkState  _bookmarkState;
  drift::history::HistoryState     _historyState;
  drift::browser::BrowserState     _browserState;
  std::unique_ptr<drift::storage::SqliteBookmarkRepo> _bookmarkRepo;
  std::unique_ptr<drift::storage::SqliteHistoryRepo>  _historyRepo;
  uint64_t                         _nextTabId;
  std::vector<std::unique_ptr<drift::engine::CefEngineAdapter>> _engines;
  NSMutableDictionary<NSNumber*, NSString*>* _dlFilePaths;   // downloadId -> file path
  NSMutableDictionary<NSNumber*, NSDate*>*   _dlStartTimes;  // downloadId -> start time
}

- (instancetype)initWithURL:(NSString*)url width:(int)w height:(int)h {
  if (!(self = [super init])) return nil;
  _tabs           = [NSMutableArray array];
  _closingTabs    = [NSMutableArray array];
  _nextTabId      = 0;
  _dlFilePaths    = [NSMutableDictionary dictionary];
  _dlStartTimes   = [NSMutableDictionary dictionary];
  _bookmarksBarVisible = NO;

  NSRect frame = NSMakeRect(100, 100, w, h);
  _window = [[NSWindow alloc]
    initWithContentRect:frame
              styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                        NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable |
                        NSWindowStyleMaskFullSizeContentView
                backing:NSBackingStoreBuffered defer:NO];
  _window.title    = @"Drift";
  _window.delegate = self;
  _window.minSize  = NSMakeSize(640, 480);
  _window.backgroundColor = [NSColor colorWithSRGBRed:0.980 green:0.969 blue:0.949 alpha:1]; // --drift-base #FAF7F2
  _window.titlebarAppearsTransparent = YES;
  _window.titleVisibility = NSWindowTitleHidden;

  NSView* root    = _window.contentView;                         // spans full window frame
  CGFloat fullH   = root.bounds.size.height;                     // includes title bar area
  CGFloat usableH = _window.contentLayoutRect.size.height;       // area below title bar
  CGFloat titleH  = fullH - usableH;

  // Cream fill over the now-transparent native title bar area
  NSView* titleBarBg = [[DriftTitleBarView alloc] initWithFrame:NSMakeRect(0, usableH, w, titleH)];
  titleBarBg.wantsLayer = YES;
  titleBarBg.layer.backgroundColor =
    [NSColor colorWithSRGBRed:0.949 green:0.929 blue:0.898 alpha:1].CGColor;
  titleBarBg.autoresizingMask = NSViewWidthSizable | NSViewMinYMargin;
  [root addSubview:titleBarBg];

  // Tab strip sits just below the title bar area, anchored to the top
  NSRect stripFrame = NSMakeRect(0, usableH - kStripHeight, w, kStripHeight);
  _tabStrip = [[DriftTabStripView alloc] initWithFrame:stripFrame];
  _tabStrip.autoresizingMask = NSViewWidthSizable | NSViewMinYMargin;
  _tabStrip.delegate = self;
  [root addSubview:_tabStrip];

  // Nav bar row — address bar, sits below tab strip
  CGFloat navY = usableH - kStripHeight - kNavBarHeight;
  NSView* navBar = [[NSView alloc] initWithFrame:NSMakeRect(0, navY, w, kNavBarHeight)];
  navBar.wantsLayer = YES;
  navBar.layer.backgroundColor =
    [NSColor colorWithSRGBRed:0.980 green:0.969 blue:0.949 alpha:1].CGColor;  // #FAF7F2
  navBar.autoresizingMask = NSViewWidthSizable | NSViewMinYMargin;
  [root addSubview:navBar];

  NSView* hairline = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, w, 0.5)];
  hairline.wantsLayer = YES;
  hairline.layer.backgroundColor =
    [NSColor colorWithSRGBRed:0.110 green:0.078 blue:0.063 alpha:0.045].CGColor;
  hairline.autoresizingMask = NSViewWidthSizable;
  [navBar addSubview:hairline];

  // Nav buttons — back, forward, reload/stop — left of the URL pill
  static const CGFloat kNavBtnSize = 26.0;
  CGFloat btnY = (kNavBarHeight - kNavBtnSize) / 2;

  NSColor* navTint = [NSColor colorWithSRGBRed:0.478 green:0.431 blue:0.396 alpha:1]; // --drift-text-secondary #7A6E65

  _backButton = [DriftNavButton buttonWithImage:
    [NSImage imageWithSystemSymbolName:@"chevron.left" accessibilityDescription:@"Back"]
    target:self action:@selector(goBack:)];
  _backButton.bordered = NO;
  _backButton.contentTintColor = navTint;
  _backButton.frame    = NSMakeRect(6, btnY, kNavBtnSize, kNavBtnSize);
  _backButton.autoresizingMask = NSViewMaxXMargin | NSViewMinYMargin | NSViewMaxYMargin;
  _backButton.enabled  = NO;
  [navBar addSubview:_backButton];

  _forwardButton = [DriftNavButton buttonWithImage:
    [NSImage imageWithSystemSymbolName:@"chevron.right" accessibilityDescription:@"Forward"]
    target:self action:@selector(goForward:)];
  _forwardButton.bordered = NO;
  _forwardButton.contentTintColor = navTint;
  _forwardButton.frame    = NSMakeRect(34, btnY, kNavBtnSize, kNavBtnSize);
  _forwardButton.autoresizingMask = NSViewMaxXMargin | NSViewMinYMargin | NSViewMaxYMargin;
  _forwardButton.enabled  = NO;
  [navBar addSubview:_forwardButton];

  _reloadStopButton = [DriftNavButton buttonWithImage:
    [NSImage imageWithSystemSymbolName:@"arrow.clockwise" accessibilityDescription:@"Reload"]
    target:self action:@selector(reloadOrStop:)];
  _reloadStopButton.bordered = NO;
  _reloadStopButton.contentTintColor = navTint;
  _reloadStopButton.frame    = NSMakeRect(62, btnY, kNavBtnSize, kNavBtnSize);
  _reloadStopButton.autoresizingMask = NSViewMaxXMargin | NSViewMinYMargin | NSViewMaxYMargin;
  [navBar addSubview:_reloadStopButton];

  // Progress bar — amber line at bottom of nav bar during page loads
  _progressBar = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 0, 2)];
  _progressBar.wantsLayer = YES;
  _progressBar.layer.backgroundColor =
    [NSColor colorWithSRGBRed:0.831 green:0.510 blue:0.039 alpha:1].CGColor;  // #D4820A amber
  _progressBar.autoresizingMask = NSViewNotSizable;
  _progressBar.hidden = YES;
  [navBar addSubview:_progressBar];

  // URL pill: offset past nav buttons on left, tight inset on right
  CGFloat pillL = 94, pillR = 12;
  CGFloat pillY  = (kNavBarHeight - kNavPillH) / 2;
  CGFloat pillW0 = w - pillL - pillR;
  _urlPill = [[NSView alloc] initWithFrame:NSMakeRect(pillL, pillY, pillW0, kNavPillH)];
  _urlPill.wantsLayer = YES;
  _urlPill.layer.backgroundColor =
    [NSColor colorWithSRGBRed:0.949 green:0.929 blue:0.898 alpha:1].CGColor;  // #F2EDE4
  _urlPill.layer.cornerRadius = kNavPillH / 2;
  _urlPill.layer.borderColor  =
    [NSColor colorWithSRGBRed:0.110 green:0.078 blue:0.063 alpha:0.06].CGColor;
  _urlPill.layer.borderWidth  = 1;
  _urlPill.autoresizingMask   = NSViewWidthSizable | NSViewMinYMargin | NSViewMaxYMargin;
  [navBar addSubview:_urlPill];

  // Lock icon (SF Symbol, pine green) — visible only on https
  NSImage* lockImg = [NSImage imageWithSystemSymbolName:@"lock.fill"
                                  accessibilityDescription:@"Secure"];
  _lockIcon = [[NSImageView alloc] initWithFrame:NSMakeRect(12, (kNavPillH - 12) / 2, 12, 12)];
  _lockIcon.image = lockImg;
  _lockIcon.contentTintColor =
    [NSColor colorWithSRGBRed:0.176 green:0.314 blue:0.086 alpha:1];  // #2D5016
  _lockIcon.autoresizingMask = NSViewMaxXMargin | NSViewMinYMargin | NSViewMaxYMargin;
  _lockIcon.hidden = YES;
  [_urlPill addSubview:_lockIcon];

  // URL text field — 36px right margin to leave room for star button
  CGFloat fieldX = 30, fieldH = 16;
  _urlField = [[DriftURLField alloc]
    initWithFrame:NSMakeRect(fieldX, (kNavPillH - fieldH) / 2, pillW0 - fieldX - 36, fieldH)];
  _urlField.bordered              = NO;
  _urlField.drawsBackground       = NO;
  _urlField.focusRingType         = NSFocusRingTypeNone;
  _urlField.cell.scrollable       = YES;
  _urlField.cell.usesSingleLineMode = YES;
  _urlField.delegate              = self;
  _urlField.target                = self;
  _urlField.action                = @selector(urlFieldAction:);
  _urlField.font               = [NSFont fontWithName:@"Inter" size:12.5]
                                   ?: [NSFont systemFontOfSize:12.5];
  _urlField.placeholderString     = @"Search or type a URL";
  _urlField.autoresizingMask      = NSViewWidthSizable | NSViewMinYMargin | NSViewMaxYMargin;
  [_urlPill addSubview:_urlField];

  // Attributed display label overlays the editable field when idle.
  // hitTest returns nil so clicks pass through to the field below.
  _urlDisplayLabel = [[DriftURLDisplayLabel alloc] initWithFrame:_urlField.frame];
  _urlDisplayLabel.bordered        = NO;
  _urlDisplayLabel.drawsBackground = NO;
  _urlDisplayLabel.editable        = NO;
  _urlDisplayLabel.selectable      = NO;
  _urlDisplayLabel.focusRingType   = NSFocusRingTypeNone;
  _urlDisplayLabel.hidden          = YES;
  _urlDisplayLabel.cell.usesSingleLineMode = YES;
  [_urlDisplayLabel.cell setLineBreakMode:NSLineBreakByTruncatingTail];
  _urlDisplayLabel.autoresizingMask = NSViewWidthSizable | NSViewMinYMargin | NSViewMaxYMargin;
  [_urlPill addSubview:_urlDisplayLabel];

  // Star (bookmark) button — right edge of URL pill
  _starButton = [NSButton buttonWithImage:
    [NSImage imageWithSystemSymbolName:@"star" accessibilityDescription:@"Bookmark"]
    target:self action:@selector(toggleBookmark:)];
  _starButton.bordered = NO;
  _starButton.contentTintColor = navTint;
  _starButton.frame = NSMakeRect(pillW0 - 26, (kNavPillH - 16) / 2, 16, 16);
  _starButton.autoresizingMask = NSViewMinXMargin | NSViewMinYMargin | NSViewMaxYMargin;
  [_urlPill addSubview:_starButton];

  // Content container — height managed by relayoutContent, not autoresizing
  _contentContainer = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, w, navY)];
  _contentContainer.autoresizingMask = NSViewWidthSizable;
  [root addSubview:_contentContainer];

  // Bookmarks bar — between nav bar and content, initially hidden
  _bookmarksBar = [[NSView alloc] initWithFrame:NSMakeRect(0, navY - kBookmarksBarH, w, kBookmarksBarH)];
  _bookmarksBar.wantsLayer = YES;
  _bookmarksBar.layer.backgroundColor =
    [NSColor colorWithSRGBRed:0.949 green:0.929 blue:0.898 alpha:1].CGColor;  // --drift-strip
  _bookmarksBar.autoresizingMask = NSViewWidthSizable | NSViewMinYMargin;
  _bookmarksBar.hidden = YES;
  NSView* bbLine = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, w, 0.5)];
  bbLine.wantsLayer = YES;
  bbLine.layer.backgroundColor =
    [NSColor colorWithSRGBRed:0.776 green:0.733 blue:0.678 alpha:0.5].CGColor;
  bbLine.autoresizingMask = NSViewWidthSizable;
  [_bookmarksBar addSubview:bbLine];
  [root addSubview:_bookmarksBar];

  // Download bar — overlays bottom of window, initially hidden
  _downloadBar = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, w, kDownloadBarH)];
  _downloadBar.wantsLayer = YES;
  _downloadBar.layer.backgroundColor =
    [NSColor colorWithSRGBRed:0.980 green:0.969 blue:0.949 alpha:0.97].CGColor;
  _downloadBar.autoresizingMask = NSViewWidthSizable | NSViewMaxYMargin;
  _downloadBar.hidden = YES;
  NSView* dbLine = [[NSView alloc]
    initWithFrame:NSMakeRect(0, kDownloadBarH - 0.5, w, 0.5)];
  dbLine.wantsLayer = YES;
  dbLine.layer.backgroundColor =
    [NSColor colorWithSRGBRed:0.776 green:0.733 blue:0.678 alpha:1].CGColor;
  dbLine.autoresizingMask = NSViewWidthSizable | NSViewMinYMargin;
  [_downloadBar addSubview:dbLine];
  // Items container — cleared on each refresh; separate from the border line above
  _downloadItemsView = [[NSView alloc]
    initWithFrame:NSMakeRect(0, 0, w, kDownloadBarH - 1)];
  _downloadItemsView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
  [_downloadBar addSubview:_downloadItemsView];
  [root addSubview:_downloadBar];

  [_window makeKeyAndOrderFront:nil];
  [self openTabWithURL:url];

  [self setupPersistence];

  // Local monitor: intercept ⌘A when the URL field editor is active.
  // CEF's event handling can consume Command+key events before they reach
  // NSTextView's key-binding system, so we handle it explicitly here.
  __unsafe_unretained typeof(self) ws = self;  // MRC: no __weak; monitor removed in dealloc
  _cmdAMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskKeyDown
      handler:^NSEvent*(NSEvent* e) {
        NSEventModifierFlags cmd   = NSEventModifierFlagCommand;
        NSEventModifierFlags shift = NSEventModifierFlagShift;
        NSEventModifierFlags mods  = e.modifierFlags &
          (cmd | shift | NSEventModifierFlagOption | NSEventModifierFlagControl);
        NSString* ch = e.charactersIgnoringModifiers;

        if (mods == cmd) {
          if ([ch isEqualToString:@"a"]) {
            NSTextView* ed = (NSTextView*)[ws.urlField currentEditor];
            if (ed) { [ed selectAll:nil]; return nil; }
          }
          if ([ch isEqualToString:@"t"]) { [ws openTabWithURL:@"about:blank"]; return nil; }
          if ([ch isEqualToString:@"w"]) { [ws closeActiveTab];                 return nil; }
          if ([ch isEqualToString:@"l"]) { [ws focusAddressBar];                return nil; }
          if ([ch isEqualToString:@"r"]) { [ws reloadActiveTab];                return nil; }
          if ([ch isEqualToString:@"d"]) { [ws toggleBookmark:nil];             return nil; }
          if ([ch isEqualToString:@"y"]) { [ws showHistoryPanel];               return nil; }
          if (ch.length == 1) {
            unichar c = [ch characterAtIndex:0];
            if (c >= '1' && c <= '9') {
              NSInteger idx = (NSInteger)(c - '1');
              if (idx < (NSInteger)ws.tabs.count) [ws switchToIndex:idx];
              return nil;
            }
          }
        }
        if (mods == (cmd | shift)) {
          if ([ch.lowercaseString isEqualToString:@"t"]) {
            [ws reopenLastClosedTab]; return nil;
          }
          if ([ch.lowercaseString isEqualToString:@"b"]) {
            [ws toggleBookmarksBar]; return nil;
          }
        }
        NSEventModifierFlags ctrl = NSEventModifierFlagControl;
        if (e.keyCode == 48 && ws.tabs.count > 1) {  // keyCode 48 = Tab
          NSInteger cur = [ws activeIndexInStrip];
          if (mods == ctrl) {  // ⌃Tab → next tab
            [ws switchToIndex:(cur + 1) % (NSInteger)ws.tabs.count]; return nil;
          }
          if (mods == (ctrl | shift)) {  // ⌃⇧Tab → previous tab
            [ws switchToIndex:(cur - 1 + (NSInteger)ws.tabs.count) % (NSInteger)ws.tabs.count]; return nil;
          }
        }
        return e;
      }];
  return self;
}

- (void)dealloc {
  if (_cmdAMonitor) [NSEvent removeMonitor:_cmdAMonitor];
  [super dealloc];
}

// ── Tab state helpers ─────────────────────────────────────────────────────────

- (NSInteger)activeIndexInStrip {
  if (!_browserState.active_tab_id.has_value()) return -1;
  uint64_t aid = _browserState.active_tab_id->value;
  for (NSInteger i = 0; i < (NSInteger)_tabs.count; i++)
    if (_tabs[i].tabId == aid) return i;
  return -1;
}

- (DriftTabModel*)tabModelForTabId:(uint64_t)tabId {
  for (DriftTabModel* t in _tabs) if (t.tabId == tabId) return t;
  return nil;
}

- (const drift::browser::TabState*)tabStateForTabId:(uint64_t)tabId {
  for (auto& ts : _browserState.tabs)
    if (ts.id.value == tabId) return &ts;
  return nullptr;
}

// ── Layout ────────────────────────────────────────────────────────────────────

- (void)windowDidResize:(NSNotification*)__unused n {
  [self relayoutContent];
}

- (void)relayoutContent {
  CGFloat usableH = _window.contentLayoutRect.size.height;
  CGFloat cw      = _window.contentView.bounds.size.width;
  CGFloat navY    = usableH - kStripHeight - kNavBarHeight;
  CGFloat barH    = _bookmarksBarVisible ? kBookmarksBarH : 0;
  _contentContainer.frame = NSMakeRect(0, 0, cw, navY - barH);
  _bookmarksBar.frame     = NSMakeRect(0, navY - kBookmarksBarH, cw, kBookmarksBarH);
  _downloadBar.frame      = NSMakeRect(0, 0, cw, kDownloadBarH);
  NSRect bb = _contentContainer.bounds;
  for (DriftTabModel* t in _tabs) {
    if (!t.engine) continue;
    NSView* v = (__bridge NSView*)t.engine->native_view_handle();
    v.frame = bb;
  }
  [self refreshBookmarksBar];
}

// ── Tab management ────────────────────────────────────────────────────────────

- (void)openTabWithURL:(NSString*)url {
  uint64_t tid = ++_nextTabId;
  DriftTabModel* tab = [[DriftTabModel alloc] init];
  tab.tabId = tid;
  [_tabs addObject:tab];

  (void)drift::browser::apply_event(_browserState,
      drift::browser::TabCreatedEvent{
          drift::browser::TabId{tid},
          url ? std::string(url.UTF8String) : "",
          /*activate=*/false});  // switchToTabId: will apply TabSelectedEvent

  CefWindowInfo info;
  NSRect b = _contentContainer.bounds;
  info.SetAsChild((__bridge CefWindowHandle)_contentContainer,
                  {0, 0, (int)b.size.width, (int)b.size.height});

  CefRefPtr<DriftClient> client(new DriftClient);
  CefBrowserSettings settings;
  CefBrowserHost::CreateBrowser(info, client,
    CefString(url ? [url UTF8String] : ""), settings, nullptr, nullptr);

  [self switchToTabId:tid];
  // DriftOnAddressChange claims focus once CEF commits about:blank — after all async focus-stealing has settled
  if ([url isEqualToString:@"about:blank"]) _urlBarFocusPending = YES;
}

- (void)switchToTabId:(uint64_t)tid {
  if ([_urlField currentEditor]) [_window makeFirstResponder:nil];
  for (DriftTabModel* t in _tabs) {
    if (t.engine) {
      NSView* v = (__bridge NSView*)t.engine->native_view_handle();
      v.hidden  = YES;
    }
  }
  (void)drift::browser::apply_event(_browserState,
      drift::browser::TabSelectedEvent{drift::browser::TabId{tid}});
  DriftTabModel* active = [self tabModelForTabId:tid];
  if (active && active.engine) {
    NSView* v          = (__bridge NSView*)active.engine->native_view_handle();
    v.hidden           = NO;
    v.frame            = _contentContainer.bounds;
    v.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
  }
  const drift::browser::TabState* ts = [self tabStateForTabId:tid];
  _window.title = (ts && !ts->title.empty())
    ? [NSString stringWithUTF8String:ts->title.c_str()] : @"New Tab";
  [self refreshStrip];
  [self refreshNavBar];
  [self refreshNavButtons];
  [self refreshStarButton];
}

- (void)switchToIndex:(NSInteger)idx {
  if (idx < 0 || idx >= (NSInteger)_tabs.count) return;
  [self switchToTabId:_tabs[idx].tabId];
}

- (void)refreshStrip {
  NSInteger activeIdx = [self activeIndexInStrip];
  NSMutableArray<DriftTabInfo*>* infos = [NSMutableArray array];
  for (NSInteger i = 0; i < (NSInteger)_tabs.count; i++) {
    DriftTabModel* t = _tabs[i];
    const drift::browser::TabState* ts = [self tabStateForTabId:t.tabId];
    DriftTabInfo* info = [[DriftTabInfo alloc] init];
    info.index        = i;
    info.title        = (ts && !ts->title.empty())
      ? [NSString stringWithUTF8String:ts->title.c_str()] : @"New Tab";
    info.favicon      = t.favicon;
    info.isPinned     = ts ? ts->pinned : NO;
    info.isActive     = (i == activeIdx);
    info.isLoading    = ts ? ts->loading : NO;
    NSString* urlStr  = ts ? [NSString stringWithUTF8String:ts->url.c_str()] : @"";
    NSURL* u          = [NSURL URLWithString:urlStr];
    NSString* host    = u.host ?: urlStr;
    info.faviconLetter= host.length ? [[host substringToIndex:1] uppercaseString] : @"?";
    [infos addObject:info];
  }
  _tabStrip.tabs        = infos;
  _tabStrip.activeIndex = activeIdx;
  [_tabStrip setNeedsDisplay:YES];
}

- (DriftTabModel*)tabForBrowserId:(int)bid {
  for (DriftTabModel* t in _tabs) if (t.browserId == bid) return t;
  return nil;
}

- (NSInteger)indexForBrowserId:(int)bid {
  for (NSInteger i = 0; i < (NSInteger)_tabs.count; i++)
    if (_tabs[i].browserId == bid) return i;
  return -1;
}

- (drift::engine::IBrowserEngine*)adoptEngineForBrowser:(CefRefPtr<CefBrowser>)browser {
  auto adapter = std::make_unique<drift::engine::CefEngineAdapter>(browser);
  drift::engine::IBrowserEngine* ptr = adapter.get();
  _engines.push_back(std::move(adapter));
  return ptr;
}

- (void)releaseEngineForBrowserId:(int)bid {
  _engines.erase(std::remove_if(_engines.begin(), _engines.end(),
    [bid](const std::unique_ptr<drift::engine::CefEngineAdapter>& e) {
      return e->browser_id() == bid;
    }), _engines.end());
}

// ── DriftTabStripDelegate ─────────────────────────────────────────────────────

- (void)tabStripDidRequestNewTab:(DriftTabStripView*)__unused strip {
  [self openTabWithURL:@"about:blank"];
}

- (void)tabStrip:(DriftTabStripView*)__unused strip didSelectTabAtIndex:(NSInteger)i {
  [self switchToIndex:i];
}

- (void)tabStrip:(DriftTabStripView*)__unused strip didCloseTabAtIndex:(NSInteger)i {
  if (i < 0 || i >= (NSInteger)_tabs.count) return;
  // Chrome-style: closing the last tab quits Drift.
  if (_tabs.count == 1) { [NSApp terminate:nil]; return; }
  DriftTabModel* tab = _tabs[i];

  // TabClosedEvent records this in closed_tabs and updates active_tab_id to the next tab
  (void)drift::browser::apply_event(_browserState,
      drift::browser::TabClosedEvent{drift::browser::TabId{tab.tabId}});

  // Move to closingTabs BEFORE calling CloseBrowser so CefRefPtr stays alive
  // until CEF calls OnBeforeClose. Only then does DriftOnBrowserClosed release it.
  [_tabs removeObjectAtIndex:i];
  [_closingTabs addObject:tab];

  if (_browserState.active_tab_id.has_value())
    [self switchToTabId:_browserState.active_tab_id->value];

  if (tab.engine) {
    tab.engine->close();
  } else {
    [_closingTabs removeObject:tab];
    if (_tabs.count == 0) [_window close];
  }
}

- (void)tabStrip:(DriftTabStripView*)__unused strip
  didMoveTabFrom:(NSInteger)from to:(NSInteger)to {
  if (from == to || from < 0 || to < 0 ||
      from >= (NSInteger)_tabs.count || to >= (NSInteger)_tabs.count) return;
  DriftTabModel* t = _tabs[from];
  (void)drift::browser::apply_event(_browserState,
      drift::browser::TabMovedEvent{
          drift::browser::TabId{t.tabId}, (std::uint32_t)to});
  [_tabs removeObjectAtIndex:from];
  [_tabs insertObject:t atIndex:to];
  [self refreshStrip];
}

- (void)tabStrip:(DriftTabStripView*)__unused strip didTogglePinAtIndex:(NSInteger)i {
  if (i < 0 || i >= (NSInteger)_tabs.count) return;
  uint64_t tid = _tabs[i].tabId;
  const drift::browser::TabState* ts = [self tabStateForTabId:tid];
  (void)drift::browser::apply_event(_browserState,
      drift::browser::TabPinnedEvent{
          drift::browser::TabId{tid}, ts ? !ts->pinned : true});
  [self refreshStrip];
}

- (void)tabStrip:(DriftTabStripView*)__unused strip didRequestDuplicateTabAtIndex:(NSInteger)i {
  if (i < 0 || i >= (NSInteger)_tabs.count) return;
  const drift::browser::TabState* ts = [self tabStateForTabId:_tabs[i].tabId];
  NSString* url = ts && !ts->url.empty()
    ? [NSString stringWithUTF8String:ts->url.c_str()] : @"about:blank";
  [self openTabWithURL:url];
}

- (void)tabStrip:(DriftTabStripView*)__unused strip didCloseOtherTabsExceptIndex:(NSInteger)keepIdx {
  if (keepIdx < 0 || keepIdx >= (NSInteger)_tabs.count) return;
  // Close after keepIdx first (no index shift), then before going backwards
  for (NSInteger j = (NSInteger)_tabs.count - 1; j > keepIdx; j--)
    [self tabStrip:_tabStrip didCloseTabAtIndex:j];
  for (NSInteger j = keepIdx - 1; j >= 0; j--)
    [self tabStrip:_tabStrip didCloseTabAtIndex:j];
}

// ── Address bar ───────────────────────────────────────────────────────────────

- (void)refreshNavBar {
  if ([_urlField currentEditor]) return;
  NSInteger activeIdx = [self activeIndexInStrip];
  if (activeIdx < 0 || activeIdx >= (NSInteger)_tabs.count) {
    _urlField.stringValue = @"";
    _lockIcon.hidden = YES;
    _urlDisplayLabel.hidden = YES;
    return;
  }
  const drift::browser::TabState* ts = [self tabStateForTabId:_tabs[activeIdx].tabId];
  NSString* url = ts ? [NSString stringWithUTF8String:ts->url.c_str()] : @"";
  if (!url.length || [url isEqualToString:@"about:blank"]) {
    _urlField.stringValue = @"";
    _lockIcon.hidden = YES;
    _urlDisplayLabel.hidden = YES;
    return;
  }
  _lockIcon.hidden = ![url hasPrefix:@"https://"];
  _urlField.stringValue = url;
  _urlField.textColor = [NSColor clearColor];
  _urlDisplayLabel.attributedStringValue = [self displayStringForURL:url];
  _urlDisplayLabel.hidden = NO;
}

- (NSAttributedString*)displayStringForURL:(NSString*)urlStr {
  NSURL* parsed = [NSURL URLWithString:urlStr];
  NSString* host = parsed.host ?: urlStr;
  NSString* rest = parsed ? (parsed.path ?: @"") : @"";
  if (parsed.query) rest = [rest stringByAppendingFormat:@"?%@", parsed.query];
  NSFont* font = [NSFont fontWithName:@"Inter" size:12.5]
                 ?: [NSFont systemFontOfSize:12.5];
  NSMutableAttributedString* as = [NSMutableAttributedString new];
  [as appendAttributedString:[[NSAttributedString alloc] initWithString:host attributes:@{
    NSFontAttributeName: font,
    NSForegroundColorAttributeName:
      [NSColor colorWithSRGBRed:0.110 green:0.078 blue:0.063 alpha:1]  // --drift-text #1C1410
  }]];
  if (rest.length)
    [as appendAttributedString:[[NSAttributedString alloc] initWithString:rest attributes:@{
      NSFontAttributeName: font,
      NSForegroundColorAttributeName:
        [NSColor colorWithSRGBRed:0.478 green:0.431 blue:0.396 alpha:1]  // --drift-text-secondary #7A6E65
    }]];
  return as;
}

- (NSString*)navigationURLForInput:(NSString*)text {
  NSString* s = [text stringByTrimmingCharactersInSet:
    [NSCharacterSet whitespaceAndNewlineCharacterSet]];
  if (!s.length) return nil;
  if ([s hasPrefix:@"http://"] || [s hasPrefix:@"https://"] || [s hasPrefix:@"file://"])
    return s;
  if ([s containsString:@"."] && ![s containsString:@" "])
    return [@"https://" stringByAppendingString:s];
  NSString* enc = [s stringByAddingPercentEncodingWithAllowedCharacters:
    [NSCharacterSet URLQueryAllowedCharacterSet]];
  return [@"https://www.google.com/search?q=" stringByAppendingString:enc ?: s];
}

- (void)navigateActiveTabToURL:(NSString*)urlStr {
  if (!urlStr) return;
  NSInteger activeIdx = [self activeIndexInStrip];
  if (activeIdx < 0 || activeIdx >= (NSInteger)_tabs.count) return;
  DriftTabModel* t = _tabs[activeIdx];
  if (t.engine) t.engine->navigate([urlStr UTF8String]);
}

- (void)closeActiveTab {
  NSInteger activeIdx = [self activeIndexInStrip];
  if (activeIdx >= 0 && activeIdx < (NSInteger)_tabs.count)
    [self tabStrip:_tabStrip didCloseTabAtIndex:activeIdx];
}

- (void)focusAddressBar {
  [_window makeFirstResponder:_urlField];
  // ⌘L: field editor is installed synchronously by makeFirstResponder, so selectAll is safe here
  NSTextView* ed = (NSTextView*)[_urlField currentEditor];
  if (ed) [ed setSelectedRange:NSMakeRange(0, ed.string.length)];
}

- (void)reloadActiveTab {
  NSInteger activeIdx = [self activeIndexInStrip];
  if (activeIdx < 0 || activeIdx >= (NSInteger)_tabs.count) return;
  DriftTabModel* t = _tabs[activeIdx];
  if (t.engine) t.engine->reload();
}

- (void)reopenLastClosedTab {
  if (_browserState.closed_tabs.empty()) return;
  std::string url = _browserState.closed_tabs.front().url;
  _browserState.closed_tabs.erase(_browserState.closed_tabs.begin());
  [self openTabWithURL:[NSString stringWithUTF8String:url.c_str()]];
}

- (void)refreshNavButtons {
  NSInteger activeIdx = [self activeIndexInStrip];
  if (activeIdx < 0 || activeIdx >= (NSInteger)_tabs.count) {
    _backButton.enabled = _forwardButton.enabled = _reloadStopButton.enabled = NO;
    _progressBar.hidden = YES;
    return;
  }
  const drift::browser::TabState* ts = [self tabStateForTabId:_tabs[activeIdx].tabId];
  _backButton.enabled    = ts ? ts->can_go_back : NO;
  _forwardButton.enabled = ts ? ts->can_go_forward : NO;
  _reloadStopButton.enabled = YES;
  bool loading = ts ? ts->loading : false;
  NSString* sym  = loading ? @"xmark"  : @"arrow.clockwise";
  NSString* a11y = loading ? @"Stop"   : @"Reload";
  _reloadStopButton.image = [NSImage imageWithSystemSymbolName:sym
                                       accessibilityDescription:a11y];
  double prog = ts ? ts->load_progress : 0.0;
  _progressBar.hidden = !loading || prog <= 0;
  if (!_progressBar.hidden) {
    CGFloat barW = _progressBar.superview.bounds.size.width * (CGFloat)prog;
    NSRect f = _progressBar.frame;
    f.size.width = MAX(barW, 4);
    _progressBar.frame = f;
  }
}

- (IBAction)goBack:(id)__unused sender {
  NSInteger activeIdx = [self activeIndexInStrip];
  if (activeIdx < 0 || activeIdx >= (NSInteger)_tabs.count) return;
  DriftTabModel* t = _tabs[activeIdx];
  const drift::browser::TabState* ts = [self tabStateForTabId:t.tabId];
  if (t.engine && ts && ts->can_go_back) t.engine->go_back();
}

- (IBAction)goForward:(id)__unused sender {
  NSInteger activeIdx = [self activeIndexInStrip];
  if (activeIdx < 0 || activeIdx >= (NSInteger)_tabs.count) return;
  DriftTabModel* t = _tabs[activeIdx];
  const drift::browser::TabState* ts = [self tabStateForTabId:t.tabId];
  if (t.engine && ts && ts->can_go_forward) t.engine->go_forward();
}

- (IBAction)reloadOrStop:(id)__unused sender {
  NSInteger activeIdx = [self activeIndexInStrip];
  if (activeIdx < 0 || activeIdx >= (NSInteger)_tabs.count) return;
  DriftTabModel* t = _tabs[activeIdx];
  const drift::browser::TabState* ts = [self tabStateForTabId:t.tabId];
  if (!t.engine) return;
  if (ts && ts->loading) t.engine->stop();
  else                   t.engine->reload();
}

- (IBAction)urlFieldAction:(id)__unused sender {
  // currentEditor holds the live field editor content; stringValue returns the stale display value
  NSString* text = [(NSText*)[_urlField currentEditor] string] ?: _urlField.stringValue;
  NSString* nav = [self navigationURLForInput:text];
  if (nav) [self navigateActiveTabToURL:nav];
  [_window makeFirstResponder:nil];
}

- (void)controlTextDidBeginEditing:(NSNotification*)n {
  _urlDisplayLabel.hidden = YES;
  NSColor* editColor = [NSColor colorWithSRGBRed:0.110 green:0.078 blue:0.063 alpha:1]; // --drift-text #1C1410
  // CRITICAL: also set field's own textColor to dark. Editor may inherit field.textColor before
  // our attribute overrides take effect — a race that made typed text invisible (BUG-19).
  // refreshNavBar resets to clearColor when display label is next shown.
  _urlField.textColor = editColor;
  NSTextView* editor = (NSTextView*)[n.userInfo objectForKey:@"NSFieldEditor"];
  NSFont* font = [NSFont fontWithName:@"Inter" size:12.5] ?: [NSFont systemFontOfSize:12.5];
  [editor.textStorage setAttributes:@{NSFontAttributeName: font,
                                      NSForegroundColorAttributeName: editColor}
                              range:NSMakeRange(0, editor.string.length)];
  editor.typingAttributes = @{NSFontAttributeName: font, NSForegroundColorAttributeName: editColor};
  // Selection on click handled by DriftURLField.mouseDown:; on ⌘L by focusAddressBar.
}

- (void)controlTextDidEndEditing:(NSNotification*)__unused n {
  _urlBarFocusPending = NO;
  [self refreshNavBar];
}

// ── Bookmarks ─────────────────────────────────────────────────────────────────

- (NSString*)driftDataPath:(NSString*)filename {
  NSString* dir = [[NSSearchPathForDirectoriesInDomains(
    NSApplicationSupportDirectory, NSUserDomainMask, YES) firstObject]
    stringByAppendingPathComponent:@"Drift"];
  [[NSFileManager defaultManager] createDirectoryAtPath:dir
    withIntermediateDirectories:YES attributes:nil error:nil];
  return [dir stringByAppendingPathComponent:filename];
}

- (void)setupPersistence {
  NSString* dbPath = [self driftDataPath:@"drift.sqlite"];
  BOOL needsMigration = ![[NSFileManager defaultManager] fileExistsAtPath:dbPath];
  _bookmarkRepo = std::make_unique<drift::storage::SqliteBookmarkRepo>(dbPath.UTF8String);
  _historyRepo  = std::make_unique<drift::storage::SqliteHistoryRepo>(dbPath.UTF8String);
  if (needsMigration) {
    [self loadBookmarksFromJSON];
    [self loadHistoryFromJSON];
    _bookmarkRepo->save_all(_bookmarkState.bookmarks);
    _historyRepo->save_all(_historyState.entries);
  } else {
    _bookmarkState.bookmarks = _bookmarkRepo->load_all();
    _historyState.entries    = _historyRepo->load_all();
  }
}

- (void)loadBookmarksFromJSON {
  NSData* data = [NSData dataWithContentsOfFile:[self driftDataPath:@"bookmarks.json"]];
  if (!data) return;
  NSArray* arr = [NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
  if (![arr isKindOfClass:[NSArray class]]) return;
  for (NSDictionary* d in arr) {
    if (![d isKindOfClass:[NSDictionary class]]) continue;
    drift::bookmarks::Bookmark b;
    b.url           = [d[@"url"]       UTF8String] ?: "";
    b.title         = [d[@"title"]     UTF8String] ?: "";
    b.timestamp_utc = [d[@"timestamp"] UTF8String] ?: "";
    (void)drift::bookmarks::add_bookmark(_bookmarkState, std::move(b));
  }
}

- (void)saveBookmarks {
  _bookmarkRepo->save_all(_bookmarkState.bookmarks);
}

- (BOOL)isActiveTabBookmarked {
  NSInteger activeIdx = [self activeIndexInStrip];
  if (activeIdx < 0 || activeIdx >= (NSInteger)_tabs.count) return NO;
  const drift::browser::TabState* ts = [self tabStateForTabId:_tabs[activeIdx].tabId];
  return ts ? drift::bookmarks::has_bookmark(_bookmarkState, ts->url) : NO;
}

- (IBAction)toggleBookmark:(id)__unused sender {
  NSInteger activeIdx = [self activeIndexInStrip];
  if (activeIdx < 0 || activeIdx >= (NSInteger)_tabs.count) return;
  const drift::browser::TabState* tab = [self tabStateForTabId:_tabs[activeIdx].tabId];
  if (!tab || !drift::bookmarks::is_bookmarkable_url(tab->url)) return;

  if (drift::bookmarks::has_bookmark(_bookmarkState, tab->url)) {
    (void)drift::bookmarks::remove_bookmark(_bookmarkState, tab->url);
  } else {
    drift::bookmarks::Bookmark b;
    b.url   = tab->url;
    b.title = tab->title.empty() ? tab->url : tab->title;
    NSDateFormatter* fmt = [NSDateFormatter new];
    fmt.dateFormat = @"yyyy-MM-dd'T'HH:mm:ss'Z'";
    fmt.timeZone   = [NSTimeZone timeZoneWithName:@"UTC"];
    NSString* ts = [fmt stringFromDate:[NSDate date]];
    b.timestamp_utc = ts ? std::string(ts.UTF8String) : "";
    (void)drift::bookmarks::add_bookmark(_bookmarkState, std::move(b));
  }
  [self saveBookmarks];
  [self refreshStarButton];
  [self refreshBookmarksBar];
}

- (void)refreshStarButton {
  BOOL bookmarked = [self isActiveTabBookmarked];
  NSString* sym = bookmarked ? @"star.fill" : @"star";
  _starButton.image = [NSImage imageWithSystemSymbolName:sym accessibilityDescription:@"Bookmark"];
  _starButton.contentTintColor = bookmarked
    ? [NSColor colorWithSRGBRed:0.831 green:0.510 blue:0.039 alpha:1]   // amber — bookmarked
    : [NSColor colorWithSRGBRed:0.478 green:0.431 blue:0.396 alpha:1];  // secondary
}

- (void)toggleBookmarksBar {
  _bookmarksBarVisible = !_bookmarksBarVisible;
  [self relayoutContent];
}

- (void)refreshBookmarksBar {
  _bookmarksBar.hidden = !_bookmarksBarVisible;
  if (!_bookmarksBarVisible) return;

  // Remove old bookmark buttons (NSButton); hairline (plain NSView) stays
  for (NSView* v in [_bookmarksBar.subviews copy])
    if ([v isKindOfClass:[NSButton class]]) [v removeFromSuperview];

  CGFloat x = 8;
  NSFont* font = [NSFont fontWithName:@"Inter" size:11] ?: [NSFont systemFontOfSize:11];
  NSDictionary* attrs = @{NSFontAttributeName: font};

  auto& bks = _bookmarkState.bookmarks;
  for (NSInteger i = 0; i < (NSInteger)bks.size(); i++) {
    const auto& bk = bks[(size_t)i];
    NSString* nsTitle = bk.title.empty() ? @(bk.url.c_str()) : @(bk.title.c_str());
    CGFloat btnW = MIN([nsTitle sizeWithAttributes:attrs].width + 16, 150);
    if (x + btnW > _bookmarksBar.bounds.size.width - 8) break;

    NSButton* btn = [NSButton buttonWithTitle:nsTitle
                                       target:self
                                       action:@selector(bookmarkButtonClicked:)];
    btn.frame   = NSMakeRect(x, 4, btnW, 18);
    btn.font    = font;
    btn.bordered = NO;
    btn.tag     = i;
    btn.toolTip = @(bk.url.c_str());
    [_bookmarksBar addSubview:btn];
    x += btnW + 4;
  }
}

- (void)bookmarkButtonClicked:(NSButton*)sender {
  NSInteger i = sender.tag;
  auto& bks = _bookmarkState.bookmarks;
  if (i >= 0 && i < (NSInteger)bks.size())
    [self navigateActiveTabToURL:@(bks[(size_t)i].url.c_str())];
}

// ── History ───────────────────────────────────────────────────────────────────

- (void)loadHistoryFromJSON {
  NSData* data = [NSData dataWithContentsOfFile:[self driftDataPath:@"history.json"]];
  if (!data) return;
  NSArray* arr = [NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
  if (![arr isKindOfClass:[NSArray class]]) return;
  for (NSDictionary* d in arr) {
    if (![d isKindOfClass:[NSDictionary class]]) continue;
    std::string url   = [d[@"url"]       ?: @"" UTF8String];
    std::string title = [d[@"title"]     ?: @"" UTF8String];
    std::string ts    = [d[@"timestamp"] ?: @"" UTF8String];
    if (!url.empty() && drift::history::is_recordable_url(url))
      _historyState.entries.push_back({url, title, ts});
  }
}

- (void)saveHistory {
  _historyRepo->save_all(_historyState.entries);
}

- (void)recordHistoryEntry:(NSString*)url title:(NSString*)title {
  if (!url.length) return;
  NSDateFormatter* fmt = [NSDateFormatter new];
  fmt.dateFormat = @"yyyy-MM-dd'T'HH:mm:ss'Z'";
  fmt.timeZone   = [NSTimeZone timeZoneWithName:@"UTC"];
  drift::history::HistoryEntry e{
    [url UTF8String],
    [(title.length ? title : url) UTF8String],
    [[fmt stringFromDate:[NSDate date]] UTF8String],
  };
  if (drift::history::add_entry(_historyState, std::move(e)))
    return;  // non-recordable URL; do not save
  [self saveHistory];
}

- (void)showHistoryPanel {
  if (_historyPanel) {
    [_historyPanel makeKeyAndOrderFront:nil];
    [_historyTableView reloadData];
    return;
  }
  _historyPanel = [[NSPanel alloc]
    initWithContentRect:NSMakeRect(0, 0, 480, 560)
              styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                        NSWindowStyleMaskResizable
                backing:NSBackingStoreBuffered defer:NO];
  _historyPanel.title = @"History";
  [_historyPanel center];

  NSView* pv = _historyPanel.contentView;
  CGFloat pw = pv.bounds.size.width, ph = pv.bounds.size.height;

  // Table view
  _historyTableView = [[NSTableView alloc] initWithFrame:NSZeroRect];
  NSTableColumn* col = [[NSTableColumn alloc] initWithIdentifier:@"entry"];
  col.width = pw - 4;
  col.editable = NO;
  [_historyTableView addTableColumn:col];
  _historyTableView.headerView = nil;
  _historyTableView.dataSource = self;
  _historyTableView.delegate   = self;
  _historyTableView.target     = self;
  _historyTableView.doubleAction = @selector(historyTableDoubleClicked:);

  NSScrollView* sv = [[NSScrollView alloc]
    initWithFrame:NSMakeRect(0, 36, pw, ph - 36)];
  sv.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
  sv.hasVerticalScroller = YES;
  sv.documentView = _historyTableView;
  [pv addSubview:sv];
  [_historyTableView sizeToFit];

  // Clear All button
  NSButton* clearBtn = [NSButton buttonWithTitle:@"Clear History"
                                          target:self
                                          action:@selector(clearHistory:)];
  clearBtn.frame = NSMakeRect(pw - 124, 6, 114, 26);
  clearBtn.autoresizingMask = NSViewMinXMargin | NSViewMaxYMargin;
  [pv addSubview:clearBtn];

  [_historyTableView reloadData];
  [_historyPanel makeKeyAndOrderFront:nil];
  [_window addChildWindow:_historyPanel ordered:NSWindowAbove];
}

// NSTableViewDataSource
- (NSInteger)numberOfRowsInTableView:(NSTableView*)__unused tv {
  return (NSInteger)_historyState.entries.size();
}

- (id)tableView:(NSTableView*)__unused tv
  objectValueForTableColumn:(NSTableColumn*)__unused col
                        row:(NSInteger)row {
  if (row < 0 || row >= (NSInteger)_historyState.entries.size()) return @"";
  const auto& e = _historyState.entries[(std::size_t)row];
  NSString* ts = [NSString stringWithUTF8String:e.timestamp_utc.c_str()];
  NSString* dateStr = ts.length >= 10 ? [ts substringToIndex:10] : ts;
  NSString* label = e.title.empty()
    ? [NSString stringWithUTF8String:e.url.c_str()]
    : [NSString stringWithUTF8String:e.title.c_str()];
  return [NSString stringWithFormat:@"%@   %@", dateStr, label];
}

- (void)historyTableDoubleClicked:(NSTableView*)__unused sender {
  NSInteger row = _historyTableView.clickedRow;
  if (row < 0 || row >= (NSInteger)_historyState.entries.size()) return;
  [self navigateActiveTabToURL:
    [NSString stringWithUTF8String:_historyState.entries[(std::size_t)row].url.c_str()]];
}

- (IBAction)clearHistory:(id)__unused sender {
  drift::history::clear_all(_historyState);
  [self saveHistory];
  [_historyTableView reloadData];
}

// ── Downloads ─────────────────────────────────────────────────────────────────

- (void)refreshDownloadBar {
  // Clear item container (border line lives in _downloadBar directly and is unaffected)
  for (NSView* v in [_downloadItemsView.subviews copy]) [v removeFromSuperview];

  // Count visible (non-cancelled) downloads
  std::size_t visibleCount = 0;
  for (const auto& d : _browserState.downloads)
    if (d.status != drift::browser::DownloadStatus::cancelled) ++visibleCount;

  if (!visibleCount) { _downloadBar.hidden = YES; return; }
  _downloadBar.hidden = NO;

  NSFont* font = [NSFont fontWithName:@"Inter" size:10] ?: [NSFont systemFontOfSize:10];
  NSColor* textColor = [NSColor colorWithSRGBRed:0.110 green:0.078 blue:0.063 alpha:1];
  NSColor* secColor  = [NSColor colorWithSRGBRed:0.478 green:0.431 blue:0.396 alpha:1];

  NSString* (^fmt)(int64_t) = ^NSString*(int64_t b) {
    if (b < 1024)              return [NSString stringWithFormat:@"%lld B",  b];
    if (b < 1024LL*1024)       return [NSString stringWithFormat:@"%.1f KB", b / 1024.0];
    if (b < 1024LL*1024*1024)  return [NSString stringWithFormat:@"%.1f MB", b / (1024.0*1024.0)];
    return [NSString stringWithFormat:@"%.2f GB", b / (1024.0*1024.0*1024.0)];
  };

  CGFloat x = 12;
  for (const auto& d : _browserState.downloads) {
    if (d.status == drift::browser::DownloadStatus::cancelled) continue;

    CGFloat itemW = 230;
    CGFloat viewW = _downloadItemsView.bounds.size.width;
    if (viewW > 0 && x + itemW > viewW - 12) break;

    CGFloat textW = itemW - 48;  // leave 48px right margin for two icon buttons
    NSInteger downloadId = (NSInteger)d.id.value;
    NSString* filePath = _dlFilePaths[@(downloadId)];
    NSDate*   startTime = _dlStartTimes[@(downloadId)];
    BOOL isComplete = (d.status == drift::browser::DownloadStatus::complete);
    NSString* filename = d.file_name.empty()
      ? @"Download"
      : [NSString stringWithUTF8String:d.file_name.c_str()];

    // Filename
    NSTextField* nameLabel = [NSTextField labelWithString:filename];
    nameLabel.frame = NSMakeRect(x, 28, textW, 13);
    nameLabel.font = font;
    nameLabel.textColor = textColor;
    nameLabel.cell.lineBreakMode = NSLineBreakByTruncatingMiddle;
    [_downloadItemsView addSubview:nameLabel];

    // Status label — size/progress + speed (in-progress) or completion summary (done)
    NSString* status;
    NSTimeInterval elapsed = startTime ? -[startTime timeIntervalSinceNow] : 0;
    if (isComplete) {
      NSString* summary = elapsed > 0.5
        ? [NSString stringWithFormat:@"%.1fs · avg %@/s",
            elapsed, fmt((int64_t)(d.received_bytes / elapsed))]
        : @"";
      status = summary.length
        ? [NSString stringWithFormat:@"%@ in %@", fmt((int64_t)d.received_bytes), summary]
        : [NSString stringWithFormat:@"Complete — %@", fmt((int64_t)d.received_bytes)];
    } else if (d.total_bytes > 0) {
      NSString* speed = (elapsed > 0.5 && d.received_bytes > 0)
        ? [NSString stringWithFormat:@" · %@/s", fmt((int64_t)(d.received_bytes / elapsed))]
        : @"";
      status = [NSString stringWithFormat:@"%@ / %@%@",
                fmt((int64_t)d.received_bytes), fmt((int64_t)d.total_bytes), speed];
    } else if (d.received_bytes > 0) {
      NSString* speed = (elapsed > 0.5)
        ? [NSString stringWithFormat:@" · %@/s", fmt((int64_t)(d.received_bytes / elapsed))]
        : @"";
      status = [NSString stringWithFormat:@"%@%@", fmt((int64_t)d.received_bytes), speed];
    } else {
      status = @"Waiting…";
    }
    NSTextField* statusLabel = [NSTextField labelWithString:status];
    statusLabel.frame = NSMakeRect(x, 14, textW, 12);
    statusLabel.font = [NSFont fontWithName:@"Inter" size:9] ?: [NSFont systemFontOfSize:9];
    statusLabel.textColor = secColor;
    [_downloadItemsView addSubview:statusLabel];

    // Progress track
    NSView* track = [[NSView alloc] initWithFrame:NSMakeRect(x, 8, textW, 3)];
    track.wantsLayer = YES;
    track.layer.backgroundColor =
      [NSColor colorWithSRGBRed:0.776 green:0.733 blue:0.678 alpha:0.5].CGColor;
    track.layer.cornerRadius = 1.5;
    [_downloadItemsView addSubview:track];

    // Progress fill
    double pct = (d.total_bytes > 0)
      ? (double)d.received_bytes / d.total_bytes : (isComplete ? 1.0 : 0.0);
    CGFloat fillW = textW * (CGFloat)pct;
    if (fillW > 0) {
      NSView* fill = [[NSView alloc] initWithFrame:NSMakeRect(x, 8, fillW, 3)];
      fill.wantsLayer = YES;
      fill.layer.backgroundColor = isComplete
        ? [NSColor colorWithSRGBRed:0.176 green:0.314 blue:0.086 alpha:1].CGColor  // green
        : [NSColor colorWithSRGBRed:0.831 green:0.510 blue:0.039 alpha:1].CGColor; // amber
      fill.layer.cornerRadius = 1.5;
      [_downloadItemsView addSubview:fill];
    }

    // "Show in Finder" button — only when file exists
    BOOL fileExists = filePath &&
        [[NSFileManager defaultManager] fileExistsAtPath:filePath];
    if (fileExists) {
      NSButton* finderBtn = [NSButton buttonWithTitle:@""
                                              target:self
                                              action:@selector(showInFinderClicked:)];
      finderBtn.image = [NSImage imageWithSystemSymbolName:@"folder"
                                 accessibilityDescription:@"Show in Finder"];
      finderBtn.bezelStyle = NSBezelStyleInline;
      finderBtn.bordered = NO;
      finderBtn.frame = NSMakeRect(x + itemW - 42, 16, 18, 18);
      finderBtn.tag = downloadId;
      finderBtn.contentTintColor = secColor;
      [_downloadItemsView addSubview:finderBtn];
    }

    // "Clear" button
    NSButton* clearBtn = [NSButton buttonWithTitle:@""
                                           target:self
                                           action:@selector(clearDownloadClicked:)];
    clearBtn.image = [NSImage imageWithSystemSymbolName:@"xmark"
                               accessibilityDescription:@"Clear"];
    clearBtn.bezelStyle = NSBezelStyleInline;
    clearBtn.bordered = NO;
    clearBtn.frame = NSMakeRect(x + itemW - 22, 16, 18, 18);
    clearBtn.tag = downloadId;
    clearBtn.contentTintColor = secColor;
    [_downloadItemsView addSubview:clearBtn];

    x += itemW + 12;
  }
}


- (void)showInFinderClicked:(NSButton*)sender {
  NSString* fp = _dlFilePaths[@(sender.tag)];
  if (fp && [[NSFileManager defaultManager] fileExistsAtPath:fp])
    [[NSWorkspace sharedWorkspace] selectFile:fp inFileViewerRootedAtPath:@""];
}

- (void)clearDownloadClicked:(NSButton*)sender {
  drift::browser::DownloadId did{(std::uint64_t)sender.tag};
  std::erase_if(_browserState.downloads,
      [did](const drift::browser::DownloadState& d) { return d.id == did; });
  [_dlFilePaths removeObjectForKey:@(sender.tag)];
  [_dlStartTimes removeObjectForKey:@(sender.tag)];
  [self refreshDownloadBar];
}

// ── Download bridge handlers ───────────────────────────────────────────────

- (void)handleDownloadStarted:(int)downloadId
                     filename:(const char*)filename
                   totalBytes:(int64_t)totalBytes
                     filePath:(const char*)filePath {
  drift::browser::DownloadId did{(std::uint64_t)downloadId};
  auto& downloads = _browserState.downloads;
  auto it = std::find_if(downloads.begin(), downloads.end(),
      [did](const drift::browser::DownloadState& d) { return d.id == did; });
  // Preserve received_bytes if a placeholder was created by an early update event
  std::uint64_t curReceived = (it != downloads.end()) ? it->received_bytes : 0;
  std::string fname = filename ? filename : "download";
  (void)drift::browser::apply_event(_browserState,
      drift::browser::DownloadUpdatedEvent{
          did, std::nullopt, fname, curReceived, (std::uint64_t)totalBytes,
          drift::browser::DownloadStatus::in_progress});
  if (filePath)
    _dlFilePaths[@(downloadId)] = [NSString stringWithUTF8String:filePath];
  if (!_dlStartTimes[@(downloadId)])
    _dlStartTimes[@(downloadId)] = [NSDate date];
  [self refreshDownloadBar];
}

- (void)handleDownloadUpdated:(int)downloadId
                receivedBytes:(int64_t)receivedBytes
                   isComplete:(BOOL)isComplete
                   isCanceled:(BOOL)isCanceled {
  drift::browser::DownloadId did{(std::uint64_t)downloadId};
  auto& downloads = _browserState.downloads;
  auto it = std::find_if(downloads.begin(), downloads.end(),
      [did](const drift::browser::DownloadState& d) { return d.id == did; });
  std::string fname = (it != downloads.end()) ? it->file_name : "";
  std::uint64_t total = (it != downloads.end()) ? it->total_bytes : 0;
  // Guard: received may temporarily exceed an unknown total during race
  if (total > 0 && (std::uint64_t)receivedBytes > total) total = 0;
  auto status = isCanceled ? drift::browser::DownloadStatus::cancelled
              : isComplete  ? drift::browser::DownloadStatus::complete
                            : drift::browser::DownloadStatus::in_progress;
  (void)drift::browser::apply_event(_browserState,
      drift::browser::DownloadUpdatedEvent{
          did, std::nullopt, fname, (std::uint64_t)receivedBytes, total, status});
  if (!_dlStartTimes[@(downloadId)])
    _dlStartTimes[@(downloadId)] = [NSDate date];
  [self refreshDownloadBar];
}

// Shows NSSavePanel then triggers StartDownload with the chosen path stored for OnBeforeDownload.
- (void)showSavePanelForURL:(NSString*)url browserId:(int)browserId {
  NSString* suggested = [[[url lastPathComponent] componentsSeparatedByString:@"?"] firstObject];
  suggested = [suggested stringByRemovingPercentEncoding];
  if (!suggested.length) suggested = @"download";

  NSSavePanel* panel = [NSSavePanel savePanel];
  panel.nameFieldStringValue = suggested;

  __unsafe_unretained DriftBrowserWindowController* ws = self;
  [panel beginSheetModalForWindow:_window completionHandler:^(NSInteger result) {
    if (result != NSModalResponseOK || !panel.URL.path) return;
    // Store chosen path keyed by URL so OnBeforeDownload can pick it up
    if (!gPendingSavePaths) gPendingSavePaths = [[NSMutableDictionary alloc] init];
    gPendingSavePaths[url] = panel.URL.path;
    DriftTabModel* t = [ws tabForBrowserId:browserId];
    if (t && t.engine) t.engine->start_download([url UTF8String]);
  }];
}

// ── CEF bridge handlers (called by C bridge functions below) ─────────────────

- (void)handleAddressChange:(int)browserId url:(const char*)url {
  DriftTabModel* t = [self tabForBrowserId:browserId];
  if (!t) return;
  const drift::browser::TabState* cur = [self tabStateForTabId:t.tabId];
  (void)drift::browser::apply_event(_browserState,
      drift::browser::NavigationStateEvent{
          drift::browser::TabId{t.tabId},
          url ? url : "",
          cur ? cur->title : "",
          cur ? cur->loading : false,
          cur ? cur->can_go_back : false,
          cur ? cur->can_go_forward : false});
  if ([self activeIndexInStrip] >= 0 && _tabs[[self activeIndexInStrip]] == t) {
    [self refreshNavBar];
    [self refreshStarButton];
    if (_urlBarFocusPending) {
      const drift::browser::TabState* ts = [self tabStateForTabId:t.tabId];
      _urlBarFocusPending = NO;
      if (ts && ts->url == "about:blank" && ![_urlField currentEditor])
        [self focusAddressBar];
    }
  }
}

- (void)handleTitleChange:(int)browserId title:(const char*)title {
  DriftTabModel* t = [self tabForBrowserId:browserId];
  if (!t) return;
  NSString* titleStr = [NSString stringWithUTF8String:title ?: ""];
  const drift::browser::TabState* cur = [self tabStateForTabId:t.tabId];
  (void)drift::browser::apply_event(_browserState,
      drift::browser::NavigationStateEvent{
          drift::browser::TabId{t.tabId},
          cur ? cur->url : "",
          title ? title : "",
          cur ? cur->loading : false,
          cur ? cur->can_go_back : false,
          cur ? cur->can_go_forward : false});
  if ([self activeIndexInStrip] >= 0 && _tabs[[self activeIndexInStrip]] == t)
    _window.title = titleStr;
  [self refreshStrip];
  [self recordHistoryEntry:
    (cur && !cur->url.empty() ? [NSString stringWithUTF8String:cur->url.c_str()] : @"")
    title:titleStr];
}

- (void)handleLoadingStateChange:(int)browserId
                         loading:(bool)loading
                      canGoBack:(bool)canGoBack
                    canGoForward:(bool)canGoForward {
  DriftTabModel* t = [self tabForBrowserId:browserId];
  if (!t) return;
  const drift::browser::TabState* cur = [self tabStateForTabId:t.tabId];
  (void)drift::browser::apply_event(_browserState,
      drift::browser::NavigationStateEvent{
          drift::browser::TabId{t.tabId},
          cur ? cur->url : "",
          cur ? cur->title : "",
          loading, canGoBack, canGoForward});
  if (!loading) {
    // Reset progress on load-complete
    const drift::browser::TabState* ts = [self tabStateForTabId:t.tabId];
    if (ts) const_cast<drift::browser::TabState*>(ts)->load_progress = 0.0;
  }
  [self refreshStrip];
  if ([self activeIndexInStrip] >= 0 && _tabs[[self activeIndexInStrip]] == t)
    [self refreshNavButtons];
}

- (void)handleLoadProgressChange:(int)browserId progress:(double)progress {
  DriftTabModel* t = [self tabForBrowserId:browserId];
  if (!t) return;
  (void)drift::browser::apply_event(_browserState,
      drift::browser::LoadProgressEvent{
          drift::browser::TabId{t.tabId}, progress});
  if ([self activeIndexInStrip] >= 0 && _tabs[[self activeIndexInStrip]] == t)
    [self refreshNavButtons];
}

// ── NSWindowDelegate ──────────────────────────────────────────────────────────

- (BOOL)windowShouldClose:(NSWindow*)__unused sender {
  [_window orderOut:nil];   // hide immediately for instant visual feedback
  [NSApp terminate:nil];    // → DriftApplication.terminate: → CefQuitMessageLoop()
  return NO;
}

@end

// ─────────────────────────────────────────────────────────────────────────────
// Singleton window + C++ bridge implementations
// ─────────────────────────────────────────────────────────────────────────────
static DriftBrowserWindowController* gWindow = nil;

void DriftCreateBrowserWindow(const char* initial_url, int width, int height) {
  NSString* url = [NSString stringWithUTF8String:initial_url ?: "https://example.com"];
  gWindow = [[DriftBrowserWindowController alloc] initWithURL:url width:width height:height];
}

void DriftOnBrowserCreated(CefRefPtr<CefBrowser> browser) {
  if (!gWindow) return;
  for (DriftTabModel* t in gWindow.tabs) {
    if (t.browserId != -1) continue;
    t.browserId = browser->GetIdentifier();
    t.engine    = [gWindow adoptEngineForBrowser:browser];
    NSView* v = (__bridge NSView*)t.engine->native_view_handle();
    NSInteger myIdx = (NSInteger)[gWindow.tabs indexOfObject:t];
    if (myIdx == [gWindow activeIndexInStrip]) {
      v.hidden           = NO;
      v.frame            = gWindow.contentContainer.bounds;
      v.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    } else {
      v.hidden = YES;
    }
    [gWindow refreshStrip];
    return;
  }
}

void DriftOnBrowserClosed(int browser_id) {
  if (!gWindow) return;

  // Null engine pointer before erasing the adapter.
  for (DriftTabModel* t in gWindow.tabs)
    if (t.browserId == browser_id) { t.engine = nullptr; break; }
  for (DriftTabModel* t in gWindow.closingTabs)
    if (t.browserId == browser_id) { t.engine = nullptr; break; }

  // Safety: remove from active tabs if not already removed by tabStrip:didCloseTabAtIndex:
  NSInteger idx = [gWindow indexForBrowserId:browser_id];
  if (idx >= 0) [gWindow.tabs removeObjectAtIndex:idx];

  // Remove from closingTabs (CEF's final callback; safe to destroy adapter now).
  NSMutableArray<DriftTabModel*>* closing = gWindow.closingTabs;
  for (NSInteger i = 0; i < (NSInteger)closing.count; i++) {
    if (closing[i].browserId == browser_id) {
      [closing removeObjectAtIndex:i];
      break;
    }
  }

  // Destroy the CefEngineAdapter (releases CefRefPtr).
  [gWindow releaseEngineForBrowserId:browser_id];

  if (gWindow.tabs.count == 0 && gWindow.closingTabs.count == 0) {
    CefQuitMessageLoop();
  } else {
    [gWindow refreshStrip];
  }
}

void DriftOnAddressChange(int browser_id, const char* url) {
  if (!gWindow) return;
  [gWindow handleAddressChange:browser_id url:url];
}

void DriftOnTitleChange(int browser_id, const char* title) {
  if (!gWindow) return;
  [gWindow handleTitleChange:browser_id title:title];
}

void DriftOnFaviconURLChange(int browser_id, const char* first_icon_url) {
  if (!gWindow || !first_icon_url) return;
  DriftTabModel* t = [gWindow tabForBrowserId:browser_id];
  if (t) t.faviconURL = [NSString stringWithUTF8String:first_icon_url];
  // Actual fetch deferred — letter placeholder serves for now.
}

void DriftOnLoadingStateChange(int browser_id, bool loading,
                               bool canGoBack, bool canGoForward) {
  if (!gWindow) return;
  [gWindow handleLoadingStateChange:browser_id
                            loading:loading
                         canGoBack:canGoBack
                       canGoForward:canGoForward];
}

void DriftOnLoadingProgressChange(int browser_id, double progress) {
  if (!gWindow) return;
  [gWindow handleLoadProgressChange:browser_id progress:progress];
}

void DriftOpenURLInNewTab(const char* url) {
  if (!gWindow || !url) return;
  [gWindow openTabWithURL:[NSString stringWithUTF8String:url]];
}

void DriftCopyToClipboard(const char* text) {
  if (!text) return;
  NSPasteboard* pb = [NSPasteboard generalPasteboard];
  [pb clearContents];
  [pb setString:[NSString stringWithUTF8String:text] forType:NSPasteboardTypeString];
}

void DriftOnDownloadStarted(int download_id, const char* filename,
                            int64_t total_bytes, const char* file_path) {
  if (!gWindow) return;
  [gWindow handleDownloadStarted:download_id filename:filename
                      totalBytes:total_bytes filePath:file_path];
}

void DriftSaveLinkAs(int browser_id, const char* url) {
  if (!gWindow || !url) return;
  [gWindow showSavePanelForURL:[NSString stringWithUTF8String:url] browserId:browser_id];
}

std::string DriftConsumePendingSavePath(const std::string& url) {
  if (!gPendingSavePaths) return "";
  NSString* key = [NSString stringWithUTF8String:url.c_str()];
  NSString* path = gPendingSavePaths[key];
  if (!path) return "";
  std::string result = path.UTF8String;
  [gPendingSavePaths removeObjectForKey:key];
  return result;
}

void DriftOnDownloadUpdated(int download_id, int64_t received_bytes,
                            bool is_complete, bool is_canceled) {
  if (!gWindow) return;
  [gWindow handleDownloadUpdated:download_id receivedBytes:received_bytes
                      isComplete:is_complete isCanceled:is_canceled];
}

// ── Extraction bridges ────────────────────────────────────────────────────────

static std::atomic<uint64_t> gNextRequestId{1};

void DriftOnLoadEnd(int browser_id, const char* url) {
  if (!gWindow || !url) return;
  DriftTabModel* t = [gWindow tabForBrowserId:browser_id];
  if (!t || !t.engine) return;
  // Generate a monotonic request_id for this extraction attempt.
  uint64_t rid = gNextRequestId.fetch_add(1, std::memory_order_relaxed);
  t.engine->request_extraction(std::to_string(rid));
}

void DriftOnExtractionCompleted(int browser_id, const char* /*request_id*/,
                                const char* result_json) {
  if (!gWindow || !result_json) return;
  // EP-03 will consume this to show the recognition indicator and object card.
  // For EP-02 we just log it so the pipeline is end-to-end verifiable.
  DriftTabModel* t = [gWindow tabForBrowserId:browser_id];
  if (!t) return;
  NSString* json = [NSString stringWithUTF8String:result_json];
  // Only log when a product was recognized (confidence field present and > 0)
  if ([json containsString:@"\"product\""]) {
    NSLog(@"[Drift] Product recognized on browser %d: %@", browser_id, json);
  }
}
