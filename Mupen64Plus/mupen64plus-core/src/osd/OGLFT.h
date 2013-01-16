// -*- c++ -*-
/*
 * OGLFT: A library for drawing text with OpenGL using the FreeType library
 * Copyright (C) 2002 lignum Computing, Inc. <oglft@lignumcomputing.com>
 * $Id: OGLFT.h,v 1.15 2003/10/01 14:41:09 allen Exp $
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

#ifndef OGLFT_H
#define OGLFT_H

#include <cmath>
#include <map>
#include <list>
#include <vector>
#include <wchar.h>

#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#if defined(__MACOSX__)
#include <OpenGL/glu.h>
#elif defined(__MACOS__)
#include <glu.h>
#else
#include <GL/glu.h>
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_TRIGONOMETRY_H

namespace OGLFT 
{
    enum Coordinates 
    {
        X, Y, Z, W
    };

    enum ColorSpace 
    {
        R, G, B, A
    };

    // global library functions
    bool Init_FT(void);
    bool Uninit_FT(void);

    struct Advance 
    {
        float dx_;
        float dy_;

        Advance ( float dx = 0, float dy = 0 ) : dx_( dx ), dy_( dy )
        {
            return;
        }

        Advance ( FT_Vector v )
        {
            dx_ = (float) (v.x / 64.);
            dy_ = (float) (v.y / 64.);
        }

        Advance& operator+= ( const FT_Vector v )
        {
            dx_ += (float) (v.x / 64.);
            dy_ += (float) (v.y / 64.);
            return *this;
        }
    };

    struct BBox 
    {
        float x_min_;
        float y_min_;
        float x_max_;
        float y_max_;
        Advance advance_;

        BBox () : x_min_( 0 ), y_min_( 0 ), x_max_( 0 ), y_max_( 0 )
        {
            return;
        }
        BBox ( FT_BBox ft_bbox )
        {
            x_min_ = (float) (ft_bbox.xMin / 64.);
            y_min_ = (float) (ft_bbox.yMin / 64.);
            x_max_ = (float) (ft_bbox.xMax / 64.);
            y_max_ = (float) (ft_bbox.yMax / 64.);
        }
        BBox& operator*= ( double k )
        {
            x_min_ *= (float) k;
            y_min_ *= (float) k;
            x_max_ *= (float) k;
            y_max_ *= (float) k;
            advance_.dx_ *= (float) k;
            advance_.dy_ *= (float) k;

            return *this;
        }
        BBox& operator+= ( const BBox& b )
        {
            float new_value;

            new_value = b.x_min_ + advance_.dx_;
            if ( new_value < x_min_ ) x_min_ = new_value;
            new_value = b.y_min_ + advance_.dy_;
            if ( new_value < y_min_ ) y_min_ = new_value;
            new_value = b.x_max_ + advance_.dx_;
            if ( new_value > x_max_ ) x_max_ = new_value;
            new_value = b.y_max_ + advance_.dy_;
            if ( new_value > y_max_ ) y_max_ = new_value;

            advance_.dx_ += b.advance_.dx_;
            advance_.dy_ += b.advance_.dy_;

            return *this;
        }
    };
    typedef std::vector<GLuint> DisplayLists;
    typedef DisplayLists::const_iterator DLCI;
    typedef DisplayLists::iterator DLI;
    class Face 
    {
        public:
            enum HorizontalJustification 
            {
                LEFT,
                ORIGIN,
                CENTER,
                RIGHT
            };

            enum VerticalJustification 
            {
                BOTTOM,
                BASELINE,
                MIDDLE,
                TOP
            };
            
            enum GlyphCompileMode 
            {
                COMPILE,
                IMMEDIATE
            };

        private:
            struct FaceData 
            {
                FT_Face face_;
                bool free_on_exit_;
                FaceData ( FT_Face face, bool free_on_exit = true )
                    : face_( face ), free_on_exit_( free_on_exit )
                {
                    return;
                }
            };
            
        protected:
            std::vector< FaceData > faces_;
            bool valid_;
            enum GlyphCompileMode compile_mode_;
            float point_size_;
            FT_UInt resolution_;
            bool advance_;
            GLfloat foreground_color_[4];
            GLfloat background_color_[4];
            enum HorizontalJustification horizontal_justification_;
            enum VerticalJustification vertical_justification_;
            GLfloat string_rotation_;
            FT_UInt rotation_reference_glyph_;
            FT_Face rotation_reference_face_;
            GLfloat rotation_offset_y_;
            typedef std::map< FT_UInt, GLuint > GlyphDLists;
            typedef GlyphDLists::const_iterator GDLCI;
            typedef GlyphDLists::iterator GDLI;
            GlyphDLists glyph_dlists_;
            DisplayLists character_display_lists_;

        public:
            Face ( const char* filename, float point_size = 12, FT_UInt resolution = 100 );
            Face ( FT_Face face, float point_size = 12, FT_UInt resolution = 100 );
            virtual ~Face ( void );
            bool isValid ( void ) const { return valid_; }
            bool addAuxiliaryFace ( const char* filename );
            bool addAuxiliaryFace ( FT_Face face );
            void setCompileMode ( enum GlyphCompileMode compile_mode ) { compile_mode_ = compile_mode; }
            enum GlyphCompileMode compileMode ( void ) const { return compile_mode_; }
            void setPointSize ( float point_size );
            float pointSize ( void ) { return point_size_; }
            void setResolution ( FT_UInt resolution );
            FT_UInt resolution ( void ) { return resolution_; }
            void setAdvance ( bool advance ) { advance_ = advance; }
            bool advance ( void ) const { return advance_; }
            void setForegroundColor ( GLfloat red = 0.0, GLfloat green = 0.0, GLfloat blue = 0.0, GLfloat alpha = 1.0 );
            void setForegroundColor ( const GLfloat foreground_color[4] );
            GLfloat foregroundRed ( void ) const { return foreground_color_[R]; }
            GLfloat foregroundGreen ( void ) const { return foreground_color_[G]; }
            GLfloat foregroundBlue ( void ) const { return foreground_color_[B]; }
            GLfloat foregroundAlpha ( void ) const { return foreground_color_[A]; }
            void setBackgroundColor ( GLfloat red = 1.0, GLfloat green = 1.0, GLfloat blue = 1.0, GLfloat alpha = 0.0 );
            void setBackgroundColor ( const GLfloat background_color[4] );
            GLfloat backgroundRed ( void ) const { return background_color_[R]; }
            GLfloat backgroundGreen ( void ) const { return background_color_[G]; }
            GLfloat backgroundBlue ( void ) const { return background_color_[B]; }
            GLfloat backgroundAlpha ( void ) const { return background_color_[A]; }
            virtual void setCharacterRotationZ ( GLfloat character_rotation_z ) = 0;
            virtual GLfloat characterRotationZ ( void ) const = 0;
            void setCharacterRotationReference ( unsigned char c );
            void setStringRotation ( GLfloat string_rotation );
            GLfloat stringRotation ( void ) const { return string_rotation_; }
            void setHorizontalJustification ( enum HorizontalJustification horizontal_justification )
            {
                horizontal_justification_ = horizontal_justification;
            }
            enum HorizontalJustification horizontalJustification ( void ) const { return horizontal_justification_; }
            void setVerticalJustification ( enum VerticalJustification vertical_justification )
            {
                vertical_justification_ = vertical_justification;
            }

            enum VerticalJustification verticaljustification ( void ) const { return vertical_justification_; }
            void setCharacterDisplayLists ( const DisplayLists& character_display_lists ) { character_display_lists_ = character_display_lists; }
            DisplayLists& characterDisplayLists ( void ) { return character_display_lists_; }
            virtual double height ( void ) const = 0;
            virtual BBox measure ( unsigned char c ) = 0;
            virtual BBox measure ( wchar_t c ) = 0;
            virtual BBox measure ( const char* s );
            virtual BBox measureRaw ( const char* s );
            virtual BBox measure ( const wchar_t* s );
            virtual BBox measure ( const wchar_t* format, double number );
            virtual BBox measureRaw ( const wchar_t* s );
            GLuint compile ( unsigned char c );
            GLuint compile ( const wchar_t c );
            void draw ( const char* s );
            void draw ( const wchar_t* s );
            void draw ( unsigned char c );
            void draw ( const wchar_t c );
            void draw ( GLfloat x, GLfloat y, unsigned char c );
            void draw ( GLfloat x, GLfloat y, GLfloat z, unsigned char c );
            void draw ( GLfloat x, GLfloat y, wchar_t c );
            void draw ( GLfloat x, GLfloat y, GLfloat z, wchar_t c );
            void draw ( GLfloat x, GLfloat y, const char* s, float *sizebox );
            void draw ( GLfloat x, GLfloat y, GLfloat z, const char* s );
            void draw ( GLfloat x, GLfloat y, const wchar_t* s );
            void draw ( GLfloat x, GLfloat y, GLfloat z, const wchar_t* s );
            int ascender ( void ) { return faces_.front().face_->ascender; }
            int descender ( void ) { return faces_.front().face_->descender; }
            BBox measure_nominal ( const char* s );
            BBox measure_nominal ( const wchar_t* s );

        protected:
            virtual GLuint compileGlyph ( FT_Face face, FT_UInt glyph_index ) = 0;
            virtual void renderGlyph ( FT_Face face, FT_UInt glyph_index ) = 0;
            virtual void setCharSize ( void ) = 0;
            virtual void clearCaches ( void ) = 0;
            virtual void setRotationOffset ( void ) = 0;

        private:
            void init ( void );
    };

    class Raster : public Face 
    {
        protected:
            GLfloat character_rotation_z_;
        public:
            Raster ( const char* filename, float point_size = 12, FT_UInt resolution = 100 );
            Raster ( FT_Face face, float point_size = 12, FT_UInt resolution = 100 );
            virtual ~Raster ( void );
            void setCharacterRotationZ ( GLfloat character_rotation_z );
            GLfloat characterRotationZ ( void ) const { return character_rotation_z_; }
            double height ( void ) const;
            BBox measure ( unsigned char c );
            BBox measure ( wchar_t c );
            BBox measure ( const char* s ) { return Face::measure( s ); }
            BBox measure ( const wchar_t* format, double number ) { return Face::measure( format, number ); }

        private:
            void init ( void );
            GLuint compileGlyph ( FT_Face face, FT_UInt glyph_index );
            void setCharSize ( void );
            void setRotationOffset ( void );
            void clearCaches ( void );
    };
    class Monochrome : public Raster
    {
        public:
            Monochrome ( const char* filename, float point_size = 12, FT_UInt resolution = 100 );
            Monochrome ( FT_Face face, float point_size = 12, FT_UInt resolution = 100 );
            ~Monochrome ( void );

        private:
            GLubyte* invertBitmap ( const FT_Bitmap& bitmap );
            void renderGlyph ( FT_Face face, FT_UInt glyph_index );
    };

}
#endif /* OGLFT_H */

