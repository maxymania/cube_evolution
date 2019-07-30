// renderextras.cpp: misc gl render code and the HUD

#include "cube.h"

void line(int x1, int y1, float z1, int x2, int y2, float z2)
{
    /* Don't need to swap y and z, because gufExtraLine does this. */
    xtraverts += gufExtraLine(x1,y1,z1,x2,y2,z2);
};

void linestyle(float width, int r, int g, int b)
{
    glLineWidth(width);
    gufExtraLineColor(r,g,b);
};

void box(block &b, float z1, float z2, float z3, float z4)
{
    gufExtraBoxBegin();
    gufExtraBoxVertex((float)b.x,      z1, (float)b.y);
    gufExtraBoxVertex((float)b.x+b.xs, z2, (float)b.y);
    gufExtraBoxVertex((float)b.x+b.xs, z3, (float)b.y+b.ys);
    gufExtraBoxVertex((float)b.x,      z4, (float)b.y+b.ys);
    gufExtraBoxEnd();
    xtraverts += 4;
};

void dot(int x, int y, float z)
{
    const float DOF = 0.1f;
    gufExtraDot(x,z,y,DOF);
    xtraverts += 4;
};

void blendbox(int x1, int y1, int x2, int y2, bool border)
{
    glDepthMask(GL_FALSE);
    gufSetEnabled(GUF_TEXTURE_2D,false);
    glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

    if(border) gufExtraBlendboxFill(x1,y1,x2,y2,0.5, 0.3, 0.4);
    else gufExtraBlendboxFill(x1,y1,x2,y2,1.0, 1.0, 1.0);
    
    glDisable(GL_BLEND);
    gufExtraBlendboxSurround(x1,y1,x2,y2,0.2, 0.7, 0.4);
    
    xtraverts += 8;
    glEnable(GL_BLEND);
    gufSetEnabled(GUF_TEXTURE_2D,true);
    glDepthMask(GL_TRUE);
};

const int MAXSPHERES = 50;
struct sphere { vec o; float size, max; int type; sphere *next; };
sphere spheres[MAXSPHERES], *slist = NULL, *sempty = NULL;
bool sinit = false;

void newsphere(vec &o, float max, int type)
{
    if(!sinit)
    {
        loopi(MAXSPHERES)
        {
            spheres[i].next = sempty;
            sempty = &spheres[i];
        };
        sinit = true;
    };
    if(sempty)
    {
        sphere *p = sempty;
        sempty = p->next;
        p->o = o;
        p->max = max;
        p->size = 1;
        p->type = type;
        p->next = slist;
        slist = p;
    };
};

void renderspheres(int time)
{
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBindTexture(GL_TEXTURE_2D, 4);  

    for(sphere *p, **pp = &slist; p = *pp;)
    {
        gufPushMatrix();
        float size = p->size/p->max;
        gufGeometrySetColor(1.0f, 1.0f, 1.0f, 1.0f-size);
        gufTranslatef(p->o.x, p->o.z, p->o.y);
        gufRotatef(lastmillis/5.0f, 1, 1, 1);
        gufScalef(p->size, p->size, p->size);
        gufGeometryCallList(1);
        gufScalef(0.8f, 0.8f, 0.8f);
        gufGeometryCallList(1);
        gufPopMatrix();
        xtraverts += 12*6*2;

        if(p->size>p->max)
        {
            *pp = p->next;
            p->next = sempty;
            sempty = p;
        }
        else
        {
            p->size += time/100.0f;   
            pp = &p->next;
        };
    };

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
};

string closeent;
const char *entnames[] =
{
    "none?", "light", "playerstart",
    "shells", "bullets", "rockets", "riflerounds",
    "health", "healthboost", "greenarmour", "yellowarmour", "quaddamage", 
    "teleport", "teledest", 
    "mapmodel", "monster", "trigger", "jumppad",
    "?", "?", "?", "?", "?", 
};

void renderents()       // show sparkly thingies for map entities in edit mode
{
    closeent[0] = 0;
    if(!editmode) return;
    loopv(ents)
    {
        entity &e = ents[i];
        if(e.type==NOTUSED) continue;
        vec v = { e.x, e.y, e.z };
        particle_splash(2, 2, 40, v);
    };
    int e = closestent();
    if(e>=0)
    {
        entity &c = ents[e];
        sprintf_s(closeent)("closest entity = %s (%d, %d, %d, %d), selection = (%d, %d)", entnames[c.type], c.attr1, c.attr2, c.attr3, c.attr4, getvar("selxs"), getvar("selys"));
    };
};

void loadsky(char *basename)
{
    static string lastsky = "";
    if(strcmp(lastsky, basename)==0) return;
    const char *side[] = { "ft", "bk", "lf", "rt", "dn", "up" };
    int texnum = 14;
    loopi(6)
    {
        sprintf_sd(name)("packages/%s_%s.jpg", basename, side[i]);
        int xs, ys;
        if(!installtex(texnum+i, path(name), xs, ys, true)) conoutf("could not load sky textures");
    };
    strcpy_s(lastsky, basename);
};

COMMAND(loadsky, ARG_1STR);

float cursordepth = 0.9f;
GLint viewport[4];
double mm[16], pm[16];
vec worldpos;

void readmatrices()
{
    glGetIntegerv(GL_VIEWPORT, viewport);
    gufDumpTypeMatrixd(GUF_MODELVIEW, mm);
    gufDumpTypeMatrixd(GUF_PROJECTION, pm);
};


// find out the 3d target of the crosshair in the world easily and very acurately.
// sadly many very old cards and drivers appear to fuck up on glReadPixels() and give false
// coordinates, making shooting and such impossible.
// also hits map entities which is unwanted.
// could be replaced by a more acurate version of monster.cpp los() if needed

void readdepth(int w, int h)
{
    gufReadDepthBuffer(w/2,h/2,cursordepth);
    //cursordepth = 0.99f;
    double worldx = 0, worldy = 0, worldz = 0;
    gufUnProject(w/2, h/2, cursordepth, mm, pm, viewport, &worldx, &worldz, &worldy);
    worldpos.x = (float)worldx;
    worldpos.y = (float)worldy;
    worldpos.z = (float)worldz;
    vec r = { (float)mm[0], (float)mm[4], (float)mm[8] };
    vec u = { (float)mm[1], (float)mm[5], (float)mm[9] };
    setorient(r, u);
};

void drawicon(float tx, float ty, int x, int y)
{
    glBindTexture(GL_TEXTURE_2D, 5);
    gufExtraIconBegin();
    tx /= 192;
    ty /= 192;
    float o = 1/3.0f;
    int s = 120;
    gufExtraIconVertex(tx  , ty  , x  , y  );
    gufExtraIconVertex(tx+o, ty  , x+s, y  );
    gufExtraIconVertex(tx+o, ty+o, x+s, y+s);
    gufExtraIconVertex(tx  , ty+o, x  , y+s);
    gufExtraIconEnd();
    xtraverts += 4;
};

void invertperspective()
{
    // This only generates a valid inverse matrix for matrices generated by gluPerspective()
    double inv[16];
    memset(inv, 0, sizeof(inv));

    inv[0*4+0] = 1.0/pm[0*4+0];
    inv[1*4+1] = 1.0/pm[1*4+1];
    inv[2*4+3] = 1.0/pm[3*4+2];
    inv[3*4+2] = -1.0;
    inv[3*4+3] = pm[2*4+2]/pm[3*4+2];

    gufLoadMatrixd(inv);
};

VARP(crosshairsize, 0, 15, 50);

int dblend = 0;
void damageblend(int n) { dblend += n; };

VAR(hidestats, 0, 0, 1);
VARP(crosshairfx, 0, 1, 1);

void gl_drawhud(int w, int h, int curfps, int nquads, int curvert, bool underwater)
{
    readmatrices();
    if(editmode)
    {
        if(cursordepth==1.0f) worldpos = player1->o;
	gufSetEnabled(GUF_WIREFRAME, true);
        cursorupdate();
	gufSetEnabled(GUF_WIREFRAME, false);
    };

    glDisable(GL_DEPTH_TEST);
    invertperspective();
    gufPushMatrix();
    gufOrtho(0, VIRTW, VIRTH, 0, -1, 1);
    glEnable(GL_BLEND);

    glDepthMask(GL_FALSE);

    if(dblend || underwater)
    {
        glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
	if(dblend) gufExtraBlendScreen(VIRTW,VIRTH,0.0f, 0.9f, 0.9f);
        else gufExtraBlendScreen(VIRTW,VIRTH,0.9f, 0.5f, 0.0f);
	
        dblend -= curtime/3;
        if(dblend<0) dblend = 0;
    };

    gufSetEnabled(GUF_TEXTURE_2D,true);

    char *command = getcurcommand();
    char *player = playerincrosshair();
    if(command) draw_textf(charp"> %s_", 20, 1570, 2, command);
    else if(closeent[0]) draw_text(closeent, 20, 1570, 2);
    else if(player) draw_text(player, 20, 1570, 2);

    renderscores();
    if(!rendermenu())
    {
        glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);
        glBindTexture(GL_TEXTURE_2D, 1);
        gufExtraIconBegin();
	gufExtraIconSetRGB(255,255,255);
        if(crosshairfx)
        {
            if(player1->gunwait) gufExtraIconSetRGB(128,128,128);
            else if(player1->health<=25) gufExtraIconSetRGB(255,0,0);
            else if(player1->health<=50) gufExtraIconSetRGB(255,128,0);
        }
        float chsize = (float)crosshairsize;
        gufExtraIconVertex(0.0, 0.0, VIRTW/2 - chsize, VIRTH/2 - chsize);
        gufExtraIconVertex(1.0, 0.0, VIRTW/2 + chsize, VIRTH/2 - chsize);
        gufExtraIconVertex(1.0, 1.0, VIRTW/2 + chsize, VIRTH/2 + chsize);
        gufExtraIconVertex(0.0, 1.0, VIRTW/2 - chsize, VIRTH/2 + chsize);
        gufExtraIconEnd();
    };

    gufPopMatrix();

    gufPushMatrix();
    gufOrtho(0, VIRTW*4/3, VIRTH*4/3, 0, -1, 1);
    renderconsole();

    if(!hidestats)
    {
        gufPopMatrix();
        gufPushMatrix();
        gufOrtho(0, VIRTW*3/2, VIRTH*3/2, 0, -1, 1);
        draw_textf(charp"fps %d", 3200, 2390, 2, curfps);
        draw_textf(charp"wqd %d", 3200, 2460, 2, nquads); 
        draw_textf(charp"wvt %d", 3200, 2530, 2, curvert);
        draw_textf(charp"evt %d", 3200, 2600, 2, xtraverts);
    };
    
    gufPopMatrix();

    if(player1->state==CS_ALIVE)
    {
        gufPushMatrix();
        gufOrtho(0, VIRTW/2, VIRTH/2, 0, -1, 1);
        draw_textf(charp"%d",  90, 827, 2, player1->health);
        if(player1->armour) draw_textf(charp"%d", 390, 827, 2, player1->armour);
        draw_textf(charp"%d", 690, 827, 2, player1->ammo[player1->gunselect]);
        gufPopMatrix();
        gufPushMatrix();
        gufOrtho(0, VIRTW, VIRTH, 0, -1, 1);
        glDisable(GL_BLEND);
        drawicon(128, 128, 20, 1650);
        if(player1->armour) drawicon((float)(player1->armourtype*64), 0, 620, 1650); 
        int g = player1->gunselect;
        int r = 64;
        if(g>2) { g -= 3; r = 128; };
        drawicon((float)(g*64), (float)r, 1220, 1650);
        gufPopMatrix();
    };

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    gufSetEnabled(GUF_TEXTURE_2D,false);
    glEnable(GL_DEPTH_TEST);
};

