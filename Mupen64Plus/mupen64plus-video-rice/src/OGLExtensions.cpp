/* OGLExtensions.cpp
Copyright (C) 2009 Richard Goedeken

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/* This source file contains code for assigning function pointers to some OpenGL functions */
/* This is only necessary because Windows does not contain development support for OpenGL versions beyond 1.1 */

#include <SDL_opengl.h>
#include "OGLExtensions.h"
#include "Video.h"

static void APIENTRY EmptyFunc(void) { return; }

bool                                 bNvidiaExtensionsSupported = false;
PFUNCGLCOMBINERPARAMETERFVNVPROC     pglCombinerParameterfvNV = (PFUNCGLCOMBINERPARAMETERFVNVPROC) EmptyFunc;
PFUNCGLFINALCOMBINERINPUTNVPROC      pglFinalCombinerInputNV = (PFUNCGLFINALCOMBINERINPUTNVPROC) EmptyFunc;
PFUNCGLCOMBINEROUTPUTNVPROC          pglCombinerOutputNV = (PFUNCGLCOMBINEROUTPUTNVPROC) EmptyFunc;
PFUNCGLCOMBINERINPUTNVPROC           pglCombinerInputNV = (PFUNCGLCOMBINERINPUTNVPROC) EmptyFunc;
PFUNCGLCOMBINERPARAMETERINVPROC      pglCombinerParameteriNV = (PFUNCGLCOMBINERPARAMETERINVPROC) EmptyFunc;

PFUNCGLACTIVETEXTUREPROC             pglActiveTexture = (PFUNCGLACTIVETEXTUREPROC) EmptyFunc;
PFUNCGLACTIVETEXTUREARBPROC          pglActiveTextureARB = (PFUNCGLACTIVETEXTUREARBPROC) EmptyFunc;
PFUNCGLMULTITEXCOORD2FPROC           pglMultiTexCoord2f = (PFUNCGLMULTITEXCOORD2FPROC) EmptyFunc;
PFUNCGLMULTITEXCOORD2FVPROC          pglMultiTexCoord2fv = (PFUNCGLMULTITEXCOORD2FVPROC) EmptyFunc;
PFUNCGLDELETEPROGRAMSARBPROC         pglDeleteProgramsARB = (PFUNCGLDELETEPROGRAMSARBPROC) EmptyFunc;
PFUNCGLPROGRAMSTRINGARBPROC          pglProgramStringARB = (PFUNCGLPROGRAMSTRINGARBPROC) EmptyFunc;
PFUNCGLBINDPROGRAMARBPROC            pglBindProgramARB = (PFUNCGLBINDPROGRAMARBPROC) EmptyFunc;
PFUNCGLGENPROGRAMSARBPROC            pglGenProgramsARB = (PFUNCGLGENPROGRAMSARBPROC) EmptyFunc;
PFUNCGLPROGRAMENVPARAMETER4FVARBPROC pglProgramEnvParameter4fvARB = (PFUNCGLPROGRAMENVPARAMETER4FVARBPROC) EmptyFunc;
PFUNCGLFOGCOORDPOINTEREXTPROC        pglFogCoordPointerEXT = (PFUNCGLFOGCOORDPOINTEREXTPROC) EmptyFunc;
PFUNCGLCLIENTACTIVETEXTUREARBPROC    pglClientActiveTextureARB = (PFUNCGLCLIENTACTIVETEXTUREARBPROC) EmptyFunc;

#define INIT_ENTRY_POINT(type, funcname) \
  p##funcname = (type) CoreVideo_GL_GetProcAddress(#funcname); \
  if (p##funcname == NULL) { DebugMessage(M64MSG_WARNING, \
  "Couldn't get address of OpenGL function: '%s'", #funcname); p##funcname = (type) EmptyFunc; }

void OGLExtensions_Init(void)
{
    /* nvidia extensions are a special case */
    bNvidiaExtensionsSupported = true;
    pglCombinerParameterfvNV = (PFUNCGLCOMBINERPARAMETERFVNVPROC) CoreVideo_GL_GetProcAddress("glCombinerParameterfvNV");
    if (pglCombinerParameterfvNV == NULL) bNvidiaExtensionsSupported = false;
    pglFinalCombinerInputNV = (PFUNCGLFINALCOMBINERINPUTNVPROC) CoreVideo_GL_GetProcAddress("glFinalCombinerInputNV");
    if (pglFinalCombinerInputNV == NULL) bNvidiaExtensionsSupported = false;
    pglCombinerOutputNV = (PFUNCGLCOMBINEROUTPUTNVPROC) CoreVideo_GL_GetProcAddress("glCombinerOutputNV");
    if (pglCombinerOutputNV == NULL) bNvidiaExtensionsSupported = false;
    pglCombinerInputNV = (PFUNCGLCOMBINERINPUTNVPROC) CoreVideo_GL_GetProcAddress("glCombinerInputNV");
    if (pglCombinerInputNV == NULL) bNvidiaExtensionsSupported = false;
    pglCombinerParameteriNV = (PFUNCGLCOMBINERPARAMETERINVPROC) CoreVideo_GL_GetProcAddress("glCombinerParameteriNV");
    if (pglCombinerParameteriNV == NULL) bNvidiaExtensionsSupported = false;

    INIT_ENTRY_POINT(PFUNCGLACTIVETEXTUREPROC,             glActiveTexture);
    INIT_ENTRY_POINT(PFUNCGLACTIVETEXTUREARBPROC,          glActiveTextureARB);
    INIT_ENTRY_POINT(PFUNCGLMULTITEXCOORD2FPROC,           glMultiTexCoord2f);
    INIT_ENTRY_POINT(PFUNCGLMULTITEXCOORD2FVPROC,          glMultiTexCoord2fv);
    INIT_ENTRY_POINT(PFUNCGLDELETEPROGRAMSARBPROC,         glDeleteProgramsARB);
    INIT_ENTRY_POINT(PFUNCGLPROGRAMSTRINGARBPROC,          glProgramStringARB);
    INIT_ENTRY_POINT(PFUNCGLBINDPROGRAMARBPROC,            glBindProgramARB);
    INIT_ENTRY_POINT(PFUNCGLGENPROGRAMSARBPROC,            glGenProgramsARB);
    INIT_ENTRY_POINT(PFUNCGLPROGRAMENVPARAMETER4FVARBPROC, glProgramEnvParameter4fvARB);
    INIT_ENTRY_POINT(PFUNCGLFOGCOORDPOINTEREXTPROC,        glFogCoordPointerEXT);
    INIT_ENTRY_POINT(PFUNCGLCLIENTACTIVETEXTUREARBPROC,    glClientActiveTextureARB);
}


