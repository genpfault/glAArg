# glAArg

OpenGL AntiAliased rendering glue

http://arek.bdmonkeys.net/bugs/invariance/


## Stuff used to make this:

This is source code for drawing 2D antialiased points and lines using OpenGL, via texture antialiasing. Compared to regular `GL_SMOOTH` or FSAA antialiasing, this method produces better quality results, and it works identically across GPUs.

The glAArg implementation is contained in the AA* files. The included main.c builds a small example GLUT application which shows basic usage patterns.

The code is very much a work in progress, so there are missing features and a few bugs which are noted below. The API is also subject to change in future versions.

This software is free as in speech and free as in beer. You can use it for any purpose. No warranty, etc.


## Version History

* 0.2.2	Mon Jul 19 2004
 	* Point optimization for immediate mode (benchmarks ~30% faster)
* 0.2.1	Wed Jul 14 2004
	* Reduce texture footprint and mipmap generation time by 75%. (See usage NOTE below)
	* Use APPLE_client_storage only if the extension is supported (i.e. on Mac)
* 0.2	Tue Jul 13 2004
	* Initial pass at cross-platform compilation support for Mac OS X, Linux, and Windows (all using gcc 3.3)
	* Changed glAAGenerateAATex 'alias' parameter to float, for mipmap filter control.
* 0.1.5	Tue Jun 29 2004
	* Workaround for Radeon 7000 mipmap hardware bug. Thanks to ChrisB and PhilC at ATI for tracking it down. 
 	* Fix for reduced frsqrte precision on G5.
* 0.1.4	Fri Apr 16 2004
	* Add glError() debug macro and hide spurious error after fence finishing.
* 0.1.3	Thu Apr  7 2004
	* Fix point light defaults. DTS submission for Radeon 7000 mipmap selection problem.
* 0.1.2	Thu Apr  6 2004
	* VAR is now off by default to match regular immediate mode GL. Client must enable it (and handle flushes.)
	* Fixed header includes and extern "C" problems.
* 0.1.1	Wed Mar 31 2004
	* Changed glAAPointLight API to use [-1, 1] coordinates clamped inside a unit circle.
	* Fixed miplevel brightness for negative falloff values.
	* Optimized VAR usage pattern to reduce fence finishing.
* 0.1	Thu Mar 25 2004
 	* Initial public release for iDevGames Vectorized contest.
	* Basic points and lines only.


## What's Implemented

* AA texture generation with exponential falloff.
* 2D point drawing with floating point size and positioning.
* cheap pseudo point light sourcing.
* 2D line drawing with floating point width, length, and positioning. Also line strips and line loops.
* line stipple. (buggy)
* basic color and state management.

### What's Not Implemented Yet

* proper multiple GL context support.
* programmatic texture size.
* line endcap styles.
* triangles, triangle strips, triangle fans, polygons.
* quads, quad strips.
* multitextured primitives
* 3D primitives

### What's Broken

* line stipple is buggy. It works on single line segments but does not wrap across segments correctly yet.
* line stipple phase does not work as intended yet. (This feature is not present in regular OpenGL.)
* hardware mipmap generation does not work reliably on the Geforce2 MX/4MX so is disabled for now.


## Usage Patterns

Currently, glAArg is intended for small simple projects that use immediate mode. The client app is responsible for most of the state-handling, so you will have to do extra work to integrate glAArg into an existing complex project that uses vertex arrays, multiple contexts, etc.

In general, the API is just a drop-in wrapper for regular immediate mode GL. So for code that looks like:

	glLineWidth(4.0);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glBegin(GL_LINES);
	glVertex2f(123,     45);
	glVertex2f(123.4, -111);
	glEnd();

You can just replace each gl* function with glAA* and it will work. Not all of the GL API is wrapped though, only the parts pertaining to 2D drawing that I personally use often. For example, glAAColor4f() exists, but not glAAColor4fv(). See the header for available functions.

There are a few details where glAArg's behavior is different than regular OpenGL:
* glAALineWidth() and glAAPointSize() are allowed inside a glAABegin/glAAEnd pair.
* everything is calculated in floating point accuracy. You can make points 1.23 px big or lines 0.01 px wide.
* a line 0.0 pixels long is treated as a line 1.0 pixels long. Regular GL does this for width but not length.


## Usage Notes

There is some setup required in order to get glAArg to do its thing. First of all, regular GL setup: you have to enable blending, and should enable alpha testing. The other bits shown are optional but recommended:

    // gl init
    glDisable(GL_LIGHTING);
    glDisable(GL_DITHER);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 1/255.0);

You also need to configure the texture matrix. Currently the AA texture size is #defined in AAPrimitives.h as a constant named "phf". This value represents the half-width of the AA point in the texture glAArg manages. You must scale the texture matrix to fit that size, as shown. A future update will allow programmatic texture sizes which will make this unnecessary.
		
    // texture init
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(0.25/phf, 0.25/phf, 1.0);  // glAArg requires rectangle coordinates
    glEnable(GL_TEXTURE_2D);

NOTE: prior to glAArg 0.2.1, the glScale values were 0.125/phf. They have changed to 0.25/phf to match the reduced texture footprint.

NOTE: On certain systems the texture matrix might be reset to the identity between your GL init and the first time the context is drawn. This is known to happen on systems with Geforce 3, Geforce 4Ti, or Geforce FX video cards running Mac OS X 10.3.4, when using fullscreen contexts. If nothing shows up (in glAArg 0.2 or earlier) or everything is speckled (in glAArg 0.2.1 or later) then check your texture matrix. The simplest workaround is to reset the matrix before each draw. You'll also have to do this if you mix glAArg primitives with regular textured content.

Lastly, you have to init glAArg and create at least one texture. You should also enable vertex array buffering here.

	// glAArg init
	glAAInit();
	glAAGenerateAATex(0.0f, 0, NO);			// hard-edge antialiased texture
	glAAGenerateAATex(0.5f, 1, NO);			// softer antialiased texture
	glAAEnable(GLAA_VERTEX_ARRAY);				// we want VAR acceleration

Currently, you can create up to eight AA textures, but they all have to be the same size (phf). It is your responsibility to bind to the proper texture before drawing. The texture IDs are exposed in the global array glAA_texture[].

NOTE: As of glAArg 0.1.2, vertices are submitted by default synchronously to GL in immediate mode. For better performance you should enable vertex arrays as shown above. However this has the side effect of forcing you to manage flushing the vertex array around state changes. For example:

	glAABegin(GL_LINES);
	glAAVertex2f(123,     45);
	glAAVertex2f(123.4, -111);
	glAAEnd();
	glAAFlush();
	glTranslatef(10, 0, 0);
	glAABegin(GL_POINTS);
	glAAVertex2f(50,      50);
	glAAEnd();

Here, we want the modelview matrix translation to affect the point. Because all submitted vertices are buffered with vertex arrays, and will be drawn with the current GL state at the time the vertex array is flushed, you must explicitly flush around GL state changes to have the proper state used.

You can also temporarily disable vertex array buffering to save glAArg generated geometry into display lists.

NOTE: as of glAArg 0.2, the glAAEnable constant for vertex arrays has changed to GLAA_VERTEX_ARRAY. This is #defined in AAPrimitives.h as the same value as the previous GL_VERTEX_ARRAY_RANGE_APPLE constant, but properly reflects that fact that buffering will happen via some flavor of vertex arrays (possibly vanilla VA, or VAR or VBO, depending what hardware and extensions are present.)


## Porting

All of glAArg is written in plain C. You are welcome to use it on any platform you like.

The included GNU makefile will build the "glAArg Demo" application on Mac OS X, Linux, and MINGW under Windows.
There is currently an APPLE-specific path for VAR, other platforms will default to immediate mode. I intend to add VBO support in a future update.


## Contact

Questions? Bugs? Feature requests?
arekkusu@mac.com