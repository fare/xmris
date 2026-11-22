#include "xmris.h"
#if defined(__ANDROID__) || defined(SDL_PLATFORM_IOS)
#include "android/input.h"
#endif
#include "defmris.h"
#include "courier18.h"

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
static Pixmap frame, pattern;
static Uint32 foreground, background;
static int dirty;

static Pixmap surface(int w, int h)
{
  Pixmap s = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_XRGB8888);
  if (!s) fatal_error("%s", SDL_GetError());
  return s;
}

static Uint32 *row(Pixmap s, int y)
{
  return (Uint32 *)((Uint8 *)s->pixels + y * s->pitch);
}

void video_copy(Pixmap src, Pixmap dst, GC gc, int x, int y,
                unsigned w, unsigned h, int dx, int dy)
{
  int width = w, height = h, j, i;
  if (x < 0) { dx -= x; width += x; x = 0; }
  if (y < 0) { dy -= y; height += y; y = 0; }
  if (dx < 0) { x -= dx; width += dx; dx = 0; }
  if (dy < 0) { y -= dy; height += dy; dy = 0; }
  width = SDL_min(width, SDL_min(src->w-x, dst->w-dx));
  height = SDL_min(height, SDL_min(src->h-y, dst->h-dy));
  if (width <= 0) return;
  for (j = 0; j < height; ++j) {
    int k = src == dst && dy > y ? height - 1 - j : j;
    Uint32 *s = row(src,y+k)+x, *d = row(dst,dy+k)+dx;
    if (gc == GC_COPY) memmove(d,s,width*sizeof(*d));
    else for (i=0; i<width; ++i) {
      if (gc == GC_OR) d[i] |= s[i];
      else if (gc == GC_AND) d[i] &= s[i];
      else if (gc == GC_MASK) d[i] &= ~s[i];
      else d[i] = s[i];
    }
  }
}

void video_pattern(unsigned long fg, unsigned long bg, Pixmap stipple)
{
  foreground = fg; background = bg; pattern = stipple;
}

void video_fill(Pixmap dst, GC gc, int x, int y, unsigned w, unsigned h)
{
  SDL_Rect rect = {x,y,w,h};
  if (gc == GC_BOARD && pattern) {
    int i,j;
    for (j=SDL_max(y,0); j<SDL_min(dst->h,y+(int)h); ++j)
      for (i=SDL_max(x,0); i<SDL_min(dst->w,x+(int)w); ++i)
        row(dst,j)[i] = row(pattern,j%pattern->h)[i%pattern->w] ? foreground : background;
  } else SDL_FillSurfaceRect(dst,&rect,gc == GC_COPY ? 0xffffff :
      gc == GC_CLEAR ? display.white : gc == GC_BORDER ? display.border : display.black);
}

void video_line(Pixmap dst, GC gc, int x, int y, int dx, int dy)
{
  int w=abs(dx-x), h=-abs(dy-y), err=w+h, sx=x<dx?1:-1, sy=y<dy?1:-1;
  for (;;) {
    int e=2*err;
    video_fill(dst,gc,x,y,1,1);
    if (x==dx && y==dy) break;
    if (e>=h) { err+=h; x+=sx; }
    if (e<=w) { err+=w; y+=sy; }
  }
}

void video_rect(Pixmap dst, GC gc, int x, int y, unsigned w, unsigned h)
{
  video_fill(dst,gc,x,y,w+1,1); video_fill(dst,gc,x,y+h,w+1,1);
  video_fill(dst,gc,x,y,1,h+1); video_fill(dst,gc,x+w,y,1,h+1);
}

void video_text(Pixmap dst, GC gc, int x, int y, const char *str, int n)
{
  Uint32 color = gc == GC_CLEAR ? display.white : display.black;
  for (int i = 0; i < n; ++i) {
    unsigned ch = (unsigned char)str[i];
    if (ch < 32 || (ch >= 127 && ch < 160)) ch = '?';
    for (int gy = 0; gy < courier_ascent + courier_descent; ++gy) {
      int dy = y - font.ascent + gy;
      if (dy < 0 || dy >= dst->h) continue;
      for (int gx = 0; gx < courier_width; ++gx) {
        int dx = x + i * courier_width + gx;
        if (dx >= 0 && dx < dst->w && (courier_rows[ch-32][gy] & (1u << gx)))
          row(dst,dy)[dx] = color;
      }
    }
  }
}

void video_opaque_text(Pixmap dst, GC gc, int x, int y, const char *str, int n)
{
  video_fill(dst,GC_CLEAR,x,y-font.ascent,n*font.width,font.ascent+font.descent);
  video_text(dst,gc,x,y,str,n);
}

void present_area(int x,int y,unsigned w,unsigned h)
{
  video_copy(display.copy,frame,GC_COPY,x,y,w,h,x,y);
  dirty = 1;
}

void present_load(int left,int right)
{
  int y = PIXELY(CELLS_DOWN,CELL_HEIGHT);
  for (int x = SDL_max(0,left); x <= SDL_min(right,frame->w-1); ++x)
    row(frame,y)[x] ^= display.white ^ display.black;
  dirty = 1;
}

void video_sync(void)
{
  if (!dirty) return;
  SDL_UpdateTexture(texture,NULL,frame->pixels,frame->pitch);
  SDL_SetRenderDrawColor(renderer,0,0,0,255);
  SDL_RenderClear(renderer);
  SDL_FRect board_rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
#if defined(__ANDROID__) || defined(SDL_PLATFORM_IOS)
  board_rect = phone_board(renderer);
#endif
  SDL_RenderTexture(renderer,texture,NULL,&board_rect);
#if defined(__ANDROID__) || defined(SDL_PLATFORM_IOS)
  phone_draw(renderer);
#endif
  SDL_RenderPresent(renderer);
  dirty = 0;
}

static unsigned index_at(const SPRITE_DEF *def,int x,int y,int height)
{
  unsigned index=0, p;
  int pitch=(def->size.x+7)/8;
  if (x<0 || x>=(int)def->size.x || y<0 || y>=height) return 0;
  for (p=0;p<def->planes;++p)
    index |= ((def->bitmap[(p*height+y)*pitch+x/8] >> (x%8))&1) << p;
  return index;
}

static void sprite(unsigned n)
{
  SPRITE_DEF *def=&sprites_def[n];
  SPRITE *dst=&sprites[n], *src;
  int x,y,w,h;
  if (dst->image) return;
  if (!def->bitmap) {
    sprite(def->copy);
    src=&sprites[def->copy];
    if (def->flags==REFLECT_ALIAS) { *dst=*src; return; }
    w=def->flags==REFLECT_DIAGONAL ? src->size.y : src->size.x;
    h=def->flags==REFLECT_DIAGONAL ? src->size.x : src->size.y;
  } else {
    w=def->size.x; h=def->size.y/def->planes;
  }
  dst->size=(SIZE){w,h};
  dst->image=surface(w,h); dst->mask=surface(w,h);
  for (y=0;y<h;++y) for (x=0;x<w;++x) {
    Uint32 mask=0, rgb=0;
    if (def->bitmap) {
      unsigned index=index_at(def,x,y,h);
      if (index) {
        unsigned long color=def->colors[0] ? def->colors[0][index] : COLOR_BACKGROUND;
        mask=0xffffff;
        if (color<COLORS) rgb=colors[color].pixel;
      } else if ((def->flags&BORDER_HALO_MASK) &&
          (index_at(def,x-1,y,h)||index_at(def,x+1,y,h)||
           index_at(def,x,y-1,h)||index_at(def,x,y+1,h))) {
        mask=0xffffff; rgb=display.white;
      }
    } else {
      int sx=def->flags==REFLECT_DIAGONAL ? y : def->flags==REFLECT_VERTICAL ? w-1-x : x;
      int sy=def->flags==REFLECT_DIAGONAL ? x : def->flags==REFLECT_HORIZONTAL ? h-1-y : y;
      mask=row(src->mask,sy)[sx]; rgb=row(src->image,sy)[sx];
    }
    row(dst->mask,y)[x]=mask;
    row(dst->image,y)[x]=rgb;
  }
}

SDL_Window *sdl_video_open(int scale, int fullscreen, SDL_RendererLogicalPresentation mode)
{
  int i,x,y;
  display.white=0xffffff; display.background=COLOUR_ONE; display.foreground=COLOUR_ZERO;
  colors[COLOR_WHITE].pixel=display.white;
  for (i=2;i<COLORS;++i)
    if (color_names[i].source[0]) colors[i].pixel=strtoul(color_names[i].source[0]+1,NULL,16);
  for (i=0;i<GCS;++i) gcs[i]=i;
  for (i=0;i<SPRITES;++i) sprite(i);
  SPRITE *digits=&sprites[SPRITE_DIGITS];
  for (y=0;y<(int)digits->size.y;++y) for (x=0;x<(int)digits->size.x;++x)
    row(digits->image,y)[x] |= display.white & ~row(digits->mask,y)[x];
  for (i=0;i<FILLS;++i) {
    SPRITE_DEF *def=&fills_def[i];
    fills[i].size=def->size;
    Pixmap s=fills[i].mask=surface(def->size.x,def->size.y);
    for (y=0;y<s->h;++y) for (x=0;x<s->w;++x)
      row(s,y)[x]=(def->bitmap[y*((s->w+7)/8)+x/8]>>(x%8))&1;
  }
  for (i=0;i<BOARD_SCORES;++i) {
    update.score.list[i].image=surface(DIGIT_WIDTH*4,DIGIT_HEIGHT);
    update.score.list[i].mask=surface(DIGIT_WIDTH*4,DIGIT_HEIGHT);
  }
  font.width = courier_width;
  font.ascent = courier_ascent;
  font.descent = courier_descent;
  font.center = (font.ascent - font.descent) / 2;
  font.digitup = courier_digit_up;
  font.digitdown = courier_digit_down;
  display.copy=surface(WINDOW_WIDTH,WINDOW_HEIGHT);
  display.back=surface(WINDOW_WIDTH,WINDOW_HEIGHT);
  frame=surface(WINDOW_WIDTH,WINDOW_HEIGHT);
  video_fill(display.copy,GC_CLEAR,0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
  window=SDL_CreateWindow("Mr Is",WINDOW_WIDTH*scale,WINDOW_HEIGHT*scale,
      SDL_WINDOW_RESIZABLE | (fullscreen ? SDL_WINDOW_FULLSCREEN : 0));
  if (window) renderer=SDL_CreateRenderer(window,NULL);
  if (renderer) texture=SDL_CreateTexture(renderer,SDL_PIXELFORMAT_XRGB8888,
      SDL_TEXTUREACCESS_STREAMING,WINDOW_WIDTH,WINDOW_HEIGHT);
  if (!texture) fatal_error("%s",SDL_GetError());
  SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_NEAREST);
  int logical_height = WINDOW_HEIGHT;
#if defined(__ANDROID__) || defined(SDL_PLATFORM_IOS)
  mode = SDL_LOGICAL_PRESENTATION_DISABLED;
#endif
  if (!SDL_SetRenderLogicalPresentation(renderer,WINDOW_WIDTH,logical_height,mode))
    fatal_error("%s",SDL_GetError());
  return window;
}
