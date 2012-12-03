/* OGLDebug.h
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

#if !defined(OPENGL_DEBUG_H)
#define OPENGL_DEBUG_H

#if defined(OPENGL_DEBUG)
  #define OPENGL_CHECK_ERRORS { const GLenum errcode = glGetError(); if (errcode != GL_NO_ERROR) fprintf(stderr, "OpenGL Error code %i in '%s' line %i\n", errcode, __FILE__, __LINE__-1); }
#else
  #define OPENGL_CHECK_ERRORS
#endif

/*  Dump client state (for informational purposes)
        int rval = 0;
        void *ptr;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &rval);
        printf("GL_ARRAY_BUFFER_BINDING: %i\n", rval);
        glGetPointerv(GL_COLOR_ARRAY_POINTER, &ptr);
        printf("GL_COLOR_ARRAY: %i (%lx)\n", (int) glIsEnabled(GL_COLOR_ARRAY), (int) ptr);
        glGetPointerv(GL_FOG_COORD_ARRAY_POINTER, &ptr);
        printf("GL_FOG_COORDINATE_ARRAY_EXT: %i (%lx)\n", (int) glIsEnabled(GL_FOG_COORDINATE_ARRAY_EXT), (int) ptr);
        glGetPointerv(GL_INDEX_ARRAY_POINTER, &ptr);
        printf("GL_INDEX_ARRAY: %i (%lx)\n", (int) glIsEnabled(GL_INDEX_ARRAY), (int) ptr);
        glGetPointerv(GL_NORMAL_ARRAY_POINTER, &ptr);
        printf("GL_NORMAL_ARRAY: %i (%lx)\n", (int) glIsEnabled(GL_NORMAL_ARRAY), (int) ptr);
        glGetPointerv(GL_SECONDARY_COLOR_ARRAY_POINTER, &ptr);
        printf("GL_SECONDARY_COLOR_ARRAY: %i (%lx)\n", (int) glIsEnabled(GL_SECONDARY_COLOR_ARRAY), (int) ptr);
        glGetPointerv(GL_TEXTURE_COORD_ARRAY_POINTER, &ptr);
        printf("GL_TEXTURE_COORD_ARRAY: %i (%lx)\n", (int) glIsEnabled(GL_TEXTURE_COORD_ARRAY), (int) ptr);
        glGetPointerv(GL_VERTEX_ARRAY_POINTER, &ptr);
        printf("GL_VERTEX_ARRAY: %i (%lx)\n", (int) glIsEnabled(GL_VERTEX_ARRAY), (int) ptr);
*/

#endif /* OPENGL_DEBUG_H */
