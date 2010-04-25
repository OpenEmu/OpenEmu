/*
 * OGLFT: A library for drawing text with OpenGL using the FreeType library
 * Copyright (C) 2002 lignum Computing, Inc. <oglft@lignumcomputing.com>
 * $Id: OGLFT.cpp,v 1.11 2003/10/01 14:21:18 allen Exp $
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <iostream>
#include <iomanip>
#include <string.h>
#include "OGLFT.h"

int wstrlen(const wchar_t * s) 
{
    int r = 0;
    while (*s++) r++;
    return r;
}

namespace OGLFT 
{
    // This is the static instance of the FreeType library wrapper ...
    Library Library::library;

    // ... and this is the FreeType library handle itself.
    FT_Library Library::library_;

    
    // The static instance above causes this constructor to be called
    // when the object module is loaded.
    Library::Library (void)
    {
        FT_Error error = FT_Init_FreeType(&library_);
        if(error != 0) std::cerr << "[OGLFT] Could not initialize the FreeType library." << std::endl;
    }

    Library::~Library (void)
    {
        FT_Error error = FT_Done_FreeType(library_);
        if(error != 0) std::cerr << "[OGLFT] Could not terminate the FreeType library." << std::endl;
    }

    // Return the only instance in the process
    FT_Library& Library::instance (void)
    {
        return library_;
    }

    // Load a new face
    Face::Face (const char* filename, float point_size, FT_UInt resolution)
        : point_size_(point_size), resolution_(resolution)
    {
        valid_ = true;
        FT_Face ft_face;
        FT_Error error = FT_New_Face(Library::instance(), filename, 0, &ft_face);
        if(error != 0) 
        {
            valid_ = false;
            return;
        }

        // As of FreeType 2.1: only a UNICODE charmap is automatically activated.
        // If no charmap is activated automatically, just use the first one.
        if(ft_face->charmap == 0 && ft_face->num_charmaps > 0) FT_Select_Charmap(ft_face, ft_face->charmaps[0]->encoding);

        faces_.push_back(FaceData(ft_face));

        init();
    }

    // Go with a face that the user has already opened.
    Face::Face (FT_Face face, float point_size, FT_UInt resolution)
        : point_size_(point_size), resolution_(resolution)
    {
        valid_ = true;

        // As of FreeType 2.1: only a UNICODE charmap is automatically activated.
        // If no charmap is activated automatically, just use the first one.
        if(face->charmap == 0 && face->num_charmaps > 0) FT_Select_Charmap(face, face->charmaps[0]->encoding);

        faces_.push_back(FaceData(face, false));

        init();
    }

    // Standard initialization behavior once the font file is opened.
    void Face::init (void)
    {
        // By default, each glyph is compiled into a display list the first
        // time it is encountered
        compile_mode_ = COMPILE;

        // By default, all drawing is wrapped with push/pop matrix so that the
        // MODELVIEW matrix is not modified. If advance_ is set, then subsequent
        // drawings follow from the advance of the last glyph rendered.
        advance_ = false;

        // Initialize the default colors
        foreground_color_[R] = 0.; foreground_color_[G] = 0.; foreground_color_[B] = 0.; foreground_color_[A] = 1.;
        background_color_[R] = 1.; background_color_[G] = 1.; background_color_[B] = 1.; background_color_[A] = 0.;

        // The default positioning of the text is at the origin of the first glyph
        horizontal_justification_ = ORIGIN;
        vertical_justification_ = BASELINE;

        // By default, strings are rendered in their nominal direction
        string_rotation_ = 0;

        // setCharacterRotationReference calls the virtual function clearCaches()
        // so it is up to a subclass to set the real default
        rotation_reference_glyph_ = 0;
        rotation_reference_face_ = 0;
        rotation_offset_y_ = 0.;
    }

    Face::~Face (void)
    {
        for(unsigned int i=0; i<faces_.size(); i++)
            if(faces_[i].free_on_exit_)
                FT_Done_Face(faces_[i].face_);
    }

    // Add another Face to select characters from
    bool Face::addAuxiliaryFace (const char* filename)
    {
        FT_Face ft_face;

        FT_Error error = FT_New_Face(Library::instance(), filename, 0, &ft_face);

        if(error != 0) return false;

        faces_.push_back(FaceData(ft_face));
        setCharSize();

        return true;
    }

    // Add another Face to select characters from
    bool Face::addAuxiliaryFace (FT_Face face)
    {
        faces_.push_back(FaceData(face, false));

        setCharSize();

        return true;
    }

    // Note: Changing the point size also clears the display list cache
    void Face::setPointSize (float point_size)
    {
        if(point_size != point_size_) 
        {
            point_size_ = point_size;
            clearCaches();
            setCharSize();
        }
    }

     // Note: Changing the resolution also clears the display list cache
    void Face::setResolution (FT_UInt resolution)
    {
        if(resolution != resolution_) 
        {
            resolution_ = resolution;
            clearCaches();
            setCharSize();
        }
    }

    // Note: Changing the background color also clears the display list cache.
    void Face::setBackgroundColor (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
    {
        if(background_color_[R] != red||background_color_[G] != green||background_color_[B] != blue||background_color_[A] != alpha) 
        {
            background_color_[R] = red;
            background_color_[G] = green;
            background_color_[B] = blue;
            background_color_[A] = alpha;
        }
    }

    // Note: Changing the foreground color also clears the display list cache.
    void Face::setForegroundColor (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
    {
        if(foreground_color_[R] != red||foreground_color_[G] != green||foreground_color_[B] != blue||foreground_color_[A] != alpha) 
        {
            foreground_color_[R] = red;
            foreground_color_[G] = green;
            foreground_color_[B] = blue;
            foreground_color_[A] = alpha;
        }
    }

    // Note: Changing the foreground color also clears the display list cache.
    void Face::setForegroundColor (const GLfloat foreground_color[4])
    {
        foreground_color_[R] = foreground_color[R];
        foreground_color_[G] = foreground_color[G];
        foreground_color_[B] = foreground_color[B];
        foreground_color_[A] = foreground_color[A];
    }

    // Note: Changing the background color also clears the display list cache.
    void Face::setBackgroundColor (const GLfloat background_color[4])
    {
        background_color_[R] = background_color[R];
        background_color_[G] = background_color[G];
        background_color_[B] = background_color[B];
        background_color_[A] = background_color[A];
    }

    // Note: Changing the string rotation angle clears the display list cache
    void Face::setStringRotation (GLfloat string_rotation)
    {
        if(string_rotation != string_rotation_) 
        {
            string_rotation_ = string_rotation;

            clearCaches();

            // Note that this affects ALL glyphs accessed through
            // the Face, both the vector and the raster glyphs. Very nice!
            if (string_rotation_ != 0) 
            {
                float angle;
                if (string_rotation_<0) 
                    angle = 360.0f - fmod(fabs(string_rotation_), 360.f);
                else 
                    angle = fmod(string_rotation_, 360.f);

                FT_Matrix rotation_matrix;
                FT_Vector sinus;

                FT_Vector_Unit(&sinus, (FT_Angle)(angle * 0x10000L));

                rotation_matrix.xx = sinus.x;
                rotation_matrix.xy = -sinus.y;
                rotation_matrix.yx = sinus.y;
                rotation_matrix.yy = sinus.x;

                for(unsigned int i=0; i<faces_.size(); i++) FT_Set_Transform(faces_[i].face_, &rotation_matrix, 0);
            }
            else for(unsigned int i=0; i<faces_.size(); i++) FT_Set_Transform(faces_[i].face_, 0, 0);
        }
    }

    // Note: Changing the rotation reference character clears the display list cache.
    void Face::setCharacterRotationReference (unsigned char c)
    {
        unsigned int f;
        FT_UInt glyph_index = 0;

        for(f=0; f<faces_.size(); f++) 
        {
            glyph_index = FT_Get_Char_Index(faces_[f].face_, c);
            if(glyph_index != 0) break;
        }

        if(f<faces_.size() && glyph_index != rotation_reference_glyph_) 
        {
            FT_Error error = FT_Load_Glyph(faces_[f].face_, glyph_index, FT_LOAD_DEFAULT);

            if(error != 0) return;

            rotation_reference_glyph_ = glyph_index;
            rotation_reference_face_ = faces_[f].face_;
            setRotationOffset();
            
            clearCaches();
        }
    }

    BBox Face::measure (const char* s)
    {
        BBox bbox;
        char c;

        if((c = *s++) != 0) 
        {
            bbox = measure((unsigned char)c);

            for(c = *s; c != 0; c = *++s) 
            {
                BBox char_bbox = measure((unsigned char)c);
                bbox += char_bbox;
            }
        }

        return bbox;
    }

    BBox Face::measureRaw (const char* s)
    {
        BBox bbox;

        for(char c = *s; c != 0; c = *++s) 
        {
            BBox char_bbox;

            unsigned int f;
            FT_UInt glyph_index = 0;

            for(f=0; f<faces_.size(); f++) 
            {
                glyph_index = FT_Get_Char_Index(faces_[f].face_, c);
                if(glyph_index != 0) break;
            }

            if(glyph_index == 0) continue;

            FT_Error error = FT_Load_Glyph(faces_[f].face_, glyph_index, FT_LOAD_DEFAULT);
            if(error != 0) continue;

            FT_Glyph glyph;
            error = FT_Get_Glyph(faces_[f].face_->glyph, &glyph);
            if(error != 0) continue;

            FT_BBox ft_bbox;
            FT_Glyph_Get_CBox(glyph, ft_glyph_bbox_unscaled, &ft_bbox);

            FT_Done_Glyph(glyph);

            char_bbox = ft_bbox;
            char_bbox.advance_ = faces_[f].face_->glyph->advance;

            bbox += char_bbox;
        }

        return bbox;
    }

    BBox Face::measure (const wchar_t* s)
    {
        BBox bbox;
        int i;

        if(wstrlen(s) > 0) 
        {
            bbox = measure(s[0]);
            for(i = 1; i < wstrlen(s); i++)
            {
                BBox char_bbox = measure(s[i]);
                bbox += char_bbox;
            }
        }
        return bbox;
    }

    BBox Face::measure (const wchar_t* format, double number)
    {
        return measure(format, number);
    }

    BBox Face::measureRaw (const wchar_t* s)
    {
        BBox bbox;
        int i;

        for(i = 0; i < wstrlen(s); i++)
        {
            BBox char_bbox;

            unsigned int f;
            FT_UInt glyph_index = 0;

            for(f=0; f<faces_.size(); f++) 
            {
                glyph_index = FT_Get_Char_Index(faces_[f].face_, s[i]);
                if(glyph_index != 0) break;
            }

            if(glyph_index == 0) 
            {
                continue;
            }

            FT_Error error = FT_Load_Glyph(faces_[f].face_, glyph_index, FT_LOAD_DEFAULT);
            if(error != 0) continue;

            FT_Glyph glyph;
            error = FT_Get_Glyph(faces_[f].face_->glyph, &glyph);
            if(error != 0) continue;

            FT_BBox ft_bbox;
            FT_Glyph_Get_CBox(glyph, ft_glyph_bbox_unscaled, &ft_bbox);

            FT_Done_Glyph(glyph);

            char_bbox = ft_bbox;
            char_bbox.advance_ = faces_[f].face_->glyph->advance;

            bbox += char_bbox;
        }

        return bbox;
    }
    
    // Measure the bounding box as if the (latin1) string were not rotated
    BBox Face::measure_nominal (const char* s)
    {
        if(string_rotation_ == 0.) return measure(s);
    
        for(unsigned int f=0; f<faces_.size(); f++) FT_Set_Transform(faces_[f].face_, 0, 0);

        BBox bbox = measure(s);

        float angle;
        if(string_rotation_<0.)
            angle = 360.0f - fmod(fabs(string_rotation_), 360.f);
        else 
            angle = fmod(string_rotation_, 360.f);

        FT_Matrix rotation_matrix;
        FT_Vector sinus;

        FT_Vector_Unit(&sinus, (FT_Angle)(angle * 0x10000L));

        rotation_matrix.xx = sinus.x;
        rotation_matrix.xy = -sinus.y;
        rotation_matrix.yx = sinus.y;
        rotation_matrix.yy = sinus.x;

        for(unsigned int f=0; f<faces_.size(); f++) FT_Set_Transform(faces_[f].face_, &rotation_matrix, 0);

        return bbox;
    }

    // Measure the bounding box as if the (UNICODE) string were not rotated
    BBox Face::measure_nominal (const wchar_t* s)
    {
        if(string_rotation_ == 0.)return measure(s);

        for(unsigned int f=0; f<faces_.size(); f++)FT_Set_Transform(faces_[f].face_, 0, 0);

        BBox bbox = measure(s);

        float angle;
        if(string_rotation_<0.0) 
            angle = 360.0f - fmod(fabs(string_rotation_), 360.f);
        else 
            angle = fmod(string_rotation_, 360.f);

        FT_Matrix rotation_matrix;
        FT_Vector sinus;

        FT_Vector_Unit(&sinus, (FT_Angle)(angle * 0x10000L));

        rotation_matrix.xx = sinus.x;
        rotation_matrix.xy = -sinus.y;
        rotation_matrix.yx = sinus.y;
        rotation_matrix.yy = sinus.x;

        for(unsigned int f=0; f<faces_.size(); f++)FT_Set_Transform(faces_[f].face_, &rotation_matrix, 0);

        return bbox;
    }

    // Compile a (latin1) character glyph into a display list and cache
    // it for later

    GLuint Face::compile (unsigned char c)
    {
        // See if we've done it already
        GDLCI fgi = glyph_dlists_.find(c);

        if(fgi != glyph_dlists_.end())return fgi->second;

        unsigned int f;
        FT_UInt glyph_index = 0;

        for(f=0; f<faces_.size(); f++)
        {
            glyph_index = FT_Get_Char_Index(faces_[f].face_, c);
            if(glyph_index != 0) break;
        }

        if(glyph_index == 0)return 0;

        GLuint dlist = compileGlyph(faces_[f].face_, glyph_index);
        glyph_dlists_[ c ] = dlist;

        return dlist;
    }


    // Compile a (UNICODE) character glyph into a display list and cache
    // it for later
    GLuint Face::compile (const wchar_t c)
    {
        // See if we've done it already
        GDLCI fgi = glyph_dlists_.find(c);

        if(fgi != glyph_dlists_.end())return fgi->second;

        unsigned int f;
        FT_UInt glyph_index = 0;

        for(f=0; f<faces_.size(); f++)
        {
            glyph_index = FT_Get_Char_Index(faces_[f].face_, c);
            if(glyph_index != 0) break;
        }

        if(glyph_index == 0)return 0;

        GLuint dlist = compileGlyph(faces_[f].face_, glyph_index);

        glyph_dlists_[ c ] = dlist;

        return dlist;
    }

    // Assume the MODELVIEW matrix is already set and draw the (latin1)
    // string.  Note: this routine now ignores almost all settings:
    // including the position (both modelview and raster), color,
    // justification and advance settings. Consider this to be the raw
    // drawing routine for which you are responsible for most of the
    // setup.
    void Face::draw (const char* s)
    {
        DLCI character_display_list = character_display_lists_.begin();

        for(char c = *s; c != 0; c = *++s)
        {
            if(character_display_list != character_display_lists_.end())
            {
                glCallList(*character_display_list);
                character_display_list++;
            }
            draw((unsigned char)c);
        }
    }

    // Assume the MODELVIEW matrix is already set and draw the (UNICODE)
    // string.  Note: this routine now ignores almost all settings:
    // including the position (both modelview and raster), color,
    // justification and advance settings. Consider this to be the raw
    // drawing routine for which you are responsible for most of the
    // setup.
    void Face::draw (const wchar_t* s)
    {
        DLCI character_display_list = character_display_lists_.begin();
        int i;

        for(i = 0; i < wstrlen(s); i++)
        {
            if(character_display_list != character_display_lists_.end())
            {
                glCallList(*character_display_list);
                character_display_list++;
            }
            draw(s[i]);
        }
    }

    // Assume the MODELVIEW matrix is already setup and draw the
    // (latin1) character.
    void Face::draw (unsigned char c)
    {
        // See if we've done it already
        GDLCI fgi = glyph_dlists_.find(c);

        if(fgi != glyph_dlists_.end()) 
        {
            glCallList(fgi->second);
            return;
        }

        unsigned int f;
        FT_UInt glyph_index = 0;

        for(f=0; f<faces_.size(); f++)
        {
            glyph_index = FT_Get_Char_Index(faces_[f].face_, c);
            if(glyph_index != 0) break;
        }

        if(glyph_index == 0) return;

        if(compile_mode_ == COMPILE)
        {
            GLuint dlist = compile(c);
            glCallList(dlist);
        }
        else renderGlyph(faces_[f].face_, glyph_index);
    }

    // Assume the MODELVIEW matrix is already setup and draw the
    // (UNICODE) character.

    void Face::draw (const wchar_t c)
    {
        // See if we've done it already
        GDLCI fgi = glyph_dlists_.find(c);

        if(fgi != glyph_dlists_.end()) 
        {
            glCallList(fgi->second);
            return;
        }

        unsigned int f;
        FT_UInt glyph_index = 0;

        for(f=0; f<faces_.size(); f++)
        {
            glyph_index = FT_Get_Char_Index(faces_[f].face_, c);
            if(glyph_index != 0) break;
        }

        if(glyph_index == 0) return;

        if(compile_mode_ == COMPILE)
        {
            GLuint dlist = compile(c);
            glCallList(dlist);
        }
        else renderGlyph(faces_[f].face_, glyph_index);
    }

    // Draw the (latin1) character at the given position. The MODELVIEW
    // matrix is modified by the glyph advance.
    void Face::draw (GLfloat x, GLfloat y, unsigned char c)
    {
        glTranslatef(x, y, 0.);

        glColor4f(foreground_color_[R], foreground_color_[G], foreground_color_[B], foreground_color_[A]);

        glRasterPos2i(0, 0);

        draw(c);
    }

    // Draw the (latin1) character at the given position. The MODELVIEW
    // matrix is modified by the glyph advance.
    void Face::draw (GLfloat x, GLfloat y, GLfloat z, unsigned char c)
    {
        glTranslatef(x, y, z);

        glColor4f(foreground_color_[R], foreground_color_[G], foreground_color_[B], foreground_color_[A]);

        glRasterPos2i(0, 0);

        draw(c);
    }

    // Draw the (UNICODE) character at the given position. The MODELVIEW
    // matrix is modified by the glyph advance.
    void Face::draw (GLfloat x, GLfloat y, wchar_t c)
    {
        glTranslatef(x, y, 0.);

        glColor4f(foreground_color_[R], foreground_color_[G], foreground_color_[B],
                foreground_color_[A]);

        glRasterPos2i(0, 0);

        draw(c);
    }

    // Draw the (UNICODE) character at the given position. The MODELVIEW
    // matrix is modified by the glyph advance.
    void Face::draw (GLfloat x, GLfloat y, GLfloat z, wchar_t c)
    {
        glTranslatef(x, y, z);

        glColor4f(foreground_color_[R], foreground_color_[G], foreground_color_[B], foreground_color_[A]);

        glRasterPos2i(0, 0);

        draw(c);
    }


    // Draw the (latin1) string at the given position.
    void Face::draw (GLfloat x, GLfloat y, const char* s, float *sizebox)
    {
        // sizebox is xmin,ymin, xmax,ymax
        if(!advance_) glPushMatrix();

        if(horizontal_justification_ != ORIGIN || vertical_justification_ != BASELINE)
        {
            glPushMatrix();

            GLfloat dx = 0, dy = 0;

            switch (horizontal_justification_)
            {
                case LEFT:   dx = -sizebox[0] + 1; break;
                case CENTER: dx = -(sizebox[0] + sizebox[2])/ 2.0f; break;
                case RIGHT:  dx = -sizebox[2] - 1; break;
                default: break;
            }
            switch (vertical_justification_)
            {
                case BOTTOM: dy = -sizebox[1] + 1; break;
                case MIDDLE: dy = -(sizebox[1] + sizebox[3])/ 2.0f; break;
                case TOP: dy = -sizebox[3] - 1; break;
                default: break;
            }

            // There is probably a less expensive way to compute this
            glRotatef(string_rotation_, 0., 0., 1.);
            glTranslatef(dx, dy, 0);
            glRotatef(-string_rotation_, 0., 0., 1.);
        }

        glTranslatef(x, y, 0.);

        glColor4f(foreground_color_[R], foreground_color_[G], foreground_color_[B], foreground_color_[A]);

        glRasterPos3i(0, 0, 0);

        draw(s);

        if(horizontal_justification_ != ORIGIN || vertical_justification_ != BASELINE) glPopMatrix();

        if(!advance_) glPopMatrix();
    }

    // Draw the (latin1) string at the given position.
    void Face::draw (GLfloat x, GLfloat y, GLfloat z, const char* s)
    {
        if(!advance_) glPushMatrix();

        if(horizontal_justification_ != ORIGIN || vertical_justification_ != BASELINE)
        {
            glPushMatrix();

            BBox bbox = measure_nominal(s);

            GLfloat dx = 0, dy = 0;

            switch (horizontal_justification_)
            {
                case LEFT:   dx = -bbox.x_min_; break;
                case CENTER: dx = -(bbox.x_min_ + bbox.x_max_)/ 2.0f; break;
                case RIGHT: dx = -bbox.x_max_; break;
                default: break;
            }
            switch (vertical_justification_)
            {
                case BOTTOM: dy = -bbox.y_min_; break;
                case MIDDLE: dy = -(bbox.y_min_ + bbox.y_max_)/ 2.0f; break;
                case TOP:    dy = -bbox.y_max_; break;
                default: break;
            }

            // There is probably a less expensive way to compute this
            glRotatef(string_rotation_, 0., 0., 1.);
            glTranslatef(dx, dy, 0);
            glRotatef(-string_rotation_, 0., 0., 1.);
        }

        glTranslatef(x, y, z);

        glColor4f(foreground_color_[R], foreground_color_[G], foreground_color_[B], foreground_color_[A]);

        glRasterPos2i(0, 0);

        draw(s);

        if(horizontal_justification_ != ORIGIN || vertical_justification_ != BASELINE) glPopMatrix();

        if(!advance_) glPopMatrix();
    }

    // Draw the (UNICODE) string at the given position.
    void Face::draw (GLfloat x, GLfloat y, const wchar_t* s)
    {
        if(!advance_)
            glPushMatrix();

        if(horizontal_justification_!=ORIGIN||vertical_justification_!=BASELINE)
            {
            glPushMatrix();

            BBox bbox = measure_nominal(s);

            GLfloat dx = 0, dy = 0;

            switch (horizontal_justification_)
                {
                case LEFT:
                    dx = -bbox.x_min_; break;
                case CENTER:
                    dx = -(bbox.x_min_ + bbox.x_max_)/ 2.0f; break;
                case RIGHT:
                    dx = -bbox.x_max_; break;
                default:
                    break;
                }
            switch (vertical_justification_)
                {
                case BOTTOM:
                    dy = -bbox.y_min_; break;
                case MIDDLE:
                    dy = -(bbox.y_min_ + bbox.y_max_)/ 2.0f; break;
                case TOP:
                    dy = -bbox.y_max_; break;
                default:
                    break;
                }

            // There is probably a less expensive way to compute this
            glRotatef(string_rotation_, 0., 0., 1.);
            glTranslatef(dx, dy, 0);
            glRotatef(-string_rotation_, 0., 0., 1.);
            }

            glTranslatef(x, y, 0.);

            glColor4f(foreground_color_[R], foreground_color_[G], foreground_color_[B],
                        foreground_color_[A]);

            glRasterPos2i(0, 0);

            draw(s);

            if(horizontal_justification_ != ORIGIN ||
                vertical_justification_ != BASELINE)
                glPopMatrix();

            if(!advance_)
                glPopMatrix();
    }

    // Draw the (UNICODE) string at the given position.
    void Face::draw (GLfloat x, GLfloat y, GLfloat z, const wchar_t* s)
    {
        if(!advance_) glPushMatrix();

        if(horizontal_justification_!= ORIGIN||vertical_justification_!= BASELINE)
            {
            glPushMatrix();

            // In 3D, we need to exert more care in the computation of the
            // bounding box of the text. NOTE: Needs to be fixed up for
            // polygonal faces, too...

            BBox bbox;
            // Code from measure_nominal, but changed to use measureRaw instead
            if(string_rotation_ == 0.)  bbox = measureRaw(s);
            else
                {
                for(unsigned int f=0; f<faces_.size(); f++)
                    FT_Set_Transform(faces_[f].face_, 0, 0);

                bbox = measureRaw(s);

                float angle;
                if(string_rotation_<0.0)
                    angle = 360.0f - fmod(fabs(string_rotation_), 360.f);
                else
                    angle = fmod(string_rotation_, 360.f);

                FT_Matrix rotation_matrix;
                FT_Vector sinus;

                FT_Vector_Unit(&sinus, (FT_Angle)(angle * 0x10000L));

                rotation_matrix.xx = sinus.x;
                rotation_matrix.xy = -sinus.y;
                rotation_matrix.yx = sinus.y;
                rotation_matrix.yy = sinus.x;

                for(unsigned int f=0; f<faces_.size(); f++)
                    FT_Set_Transform(faces_[f].face_, &rotation_matrix, 0);
                }

            GLfloat dx = 0, dy = 0;

            switch (horizontal_justification_)
                {
                case LEFT:
                    dx = bbox.x_min_; break;
                case CENTER:
                    dx = (bbox.x_min_ + bbox.x_max_)/ 2; break;
                case RIGHT:
                    dx = bbox.x_max_; break;
                default:
                    break;
                }
            switch (vertical_justification_)
                {
                case BOTTOM:
                    dy = bbox.y_min_; break;
                case MIDDLE:
                    dy = (bbox.y_min_ + bbox.y_max_)/2; break;
                case TOP:
                    dy = bbox.y_max_; break;
                default:
                    break;
                }

            GLint viewport[4];
            GLdouble modelview[16], projection[16];

            glGetIntegerv(GL_VIEWPORT, viewport);
            glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
            glGetDoublev(GL_PROJECTION_MATRIX, projection);

            GLdouble x0, y0, z0;
            gluUnProject(0, 0, 0, modelview, projection, viewport, &x0, &y0, &z0);

            GLdouble dx_m, dy_m, dz_m;
            gluUnProject(dx, dy, 0., modelview, projection, viewport,&dx_m,&dy_m,&dz_m);

            glTranslated(x0-dx_m, y0-dy_m, z0-dz_m);
            }

        glTranslatef(x, y, z);
        glColor4f(foreground_color_[R], foreground_color_[G], foreground_color_[B], foreground_color_[A]);
        glRasterPos2i(0, 0);
        draw(s);

        if(horizontal_justification_!=ORIGIN||vertical_justification_!= BASELINE)
            glPopMatrix();

        if(!advance_)
            glPopMatrix();
    }

    Raster::Raster (const char* filename, float point_size, FT_UInt resolution)
        : Face(filename, point_size, resolution)
    {
        if(!isValid()) return;

        init();
    }

    Raster::Raster (FT_Face face, float point_size, FT_UInt resolution)
        : Face(face, point_size, resolution)
    {
        init();
    }

    void Raster::init (void)
    {
        character_rotation_z_ = 0;
        setCharSize();
        setCharacterRotationReference('o');
    }

    Raster::~Raster (void)
    {
        clearCaches();
    }

    void Raster::setCharacterRotationZ (GLfloat character_rotation_z)
    {
        if(character_rotation_z != character_rotation_z_)
        {
            character_rotation_z_ = character_rotation_z;
            clearCaches();
        }
    }

    double Raster::height (void)const
    {
        if(faces_[0].face_->height > 0) return faces_[0].face_->height / 64.;
        else return faces_[0].face_->size->metrics.y_ppem;
    }

    BBox Raster::measure (unsigned char c)
    {
        BBox bbox;
        
        // For starters, just get the unscaled glyph bounding box
        unsigned int f;
        FT_UInt glyph_index = 0;

        for(f=0; f<faces_.size(); f++)
        {
            glyph_index = FT_Get_Char_Index(faces_[f].face_, c);
            if(glyph_index != 0) break;
        }

        if(glyph_index == 0) return bbox;

        FT_Error error = FT_Load_Glyph(faces_[f].face_, glyph_index, FT_LOAD_DEFAULT);
        if(error != 0) return bbox;

        FT_Glyph glyph;
        error = FT_Get_Glyph(faces_[f].face_->glyph, &glyph);
        if(error != 0) return bbox;

        FT_BBox ft_bbox;
        FT_Glyph_Get_CBox(glyph, ft_glyph_bbox_unscaled, &ft_bbox);

        FT_Done_Glyph(glyph);

        bbox = ft_bbox;
        bbox.advance_ = faces_[f].face_->glyph->advance;
    
        // In order to be accurate regarding the placement of text not
        // aligned at the glyph's origin (CENTER/MIDDLE), the bounding box
        // of the raster format has to be projected back into the
        // view's coordinates
        GLint viewport[4];
        GLdouble modelview[16], projection[16];

        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
        glGetDoublev(GL_PROJECTION_MATRIX, projection);

        // Well, first we have to get the Origin, since that is the basis
        // of the bounding box
        GLdouble x0, y0, z0;
        gluUnProject(0., 0., 0., modelview, projection, viewport, &x0, &y0, &z0);

        GLdouble x, y, z;
        gluUnProject(bbox.x_min_, bbox.y_min_, 0., modelview, projection, viewport, &x, &y, &z);
        bbox.x_min_ = (float) (x - x0);
        bbox.y_min_ = (float) (y - y0);

        gluUnProject(bbox.x_max_, bbox.y_max_, 0., modelview, projection, viewport, &x, &y, &z);
        bbox.x_max_ = (float) (x - x0);
        bbox.y_max_ = (float) (y - y0);

        gluUnProject(bbox.advance_.dx_, bbox.advance_.dy_, 0., modelview, projection, viewport, &x, &y, &z);
        bbox.advance_.dx_ = (float) (x - x0);
        bbox.advance_.dy_ = (float) (y - y0);

        return bbox;
    }

    BBox Raster::measure (wchar_t c)
    {
        BBox bbox;
        
        // For starters, just get the unscaled glyph bounding box
        unsigned int f;
        FT_UInt glyph_index = 0;

        for(f=0; f<faces_.size(); f++)
        {
            glyph_index = FT_Get_Char_Index(faces_[f].face_, c);
            if(glyph_index != 0) break;
        }

        if(glyph_index == 0) return bbox;

        FT_Error error = FT_Load_Glyph(faces_[f].face_, glyph_index,
                                        FT_LOAD_DEFAULT);
        if(error != 0) return bbox;

        FT_Glyph glyph;
        error = FT_Get_Glyph(faces_[f].face_->glyph, &glyph);
        if(error != 0) return bbox;

        FT_BBox ft_bbox;
        FT_Glyph_Get_CBox(glyph, ft_glyph_bbox_unscaled, &ft_bbox);

        FT_Done_Glyph(glyph);

        bbox = ft_bbox;
        bbox.advance_ = faces_[f].face_->glyph->advance;

        // In order to be accurate regarding the placement of text not
        // aligned at the glyph's origin (CENTER/MIDDLE), the bounding box
        // of the raster format has to be projected back into the
        // view's coordinates
        GLint viewport[4];
        GLdouble modelview[16], projection[16];

        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
        glGetDoublev(GL_PROJECTION_MATRIX, projection);

        // Well, first we have to get the Origin, since that is the basis
        // of the bounding box
        GLdouble x0, y0, z0;
        gluUnProject(0., 0., 0., modelview, projection, viewport, &x0, &y0, &z0);

        GLdouble x, y, z;
        gluUnProject(bbox.x_min_, bbox.y_min_, 0., modelview, projection, viewport, &x, &y, &z);
        bbox.x_min_ = (float) (x - x0);
        bbox.y_min_ = (float) (y - y0);

        gluUnProject(bbox.x_max_, bbox.y_max_, 0., modelview, projection, viewport, &x, &y, &z);
        bbox.x_max_ = (float) (x - x0);
        bbox.y_max_ = (float) (y - y0);

        gluUnProject(bbox.advance_.dx_, bbox.advance_.dy_, 0., modelview, projection, viewport, &x, &y, &z);
        bbox.advance_.dx_ = (float) (x - x0);
        bbox.advance_.dy_ = (float) (y - y0);

        return bbox;
    }

    GLuint Raster::compileGlyph (FT_Face face, FT_UInt glyph_index)
    {
        GLuint dlist = glGenLists(1);
        glNewList(dlist, GL_COMPILE);

        renderGlyph(face, glyph_index);

        glEndList();

        return dlist;
    }

    void Raster::setCharSize (void)
    {
        FT_Error error;
        for(unsigned int i=0; i<faces_.size(); i++)
        {
            error = FT_Set_Char_Size(faces_[i].face_,(FT_F26Dot6)(point_size_ * 64),(FT_F26Dot6)(point_size_ * 64),resolution_,resolution_);
            if(error != 0) return;
        }

        if(rotation_reference_glyph_ != 0) setRotationOffset();
    }

    void Raster::setRotationOffset (void)
    {
        FT_Error error = FT_Load_Glyph(rotation_reference_face_, rotation_reference_glyph_, FT_LOAD_RENDER);

        if(error != 0) return;

        rotation_offset_y_ = rotation_reference_face_->glyph->bitmap.rows / 2.0f;
    }

    void Raster::clearCaches (void)
    {
        GDLI fgi = glyph_dlists_.begin();

        for(; fgi != glyph_dlists_.end(); ++fgi)
        {
            glDeleteLists(fgi->second, 1);
        }

        glyph_dlists_.clear();
    }

    Monochrome::Monochrome (const char* filename, float point_size, FT_UInt resolution)
        : Raster(filename, point_size, resolution)
    {
        return;
    }

    Monochrome::Monochrome (FT_Face face, float point_size, FT_UInt resolution)
    : Raster(face, point_size, resolution)
    {
        return;
    }

    Monochrome::~Monochrome (void)
    {
        return;
    }

    GLubyte* Monochrome::invertBitmap (const FT_Bitmap& bitmap)
    {
        // In FreeType 2.0.9, the pitch of bitmaps was rounded up to an
        // even number. In general, this disagrees with what we had been
        // using for OpenGL.
        int width = bitmap.width / 8 + ((bitmap.width & 7)> 0 ? 1 : 0);

        GLubyte* inverse = new GLubyte[ bitmap.rows * width ];
        GLubyte* inverse_ptr = inverse;

        for(int r=0; r<bitmap.rows; r++)
        {
            GLubyte* bitmap_ptr = &bitmap.buffer[bitmap.pitch * (bitmap.rows - r - 1)];

            memmove(inverse_ptr, bitmap_ptr, width);
            inverse_ptr += width;
            bitmap_ptr += width;
        }

        return inverse;
    }

    void Monochrome::renderGlyph (FT_Face face, FT_UInt glyph_index)
    {
        // Start by retrieving the glyph's data.
        FT_Error error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);

        if(error != 0) return;

        FT_Glyph original_glyph;
        FT_Glyph glyph;

        error = FT_Get_Glyph(face->glyph, &original_glyph);

        if(error != 0) return;

        error = FT_Glyph_Copy(original_glyph, &glyph);

        FT_Done_Glyph(original_glyph);

        if(error != 0) return;

        // If the individual characters are rotated (as distinct from string
        // rotation), then apply that extra rotation here. This is equivalent
        // to the sequence
        // glTranslate(x_center,y_center);
        // glRotate(angle);
        // glTranslate(-x_center,-y_center);
        // which is used for the polygonal styles. The deal with the raster
        // styles is that you must retain the advance from the string rotation
        // so that the glyphs are laid out properly. So, we make a copy of
        // the string rotated glyph, and then rotate that and add back an
        // additional offset to (in effect) restore the proper origin and
        // advance of the glyph.

        if(character_rotation_z_ != 0.)
        {
            FT_Matrix rotation_matrix;
            FT_Vector sinus;

            FT_Vector_Unit(&sinus, (FT_Angle)(character_rotation_z_ * 0x10000L));

            rotation_matrix.xx = sinus.x;
            rotation_matrix.xy = -sinus.y;
            rotation_matrix.yx = sinus.y;
            rotation_matrix.yy = sinus.x;

            FT_Vector original_offset, rotation_offset;

            original_offset.x = (face->glyph->metrics.width / 2 + face->glyph->metrics.horiBearingX)/ 64 * 0x10000L;
            original_offset.y = (FT_Pos)(rotation_offset_y_ * 0x10000L);

            rotation_offset = original_offset;

            FT_Vector_Rotate(&rotation_offset, (FT_Angle)(character_rotation_z_ * 0x10000L));

            rotation_offset.x = original_offset.x - rotation_offset.x;
            rotation_offset.y = original_offset.y - rotation_offset.y;

            rotation_offset.x /= 1024;
            rotation_offset.y /= 1024;

            error = FT_Glyph_Transform(glyph, &rotation_matrix, &rotation_offset);
        }

        error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_MONO, 0, 1);

        if(error != 0)
        {
            FT_Done_Glyph(glyph);
            return;
        }

        FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph) glyph;

        // Evidently, in FreeType2, you can only get "upside-down" bitmaps and
        // OpenGL won't invert a bitmap with PixelZoom, so we have to invert the
        // glyph's bitmap ourselves.

        GLubyte* inverted_bitmap = invertBitmap(bitmap_glyph->bitmap);

        glBitmap(bitmap_glyph->bitmap.width, bitmap_glyph->bitmap.rows,
                 (GLfloat) -bitmap_glyph->left,
                 (GLfloat) (bitmap_glyph->bitmap.rows - bitmap_glyph->top),
                 face->glyph->advance.x / 64.0f,
                 face->glyph->advance.y / 64.0f,
                 inverted_bitmap);

        FT_Done_Glyph(glyph);

        delete[] inverted_bitmap;
    }

} // close OGLFT namespace

