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

#include "OgreStableHeaders.h"

#if !OGRE_NO_JSON

#include "OgreHlmsJsonInk.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsTextureManager.h"
#include "OgreTextureManager.h"

#include "OgreLwString.h"

#include "rapidjson/document.h"

namespace Ogre
{
    extern const String c_pbsBlendModes[];

    const char* c_workflows[HlmsInkDatablock::MetallicWorkflow+1] =
    {
        "specular_ogre",
        "specular_fresnel",
        "metallic"
    };
    const char* c_transparencyModes[HlmsInkDatablock::Fade+1] =
    {
        "None",
        "Transparent",
        "Fade"
    };

    HlmsJsonInk::HlmsJsonInk( HlmsManager *hlmsManager ) :
        mHlmsManager( hlmsManager )
    {
    }
    //-----------------------------------------------------------------------------------
    HlmsInkDatablock::Workflows HlmsJsonInk::parseWorkflow( const char *value )
    {
        if( !strcmp( value, "specular_ogre" ) )
            return HlmsInkDatablock::SpecularWorkflow;
        if( !strcmp( value, "specular_fresnel" ) )
            return HlmsInkDatablock::SpecularAsFresnelWorkflow;
        if( !strcmp( value, "metallic" ) )
            return HlmsInkDatablock::MetallicWorkflow;

        return HlmsInkDatablock::SpecularWorkflow;
    }
    //-----------------------------------------------------------------------------------
    InkBrdf::InkBrdf HlmsJsonInk::parseBrdf( const char *value )
    {
        if( !strcmp( value, "default" ) )
            return InkBrdf::Default;
        if( !strcmp( value, "cook_torrance" ) )
            return InkBrdf::CookTorrance;
        if( !strcmp( value, "default_uncorrelated" ) )
            return InkBrdf::DefaultUncorrelated;
        if( !strcmp( value, "default_separate_diffuse_fresnel" ) )
            return InkBrdf::DefaultSeparateDiffuseFresnel;
        if( !strcmp( value, "cook_torrance_separate_diffuse_fresnel" ) )
            return InkBrdf::CookTorranceSeparateDiffuseFresnel;

        return InkBrdf::Default;
    }
    //-----------------------------------------------------------------------------------
    HlmsInkDatablock::TransparencyModes HlmsJsonInk::parseTransparencyMode( const char *value )
    {
        if( !strcmp( value, "None" ) )
            return HlmsInkDatablock::None;
        if( !strcmp( value, "Transparent" ) )
            return HlmsInkDatablock::Transparent;
        if( !strcmp( value, "Fade" ) )
            return HlmsInkDatablock::Fade;

        return HlmsInkDatablock::None;
    }
    //-----------------------------------------------------------------------------------
    InkBlendModes HlmsJsonInk::parseBlendMode( const char *value )
    {
        for( int i=0; i<NUM_INK_BLEND_MODES; ++i )
        {
            if( !strcmp( value, c_pbsBlendModes[i].c_str() ) )
                return static_cast<InkBlendModes>( i );
        }

        return INK_BLEND_NORMAL_NON_PREMUL;
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonInk::parseFresnelMode( const char *value, bool &outIsColoured, bool &outUseIOR )
    {
        if( !strcmp( value, "coeff" ) )
        {
            outUseIOR       = false;
            outIsColoured   = false;
        }
        else if( !strcmp( value, "ior" ) )
        {
            outUseIOR       = true;
            outIsColoured   = false;
        }
        else if( !strcmp( value, "coloured" ) )
        {
            outUseIOR       = false;
            outIsColoured   = true;
        }
        else if( !strcmp( value, "coloured_ior" ) )
        {
            outUseIOR       = true;
            outIsColoured   = true;
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonInk::parseOffset( const rapidjson::Value &jsonArray, Vector4 &offsetScale )
    {
        const rapidjson::SizeType arraySize = std::min( 2u, jsonArray.Size() );
        for( rapidjson::SizeType i=0; i<arraySize; ++i )
        {
            if( jsonArray[i].IsNumber() )
                offsetScale[i] = static_cast<float>( jsonArray[i].GetDouble() );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonInk::parseScale( const rapidjson::Value &jsonArray, Vector4 &offsetScale )
    {
        const rapidjson::SizeType arraySize = std::min( 2u, jsonArray.Size() );
        for( rapidjson::SizeType i=0; i<arraySize; ++i )
        {
            if( jsonArray[i].IsNumber() )
                offsetScale[i+2u] = static_cast<float>( jsonArray[i].GetDouble() );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonInk::loadTexture( const rapidjson::Value &json, const HlmsJson::NamedBlocks &blocks,
                                   InkTextureTypes textureType, HlmsInkDatablock *datablock,
                                   InkPackedTexture textures[] )
    {
        const HlmsTextureManager::TextureMapType texMapTypes[NUM_INK_TEXTURE_TYPES] =
        {
            HlmsTextureManager::TEXTURE_TYPE_DIFFUSE,
            HlmsTextureManager::TEXTURE_TYPE_NORMALS,
            HlmsTextureManager::TEXTURE_TYPE_DIFFUSE,
            HlmsTextureManager::TEXTURE_TYPE_MONOCHROME,
            HlmsTextureManager::TEXTURE_TYPE_NON_COLOR_DATA,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL_NORMAL_MAP,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL_NORMAL_MAP,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL_NORMAL_MAP,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL_NORMAL_MAP,
            HlmsTextureManager::TEXTURE_TYPE_ENV_MAP
        };

        rapidjson::Value::ConstMemberIterator itor = json.FindMember("texture");
        if( itor != json.MemberEnd() && itor->value.IsString() )
        {
            const char *textureName = itor->value.GetString();

            HlmsTextureManager *hlmsTextureManager = mHlmsManager->getTextureManager();
            HlmsTextureManager::TextureLocation texLocation = hlmsTextureManager->
                createOrRetrieveTexture(textureName,
                texMapTypes[textureType]);

            assert(texLocation.texture->isTextureTypeArray() || textureType == INK_REFLECTION);

            //If HLMS texture manager failed to find a reflection
            //texture, have look in standard texture manager.
            //NB we only do this for reflection textures as all other
            //textures must be texture arrays for performance reasons
            if( textureType == INK_REFLECTION &&
                texLocation.texture == hlmsTextureManager->getBlankTexture().texture )
            {
                Ogre::TexturePtr tex = Ogre::TextureManager::getSingleton().getByName(textureName);
                if (tex.isNull() == false)
                {
                    texLocation.texture = tex;
                    texLocation.xIdx = 0;
                    texLocation.yIdx = 0;
                    texLocation.divisor = 1;
                }
            }

            textures[textureType].texture = texLocation.texture;
            textures[textureType].xIdx = texLocation.xIdx;
        }

        itor = json.FindMember("sampler");
        if( itor != json.MemberEnd() && itor->value.IsString() )
        {
            map<LwConstString, const HlmsSamplerblock*>::type::const_iterator it =
                    blocks.samplerblocks.find( LwConstString::FromUnsafeCStr(itor->value.GetString()) );
            if( it != blocks.samplerblocks.end() )
            {
                textures[textureType].samplerblock = it->second;
                mHlmsManager->addReference( textures[textureType].samplerblock );
            }
        }

        itor = json.FindMember("uv");
        if( itor != json.MemberEnd() && itor->value.IsUint() )
        {
            unsigned uv = itor->value.GetUint();
            datablock->setTextureUvSource( textureType, static_cast<uint8>( uv ) );
        }
    }
    //-----------------------------------------------------------------------------------
    inline Vector3 HlmsJsonInk::parseVector3Array( const rapidjson::Value &jsonArray )
    {
        Vector3 retVal( Vector3::ZERO );

        const rapidjson::SizeType arraySize = std::min( 3u, jsonArray.Size() );
        for( rapidjson::SizeType i=0; i<arraySize; ++i )
        {
            if( jsonArray[i].IsNumber() )
                retVal[i] = static_cast<float>( jsonArray[i].GetDouble() );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    inline ColourValue HlmsJsonInk::parseColourValueArray( const rapidjson::Value &jsonArray,
                                                           const ColourValue &defaultValue )
    {
        ColourValue retVal( defaultValue );

        const rapidjson::SizeType arraySize = std::min( 4u, jsonArray.Size() );
        for( rapidjson::SizeType i=0; i<arraySize; ++i )
        {
            if( jsonArray[i].IsNumber() )
                retVal[i] = static_cast<float>( jsonArray[i].GetDouble() );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonInk::loadMaterial( const rapidjson::Value &json, const HlmsJson::NamedBlocks &blocks,
                                    HlmsDatablock *datablock )
    {
        assert( dynamic_cast<HlmsInkDatablock*>(datablock) );
        HlmsInkDatablock *pbsDatablock = static_cast<HlmsInkDatablock*>(datablock);

        rapidjson::Value::ConstMemberIterator itor = json.FindMember("workflow");
        if( itor != json.MemberEnd() && itor->value.IsString() )
            pbsDatablock->setWorkflow( parseWorkflow( itor->value.GetString() ) );

        itor = json.FindMember("brdf");
        if( itor != json.MemberEnd() && itor->value.IsString() )
            pbsDatablock->setBrdf( parseBrdf(itor->value.GetString()) );

        itor = json.FindMember("two_sided");
        if( itor != json.MemberEnd() && itor->value.IsBool() )
        {
            pbsDatablock->setTwoSidedLighting( itor->value.GetBool(), true,
                                               pbsDatablock->getMacroblock(true)->mCullMode );
        }

        itor = json.FindMember("transparency");
        if( itor != json.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &subobj = itor->value;

            float transparencyValue = pbsDatablock->getTransparency();
            HlmsInkDatablock::TransparencyModes transpMode = pbsDatablock->getTransparencyMode();
            bool useAlphaFromTextures = pbsDatablock->getUseAlphaFromTextures();

            itor = subobj.FindMember( "value" );
            if( itor != subobj.MemberEnd() && itor->value.IsNumber() )
                transparencyValue = static_cast<float>( itor->value.GetDouble() );

            itor = subobj.FindMember( "mode" );
            if( itor != subobj.MemberEnd() && itor->value.IsString() )
                transpMode = parseTransparencyMode( itor->value.GetString() );

            itor = subobj.FindMember( "use_alpha_from_textures" );
            if( itor != subobj.MemberEnd() && itor->value.IsBool() )
                useAlphaFromTextures = itor->value.GetBool();

            const bool changeBlendblock = !json.HasMember( "blendblock" );
            pbsDatablock->setTransparency( transparencyValue, transpMode,
                                           useAlphaFromTextures, changeBlendblock );
        }

        InkPackedTexture packedTextures[NUM_INK_TEXTURE_TYPES];

        itor = json.FindMember("diffuse");
        if( itor != json.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &subobj = itor->value;
            loadTexture( subobj, blocks, INK_DIFFUSE, pbsDatablock, packedTextures );

            itor = subobj.FindMember( "value" );
            if( itor != subobj.MemberEnd() && itor->value.IsArray() )
                pbsDatablock->setDiffuse( parseVector3Array( itor->value ) );

            itor = subobj.FindMember( "background" );
            if( itor != subobj.MemberEnd() && itor->value.IsArray() )
                pbsDatablock->setBackgroundDiffuse( parseColourValueArray( itor->value ) );
        }

        itor = json.FindMember("specular");
        if( itor != json.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &subobj = itor->value;
            loadTexture( subobj, blocks, INK_SPECULAR, pbsDatablock, packedTextures );

            itor = subobj.FindMember( "value" );
            if( itor != subobj.MemberEnd() && itor->value.IsArray() )
                pbsDatablock->setSpecular( parseVector3Array( itor->value ) );
        }

        itor = json.FindMember("roughness");
        if( itor != json.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &subobj = itor->value;
            loadTexture( subobj, blocks, INK_ROUGHNESS, pbsDatablock, packedTextures );

            itor = subobj.FindMember( "value" );
            if( itor != subobj.MemberEnd() && itor->value.IsNumber() )
                pbsDatablock->setRoughness( static_cast<float>( itor->value.GetDouble() ) );
        }

        itor = json.FindMember("fresnel");
        if( itor != json.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &subobj = itor->value;
            loadTexture( subobj, blocks, INK_SPECULAR, pbsDatablock, packedTextures );

            bool useIOR = false;
            bool isColoured = false;
            itor = subobj.FindMember( "mode" );
            if( itor != subobj.MemberEnd() && itor->value.IsString() )
                parseFresnelMode( itor->value.GetString(), isColoured, useIOR );

            itor = subobj.FindMember( "value" );
            if( itor != subobj.MemberEnd() && (itor->value.IsArray() || itor->value.IsNumber()) )
            {
                Vector3 value;
                if( itor->value.IsArray() )
                    value = parseVector3Array( itor->value );
                else
                    value = static_cast<Real>( itor->value.GetDouble() );

                if( !useIOR )
                    pbsDatablock->setFresnel( value, isColoured );
                else
                    pbsDatablock->setIndexOfRefraction( value, isColoured );
            }
        }

        itor = json.FindMember("metallness");
        if( itor != json.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &subobj = itor->value;
            loadTexture( subobj, blocks, INK_METALLIC, pbsDatablock, packedTextures );

            itor = subobj.FindMember( "value" );
            if( itor != subobj.MemberEnd() && itor->value.IsNumber() )
                pbsDatablock->setMetallness( static_cast<float>( itor->value.GetDouble() ) );
        }

        itor = json.FindMember("normal");
        if( itor != json.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &subobj = itor->value;
            loadTexture( subobj, blocks, INK_NORMAL, pbsDatablock, packedTextures );

            itor = subobj.FindMember( "value" );
            if( itor != subobj.MemberEnd() && itor->value.IsNumber() )
                pbsDatablock->setNormalMapWeight( static_cast<float>( itor->value.GetDouble() ) );
        }

        itor = json.FindMember("detail_weight");
        if( itor != json.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &subobj = itor->value;
            loadTexture( subobj, blocks, INK_DETAIL_WEIGHT, pbsDatablock, packedTextures );
        }

        for( int i=0; i<4; ++i )
        {
            const String iAsStr = StringConverter::toString(i);
            String texTypeName = "detail_diffuse" + iAsStr;

            itor = json.FindMember(texTypeName.c_str());
            if( itor != json.MemberEnd() && itor->value.IsObject() )
            {
                const rapidjson::Value &subobj = itor->value;
                loadTexture( subobj, blocks, static_cast<InkTextureTypes>(INK_DETAIL0 + i),
                             pbsDatablock, packedTextures );

                itor = subobj.FindMember( "value" );
                if( itor != subobj.MemberEnd() && itor->value.IsNumber() )
                    pbsDatablock->setDetailMapWeight( i, static_cast<float>( itor->value.GetDouble() ) );

                itor = subobj.FindMember( "mode" );
                if( itor != subobj.MemberEnd() && itor->value.IsString() )
                    pbsDatablock->setDetailMapBlendMode( i, parseBlendMode( itor->value.GetString() ) );

                Vector4 offsetScale( 0, 0, 1, 1 );

                itor = subobj.FindMember( "offset" );
                if( itor != subobj.MemberEnd() && itor->value.IsArray() )
                    parseOffset( itor->value, offsetScale );

                itor = subobj.FindMember( "scale" );
                if( itor != subobj.MemberEnd() && itor->value.IsArray() )
                    parseScale( itor->value, offsetScale );

                pbsDatablock->setDetailMapOffsetScale( i, offsetScale );
            }

            texTypeName = "detail_normal" + iAsStr;
            itor = json.FindMember(texTypeName.c_str());
            if( itor != json.MemberEnd() && itor->value.IsObject() )
            {
                const rapidjson::Value &subobj = itor->value;
                loadTexture( subobj, blocks, static_cast<InkTextureTypes>(INK_DETAIL0_NM + i),
                             pbsDatablock, packedTextures );

                itor = subobj.FindMember( "value" );
                if( itor != subobj.MemberEnd() && itor->value.IsNumber() )
                {
                    pbsDatablock->setDetailNormalWeight( i,
                                                         static_cast<float>( itor->value.GetDouble() ) );
                }

                Vector4 offsetScale( 0, 0, 1, 1 );

                itor = subobj.FindMember( "offset" );
                if( itor != subobj.MemberEnd() && itor->value.IsArray() )
                    parseOffset( itor->value, offsetScale );

                itor = subobj.FindMember( "scale" );
                if( itor != subobj.MemberEnd() && itor->value.IsArray() )
                    parseScale( itor->value, offsetScale );

                pbsDatablock->setDetailMapOffsetScale( i + 4, offsetScale );
            }
        }

        itor = json.FindMember("reflection");
        if( itor != json.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &subobj = itor->value;
            loadTexture( subobj, blocks, INK_REFLECTION, pbsDatablock, packedTextures );
        }

        pbsDatablock->_setTextures( packedTextures );
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonInk::toQuotedStr( HlmsInkDatablock::Workflows value, String &outString )
    {
        outString += '"';
        outString += c_workflows[value];
        outString += '"';
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonInk::toQuotedStr( uint32 value, String &outString )
    {
        outString += '"';
        switch( value )
        {
        case InkBrdf::Default:
            outString += "default";
            break;
        case InkBrdf::CookTorrance:
            outString += "cook_torrance";
            break;
        case InkBrdf::DefaultUncorrelated:
            outString += "default_uncorrelated";
            break;
        case InkBrdf::DefaultSeparateDiffuseFresnel:
            outString += "default_separate_diffuse_fresnel";
            break;
        case InkBrdf::CookTorranceSeparateDiffuseFresnel:
            outString += "cook_torrance_separate_diffuse_fresnel";
            break;
        default:
            outString += "unknown / custom";
            break;
        }
        outString += '"';
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonInk::toQuotedStr( HlmsInkDatablock::TransparencyModes value, String &outString )
    {
        outString += '"';
        outString += c_transparencyModes[value];
        outString += '"';
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonInk::saveFresnel( const HlmsInkDatablock *datablock, String &outString )
    {
        saveTexture( datablock->getFresnel(), ColourValue::ZERO, "fresnel", INK_SPECULAR,
                     true, false, true, true,
                     datablock->getWorkflow() == HlmsInkDatablock::SpecularAsFresnelWorkflow,
                     datablock, outString );
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonInk::saveTexture( const char *blockName,
                                   InkTextureTypes textureType,
                                   const HlmsInkDatablock *datablock, String &outString,
                                   bool writeTexture )
    {
        saveTexture( Vector3(0.0f), ColourValue::ZERO, blockName, textureType,
                     false, false, false, false, writeTexture, datablock, outString);
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonInk::saveTexture( float value, const char *blockName,
                                   InkTextureTypes textureType,
                                   const HlmsInkDatablock *datablock, String &outString,
                                   bool writeTexture )
    {
        saveTexture( Vector3(value), ColourValue::ZERO, blockName, textureType,
                     true, false, true, false, writeTexture, datablock, outString);
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonInk::saveTexture(const Vector3 &value, const char *blockName,
                                   InkTextureTypes textureType,
                                   const HlmsInkDatablock *datablock, String &outString,
                                   bool writeTexture, const ColourValue &bgColour )
    {
        const bool writeBgDiffuse = textureType == INK_DIFFUSE;
        saveTexture( value, bgColour, blockName, textureType,
                     true, writeBgDiffuse, false, false, writeTexture,
                     datablock, outString );
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonInk::saveTexture( const Vector3 &value, const ColourValue &bgDiffuse,
                                   const char *blockName, InkTextureTypes textureType,
                                   bool writeValue, bool writeBgDiffuse, bool scalarValue,
                                   bool isFresnel, bool writeTexture,
                                   const HlmsInkDatablock *datablock, String &outString )
    {
        outString += ",\n\t\t\t\"";
        outString += blockName;
        outString += "\" :\n\t\t\t{\n";

        const size_t currentOffset = outString.size();

        if( isFresnel )
            scalarValue = !datablock->hasSeparateFresnel();

        if( writeValue )
        {
            outString += "\t\t\t\t\"value\" : ";
            if( scalarValue )
                outString += StringConverter::toString( value.x );
            else
            {
                HlmsJson::toStr( value, outString );
            }
        }

        if( writeBgDiffuse )
        {
            outString += ",\n\t\t\t\t\"background\" : ";
            HlmsJson::toStr( bgDiffuse, outString );
        }

        if( isFresnel )
        {
            if( datablock->hasSeparateFresnel() )
                outString += ",\n\t\t\t\t\"mode\" : \"coloured\"";
            else
                outString += ",\n\t\t\t\t\"mode\" : \"coeff\"";
        }

        if( textureType >= INK_DETAIL0 && textureType <= INK_DETAIL3_NM )
        {
            if( textureType >= INK_DETAIL0 && textureType <= INK_DETAIL3 )
            {
                InkBlendModes blendMode = datablock->getDetailMapBlendMode( textureType - INK_DETAIL0 );

                if( blendMode != INK_BLEND_NORMAL_NON_PREMUL )
                {
                    outString += ",\n\t\t\t\t\"mode\" : \"";
                    outString += c_pbsBlendModes[blendMode];
                    outString += '"';
                }
            }

            const Vector4 &offsetScale =
                    datablock->getDetailMapOffsetScale( textureType - INK_DETAIL0 );
            const Vector2 offset( offsetScale.x, offsetScale.y );
            const Vector2 scale( offsetScale.z, offsetScale.w );

            if( offset != Vector2::ZERO )
            {
                outString += ",\n\t\t\t\t\"offset\" : ";
                HlmsJson::toStr( offset, outString );
            }

            if( scale != Vector2::UNIT_SCALE )
            {
                outString += ",\n\t\t\t\t\"scale\" : ";
                HlmsJson::toStr( scale, outString );
            }
        }

        if( writeTexture )
        {
            HlmsTextureManager::TextureLocation texLocation;
            texLocation.texture = datablock->getTexture( textureType );
            if( !texLocation.texture.isNull() )
            {
                texLocation.xIdx = datablock->_getTextureIdx( textureType );
                texLocation.yIdx = 0;
                texLocation.divisor = 1;

                const String *texName = mHlmsManager->getTextureManager()->findAliasName( texLocation );

                if( texName )
                {
                    outString += ",\n\t\t\t\t\"texture\" : \"";
                    outString += *texName;
                    outString += '"';
                }
            }

            const HlmsSamplerblock *samplerblock = datablock->getSamplerblock( textureType );
            if( samplerblock )
            {
                outString += ",\n\t\t\t\t\"sampler\" : ";
                outString += HlmsJson::getName( samplerblock );
            }

            if( textureType < NUM_INK_SOURCES && datablock->getTextureUvSource( textureType ) != 0 )
            {
                outString += ",\n\t\t\t\t\"uv\" : ";
                outString += StringConverter::toString( datablock->getTextureUvSource( textureType ) );
            }
        }

        if( !writeValue && outString.size() != currentOffset )
        {
            //Remove an extra comma and newline characters.
            outString.erase( currentOffset, 2 );
        }

        outString += "\n\t\t\t}";
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonInk::saveMaterial( const HlmsDatablock *datablock, String &outString )
    {
        assert( dynamic_cast<const HlmsInkDatablock*>(datablock) );
        const HlmsInkDatablock *pbsDatablock = static_cast<const HlmsInkDatablock*>(datablock);

        outString += ",\n\t\t\t\"workflow\" : ";
        toQuotedStr( pbsDatablock->getWorkflow(), outString );

        if( pbsDatablock->getBrdf() != InkBrdf::Default )
        {
            outString += ",\n\t\t\t\"brdf\" : ";
            toQuotedStr( pbsDatablock->getBrdf(), outString );
        }

        if( pbsDatablock->getTwoSidedLighting() )
            outString += ",\n\t\t\t\"two_sided\" : true";

        if( pbsDatablock->getTransparencyMode() != HlmsInkDatablock::None )
        {
            outString += ",\n\t\t\t\"transparency\" :\n\t\t\t{";
            outString += "\n\t\t\t\t\"value\" : ";
            outString += StringConverter::toString( pbsDatablock->getTransparency() );
            outString += ",\n\t\t\t\t\"mode\" : ";
            toQuotedStr( pbsDatablock->getTransparencyMode(), outString );
            outString += ",\n\t\t\t\t\"use_alpha_from_textures\" : ";
            outString += pbsDatablock->getUseAlphaFromTextures() ? "true" : "false";
            outString += "\n\t\t\t}";
        }

        saveTexture( pbsDatablock->getDiffuse(),  "diffuse", INK_DIFFUSE,
                     pbsDatablock, outString, true, pbsDatablock->getBackgroundDiffuse() );
        saveTexture( pbsDatablock->getSpecular(), "specular", INK_SPECULAR,
                     pbsDatablock, outString,
                     pbsDatablock->getWorkflow() == HlmsInkDatablock::SpecularWorkflow );
        if( pbsDatablock->getWorkflow() != HlmsInkDatablock::MetallicWorkflow )
        {
            saveFresnel( pbsDatablock, outString );
        }
        else
        {
            saveTexture( pbsDatablock->getMetallness(), "metallness", INK_METALLIC,
                         pbsDatablock, outString );
        }

        if( pbsDatablock->getNormalMapWeight() != 1.0f ||
            !pbsDatablock->getTexture( INK_NORMAL ).isNull() )
        {
            saveTexture( pbsDatablock->getNormalMapWeight(), "normal", INK_NORMAL,
                         pbsDatablock, outString );
        }

        saveTexture( pbsDatablock->getRoughness(), "roughness", INK_ROUGHNESS,
                     pbsDatablock, outString );

        if( !pbsDatablock->getTexture( INK_DETAIL_WEIGHT ).isNull() )
            saveTexture( "detail_weight", INK_DETAIL_WEIGHT, pbsDatablock, outString );

        for( int i=0; i<4; ++i )
        {
            InkBlendModes blendMode = pbsDatablock->getDetailMapBlendMode( i );
            const Vector4 &offsetScale = pbsDatablock->getDetailMapOffsetScale( i );
            const Vector2 offset( offsetScale.x, offsetScale.y );
            const Vector2 scale( offsetScale.z, offsetScale.w );

            const InkTextureTypes textureType = static_cast<InkTextureTypes>(INK_DETAIL0 + i);

            if( blendMode != INK_BLEND_NORMAL_NON_PREMUL || offset != Vector2::ZERO ||
                scale != Vector2::UNIT_SCALE || pbsDatablock->getDetailMapWeight( i ) != 1.0f ||
                !pbsDatablock->getTexture( textureType ).isNull() )
            {
                char tmpBuffer[64];
                LwString blockName( LwString::FromEmptyPointer( tmpBuffer, sizeof(tmpBuffer) ) );

                blockName.a( "detail_diffuse", i );

                saveTexture( pbsDatablock->getDetailMapWeight( i ), blockName.c_str(),
                             static_cast<InkTextureTypes>(INK_DETAIL0 + i), pbsDatablock,
                             outString );
            }
        }

        for( int i=0; i<4; ++i )
        {
            const Vector4 &offsetScale = pbsDatablock->getDetailMapOffsetScale( i + 4 );
            const Vector2 offset( offsetScale.x, offsetScale.y );
            const Vector2 scale( offsetScale.z, offsetScale.w );

            const InkTextureTypes textureType = static_cast<InkTextureTypes>(INK_DETAIL0_NM + i);

            if( offset != Vector2::ZERO || scale != Vector2::UNIT_SCALE ||
                pbsDatablock->getDetailNormalWeight( i ) != 1.0f ||
                !pbsDatablock->getTexture( textureType ).isNull() )
            {
                char tmpBuffer[64];
                LwString blockName( LwString::FromEmptyPointer( tmpBuffer, sizeof(tmpBuffer) ) );

                blockName.a( "detail_normal", i );
                saveTexture( pbsDatablock->getDetailNormalWeight( i ), blockName.c_str(),
                             static_cast<InkTextureTypes>(INK_DETAIL0_NM + i), pbsDatablock,
                             outString );
            }
        }

        if( !pbsDatablock->getTexture( INK_REFLECTION ).isNull() )
            saveTexture( "reflection", INK_REFLECTION, pbsDatablock, outString );
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonInk::collectSamplerblocks( const HlmsDatablock *datablock,
                                            set<const HlmsSamplerblock*>::type &outSamplerblocks )
    {
        assert( dynamic_cast<const HlmsInkDatablock*>(datablock) );
        const HlmsInkDatablock *pbsDatablock = static_cast<const HlmsInkDatablock*>(datablock);

        for( int i=0; i<NUM_INK_TEXTURE_TYPES; ++i )
        {
            const InkTextureTypes textureType = static_cast<InkTextureTypes>( i );
            const HlmsSamplerblock *samplerblock = pbsDatablock->getSamplerblock( textureType );
            if( samplerblock )
                outSamplerblocks.insert( samplerblock );
        }
    }
}

#endif
