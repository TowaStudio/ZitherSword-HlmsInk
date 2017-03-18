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

#if !OGRE_NO_JSON
#ifndef _OgreHlmsJsonInk_H_
#define _OgreHlmsJsonInk_H_

#include "OgreHlmsInkPrerequisites.h"
#include "OgreHlmsJson.h"
#include "OgreHlmsInkDatablock.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Component
    *  @{
    */
    /** \addtogroup Material
    *  @{
    */

    class _OgreHlmsInkExport HlmsJsonInk
    {
        HlmsManager *mHlmsManager;

        static HlmsInkDatablock::Workflows parseWorkflow( const char *value );
        static InkBrdf::InkBrdf parseBrdf( const char *value );
        static HlmsInkDatablock::TransparencyModes parseTransparencyMode( const char *value );
        static InkBlendModes parseBlendMode( const char *value );
        static void parseFresnelMode( const char *value, bool &outIsColoured, bool &outUseIOR );
        static void parseOffset( const rapidjson::Value &jsonArray, Vector4 &offsetScale );
        static void parseScale( const rapidjson::Value &jsonArray, Vector4 &offsetScale );

        static inline Vector3 parseVector3Array( const rapidjson::Value &jsonArray );
        static inline ColourValue parseColourValueArray(
                const rapidjson::Value &jsonArray,
                const ColourValue &defaultValue = ColourValue::White );

        void loadTexture( const rapidjson::Value &json, const HlmsJson::NamedBlocks &blocks,
                          InkTextureTypes textureType, HlmsInkDatablock *datablock,
                          InkPackedTexture textures[NUM_INK_TEXTURE_TYPES] );

        static void toQuotedStr( HlmsInkDatablock::Workflows value, String &outString );
        static void toQuotedStr( uint32 value, String &outString );
        static void toQuotedStr( HlmsInkDatablock::TransparencyModes value, String &outString );

        void saveFresnel( const HlmsInkDatablock *datablock, String &outString );
        void saveTexture( const char *blockName,
                          InkTextureTypes textureType,
                          const HlmsInkDatablock *datablock, String &outString,
                          bool writeTexture=true );
        void saveTexture( float value, const char *blockName,
                          InkTextureTypes textureType,
                          const HlmsInkDatablock *datablock, String &outString,
                          bool writeTexture=true );
        void saveTexture( const Vector3 &value, const char *blockName,
                          InkTextureTypes textureType,
                          const HlmsInkDatablock *datablock, String &outString,
                          bool writeTexture=true, const ColourValue &bgColour=ColourValue::ZERO );

        void saveTexture( const Vector3 &value, const ColourValue &bgDiffuse, const char *blockName,
                          InkTextureTypes textureType,
                          bool writeValue, bool writeBgDiffuse, bool scalarValue,
                          bool isFresnel, bool writeTexture,
                          const HlmsInkDatablock *datablock, String &outString );

    public:
        HlmsJsonInk( HlmsManager *hlmsManager );

        void loadMaterial( const rapidjson::Value &json, const HlmsJson::NamedBlocks &blocks,
                           HlmsDatablock *datablock );
        void saveMaterial( const HlmsDatablock *datablock, String &outString );

        static void collectSamplerblocks( const HlmsDatablock *datablock,
                                          set<const HlmsSamplerblock*>::type &outSamplerblocks );
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif

#endif
