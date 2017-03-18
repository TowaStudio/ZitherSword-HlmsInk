/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#ifndef _OgreHlmsInkPrerequisites_H_
#define _OgreHlmsInkPrerequisites_H_

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
#   if defined( OGRE_STATIC_LIB ) || defined( OGRE_INK__STATIC_LIB )
#       define _OgreHlmsInkExport
#   else
#       if defined( OgreHlmsInk_EXPORTS )
#           define _OgreHlmsInkExport __declspec( dllexport )
#       else
#           if defined( __MINGW32__ )
#               define _OgreHlmsInkExport
#           else
#               define _OgreHlmsInkExport __declspec( dllimport )
#           endif
#       endif
#   endif
#elif defined ( OGRE_GCC_VISIBILITY )
#   define _OgreHlmsInkExport __attribute__ ((visibility("default")))
#else
#   define _OgreHlmsInkExport
#endif 

namespace Ogre
{
    enum InkTextureTypes
    {
        INK_DIFFUSE,
        INK_NORMAL,
        INK_SPECULAR,
        INK_METALLIC = INK_SPECULAR,
        INK_ROUGHNESS,
        INK_DETAIL_WEIGHT,
        INK_DETAIL0,
        INK_DETAIL1,
        INK_DETAIL2,
        INK_DETAIL3,
        INK_DETAIL0_NM,
        INK_DETAIL1_NM,
        INK_DETAIL2_NM,
        INK_DETAIL3_NM,
        INK_REFLECTION,
        NUM_INK_SOURCES = INK_REFLECTION,
        NUM_INK_TEXTURE_TYPES
    };

    enum InkBlendModes
    {
        /// Regular alpha blending
		INK_BLEND_NORMAL_NON_PREMUL,
        /// Premultiplied alpha blending
        INK_BLEND_NORMAL_PREMUL,
        INK_BLEND_ADD,
        INK_BLEND_SUBTRACT,
        INK_BLEND_MULTIPLY,
        INK_BLEND_MULTIPLY2X,
        INK_BLEND_SCREEN,
        INK_BLEND_OVERLAY,
        INK_BLEND_LIGHTEN,
        INK_BLEND_DARKEN,
        INK_BLEND_GRAIN_EXTRACT,
        INK_BLEND_GRAIN_MERGE,
        INK_BLEND_DIFFERENCE,
        NUM_INK_BLEND_MODES
    };

    class CubemapProbe;
    class HlmsInk;
    class IrradianceVolume;
    class ParallaxCorrectedCubemap;
}

#endif
