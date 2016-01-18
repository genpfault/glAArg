//
//  glAArg Demo 1.0.1
//  glAArg version 0.2.2
//
//  Created by arekkusu on Wed Jul 07 2004.
//  Copyright (c) 2004 arekkusu. All rights reserved.
//
// This is a small cross-platform GLUT demo app demonstrating glAArg usage.
// It has a few different modes, which you can switch through with the space bar or context menu:
//	* Test Pattern (drawn as aliased GL, smooth-fast GL, smooth-nice GL, or glAArg texture AA)
//	* Mipmaps and Lighting (inspect glAArg texture mip levels with falloff, and point lighting)
//	* Vectrogames (the old glAArg demo showing an animated logo with "glowing" vectors)
//	* Performance (a simple point and line stress test)
//
// Version history:
//	1.0.1 Mon Jul 19 2004
//		* Reworked benchmark to more accurately time, scale with machine speeds, and compare rendering paths.
//	1.0	  Tue Jul 13 2004
//		* Initial release for glAArg 0.2, incorporating the old Vectrogames demo and portions of the Cocoa AntiAliasing app.
//


#if defined(__APPLE__)
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif
#include "glAArg/AAPrimitives.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif
#include <math.h>

// performance test constants
#define BATCH 1024
#define TARGET_FPS 2
#define NUM_TESTS 4


// global function pointers for switching GL modes
static void (APIENTRY *myBegin) (GLenum mode);
static void (APIENTRY *myEnd) (void);
static void (APIENTRY *myEnable) (GLenum cap);
static void (APIENTRY *myDisable) (GLenum cap);
static void (APIENTRY *myPointSize) (GLfloat size);
static void (APIENTRY *myLineWidth) (GLfloat width);
static void (APIENTRY *myLineStipple) (GLint factor, GLushort pattern);
static void (APIENTRY *myTranslatef) (GLfloat x, GLfloat y, GLfloat z);
static void (APIENTRY *myVertex2f) (GLfloat x, GLfloat y);
static void (APIENTRY *myColor3f) (GLfloat red, GLfloat green, GLfloat blue);
static void (APIENTRY *myColor4f) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
static void (APIENTRY *myFlush) (void);

// global vars
static int viewmode = 0;										// enum [1-7], matches GLUT menu
static int winw = 480, winh = 350;								// default window ortho size
static int zoomx = 20, zoomy = 16;								// default zoom rectangle origin
static int dragx, dragy, dragging;								// zoom rect dragging control
static int t = 0;												// tick counter for animations
static char renderer[256];										// renderer string temp
static bool hasVAR = 0;											// renderer capabilities


// rand() is not fast, or random, or the same across systems. So here is RANROT-B PRNG. 
static int m_lo = 0, m_hi = ~0;
static inline void sdrand(int seed) { m_lo = seed; m_hi = ~seed; }
static inline unsigned int uirand() { m_hi = (m_hi<<16) + (m_hi>>16); m_hi += m_lo; m_lo += m_hi; return m_hi & INT_MAX; }
static inline          int  irand() { m_hi = (m_hi<<16) + (m_hi>>16); m_hi += m_lo; m_lo += m_hi; return m_hi; }
static inline float ufrand(float x) { return ((x * uirand()) / (float)INT_MAX); }
static inline float  frand(float x) { return ((x *  irand()) / (float)INT_MAX); }


// draw shadowed bitmap string
void drawstr(int x, int y, char *string) {
	int i;
	glColor3f(0.0f, 0.0f, 0.0f);
	glRasterPos2f(x+1, y-1);
	for (i = 0; string[i]; i++) {
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, string[i]);
	}
	glColor3f(1.0f, 1.0f, 1.0f);
	glRasterPos2f(x, y);
	for (i = 0; string[i]; i++) {
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, string[i]);
	}
}


// display callback, cased for each mode
void display(void) {
	int i;

	glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (viewmode < 5) {											// *** TEST PATTERN MODE ****
		float w = 480/2.0f;
		float h = 350/2.0f;

		myTranslatef(0.5, 0.5, 0);								// align for points/lines
		myLineWidth(1);
		myColor4f(1.0, 1.0, 1.0, 0.2);
		for (i=180; i>0; i--) {									// radial line test
			float n = 2*M_PI*i/180.0f;
			if (i < 90) {
				myEnable(GL_LINE_STIPPLE);
				myLineStipple(i, 0x5555);
			}
			myBegin(GL_LINES);
				myVertex2f(w+MIN(w,h)*sin(n), h+MIN(w,h)*cos(n));
				myVertex2f(w, h);
			myEnd();
		}
		myDisable(GL_LINE_STIPPLE);
		
		for (i=1; i<= 20; i++){
			myColor3f(1.0, 1.0, 1.0);
			myPointSize(i);
			myBegin(GL_POINTS);									// integral point sizes 1..20
				myVertex2f(20+i*(i+1), 20);
			myEnd();

			myPointSize(i/10.0);
			myBegin(GL_POINTS);									// fractional point sizes 0..2
				myVertex2f(18+i*4, 33);
			myEnd();
			myPointSize(1);
			myBegin(GL_POINTS);									// fractional point positioning
				myVertex2f(18+i*4+(i-1)/10.0, 27+(i-1)/10.0);
			myEnd();
			myLineWidth(i);
			myBegin(GL_LINES);									// integral line widths 1..20
				myVertex2f(20+i*(i+1), 40.5);
				myColor3f(i%2, (i%3)*0.5, (i%5)*0.25);
				myVertex2f(20+i*(i+1)+(i-1)*4, 100.5);
			myEnd();
			myLineWidth(1.0);
			myBegin(GL_LINES);
				myColor3f(1.0, 0.0, 0.0);
				myVertex2f(17.5+i*4, 107);						// fractional line lengths H (red/blue)
				myColor3f(0.0, 0.0, 1.0);
				myVertex2f(17.5+i*4+i/6.66666667, 107);
				myColor3f(1.0, 0.0, 0.0);
				myVertex2f(18+i*4, 112.5);						// fractional line lengths V (red/blue)
				myColor3f(0.0, 0.0, 1.0);
				myVertex2f(18+i*4, 112.5+i/6.66666667);
				myColor3f(1.0, 0.0, 0.0);
				myVertex2f(21.5, 120+(i-1)*3.1);				// fractional line positioning (red)
				myColor3f(1.0, 1.0, 1.0);
				myVertex2f(52.5, 120+(i-1)*3.1);
			myEnd();
			myLineWidth(2.0-(i-1)/10.0);
			myBegin(GL_LINES);									// fractional line width 2..0 (green)
				myColor3f(0.0, 1.0, 0.0);
				myVertex2f(52.5, 118+i*3);
				myColor3f(1.0, 1.0, 1.0);
				myVertex2f(83.5,118+i*3);
			myEnd();
			myEnable(GL_LINE_STIPPLE);
			myLineStipple(2, 0x96EF);
			myBegin(GL_LINES);									// stippled fractional width 2..0 (blue)
				myColor3f(0.0, 0.0, 1.0);
				myVertex2f(83.5, 119+i*3);
				myColor3f(1.0, 1.0, 1.0);
				myVertex2f(114.5, 119+i*3);
			myEnd();
			myDisable(GL_LINE_STIPPLE);
			if (i<=10){
				myLineWidth(i);									// integral line width, horz aligned (mipmap test)
				myBegin(GL_LINES);
					myVertex2f(125.5, 119.5+(i+2)*(i/2.0));
					myVertex2f(135.5, 119.5+(i+2)*(i/2.0));					
				myEnd();
			}
			myLineWidth(i/10.0);
			myBegin(GL_LINES);									// fractional line width 0..2, 1 px H
				myVertex2f(17.5+i*4, 192);
				myVertex2f(18.5+i*4, 192);
			myEnd();
			myLineWidth(1);
			myBegin(GL_LINES);									// fractional line positioning, 1 px H
				myVertex2f(17.5+i*4+(i-1)/10.0, 186);
				myVertex2f(18.5+i*4+(i-1)/10.0, 186);
			myEnd();
		}
		myFlush();												// purge any pending VAR stuff before changing matrix
	
		// switch draw env
		glPushAttrib(GL_ENABLE_BIT);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, winw, 0, winh);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);

		// draw zoom rectangle
		zoomx = CLAMP(zoomx, 0, winw-40);
		zoomy = CLAMP(zoomy, 0, winh-40);
		glColor3f(1.0, 1.0, 1.0);
		glRasterPos2f(0, 0);
		glPixelZoom(4, 4);										// this is slow, but easier than maintaining a texture.
		glCopyPixels(zoomx+1, winh-39-zoomy, 38, 38, GL_COLOR);
		glPixelZoom(1, 1);
		
		// draw zoom outline
		glEnable(GL_BLEND);
		glEnable(GL_LINE_STIPPLE);
		glLineWidth(1);
		glLineStipple(2, 0xAAAA);
		glColor4f(0.0, 0.0, 0.0, 0.6);
		glBegin(GL_LINE_LOOP);
			glVertex2f(zoomx,	 winh-zoomy-1);
			glVertex2f(zoomx+39, winh-zoomy-1);
			glVertex2f(zoomx+39, winh-zoomy-40);
			glVertex2f(zoomx,	 winh-zoomy-40);
		glEnd();
		glLineStipple(2, 0x5555);
		glColor4f(1.0, 1.0, 1.0, 0.6);		
		glBegin(GL_LINE_LOOP);
			glVertex2f(zoomx,	 winh-zoomy-1);
			glVertex2f(zoomx+39, winh-zoomy-1);
			glVertex2f(zoomx+39, winh-zoomy-40);
			glVertex2f(zoomx,	 winh-zoomy-40);
		glEnd();
		glDisable(GL_LINE_STIPPLE);
		glDisable(GL_BLEND);

		// draw label text
		switch (viewmode) {
			case 1: drawstr(3, 3, "Aliased"); break;
			case 2: drawstr(3, 3, "HWAA (Fastest)"); break;
			case 3: drawstr(3, 3, "HWAA (Nicest)"); break;
			case 4: drawstr(3, 3, "Texture AA"); break;
		}
		drawstr(155, 3, renderer);

		// restore draw env
		glPopMatrix();
		glPopAttrib();

		glutSwapBuffers();
	}

	else if (viewmode == 5) {									// *** MIPMAP AND LIGHTING MODE ****
		static float falloff = 0.0;
		static float anim = 0.01;
		static GLCoord2 light = {0, 0};
		static float xpos = 0, ypos = 0;
		static float xvel = 0.0009, yvel = 0.0007;

		// fix draw env
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, winw, 0, winh);
		glMatrixMode(GL_MODELVIEW);

		// update mipmaps 
		glAAGenerateAATex(falloff, 0, 0.5f);					// 0.5 uses nearest mipmap filtering
		falloff += anim; if ((falloff > 1.0) || (falloff < -1.0)) anim *= -1;

		// draw mipmaps for inspection, from 2x full size to 1 px	
		float mip = phf*4, row = 0;
		float tl = mip*0.5;
		float br = mip;

		glColor4f(1.0, 1.0, 1.0, 1.0);
		glTranslatef(0, 16, 0);									// position above text label
		glBegin(GL_QUADS);
		while (mip >= 1) {
			glTexCoord2f(tl, tl);
			glVertex2f  (0, row);
			glTexCoord2f(br, tl);
			glVertex2f  (0, row+mip);
			glTexCoord2f(br, br);
			glVertex2f  (mip, row+mip);
			glTexCoord2f(tl, br);
			glVertex2f  (mip, row);		
			row += mip;
			mip *= 0.5;
		}
		glEnd();
		
		// draw falloff curve
		extern float ifun(float, float, float);					// get at glAArg internal function
		glTranslatef(0.5, 0.5, 0);								// align for lines
		glAALineWidth(3);
		glAAColor4f(1.0, 0.2, 0.2, 1.0);
		glAABegin(GL_LINE_STRIP);
		for (i = 0; i < phf; i++){
			glAAVertex2f(tl+i*2, ifun((float)i, 0.0f, falloff)*tl);
		}
		glAAEnd();
		glAAFlush();											// flush lines before resetting modelview
		
		// draw light point and lit point
		glLoadIdentity();
		light.x = cos(xpos*M_PI*2);
		light.y = sin(ypos*M_PI*2);
		glAAPointSize(tl);
		glAAPointLight(light);
		glAAColor4f(0.3, 0.5, 1.0, 1.0);
		glAABegin(GL_POINTS);
			glAAVertex2f(240, 175);
		glAAEnd();
		glAAPointSize(phf);
		glAAPointLight((GLCoord2){0.0, 0.0});
		glAAColor4f(1.0, 1.0, 1.0, 1.0);
		glAABegin(GL_POINTS);
			glAAVertex2f((light.x+1)*240, (light.y+1)*175);
		glAAEnd();
		glAAFlush();
		xpos += xvel; if (xpos > 1.0) xpos -= -2.0;
		ypos += yvel; if (ypos > 1.0) ypos -= -2.0;
		
		// draw labels
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		unsigned char label[] = "Falloff: -0.00";
		sprintf(&label[9], "%+0.2f", falloff);
		drawstr(3, 3, label);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);

		// restore draw env
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glutSwapBuffers();
		glutPostRedisplay();
	}
	
	else if (viewmode == 6) {									// *** VECTROGAMES MODE ****
		const char logo[] = {
				1, 1, 5, 11, -2, 8, 17, 8, 17, -2, 11, 11, 15, 1, -18,											// V
			-2, 15, 1, 3, 1, 1, 3, 1, 15, 3, 17, 15, 17, -2, 8, 9, 8, 9, -18,									// E
			-2, 15, 1, 3, 1, 1, 3, -2, 1, 9, 1, 9, -2, 1, 15, 3, 17, 15, 17, -18,								// C
			-2, 1, 1, 6, 1, 8, 3, 8, 17, -2, 15, 1, 15, 1, -18,													// T
			-2, 1, 17, 1, 3, 3, 1, 13, 1, 15, 3, 15, 7, 13, 9, 8, 9, -2, 15, 17, 15, 17, -18,					// R
			-2, 13, 1, 3, 1, 1, 3, 1, 15, 3, 17, 13, 17, 15, 15, 15, 3, 13, 1, -2, 8, 9, 8, 9, -18,				// O
			-2, 15, 1, 3, 1, 1, 3, -2, 1, 9, 1, 9, -2, 1, 15, 3, 17, 13, 17, 15, 15, 15, 11, 13, 9, 8, 9, -18,  // G
			-2, 1, 17, 8, 1, 15, 17, -2, 8, 17, 8, 17, -2, -18,													// A
			-2, 15, 17, 15, 3, 13, 1, 3, 1, 1, 3, 1, 17, -2, 8, 9, 8, 9, -18,									// M
			-2, 15, 1, 3, 1, 1, 3, 1, 15, 3, 17, 15, 17, -2, 8, 9, 8, 9, -18,									// E
			-2, 15, 1, 3, 1, 1, 3, -2, 8, 9, 8, 9, -2, 1, 17, 13, 17, 15, 15, 0									// S
		};
				
		const float sc = 1.875f;								// logo scale factor
		const float sx = 2.25f;									// logo tracking (space between letters) factor
		
		int pre1 = 0, pre2 = 0, last = 0;						// animation vars etc
		float tx = 0, tx1 = 0, tx2 = 0;
		float s = CLAMP(t-200, 10, 70) * 0.75;
		int fade = 256-CLAMP(t-1000, 0, 256);
		
		glTranslatef(20.5, 150.5, 0);							// center logo and align for lines
		
		glAALineWidth(s);
		glBindTexture(GL_TEXTURE_2D, glAA_texture[0]);

		glAABegin(GL_LINE_STRIP);								// simple parse through the logo table
			for (i = 0; i < t; i++) {
				float x = logo[i];
				if (x == 0) break;
				else if (x == -2) { glAAEnd(); glAABegin(GL_LINE_STRIP); }
				else if (x < 0)   { tx += x*-sx; }
				else {				int color = lerpRGBA((irand()&0xFFFFFF00)|0x000000C0, 0xFFFFFFFF, 1.0f-CLAMP(t-200, 0, 200)/450.0);
									glAAColor1ui((color&0xFFFFFF00) | ((color&0x000000FF)*fade)>>8);
									pre1 = pre2; tx1 = tx2; pre2 = last; tx2 = tx; last = i;
									glAAVertex2f(tx + x*sc, logo[++i]*sc);
				}
				if ((x < 0) && (i == t)) t++;
			}

			if (t<200) {										// draw tracer over last three points
			glAAEnd();
			glAABegin(GL_LINE_STRIP);
				glAAColor1ui((irand()&0xFFFFFF00)|0x00000044);
				glAAVertex2f(tx1+ logo[pre1]*sc, logo[pre1+1]*sc);
				glAAColor1ui((irand()&0xFFFFFF00)|0x000000CC);
				glAAVertex2f(tx2+ logo[pre2]*sc, logo[pre2+1]*sc);
				glAAColor1ui((irand()&0xFFFFFF00)|0x000000FF);
				glAAVertex2f(tx + logo[last]*sc, logo[last+1]*sc);
			}
		glAAEnd();
		glAAFlush();
		
		t+=2;
		if (t>200){
			t+=2;												// animate faster after draw
			if (t<275) t+=5;									// and even faster during bloom out
		}
		
		if ((t > 200) && (t < 500))								// animate bloom
			glAAGenerateAATex((t-200)/-300.0f, 0, 0.5f);

		if (t > 1500) {											// reset texture and start over
			t = 0;
			glAAGenerateAATex(0.0f, 0, 0.0f);
		}
		
		glutSwapBuffers();
		glutPostRedisplay();
	}
	
	else {														// *** PERFORMANCE TEST MODE ****
		// this simple performance test is very skewed towards immediate mode dynamic geometry,
		// where the position, color, and size is changing randomly for every primitive.
		// fillrate is also a factor here, speeds will scale with the window size.
						
		static int mode = 0;
		static float tally = 0, time[NUM_TESTS];
		static char test[NUM_TESTS][16];
		static char label[NUM_TESTS/2][16] = { "IM", "VARi" };
		int count = 0, now = 0;
		
		if (t == 0)		t = glutGet(GLUT_ELAPSED_TIME);
		if (mode < 4)	glTranslatef(0.5f, 0.5f, 0.0f);			// aligned for points, lines
		if (mode < 2)	glAADisable(GLAA_VERTEX_ARRAY);			// force immediate mode
		do {
			switch (mode) {
				case 0:
				case 2: {
					glAABegin(GL_POINTS);
					for (i=0; i<BATCH; i++){
						glAAPointSize(ufrand(20.0f));
						glAAColor1ui(irand()|0x00000080);	glAAVertex2f(ufrand(480), ufrand(350));
					}
					glAAEnd();
					break;
				}
				case 1:
				case 3: {
					glAABegin(GL_LINES);
					for (i=0; i<BATCH; i++){
						glAALineWidth(ufrand(10.0f));		float x = ufrand(480), y = ufrand(350);   
						glAAColor1ui(irand()|0x00000080);	glAAVertex2f(x, y);
						glAAColor1ui(irand());				glAAVertex2f(x+frand(60), y+frand(43.75));
					}
					glAAEnd();
					break;
				}
			}
			count++;
		// roughly limit the number of batches by timing the submission
		} while (glutGet(GLUT_ELAPSED_TIME)-t < 1000/TARGET_FPS);
		
		// draw labels (for the previous frame)
		glAAFlush();
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, winw, 0, winh);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);

		drawstr(3,   winh-12, "Mode");
		drawstr(50,  winh-12, "K points/sec");
		drawstr(160, winh-12, "K lines/sec");
		for (i=0; (i<NUM_TESTS/2) && test[i*2][0]; i++) {
			drawstr(3,   winh-25-i*13, label[i]);
			drawstr(50,  winh-25-i*13,  test[i*2]);
			drawstr(160, winh-25-i*13,  test[i*2+1]);
		}

		glPopMatrix();
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);

		glutSwapBuffers();
		
		// measure elasped time after rendering has fully completed.
		now = glutGet(GLUT_ELAPSED_TIME);
		float secs = (now - t)/1000.0;							// time in seconds
		t = now;

		float speed = (BATCH*count/1000.0)/secs;				// thousands of primitives per second
		sprintf(test[mode], "%.1f", time[mode]?(speed+time[mode])/2:speed);
		time[mode] = speed;
		// GLUT's timer function only has millisecond resolution, so we'll
		// run each mode for at least 5 seconds to allow for some variance.
		tally += secs; if (tally > 5.0) {
			tally = 0;
			mode++;
			if ((mode == 2) && !hasVAR) {						// skip VAR if the extension isn't supported
				sprintf(test[2], "n/a");
				sprintf(test[3], "n/a");
				mode += 2;
			}
			if (mode == NUM_TESTS) mode = 0;
		}

		glAAEnable(GLAA_VERTEX_ARRAY);
		glutPostRedisplay();		
	}
}


// reshape callback
void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluOrtho2D(0, 480, 350, 0);									// origin in TOP left
	winw = width; winh = height;
}


// set function pointers and GL options depending on view mode
void setviewmode(int mode) {
	if (viewmode != mode) {
		viewmode = mode;
		switch (mode) {
			case 1:
			case 2:
			case 3:												// use regular GL calls, with various smoothing options
				myBegin			= glBegin;
				myEnd			= glEnd;
				myEnable		= glEnable;
				myDisable		= glDisable;
				myPointSize		= glPointSize;
				myLineWidth		= glLineWidth;
				myLineStipple   = glLineStipple;
				myTranslatef	= glTranslatef;
				myVertex2f		= glVertex2f;
				myColor3f		= glColor3f;
				myColor4f		= glColor4f;
				myFlush			= glFlush;
				glDisable(GL_TEXTURE_2D);
				switch (mode) {
					case 1:
						glDisable(GL_POINT_SMOOTH);
						glDisable(GL_LINE_SMOOTH);
						glDisable(GL_POLYGON_SMOOTH);
						break;
					case 2:
						glEnable(GL_POINT_SMOOTH);
						glEnable(GL_LINE_SMOOTH);
						glEnable(GL_POLYGON_SMOOTH);
						glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
						glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
						glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
						break;
					case 3:
						glEnable(GL_POINT_SMOOTH);
						glEnable(GL_LINE_SMOOTH);
						glEnable(GL_POLYGON_SMOOTH);
						glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
						glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
						glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
						break;
				}
				break;
			case 4:
			case 5:
			case 6:												
			case 7:												// use glAArg calls
				myBegin			= glAABegin;
				myEnd			= glAAEnd;
				myEnable		= glAAEnable;
				myDisable		= glAADisable;
				myPointSize		= glAAPointSize;
				myLineWidth		= glAALineWidth;
				myTranslatef	= glTranslatef;					// glAArg does not know about the modelview matrix (yet)
				myLineStipple   = glAALineStipple;
				myVertex2f		= glAAVertex2f;
				myColor3f		= glAAColor3f;
				myColor4f		= glAAColor4f;
				myFlush			= glAAFlush;
				glAAGenerateAATex(0.0f, 0, 0.0f);
				glEnable(GL_TEXTURE_2D);
				glDisable(GL_POINT_SMOOTH);
				glDisable(GL_LINE_SMOOTH);
				glDisable(GL_POLYGON_SMOOTH);
				switch (mode) {
					case 6:
					case 7:
						t = 0;
						break;
				}
				break;
		}
		glutPostRedisplay();
	}
}


// keyboard callback
void keyboard(unsigned char key, int winx, int winy){
	switch(key){
		case ' ': {						// rendering mode
			int mode = viewmode + 1; if (mode > 7) mode = 1;
			setviewmode(mode);
			break;
		}
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7': {
			setviewmode(key-'0');
			break;
		}
	}
}


// mouse button callback
void mouse(int button, int state, int winx, int winy) {	
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if ((winx > zoomx) && (winx < zoomx+40) &&
		    (winy > zoomy) && (winy < zoomy+40) && (viewmode < 5)) {
			dragging = 1;
			dragx = winx - zoomx;
			dragy = winy - zoomy;
		}
		else
			dragging = 0;
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
		dragging = 0;
}


// mouse drag callback
void motion(int winx, int winy) {
	if (dragging) {
		zoomx = winx-dragx; zoomy = winy-dragy;
		glutPostRedisplay();
	}
}


// menu selection callback
void menu(int entry) {
	keyboard(entry, 0, 0);
}


// set up inital GL state
void initGL(void) {
	// gl init
	glDisable(GL_LIGHTING);
	glDisable(GL_DITHER);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 1/255.0);
	
	// texture init
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glScalef(0.25/phf, 0.25/phf, 1.0);							// glAArg requires rectangle coordinate setup

	// glAArg init
	glAAInit();
	glAAEnable(GLAA_VERTEX_ARRAY);								// we want VAR acceleration and we will handle flushing
	setviewmode(4);												// this creates one glAArg texture

	// build the renderer info string
	char version[256];
	int i;
	strncpy(renderer, glGetString (GL_RENDERER), 255);
	if (0 == strcmp(renderer, "Generic")) {
		strncpy(renderer, "Apple Generic", 255);
	}
	else if (0 == strcmp(renderer, "GDI Generic")) {
		strncpy(renderer, "Microsoft Generic", 255);
	}
	else if ((strlen(renderer) > 14) && (0 == strcmp(renderer+strlen(renderer)-14, " OpenGL Engine"))) {
		renderer[strlen(renderer)-14] = 0;
	}
	strncat(renderer, " ", 255);
	strncpy(version, glGetString (GL_VERSION), 255);
	for (i = 0; i < 256; i++) {
		unsigned char c[2] = { version[i], 0};
		if (((c[0] >= '0' ) && (c[0] <= '9')) || c[0] == '.') strncat(renderer, c, 255);
		else break;
	}
	
	// check for extensions that affect performance test
	const GLubyte *extensions = glGetString(GL_EXTENSIONS);
	if (extensions) {
		hasVAR = (strstr((char *)extensions, "GL_APPLE_vertex_array_range") != NULL);
	}
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(winw, winh);
    glutCreateWindow("glAArg Demo");
    
	initGL();
	
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	
	glutCreateMenu(menu);
	glutAddMenuEntry("Aliased",				'1');
	glutAddMenuEntry("HWAA (Fastest)",		'2');
	glutAddMenuEntry("HWAA (Nicest)",		'3');
	glutAddMenuEntry("Texture AA",			'4');
	glutAddMenuEntry("Mipmaps and Lighting",'5');
	glutAddMenuEntry("Vectrogames",			'6');
	glutAddMenuEntry("Performance",			'7');
	glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutMainLoop();
    return 0;
}
