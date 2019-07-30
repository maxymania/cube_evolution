// rendergl.cpp: core opengl rendering stuff

#include "cube.h"


#ifdef DARWIN
#define GL_COMBINE_EXT GL_COMBINE_ARB
#define GL_COMBINE_RGB_EXT GL_COMBINE_RGB_ARB
#define GL_SOURCE0_RBG_EXT GL_SOURCE0_RGB_ARB
#define GL_SOURCE1_RBG_EXT GL_SOURCE1_RGB_ARB
#define GL_RGB_SCALE_EXT GL_RGB_SCALE_ARB
#endif

extern int curvert;

bool hasoverbright = false;

void purgetextures();

int glmaxtexsize = 256;

void gl_init(int w, int h)
{
    //#define fogvalues 0.5f, 0.6f, 0.7f, 1.0f

    glViewport(0, 0, w, h);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    
    
    gufSetEnabled(GUF_FOG,true);
    gufFogSetup();
    

    gufSetEnabled(GUF_LINE_SMOOTH,true);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    gufSetEnabled(GUF_POLYGON_OFFSET_LINE,true);
    glPolygonOffset(-3.0, -3.0);

    glCullFace(GL_FRONT);
    glEnable(GL_CULL_FACE);

    hasoverbright = gufOverbrightSupported();
    if(!hasoverbright) conoutf("WARNING: cannot use overbright lighting, using old lighting model!");
        
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glmaxtexsize);
    
    purgetextures();

    gufGeometryNewList(1);
    gufSphere(GUF_INSIDE,1, 12, 12);
    gufGeometryEndList();
};

void cleangl()
{
};

bool installtex(int tnum, char *texname, int &xs, int &ys, bool clamp)
{
    SDL_Surface *s = IMG_Load(texname);
    if(!s) { conoutf("couldn't load texture %s", texname); return false; };
    if(s->format->BitsPerPixel!=24) { conoutf("texture must be 24bpp: %s", texname); return false; };
    // loopi(s->w*s->h*3) { uchar *p = (uchar *)s->pixels+i; *p = 255-*p; };  
    glBindTexture(GL_TEXTURE_2D, tnum);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //NEAREST);
    gufOverbrightTextureEnable();
    bool scaled = false;
    xs = s->w;
    ys = s->h;
    scaled = gufCorrectTextureSize(xs,ys);
    while(xs>glmaxtexsize || ys>glmaxtexsize) { xs /= 2; ys /= 2; scaled = true; };
    void *scaledimg = s->pixels;
    if(scaled)
    {
        conoutf("warning: quality loss: scaling %s", texname);     // for voodoo cards under linux
        scaledimg = alloc(xs*ys*3);
	gufScaleImage(3, s->w, s->h, s->pixels, xs, ys, scaledimg);
    };
    if(gufBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, xs, ys, GL_RGB, GL_UNSIGNED_BYTE, scaledimg)) fatal("could not build mipmaps");
    if(scaled) free(scaledimg);
    SDL_FreeSurface(s);
    return true;
};

// management of texture slots
// each texture slot can have multople texture frames, of which currently only the first is used
// additional frames can be used for various shaders

const int MAXTEX = 1000;
int texx[MAXTEX];                           // ( loaded texture ) -> ( name, size )
int texy[MAXTEX];                           
string texname[MAXTEX];
int curtex = 0;
const int FIRSTTEX = 1000;                  // opengl id = loaded id + FIRSTTEX
// std 1+, sky 14+, mdls 20+

const int MAXFRAMES = 2;                    // increase to allow more complex shader defs
int mapping[256][MAXFRAMES];                // ( cube texture, frame ) -> ( opengl id, name )
string mapname[256][MAXFRAMES];

void purgetextures()
{
    loopi(256) loop(j,MAXFRAMES) mapping[i][j] = 0;
};

int curtexnum = 0;

void texturereset() { curtexnum = 0; };

void texture(char *aframe, char *name)
{
    int num = curtexnum++, frame = atoi(aframe);
    if(num<0 || num>=256 || frame<0 || frame>=MAXFRAMES) return;
    mapping[num][frame] = 1;
    char *n = mapname[num][frame];
    strcpy_s(n, name);
    path(n);
};

COMMAND(texturereset, ARG_NONE);
COMMAND(texture, ARG_2STR);

int lookuptexture(int tex, int &xs, int &ys)
{
    int frame = 0;                      // other frames?
    int tid = mapping[tex][frame];

    if(tid>=FIRSTTEX)
    {
        xs = texx[tid-FIRSTTEX];
        ys = texy[tid-FIRSTTEX];
        return tid;
    };

    xs = ys = 16;
    if(!tid) return 1;                  // crosshair :)

    loopi(curtex)       // lazily happens once per "texture" command, basically
    {
        if(strcmp(mapname[tex][frame], texname[i])==0)
        {
            mapping[tex][frame] = tid = i+FIRSTTEX;
            xs = texx[i];
            ys = texy[i];
            return tid;
        };
    };

    if(curtex==MAXTEX) fatal("loaded too many textures");

    int tnum = curtex+FIRSTTEX;
    strcpy_s(texname[curtex], mapname[tex][frame]);

    sprintf_sd(name)("packages%c%s", PATHDIV, texname[curtex]);

    if(installtex(tnum, name, xs, ys))
    {
        mapping[tex][frame] = tnum;
        texx[curtex] = xs;
        texy[curtex] = ys;
        curtex++;
        return tnum;
    }
    else
    {
        return mapping[tex][frame] = FIRSTTEX;  // temp fix
    };
};

void setupworld()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY); 
    setarraypointers();

    if(hasoverbright) gufOverbrightWorldInit();
};

int skyoglid;

struct strip { int tex, start, num; };
vector<strip> strips;

void renderstripssky()
{
    glBindTexture(GL_TEXTURE_2D, skyoglid);
    loopv(strips) if(strips[i].tex==skyoglid) glDrawArrays(GL_TRIANGLE_STRIP, strips[i].start, strips[i].num);
};

void renderstrips()
{
    int lasttex = -1;
    loopv(strips) if(strips[i].tex!=skyoglid)
    {
        if(strips[i].tex!=lasttex)
        {
            glBindTexture(GL_TEXTURE_2D, strips[i].tex); 
            lasttex = strips[i].tex;
        };
        glDrawArrays(GL_TRIANGLE_STRIP, strips[i].start, strips[i].num);  
    };   
};

void overbright(float amount) { if(hasoverbright) gufOverbright(amount); };

void addstrip(int tex, int start, int n)
{
    strip &s = strips.add();
    s.tex = tex;
    s.start = start;
    s.num = n;
};

VARFP(gamma, 30, 100, 300,
{
    float f = gamma/100.0f;
    // TODO: gamma not supported! Find a way.
    //if(SDL_SetGamma(f,f,f)==-1)
    //{
    //    conoutf("Could not set gamma (card/driver doesn't support it?)");
    //    conoutf("sdl: %s", SDL_GetError());
    //};
});

void transplayer()
{
    gufLoadIdentity();
    
    gufRotatef(player1->roll , 0.0f,0.0f,1.0f);
    gufRotatef(player1->pitch,-1.0f,0.0f,0.0f);
    gufRotatef(player1->yaw  , 0.0f,1.0f,0.0f);

    gufTranslatef(-player1->o.x, (player1->state==CS_DEAD ? player1->eyeheight-0.2f : 0)-player1->o.z, -player1->o.y);   
};

VARP(fov, 10, 105, 120);

int xtraverts;

VAR(fog, 64, 180, 1024);
VAR(fogcolour, 0, 0x8099B3, 0xFFFFFF);

VARP(hudgun,0,1,1);

const char *hudgunnames[] = { "hudguns/fist", "hudguns/shotg", "hudguns/chaing", "hudguns/rocket", "hudguns/rifle" };

void drawhudmodel(int start, int end, float speed, int base)
{
    rendermodel((char*)hudgunnames[player1->gunselect], start, end, 0, 1.0f, player1->o.x, player1->o.z, player1->o.y, player1->yaw+90, player1->pitch, false, 1.0f, speed, 0, base);
};

void drawhudgun(float fovy, float aspect, int farplane)
{
    if(!hudgun /*|| !player1->gunselect*/) return;
    
    glEnable(GL_CULL_FACE);
    
    gufMatrixMode(GUF_PROJECTION);
    gufLoadIdentity();
    gufPerspective(fovy, aspect, 0.3f, farplane);
    gufMatrixMode(GUF_MODELVIEW);
    
    //glClear(GL_DEPTH_BUFFER_BIT);
    int rtime = reloadtime(player1->gunselect);
    if(player1->lastaction && player1->lastattackgun==player1->gunselect && lastmillis-player1->lastaction<rtime)
    {
        drawhudmodel(7, 18, rtime/18.0f, player1->lastaction);
    }
    else
    {
        drawhudmodel(6, 1, 100, 0);
    };

    gufMatrixMode(GUF_PROJECTION);
    gufLoadIdentity();
    gufPerspective(fovy, aspect, 0.15f, farplane);
    gufMatrixMode(GUF_MODELVIEW);

    glDisable(GL_CULL_FACE);
};

void gl_drawframe(int w, int h, float curfps)
{
    float hf = hdr.waterlevel-0.3f;
    float fovy = (float)fov*h/w;
    float aspect = w/(float)h;
    bool underwater = player1->o.z<hf;
    
    
    gufFogStart((fog+64)/8);
    gufFogEnd(fog);
    float fogc[4] = { (fogcolour>>16)/256.0f, ((fogcolour>>8)&255)/256.0f, (fogcolour&255)/256.0f, 1.0f };
    gufFogColor(fogc);
    glClearColor(fogc[0], fogc[1], fogc[2], 1.0f);

    if(underwater)
    {
        fovy += (float)sin(lastmillis/1000.0)*2.0f;
        aspect += (float)sin(lastmillis/1000.0+PI)*0.1f;
	gufFogStart(0);
	gufFogEnd((fog+96)/8);
    };
    
    glClear((player1->outsidemap ? GL_COLOR_BUFFER_BIT : 0) | GL_DEPTH_BUFFER_BIT);

    gufMatrixMode(GUF_PROJECTION);
    gufLoadIdentity();
    int farplane = fog*5/2;
    gufPerspective(fovy, aspect, 0.15f, farplane);
    gufMatrixMode(GUF_MODELVIEW);

    transplayer();

    gufSetEnabled(GUF_TEXTURE_2D,true);
    
    int xs, ys;
    skyoglid = lookuptexture(DEFAULT_SKY, xs, ys);
   
    resetcubes();
            
    curvert = 0;
    strips.setsize(0);
  
    render_world(player1->o.x, player1->o.y, player1->o.z, 
            (int)player1->yaw, (int)player1->pitch, (float)fov, w, h);
    finishstrips();

    setupworld();

    renderstripssky();

    gufLoadIdentity();
    gufRotatef(player1->pitch, -1.0f, 0.0f, 0.0f);
    gufRotatef(player1->yaw ,   0.0f, 1.0f, 0.0f);
    gufRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    gufGeometrySetColor(1.0f, 1.0f, 1.0f);
    gufSetEnabled(GUF_FOG,false);
    glDepthFunc(GL_GREATER);
    draw_envbox(14, fog*4/3);
    glDepthFunc(GL_LESS);
    gufSetEnabled(GUF_FOG,true);

    transplayer();
    
    overbright(2);
    
    renderstrips();

    xtraverts = 0;

    renderclients();
    monsterrender();

    renderentities();

    renderspheres(curtime);
    renderents();

    glDisable(GL_CULL_FACE);

    drawhudgun(fovy, aspect, farplane);

    overbright(1);
    int nquads = renderwater(hf);
    
    overbright(2);
    render_particles(curtime);
    overbright(1);

    gufSetEnabled(GUF_FOG,false);

    gufSetEnabled(GUF_TEXTURE_2D,false);

    gl_drawhud(w, h, (int)curfps, nquads, curvert, underwater);

    glEnable(GL_CULL_FACE);
    gufSetEnabled(GUF_FOG,true);
};

