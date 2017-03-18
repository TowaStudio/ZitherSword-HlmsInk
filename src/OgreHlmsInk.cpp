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

#include "OgreHlmsInk.h"
#include "OgreHlmsInkDatablock.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsListener.h"
#include "OgreLwString.h"

#if !OGRE_NO_JSON
    #include "OgreHlmsJsonInk.h"
#endif

#include "OgreViewport.h"
#include "OgreRenderTarget.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreForward3D.h"
#include "Cubemaps/OgreParallaxCorrectedCubemap.h"
#include "OgreIrradianceVolume.h"

#include "OgreSceneManager.h"
#include "Compositor/OgreCompositorShadowNode.h"
#include "Vao/OgreVaoManager.h"
#include "Vao/OgreConstBufferPacked.h"
#include "Vao/OgreTexBufferPacked.h"

#include "CommandBuffer/OgreCommandBuffer.h"
#include "CommandBuffer/OgreCbTexture.h"
#include "CommandBuffer/OgreCbShaderBuffer.h"

#include "Animation/OgreSkeletonInstance.h"

#include "RenderSystems/Direct3D11/include/OgreD3D11HlmsPso.h";

namespace Ogre
{
    const IdString InkProperty::HwGammaRead       = IdString( "hw_gamma_read" );
    const IdString InkProperty::HwGammaWrite      = IdString( "hw_gamma_write" );
    const IdString InkProperty::SignedIntTex      = IdString( "signed_int_textures" );
    const IdString InkProperty::MaterialsPerBuffer= IdString( "materials_per_buffer" );
    const IdString InkProperty::LowerGpuOverhead  = IdString( "lower_gpu_overhead" );

    const IdString InkProperty::NumTextures     = IdString( "num_textures" );
    const char *InkProperty::DiffuseMap         = "diffuse_map";
    const char *InkProperty::NormalMapTex       = "normal_map_tex";
    const char *InkProperty::SpecularMap        = "specular_map";
    const char *InkProperty::RoughnessMap       = "roughness_map";
    const char *InkProperty::EnvProbeMap        = "envprobe_map";
    const char *InkProperty::DetailWeightMap    = "detail_weight_map";
    const char *InkProperty::DetailMapN         = "detail_map";     //detail_map0-4
    const char *InkProperty::DetailMapNmN       = "detail_map_nm";  //detail_map_nm0-4

    const IdString InkProperty::DetailMap0      = "detail_map0";
    const IdString InkProperty::DetailMap1      = "detail_map1";
    const IdString InkProperty::DetailMap2      = "detail_map2";
    const IdString InkProperty::DetailMap3      = "detail_map3";

    const IdString InkProperty::NormalMap         = IdString( "normal_map" );

    const IdString InkProperty::FresnelScalar     = IdString( "fresnel_scalar" );
    const IdString InkProperty::UseTextureAlpha   = IdString( "use_texture_alpha" );
    const IdString InkProperty::TransparentMode   = IdString( "transparent_mode" );
    const IdString InkProperty::FresnelWorkflow   = IdString( "fresnel_workflow" );
    const IdString InkProperty::MetallicWorkflow  = IdString( "metallic_workflow" );
    const IdString InkProperty::TwoSidedLighting  = IdString( "two_sided_lighting" );

    const IdString InkProperty::NormalWeight          = IdString( "normal_weight" );
    const IdString InkProperty::NormalWeightTex       = IdString( "normal_weight_tex" );
    const IdString InkProperty::NormalWeightDetail0   = IdString( "normal_weight_detail0" );
    const IdString InkProperty::NormalWeightDetail1   = IdString( "normal_weight_detail1" );
    const IdString InkProperty::NormalWeightDetail2   = IdString( "normal_weight_detail2" );
    const IdString InkProperty::NormalWeightDetail3   = IdString( "normal_weight_detail3" );

    const IdString InkProperty::DetailWeights     = IdString( "detail_weights" );
    const IdString InkProperty::DetailOffsetsD0   = IdString( "detail_offsetsD0" );
    const IdString InkProperty::DetailOffsetsD1   = IdString( "detail_offsetsD1" );
    const IdString InkProperty::DetailOffsetsD2   = IdString( "detail_offsetsD2" );
    const IdString InkProperty::DetailOffsetsD3   = IdString( "detail_offsetsD3" );
    const IdString InkProperty::DetailOffsetsN0   = IdString( "detail_offsetsN0" );
    const IdString InkProperty::DetailOffsetsN1   = IdString( "detail_offsetsN1" );
    const IdString InkProperty::DetailOffsetsN2   = IdString( "detail_offsetsN2" );
    const IdString InkProperty::DetailOffsetsN3   = IdString( "detail_offsetsN3" );

    const IdString InkProperty::UvDiffuse         = IdString( "uv_diffuse" );
    const IdString InkProperty::UvNormal          = IdString( "uv_normal" );
    const IdString InkProperty::UvSpecular        = IdString( "uv_specular" );
    const IdString InkProperty::UvRoughness       = IdString( "uv_roughness" );
    const IdString InkProperty::UvDetailWeight    = IdString( "uv_detail_weight" );
    const IdString InkProperty::UvDetail0         = IdString( "uv_detail0" );
    const IdString InkProperty::UvDetail1         = IdString( "uv_detail1" );
    const IdString InkProperty::UvDetail2         = IdString( "uv_detail2" );
    const IdString InkProperty::UvDetail3         = IdString( "uv_detail3" );
    const IdString InkProperty::UvDetailNm0       = IdString( "uv_detail_nm0" );
    const IdString InkProperty::UvDetailNm1       = IdString( "uv_detail_nm1" );
    const IdString InkProperty::UvDetailNm2       = IdString( "uv_detail_nm2" );
    const IdString InkProperty::UvDetailNm3       = IdString( "uv_detail_nm3" );

    const IdString InkProperty::BlendModeIndex0   = IdString( "blend_mode_idx0" );
    const IdString InkProperty::BlendModeIndex1   = IdString( "blend_mode_idx1" );
    const IdString InkProperty::BlendModeIndex2   = IdString( "blend_mode_idx2" );
    const IdString InkProperty::BlendModeIndex3   = IdString( "blend_mode_idx3" );

    const IdString InkProperty::DetailMapsDiffuse = IdString( "detail_maps_diffuse" );
    const IdString InkProperty::DetailMapsNormal  = IdString( "detail_maps_normal" );
    const IdString InkProperty::FirstValidDetailMapNm= IdString( "first_valid_detail_map_nm" );

    const IdString InkProperty::Pcf3x3            = IdString( "pcf_3x3" );
    const IdString InkProperty::Pcf4x4            = IdString( "pcf_4x4" );
    const IdString InkProperty::PcfIterations     = IdString( "pcf_iterations" );

    const IdString InkProperty::EnvMapScale       = IdString( "envmap_scale" );
    const IdString InkProperty::AmbientFixed      = IdString( "ambient_fixed" );
    const IdString InkProperty::AmbientHemisphere = IdString( "ambient_hemisphere" );
    const IdString InkProperty::TargetEnvprobeMap = IdString( "target_envprobe_map" );
    const IdString InkProperty::ParallaxCorrectCubemaps = IdString( "parallax_correct_cubemaps" );
    const IdString InkProperty::UseParallaxCorrectCubemaps= IdString( "use_parallax_correct_cubemaps" );
    const IdString InkProperty::IrradianceVolumes = IdString( "irradiance_volumes" );

    const IdString InkProperty::BrdfDefault       = IdString( "BRDF_Default" );
    const IdString InkProperty::BrdfCookTorrance  = IdString( "BRDF_CookTorrance" );
    const IdString InkProperty::FresnelSeparateDiffuse  = IdString( "fresnel_separate_diffuse" );
    const IdString InkProperty::GgxHeightCorrelated     = IdString( "GGX_height_correlated" );

    const IdString *InkProperty::UvSourcePtrs[NUM_INK_SOURCES] =
    {
        &InkProperty::UvDiffuse,
        &InkProperty::UvNormal,
        &InkProperty::UvSpecular,
        &InkProperty::UvRoughness,
        &InkProperty::UvDetailWeight,
        &InkProperty::UvDetail0,
        &InkProperty::UvDetail1,
        &InkProperty::UvDetail2,
        &InkProperty::UvDetail3,
        &InkProperty::UvDetailNm0,
        &InkProperty::UvDetailNm1,
        &InkProperty::UvDetailNm2,
        &InkProperty::UvDetailNm3
    };

    const IdString *InkProperty::DetailNormalWeights[4] =
    {
        &InkProperty::NormalWeightDetail0,
        &InkProperty::NormalWeightDetail1,
        &InkProperty::NormalWeightDetail2,
        &InkProperty::NormalWeightDetail3
    };

    const IdString *InkProperty::DetailOffsetsDPtrs[4] =
    {
        &InkProperty::DetailOffsetsD0,
        &InkProperty::DetailOffsetsD1,
        &InkProperty::DetailOffsetsD2,
        &InkProperty::DetailOffsetsD3
    };

    const IdString *InkProperty::DetailOffsetsNPtrs[4] =
    {
        &InkProperty::DetailOffsetsN0,
        &InkProperty::DetailOffsetsN1,
        &InkProperty::DetailOffsetsN2,
        &InkProperty::DetailOffsetsN3
    };

    const IdString *InkProperty::BlendModes[4] =
    {
        &InkProperty::BlendModeIndex0,
        &InkProperty::BlendModeIndex1,
        &InkProperty::BlendModeIndex2,
        &InkProperty::BlendModeIndex3
    };

    extern const String c_pbsBlendModes[];

    HlmsInk::HlmsInk( Archive *dataFolder, ArchiveVec *libraryFolders ) :
        HlmsBufferManager( HLMS_USER0, "Ink", dataFolder, libraryFolders ),
        ConstBufferPool( HlmsInkDatablock::MaterialSizeInGpuAligned,
                         ConstBufferPool::ExtraBufferParams() ),
        mShadowmapSamplerblock( 0 ),
        mShadowmapCmpSamplerblock( 0 ),
        mCurrentShadowmapSamplerblock( 0 ),
        mParallaxCorrectedCubemap( 0 ),
        mCurrentPassBuffer( 0 ),
        mGridBuffer( 0 ),
        mGlobalLightListBuffer( 0 ),
        mTexUnitSlotStart( 0 ),
        mIrradianceVolume( 0 ),
        mLastBoundPool( 0 ),
        mLastTextureHash( 0 ),
        mShadowFilter( PCF_3x3 ),
        mAmbientLightMode( AmbientAuto )
    {
        //Override defaults
        mLightGatheringMode = LightGatherForwardPlus;
    }
    //-----------------------------------------------------------------------------------
    HlmsInk::~HlmsInk()
    {
        destroyAllBuffers();
    }
    //-----------------------------------------------------------------------------------
    void HlmsInk::_changeRenderSystem( RenderSystem *newRs )
    {
        ConstBufferPool::_changeRenderSystem( newRs );
        HlmsBufferManager::_changeRenderSystem( newRs );

        if( newRs )
        {
            HlmsDatablockMap::const_iterator itor = mDatablocks.begin();
            HlmsDatablockMap::const_iterator end  = mDatablocks.end();

            while( itor != end )
            {
                assert( dynamic_cast<HlmsInkDatablock*>( itor->second.datablock ) );
                HlmsInkDatablock *datablock = static_cast<HlmsInkDatablock*>( itor->second.datablock );

                requestSlot( datablock->mTextureHash, datablock, false );
                ++itor;
            }

            HlmsSamplerblock samplerblock;
            samplerblock.mU             = TAM_BORDER;
            samplerblock.mV             = TAM_BORDER;
            samplerblock.mW             = TAM_CLAMP;
            samplerblock.mBorderColour  = ColourValue( std::numeric_limits<float>::max(),
                                                       std::numeric_limits<float>::max(),
                                                       std::numeric_limits<float>::max(),
                                                       std::numeric_limits<float>::max() );

            if( mShaderProfile != "hlsl" )
            {
                samplerblock.mMinFilter = FO_POINT;
                samplerblock.mMagFilter = FO_POINT;
                samplerblock.mMipFilter = FO_NONE;

                if( !mShadowmapSamplerblock )
                    mShadowmapSamplerblock = mHlmsManager->getSamplerblock( samplerblock );
            }

            samplerblock.mMinFilter     = FO_LINEAR;
            samplerblock.mMagFilter     = FO_LINEAR;
            samplerblock.mMipFilter     = FO_NONE;
            samplerblock.mCompareFunction   = CMPF_LESS_EQUAL;

            if( !mShadowmapCmpSamplerblock )
                mShadowmapCmpSamplerblock = mHlmsManager->getSamplerblock( samplerblock );
        }
    }
    //-----------------------------------------------------------------------------------
    const HlmsCache* HlmsInk::createShaderCacheEntry( uint32 renderableHash,
                                                            const HlmsCache &passCache,
                                                            uint32 finalHash,
                                                            const QueuedRenderable &queuedRenderable )
    {
        const HlmsCache *retVal = Hlms::createShaderCacheEntry( renderableHash, passCache, finalHash,
                                                                queuedRenderable );

        if( mShaderProfile == "hlsl" || mShaderProfile == "metal" )
        {
            mListener->shaderCacheEntryCreated( mShaderProfile, retVal, passCache,
                                                mSetProperties, queuedRenderable );
            return retVal; //D3D embeds the texture slots in the shader.
        }

        //Set samplers.
        if( !retVal->pso.pixelShader.isNull() )
        {
            GpuProgramParametersSharedPtr psParams = retVal->pso.pixelShader->getDefaultParameters();

            int texUnit = 1; //Vertex shader consumes 1 slot with its tbuffer.

            //Forward3D consumes 2 more slots.
            if( mGridBuffer )
            {
                psParams->setNamedConstant( "f3dGrid",      1 );
                psParams->setNamedConstant( "f3dLightList", 2 );
                texUnit += 2;
            }

            if( mIrradianceVolume && getProperty( HlmsBaseProp::ShadowCaster ) == 0 )
                psParams->setNamedConstant( "irradianceVolume", texUnit++ );

            if( !mPreparedPass.shadowMaps.empty() )
            {
                vector<int>::type shadowMaps;
                shadowMaps.reserve( mPreparedPass.shadowMaps.size() );
                for( size_t i=0; i<mPreparedPass.shadowMaps.size(); ++i )
                    shadowMaps.push_back( texUnit++ );

                psParams->setNamedConstant( "texShadowMap", &shadowMaps[0], shadowMaps.size(), 1 );
            }

            int cubemapTexUnit = 0;
            const int32 parallaxCorrectCubemaps = getProperty( InkProperty::ParallaxCorrectCubemaps );
            if( parallaxCorrectCubemaps )
                cubemapTexUnit = texUnit++;

            assert( dynamic_cast<const HlmsInkDatablock*>( queuedRenderable.renderable->getDatablock() ) );
            const HlmsInkDatablock *datablock = static_cast<const HlmsInkDatablock*>(
                                                        queuedRenderable.renderable->getDatablock() );

            int numTextures = getProperty( InkProperty::NumTextures );
            for( int i=0; i<numTextures; ++i )
            {
                psParams->setNamedConstant( "textureMaps[" + StringConverter::toString( i ) + "]",
                                            texUnit++ );
            }

            const int32 envProbeMap         = getProperty( InkProperty::EnvProbeMap );
            const int32 targetEnvProbeMap   = getProperty( InkProperty::TargetEnvprobeMap );
            if( (envProbeMap && envProbeMap != targetEnvProbeMap) || parallaxCorrectCubemaps )
            {
                assert( !datablock->getTexture( INK_REFLECTION ).isNull() || parallaxCorrectCubemaps );
                if( !envProbeMap || envProbeMap == targetEnvProbeMap )
                    psParams->setNamedConstant( "texEnvProbeMap", cubemapTexUnit );
                else
                    psParams->setNamedConstant( "texEnvProbeMap", texUnit++ );
            }
        }

        GpuProgramParametersSharedPtr vsParams = retVal->pso.vertexShader->getDefaultParameters();
        vsParams->setNamedConstant( "worldMatBuf", 0 );

        mListener->shaderCacheEntryCreated( mShaderProfile, retVal, passCache,
                                            mSetProperties, queuedRenderable );

        mRenderSystem->_setPipelineStateObject( &retVal->pso );

        mRenderSystem->bindGpuProgramParameters( GPT_VERTEX_PROGRAM, vsParams, GPV_ALL );
        if( !retVal->pso.pixelShader.isNull() )
        {
            GpuProgramParametersSharedPtr psParams = retVal->pso.pixelShader->getDefaultParameters();
            mRenderSystem->bindGpuProgramParameters( GPT_FRAGMENT_PROGRAM, psParams, GPV_ALL );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsInk::setDetailMapProperties( HlmsInkDatablock *datablock, PiecesMap *inOutPieces )
    {
        uint32 minNormalMap = 4;
        bool hasDiffuseMaps = false;
        bool hasNormalMaps = false;
        bool anyDetailWeight = false;
        for( size_t i=0; i<4; ++i )
        {
            uint8 blendMode = datablock->mBlendModes[i];

            setDetailTextureProperty( InkProperty::DetailMapN,   datablock, INK_DETAIL0, i );
            setDetailTextureProperty( InkProperty::DetailMapNmN, datablock, INK_DETAIL0_NM, i );

            if( !datablock->getTexture( INK_DETAIL0 + i ).isNull() )
            {
                inOutPieces[PixelShader][*InkProperty::BlendModes[i]] =
                                                "@insertpiece( " + c_pbsBlendModes[blendMode] + ")";
                hasDiffuseMaps = true;
            }

            if( !datablock->getTexture( INK_DETAIL0_NM + i ).isNull() )
            {
                minNormalMap = std::min<uint32>( minNormalMap, i );
                hasNormalMaps = true;
            }

            if( datablock->mDetailsOffsetScale[i] != Vector4( 0, 0, 1, 1 ) )
                setProperty( *InkProperty::DetailOffsetsDPtrs[i], 1 );

            if( datablock->mDetailsOffsetScale[i+4] != Vector4( 0, 0, 1, 1 ) )
                setProperty( *InkProperty::DetailOffsetsNPtrs[i], 1 );

            if( datablock->mDetailWeight[i] != 1.0f &&
                (!datablock->getTexture( INK_DETAIL0 + i ).isNull() ||
                 !datablock->getTexture( INK_DETAIL0_NM + i ).isNull()) )
            {
                anyDetailWeight = true;
            }
        }

        if( hasDiffuseMaps )
            setProperty( InkProperty::DetailMapsDiffuse, 4 );

        if( hasNormalMaps )
            setProperty( InkProperty::DetailMapsNormal, 4 );

        setProperty( InkProperty::FirstValidDetailMapNm, minNormalMap );

        if( anyDetailWeight )
            setProperty( InkProperty::DetailWeights, 1 );
    }
    //-----------------------------------------------------------------------------------
    void HlmsInk::setTextureProperty( const char *propertyName, HlmsInkDatablock *datablock,
                                      InkTextureTypes texType )
    {
        uint8 idx = datablock->getBakedTextureIdx( texType );
        if( idx != NUM_INK_TEXTURE_TYPES )
        {
            char tmpData[64];
            LwString propName = LwString::FromEmptyPointer( tmpData, sizeof(tmpData) );

            propName = propertyName; //diffuse_map

            //In the template the we subtract the "+1" for the index.
            //We need to increment it now otherwise @property( diffuse_map )
            //can translate to @property( 0 ) which is not what we want.
            setProperty( propertyName, idx + 1 );

            propName.a( "_idx" ); //diffuse_map_idx
            setProperty( propName.c_str(), idx );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsInk::setDetailTextureProperty( const char *propertyName, HlmsInkDatablock *datablock,
                                            InkTextureTypes baseTexType, uint8 detailIdx )
    {
        const InkTextureTypes texType = static_cast<InkTextureTypes>( baseTexType + detailIdx );
        const uint8 idx = datablock->getBakedTextureIdx( texType );
        if( idx != NUM_INK_TEXTURE_TYPES )
        {
            char tmpData[64];
            LwString propName = LwString::FromEmptyPointer( tmpData, sizeof(tmpData) );

            propName.a( propertyName, detailIdx ); //detail_map0

            //In the template the we subtract the "+1" for the index.
            //We need to increment it now otherwise @property( diffuse_map )
            //can translate to @property( 0 ) which is not what we want.
            setProperty( propName.c_str(), idx + 1 );

            propName.a( "_idx" ); //detail_map0_idx
            setProperty( propName.c_str(), idx );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsInk::calculateHashForPreCreate( Renderable *renderable, PiecesMap *inOutPieces )
    {
        assert( dynamic_cast<HlmsInkDatablock*>( renderable->getDatablock() ) );
        HlmsInkDatablock *datablock = static_cast<HlmsInkDatablock*>(
                                                        renderable->getDatablock() );

        const bool metallicWorkflow = datablock->getWorkflow() == HlmsInkDatablock::MetallicWorkflow;
        const bool fresnelWorkflow = datablock->getWorkflow() ==
                                                        HlmsInkDatablock::SpecularAsFresnelWorkflow;

        setProperty( InkProperty::FresnelScalar, datablock->hasSeparateFresnel() || metallicWorkflow );
        setProperty( InkProperty::FresnelWorkflow, fresnelWorkflow );
        setProperty( InkProperty::MetallicWorkflow, metallicWorkflow );

        if( datablock->getTwoSidedLighting() )
            setProperty( InkProperty::TwoSidedLighting, 1 );

        uint32 brdf = datablock->getBrdf();
        if( (brdf & InkBrdf::BRDF_MASK) == InkBrdf::Default )
        {
            setProperty( InkProperty::BrdfDefault, 1 );

            if( !(brdf & InkBrdf::FLAG_UNCORRELATED) )
                setProperty( InkProperty::GgxHeightCorrelated, 1 );
        }
        else if( (brdf & InkBrdf::BRDF_MASK) == InkBrdf::CookTorrance )
            setProperty( InkProperty::BrdfCookTorrance, 1 );

        if( brdf & InkBrdf::FLAG_SPERATE_DIFFUSE_FRESNEL )
            setProperty( InkProperty::FresnelSeparateDiffuse, 1 );

        for( size_t i=0; i<INK_REFLECTION; ++i )
        {
            uint8 uvSource = datablock->mUvSource[i];
            setProperty( *InkProperty::UvSourcePtrs[i], uvSource );

            if( !datablock->getTexture( i ).isNull() &&
                getProperty( *HlmsBaseProp::UvCountPtrs[uvSource] ) < 2 )
            {
                OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                             "Renderable needs at least 2 coordinates in UV set #" +
                             StringConverter::toString( uvSource ) +
                             ". Either change the mesh, or change the UV source settings",
                             "HlmsInk::calculateHashForPreCreate" );
            }
        }

        int numNormalWeights = 0;
        if( datablock->getNormalMapWeight() != 1.0f && !datablock->getTexture( INK_NORMAL ).isNull() )
        {
            setProperty( InkProperty::NormalWeightTex, 1 );
            ++numNormalWeights;
        }

        {
            size_t validDetailMaps = 0;
            for( size_t i=0; i<4; ++i )
            {
                if( !datablock->getTexture( INK_DETAIL0_NM + i ).isNull() )
                {
                    if( datablock->getDetailNormalWeight( i ) != 1.0f )
                    {
                        setProperty( *InkProperty::DetailNormalWeights[validDetailMaps], 1 );
                        ++numNormalWeights;
                    }

                    ++validDetailMaps;
                }
            }
        }

        setProperty( InkProperty::NormalWeight, numNormalWeights );

        setDetailMapProperties( datablock, inOutPieces );

        bool envMap = datablock->getBakedTextureIdx( INK_REFLECTION ) != NUM_INK_TEXTURE_TYPES;

        setProperty( InkProperty::NumTextures, datablock->mBakedTextures.size() - envMap );

        setTextureProperty( InkProperty::DiffuseMap,    datablock,  INK_DIFFUSE );
        setTextureProperty( InkProperty::NormalMapTex,  datablock,  INK_NORMAL );
        setTextureProperty( InkProperty::SpecularMap,   datablock,  INK_SPECULAR );
        setTextureProperty( InkProperty::RoughnessMap,  datablock,  INK_ROUGHNESS );
        setTextureProperty( InkProperty::EnvProbeMap,   datablock,  INK_REFLECTION );
        setTextureProperty( InkProperty::DetailWeightMap,datablock, INK_DETAIL_WEIGHT );

        {
            //Save the name of the cubemap for hazard prevention
            //(don't sample the cubemap and render to it at the same time).
            TexturePtr reflectionTexture = datablock->getTexture( INK_REFLECTION );
            if( !reflectionTexture.isNull() )
            {
                //Manual reflection texture
                if( datablock->getCubemapProbe() )
                    setProperty( InkProperty::UseParallaxCorrectCubemaps, 1 );
                setProperty( InkProperty::EnvProbeMap, static_cast<int32>(
                             IdString( reflectionTexture->getName() ).mHash ) );
            }
        }

        bool usesNormalMap = !datablock->getTexture( INK_NORMAL ).isNull();
        for( size_t i=INK_DETAIL0_NM; i<=INK_DETAIL3_NM; ++i )
            usesNormalMap |= !datablock->getTexture( i ).isNull();
        setProperty( InkProperty::NormalMap, usesNormalMap );

        /*setProperty( HlmsBaseProp::, !datablock->getTexture( INK_DETAIL0 ).isNull() );
        setProperty( HlmsBaseProp::DiffuseMap, !datablock->getTexture( INK_DETAIL1 ).isNull() );*/
        bool normalMapCanBeSupported = (getProperty( HlmsBaseProp::Normal ) &&
                                        getProperty( HlmsBaseProp::Tangent )) ||
                                        getProperty( HlmsBaseProp::QTangent );

        if( !normalMapCanBeSupported && usesNormalMap )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Renderable can't use normal maps but datablock wants normal maps. "
                         "Generate Tangents for this mesh to fix the problem or use a "
                         "datablock without normal maps.", "HlmsInk::calculateHashForPreCreate" );
        }

        if( datablock->mUseAlphaFromTextures && datablock->mBlendblock[0]->mIsTransparent &&
            (getProperty( InkProperty::DiffuseMap ) || getProperty( InkProperty::DetailMapsDiffuse )) )
        {
            setProperty( InkProperty::UseTextureAlpha, 1 );

            //When we don't use the alpha in the texture, transparency still works but
            //only at material level (i.e. what datablock->mTransparency says). The
            //alpha from the texture will be ignored.
            if( datablock->mTransparencyMode == HlmsInkDatablock::Transparent )
                setProperty( InkProperty::TransparentMode, 1 );
        }

        String slotsPerPoolStr = StringConverter::toString( mSlotsPerPool );
        inOutPieces[VertexShader][InkProperty::MaterialsPerBuffer] = slotsPerPoolStr;
		inOutPieces[GeometryShader][InkProperty::MaterialsPerBuffer] = slotsPerPoolStr;
        inOutPieces[PixelShader][InkProperty::MaterialsPerBuffer] = slotsPerPoolStr;
    }
    //-----------------------------------------------------------------------------------
    void HlmsInk::calculateHashForPreCaster( Renderable *renderable, PiecesMap *inOutPieces )
    {
        HlmsInkDatablock *datablock = static_cast<HlmsInkDatablock*>( renderable->getDatablock() );
        const bool hasAlphaTest = datablock->getAlphaTest() != CMPF_ALWAYS_PASS;

        HlmsPropertyVec::iterator itor = mSetProperties.begin();
        HlmsPropertyVec::iterator end  = mSetProperties.end();

        while( itor != end )
        {
            if( itor->keyName == InkProperty::FirstValidDetailMapNm )
            {
                itor->value = 0;
                ++itor;
            }
            else if( itor->keyName != InkProperty::HwGammaRead &&
                     itor->keyName != InkProperty::UvDiffuse &&
                     itor->keyName != HlmsBaseProp::Skeleton &&
                     itor->keyName != HlmsBaseProp::BonesPerVertex &&
                     itor->keyName != HlmsBaseProp::DualParaboloidMapping &&
                     itor->keyName != HlmsBaseProp::AlphaTest &&
                     itor->keyName != HlmsBaseProp::AlphaBlend &&
                     (!hasAlphaTest || !requiredPropertyByAlphaTest( itor->keyName )) )
            {
                itor = mSetProperties.erase( itor );
                end  = mSetProperties.end();
            }
            else
            {
                ++itor;
            }
        }

        if( hasAlphaTest )
        {
            //Keep GLSL happy by not declaring more textures than we'll actually need.
            uint8 numTextures = 0;
            for( int i=0; i<4; ++i )
            {
                if( datablock->mTexToBakedTextureIdx[INK_DETAIL0+i] < datablock->mBakedTextures.size() )
                {
                    numTextures = std::max<uint8>(
                                numTextures, datablock->mTexToBakedTextureIdx[INK_DETAIL0+i] + 1 );
                }
            }

            if( datablock->mTexToBakedTextureIdx[INK_DIFFUSE] < datablock->mBakedTextures.size() )
            {
                numTextures = std::max<uint8>(
                            numTextures, datablock->mTexToBakedTextureIdx[INK_DIFFUSE] + 1 );
            }

            setProperty( InkProperty::NumTextures, numTextures );

            //Set the blending mode as a piece again
            for( size_t i=0; i<4; ++i )
            {
                uint8 blendMode = datablock->mBlendModes[i];
                if( !datablock->getTexture( INK_DETAIL0 + i ).isNull() )
                {
                    inOutPieces[PixelShader][*InkProperty::BlendModes[i]] =
                                                    "@insertpiece( " + c_pbsBlendModes[blendMode] + ")";
                }
            }
        }

        String slotsPerPoolStr = StringConverter::toString( mSlotsPerPool );
        inOutPieces[VertexShader][InkProperty::MaterialsPerBuffer] = slotsPerPoolStr;
        inOutPieces[PixelShader][InkProperty::MaterialsPerBuffer] = slotsPerPoolStr;
    }
    //-----------------------------------------------------------------------------------
    bool HlmsInk::requiredPropertyByAlphaTest( IdString keyName )
    {
        bool retVal =
                keyName == InkProperty::NumTextures ||
                keyName == InkProperty::DiffuseMap ||
                keyName == InkProperty::DetailWeightMap ||
                keyName == InkProperty::DetailMap0 || keyName == InkProperty::DetailMap1 ||
                keyName == InkProperty::DetailMap2 || keyName == InkProperty::DetailMap3 ||
                keyName == InkProperty::DetailWeights ||
                keyName == InkProperty::DetailOffsetsD0 || keyName == InkProperty::DetailOffsetsD1 ||
                keyName == InkProperty::DetailOffsetsD2 || keyName == InkProperty::DetailOffsetsD3 ||
                keyName == InkProperty::UvDetailWeight ||
                keyName == InkProperty::UvDetail0 || keyName == InkProperty::UvDetail1 ||
                keyName == InkProperty::UvDetail2 || keyName == InkProperty::UvDetail3 ||
                keyName == InkProperty::BlendModeIndex0 || keyName == InkProperty::BlendModeIndex1 ||
                keyName == InkProperty::BlendModeIndex2 || keyName == InkProperty::BlendModeIndex3 ||
                keyName == InkProperty::DetailMapsDiffuse ||
                keyName == HlmsBaseProp::UvCount;

        for( int i=0; i<8 && !retVal; ++i )
            retVal |= keyName == *HlmsBaseProp::UvCountPtrs[i];

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    HlmsCache HlmsInk::preparePassHash( const CompositorShadowNode *shadowNode, bool casterPass,
                                        bool dualParaboloid, SceneManager *sceneManager )
    {
        mSetProperties.clear();

        //The properties need to be set before preparePassHash so that
        //they are considered when building the HlmsCache's hash.
        if( shadowNode && !casterPass )
        {
            //Shadow receiving can be improved in performance by using gather sampling.
            //(it's the only feature so far that uses gather)
            const RenderSystemCapabilities *capabilities = mRenderSystem->getCapabilities();
            if( capabilities->hasCapability( RSC_TEXTURE_GATHER ) )
                setProperty( HlmsBaseProp::TexGather, 1 );

            if( mShadowFilter == PCF_3x3 )
            {
                setProperty( InkProperty::Pcf3x3, 1 );
                setProperty( InkProperty::PcfIterations, 4 );
            }
            else if( mShadowFilter == PCF_4x4 )
            {
                setProperty( InkProperty::Pcf4x4, 1 );
                setProperty( InkProperty::PcfIterations, 9 );
            }
            else
            {
                setProperty( InkProperty::PcfIterations, 1 );
            }
        }

        mTargetEnvMap.setNull();

        AmbientLightMode ambientMode = mAmbientLightMode;
        ColourValue upperHemisphere = sceneManager->getAmbientLightUpperHemisphere();
        ColourValue lowerHemisphere = sceneManager->getAmbientLightLowerHemisphere();

        const float envMapScale = upperHemisphere.a;
        //Ignore alpha channel
        upperHemisphere.a = lowerHemisphere.a = 1.0;

        if( !casterPass )
        {
            if( mAmbientLightMode == AmbientAuto )
            {
                if( upperHemisphere == lowerHemisphere )
                {
                    if( upperHemisphere == ColourValue::Black )
                        ambientMode = AmbientNone;
                    else
                        ambientMode = AmbientFixed;
                }
                else
                {
                    ambientMode = AmbientHemisphere;
                }
            }

            if( ambientMode == AmbientFixed )
                setProperty( InkProperty::AmbientFixed, 1 );
            if( ambientMode == AmbientHemisphere )
                setProperty( InkProperty::AmbientHemisphere, 1 );

            if( envMapScale != 1.0f )
                setProperty( InkProperty::EnvMapScale, 1 );

            //Save cubemap's name so that we never try to render & sample to/from it at the same time
            const CompositorTexture &compoTarget = sceneManager->getCompositorTarget();
            if( !compoTarget.textures->empty() )
            {
                const TexturePtr &firstTargetTex = (*compoTarget.textures)[0];
                if( firstTargetTex->getTextureType() == TEX_TYPE_CUBE_MAP )
                {
                    setProperty( InkProperty::TargetEnvprobeMap,
                                 static_cast<int32>( IdString(firstTargetTex->getName()).mHash ) );
                    mTargetEnvMap = firstTargetTex;
                }
            }

            if( mParallaxCorrectedCubemap )
                setProperty( InkProperty::ParallaxCorrectCubemaps, 1 );

            if( mIrradianceVolume )
                setProperty( InkProperty::IrradianceVolumes, 1 );
        }

        if( mOptimizationStrategy == LowerGpuOverhead )
            setProperty( InkProperty::LowerGpuOverhead, 1 );

        HlmsCache retVal = Hlms::preparePassHashBase( shadowNode, casterPass,
                                                      dualParaboloid, sceneManager );

        RenderTarget *renderTarget = sceneManager->getCurrentViewport()->getTarget();

        const RenderSystemCapabilities *capabilities = mRenderSystem->getCapabilities();
        setProperty( InkProperty::HwGammaRead, capabilities->hasCapability( RSC_HW_GAMMA ) );
        setProperty( InkProperty::HwGammaWrite, capabilities->hasCapability( RSC_HW_GAMMA ) &&
                                                        renderTarget->isHardwareGammaEnabled() );
        setProperty( InkProperty::SignedIntTex, capabilities->hasCapability(
                                                            RSC_TEXTURE_SIGNED_INT ) );
        retVal.setProperties = mSetProperties;

        Camera *camera = sceneManager->getCameraInProgress();
        Matrix4 viewMatrix = camera->getViewMatrix(true);

        Matrix4 projectionMatrix = camera->getProjectionMatrixWithRSDepth();

        if( renderTarget->requiresTextureFlipping() )
        {
            projectionMatrix[1][0] = -projectionMatrix[1][0];
            projectionMatrix[1][1] = -projectionMatrix[1][1];
            projectionMatrix[1][2] = -projectionMatrix[1][2];
            projectionMatrix[1][3] = -projectionMatrix[1][3];
        }

        int32 numLights             = getProperty( HlmsBaseProp::LightsSpot );
        int32 numDirectionalLights  = getProperty( HlmsBaseProp::LightsDirNonCaster );
        int32 numShadowMaps         = getProperty( HlmsBaseProp::NumShadowMaps );
        int32 numPssmSplits         = getProperty( HlmsBaseProp::PssmSplits );

        //mat4 viewProj;
        size_t mapSize = 16 * 4;

        mGridBuffer             = 0;
        mGlobalLightListBuffer  = 0;

        if( !casterPass )
        {
            ForwardPlusBase *forwardPlus = sceneManager->_getActivePassForwardPlus();
            if( forwardPlus )
            {
                mapSize += forwardPlus->getConstBufferSize();
                mGridBuffer             = forwardPlus->getGridBuffer( camera );
                mGlobalLightListBuffer  = forwardPlus->getGlobalLightListBuffer( camera );
            }

            if( mParallaxCorrectedCubemap )
            {
                mParallaxCorrectedCubemap->_notifyPreparePassHash( viewMatrix );
                mapSize += mParallaxCorrectedCubemap->getConstBufferSize();
            }

            //mat4 view + mat4 shadowRcv[numShadowMaps].texViewProj +
            //              vec2 shadowRcv[numShadowMaps].shadowDepthRange +
            //              vec2 padding +
            //              vec4 shadowRcv[numShadowMaps].invShadowMapSize +
            //mat3 invViewMatCubemap (upgraded to three vec4)
            mapSize += ( 16 + (16 + 2 + 2 + 4) * numShadowMaps + 4 * 3 ) * 4;

            //vec3 ambientUpperHemi + float envMapScale
            if( ambientMode == AmbientFixed || ambientMode == AmbientHemisphere || envMapScale != 1.0f )
                mapSize += 4 * 4;

            //vec3 ambientLowerHemi + padding + vec3 ambientHemisphereDir + padding
            if( ambientMode == AmbientHemisphere )
                mapSize += 8 * 4;

            //vec3 irradianceOrigin + float maxPower +
            //vec3 irradianceSize + float invHeight + mat4 invView
            if( mIrradianceVolume )
                mapSize += (4 + 4 + 4*4) * 4;

            //float pssmSplitPoints N times.
            mapSize += numPssmSplits * 4;
            mapSize = alignToNextMultiple( mapSize, 16 );

            if( shadowNode )
            {
                //Six variables * 4 (padded vec3) * 4 (bytes) * numLights
                mapSize += ( 6 * 4 * 4 ) * numLights;
            }
            else
            {
                //Three variables * 4 (padded vec3) * 4 (bytes) * numDirectionalLights
                mapSize += ( 3 * 4 * 4 ) * numDirectionalLights;
            }
        }
        else
        {
            mapSize += (2 + 2) * 4;
        }

        mapSize += mListener->getPassBufferSize( shadowNode, casterPass, dualParaboloid,
                                                 sceneManager );

        //Arbitrary 16kb (minimum supported by GL), should be enough.
        const size_t maxBufferSize = 16 * 1024;

        assert( mapSize <= maxBufferSize );

        if( mCurrentPassBuffer >= mPassBuffers.size() )
        {
            mPassBuffers.push_back( mVaoManager->createConstBuffer( maxBufferSize, BT_DYNAMIC_PERSISTENT,
                                                                    0, false ) );
        }

        ConstBufferPacked *passBuffer = mPassBuffers[mCurrentPassBuffer++];
        float *passBufferPtr = reinterpret_cast<float*>( passBuffer->map( 0, mapSize ) );

#ifndef NDEBUG
        const float *startupPtr = passBufferPtr;
#endif

        //---------------------------------------------------------------------------
        //                          ---- VERTEX SHADER ----
        //---------------------------------------------------------------------------

        //mat4 viewProj;
        Matrix4 viewProjMatrix = projectionMatrix * viewMatrix;
        for( size_t i=0; i<16; ++i )
            *passBufferPtr++ = (float)viewProjMatrix[0][i];

        mPreparedPass.viewMatrix        = viewMatrix;

        mPreparedPass.shadowMaps.clear();

        if( !casterPass )
        {
            //mat4 view;
            for( size_t i=0; i<16; ++i )
                *passBufferPtr++ = (float)viewMatrix[0][i];

            for( int32 i=0; i<numShadowMaps; ++i )
            {
                //mat4 shadowRcv[numShadowMaps].texViewProj
                Matrix4 viewProjTex = shadowNode->getViewProjectionMatrix( i );
                for( size_t j=0; j<16; ++j )
                    *passBufferPtr++ = (float)viewProjTex[0][j];

                //vec2 shadowRcv[numShadowMaps].shadowDepthRange
                Real fNear, fFar;
                shadowNode->getMinMaxDepthRange( i, fNear, fFar );
                const Real depthRange = fFar - fNear;
                *passBufferPtr++ = fNear;
                *passBufferPtr++ = 1.0f / depthRange;
                ++passBufferPtr; //Padding
                ++passBufferPtr; //Padding


                //vec2 shadowRcv[numShadowMaps].invShadowMapSize
                //TODO: textures[0] is out of bounds when using shadow atlas. Also see how what
                //changes need to be done so that UV calculations land on the right place
                uint32 texWidth  = shadowNode->getLocalTextures()[i].textures[0]->getWidth();
                uint32 texHeight = shadowNode->getLocalTextures()[i].textures[0]->getHeight();
                *passBufferPtr++ = 1.0f / texWidth;
                *passBufferPtr++ = 1.0f / texHeight;
                *passBufferPtr++ = static_cast<float>( texWidth );
                *passBufferPtr++ = static_cast<float>( texHeight );
            }

            //---------------------------------------------------------------------------
            //                          ---- PIXEL SHADER ----
            //---------------------------------------------------------------------------

            Matrix3 viewMatrix3, invViewMatrixCubemap;
            viewMatrix.extract3x3Matrix( viewMatrix3 );
            //Cubemaps are left-handed.
            invViewMatrixCubemap = viewMatrix3;
            invViewMatrixCubemap[0][2] = -invViewMatrixCubemap[0][2];
            invViewMatrixCubemap[1][2] = -invViewMatrixCubemap[1][2];
            invViewMatrixCubemap[2][2] = -invViewMatrixCubemap[2][2];
            invViewMatrixCubemap = invViewMatrixCubemap.Inverse();

            //mat3 invViewMatCubemap
            for( size_t i=0; i<9; ++i )
            {
#ifdef OGRE_GLES2_WORKAROUND_2
                Matrix3 xRot( 1.0f, 0.0f, 0.0f,
                              0.0f, 0.0f, -1.0f,
                              0.0f, 1.0f, 0.0f );
                xRot = xRot * invViewMatrixCubemap;
                *passBufferPtr++ = (float)xRot[0][i];
#else
                *passBufferPtr++ = (float)invViewMatrixCubemap[0][i];
#endif

                //Alignment: each row/column is one vec4, despite being 3x3
                if( !( (i+1) % 3 ) )
                    ++passBufferPtr;
            }

            //vec3 ambientUpperHemi + padding
            if( ambientMode == AmbientFixed || ambientMode == AmbientHemisphere || envMapScale != 1.0f )
            {
                *passBufferPtr++ = static_cast<float>( upperHemisphere.r );
                *passBufferPtr++ = static_cast<float>( upperHemisphere.g );
                *passBufferPtr++ = static_cast<float>( upperHemisphere.b );
                *passBufferPtr++ = envMapScale;
            }

            //vec3 ambientLowerHemi + padding + vec3 ambientHemisphereDir + padding
            if( ambientMode == AmbientHemisphere )
            {
                *passBufferPtr++ = static_cast<float>( lowerHemisphere.r );
                *passBufferPtr++ = static_cast<float>( lowerHemisphere.g );
                *passBufferPtr++ = static_cast<float>( lowerHemisphere.b );
                *passBufferPtr++ = 1.0f;

                Vector3 hemisphereDir = viewMatrix3 * sceneManager->getAmbientLightHemisphereDir();
                hemisphereDir.normalise();
                *passBufferPtr++ = static_cast<float>( hemisphereDir.x );
                *passBufferPtr++ = static_cast<float>( hemisphereDir.y );
                *passBufferPtr++ = static_cast<float>( hemisphereDir.z );
                *passBufferPtr++ = 1.0f;
            }

            if( mIrradianceVolume )
            {
                const Vector3 irradianceCellSize = mIrradianceVolume->getIrradianceCellSize();
                const Vector3 irradianceVolumeOrigin = mIrradianceVolume->getIrradianceOrigin() /
                                                       irradianceCellSize;
                const float fTexWidth = static_cast<float>(
                            mIrradianceVolume->getIrradianceVolumeTexture()->getWidth() );
                const float fTexDepth = static_cast<float>(
                            mIrradianceVolume->getIrradianceVolumeTexture()->getDepth() );

                *passBufferPtr++ = static_cast<float>( irradianceVolumeOrigin.x ) / fTexWidth;
                *passBufferPtr++ = static_cast<float>( irradianceVolumeOrigin.y );
                *passBufferPtr++ = static_cast<float>( irradianceVolumeOrigin.z ) / fTexDepth;
                *passBufferPtr++ = mIrradianceVolume->getIrradianceMaxPower() *
                                   mIrradianceVolume->getPowerScale();

                const float fTexHeight = static_cast<float>(
                            mIrradianceVolume->getIrradianceVolumeTexture()->getHeight() );

                *passBufferPtr++ = 1.0f / (fTexWidth * irradianceCellSize.x);
                *passBufferPtr++ = 1.0f / irradianceCellSize.y;
                *passBufferPtr++ = 1.0f / (fTexDepth * irradianceCellSize.z);
                *passBufferPtr++ = 1.0f / fTexHeight;

                //mat4 invView;
                Matrix4 invViewMatrix = viewMatrix.inverse();
                for( size_t i=0; i<16; ++i )
                    *passBufferPtr++ = (float)invViewMatrix[0][i];
            }

            //float pssmSplitPoints
            for( int32 i=0; i<numPssmSplits; ++i )
                *passBufferPtr++ = (*shadowNode->getPssmSplits(0))[i+1];

            passBufferPtr += alignToNextMultiple( numPssmSplits, 4 ) - numPssmSplits;

            if( shadowNode )
            {
                //All directional lights (caster and non-caster) are sent.
                //Then non-directional shadow-casting shadow lights are sent.
                size_t shadowLightIdx = 0;
                size_t nonShadowLightIdx = 0;
                const LightListInfo &globalLightList = sceneManager->getGlobalLightList();
                const LightClosestArray &lights = shadowNode->getShadowCastingLights();

                const CompositorShadowNode::LightsBitSet &affectedLights =
                        shadowNode->getAffectedLightsBitSet();

                int32 shadowCastingDirLights = getProperty( HlmsBaseProp::LightsDirectional );

                for( int32 i=0; i<numLights; ++i )
                {
                    Light const *light = 0;

                    if( i >= shadowCastingDirLights && i < numDirectionalLights )
                    {
                        while( affectedLights[nonShadowLightIdx] )
                            ++nonShadowLightIdx;

                        light = globalLightList.lights[nonShadowLightIdx++];

                        assert( light->getType() == Light::LT_DIRECTIONAL );
                    }
                    else
                    {
                        light = lights[shadowLightIdx++].light;
                    }

                    Vector4 lightPos4 = light->getAs4DVector();
                    Vector3 lightPos;

                    if( light->getType() == Light::LT_DIRECTIONAL )
                        lightPos = viewMatrix3 * Vector3( lightPos4.x, lightPos4.y, lightPos4.z );
                    else
                        lightPos = viewMatrix * Vector3( lightPos4.x, lightPos4.y, lightPos4.z );

                    //vec3 lights[numLights].position
                    *passBufferPtr++ = lightPos.x;
                    *passBufferPtr++ = lightPos.y;
                    *passBufferPtr++ = lightPos.z;
                    ++passBufferPtr;

                    //vec3 lights[numLights].diffuse
                    ColourValue colour = light->getDiffuseColour() *
                                         light->getPowerScale();
                    *passBufferPtr++ = colour.r;
                    *passBufferPtr++ = colour.g;
                    *passBufferPtr++ = colour.b;
                    ++passBufferPtr;

                    //vec3 lights[numLights].specular
                    colour = light->getSpecularColour() * light->getPowerScale();
                    *passBufferPtr++ = colour.r;
                    *passBufferPtr++ = colour.g;
                    *passBufferPtr++ = colour.b;
                    ++passBufferPtr;

                    //vec3 lights[numLights].attenuation;
                    Real attenRange     = light->getAttenuationRange();
                    Real attenLinear    = light->getAttenuationLinear();
                    Real attenQuadratic = light->getAttenuationQuadric();
                    *passBufferPtr++ = attenRange;
                    *passBufferPtr++ = attenLinear;
                    *passBufferPtr++ = attenQuadratic;
                    ++passBufferPtr;

                    //vec3 lights[numLights].spotDirection;
                    Vector3 spotDir = viewMatrix3 * light->getDerivedDirection();
                    *passBufferPtr++ = spotDir.x;
                    *passBufferPtr++ = spotDir.y;
                    *passBufferPtr++ = spotDir.z;
                    ++passBufferPtr;

                    //vec3 lights[numLights].spotParams;
                    Radian innerAngle = light->getSpotlightInnerAngle();
                    Radian outerAngle = light->getSpotlightOuterAngle();
                    *passBufferPtr++ = 1.0f / ( cosf( innerAngle.valueRadians() * 0.5f ) -
                                             cosf( outerAngle.valueRadians() * 0.5f ) );
                    *passBufferPtr++ = cosf( outerAngle.valueRadians() * 0.5f );
                    *passBufferPtr++ = light->getSpotlightFalloff();
                    ++passBufferPtr;
                }

                mPreparedPass.shadowMaps.reserve( numShadowMaps );
                for( int32 i=0; i<numShadowMaps; ++i )
                    mPreparedPass.shadowMaps.push_back( shadowNode->getLocalTextures()[i].textures[0] );
            }
            else
            {
                //No shadow maps, only send directional lights
                const LightListInfo &globalLightList = sceneManager->getGlobalLightList();

                for( int32 i=0; i<numDirectionalLights; ++i )
                {
                    assert( globalLightList.lights[i]->getType() == Light::LT_DIRECTIONAL );

                    Vector4 lightPos4 = globalLightList.lights[i]->getAs4DVector();
                    Vector3 lightPos = viewMatrix3 * Vector3( lightPos4.x, lightPos4.y, lightPos4.z );

                    //vec3 lights[numLights].position
                    *passBufferPtr++ = lightPos.x;
                    *passBufferPtr++ = lightPos.y;
                    *passBufferPtr++ = lightPos.z;
                    ++passBufferPtr;

                    //vec3 lights[numLights].diffuse
                    ColourValue colour = globalLightList.lights[i]->getDiffuseColour() *
                                         globalLightList.lights[i]->getPowerScale();
                    *passBufferPtr++ = colour.r;
                    *passBufferPtr++ = colour.g;
                    *passBufferPtr++ = colour.b;
                    ++passBufferPtr;

                    //vec3 lights[numLights].specular
                    colour = globalLightList.lights[i]->getSpecularColour() * globalLightList.lights[i]->getPowerScale();
                    *passBufferPtr++ = colour.r;
                    *passBufferPtr++ = colour.g;
                    *passBufferPtr++ = colour.b;
                    ++passBufferPtr;
                }
            }

            ForwardPlusBase *forwardPlus = sceneManager->_getActivePassForwardPlus();
            if( forwardPlus )
            {
                forwardPlus->fillConstBufferData( renderTarget, passBufferPtr );
                passBufferPtr += forwardPlus->getConstBufferSize() >> 2u;
            }

            if( mParallaxCorrectedCubemap )
            {
                mParallaxCorrectedCubemap->fillConstBufferData( viewMatrix, passBufferPtr );
                passBufferPtr += mParallaxCorrectedCubemap->getConstBufferSize() >> 2u;
            }
        }
        else
        {
            //vec2 depthRange;
            Real fNear, fFar;
            shadowNode->getMinMaxDepthRange( camera, fNear, fFar );
            const Real depthRange = fFar - fNear;
            *passBufferPtr++ = fNear;
            *passBufferPtr++ = 1.0f / depthRange;
            passBufferPtr += 2;
        }

        passBufferPtr = mListener->preparePassBuffer( shadowNode, casterPass, dualParaboloid,
                                                      sceneManager, passBufferPtr );

        assert( (size_t)(passBufferPtr - startupPtr) * 4u == mapSize );

        passBuffer->unmap( UO_KEEP_PERSISTENT );

        //mTexBuffers must hold at least one buffer to prevent out of bound exceptions.
        if( mTexBuffers.empty() )
        {
            size_t bufferSize = std::min<size_t>( mTextureBufferDefaultSize,
                                                  mVaoManager->getTexBufferMaxSize() );
            TexBufferPacked *newBuffer = mVaoManager->createTexBuffer( PF_FLOAT32_RGBA, bufferSize,
                                                                       BT_DYNAMIC_PERSISTENT, 0, false );
            mTexBuffers.push_back( newBuffer );
        }

        mLastTextureHash = 0;

        mLastBoundPool = 0;

        if( mShadowmapSamplerblock && !getProperty( HlmsBaseProp::ShadowUsesDepthTexture ) )
            mCurrentShadowmapSamplerblock = mShadowmapSamplerblock;
        else
            mCurrentShadowmapSamplerblock = mShadowmapCmpSamplerblock;

        mTexUnitSlotStart = mPreparedPass.shadowMaps.size() + 1;
        if( mGridBuffer )
            mTexUnitSlotStart += 2;
        if( mIrradianceVolume )
            mTexUnitSlotStart += 1;
        if( mParallaxCorrectedCubemap )
            mTexUnitSlotStart += 1;

        uploadDirtyDatablocks();

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsInk::fillBuffersFor( const HlmsCache *cache, const QueuedRenderable &queuedRenderable,
                                    bool casterPass, uint32 lastCacheHash,
                                    uint32 lastTextureHash )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                     "Trying to use slow-path on a desktop implementation. "
                     "Change the RenderQueue settings.",
                     "HlmsInk::fillBuffersFor" );
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsInk::fillBuffersForV1( const HlmsCache *cache,
                                      const QueuedRenderable &queuedRenderable,
                                      bool casterPass, uint32 lastCacheHash,
                                      CommandBuffer *commandBuffer )
    {
        return fillBuffersFor( cache, queuedRenderable, casterPass,
                               lastCacheHash, commandBuffer, true );
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsInk::fillBuffersForV2( const HlmsCache *cache,
                                      const QueuedRenderable &queuedRenderable,
                                      bool casterPass, uint32 lastCacheHash,
                                      CommandBuffer *commandBuffer )
    {
        return fillBuffersFor( cache, queuedRenderable, casterPass,
                               lastCacheHash, commandBuffer, false );
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsInk::fillBuffersFor( const HlmsCache *cache, const QueuedRenderable &queuedRenderable,
                                    bool casterPass, uint32 lastCacheHash,
                                    CommandBuffer *commandBuffer, bool isV1 )
    {
        assert( dynamic_cast<const HlmsInkDatablock*>( queuedRenderable.renderable->getDatablock() ) );
        const HlmsInkDatablock *datablock = static_cast<const HlmsInkDatablock*>(
                                                queuedRenderable.renderable->getDatablock() );

        if( OGRE_EXTRACT_HLMS_TYPE_FROM_CACHE_HASH( lastCacheHash ) != HLMS_USER0 )
        {
            //layout(binding = 0) uniform PassBuffer {} pass
            ConstBufferPacked *passBuffer = mPassBuffers[mCurrentPassBuffer-1];
            *commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer( VertexShader,
                                                                           0, passBuffer, 0,
                                                                           passBuffer->
                                                                           getTotalSizeBytes() );
			*commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer(GeometryShader,
																		  0, passBuffer, 0,
																		  passBuffer->
																		  getTotalSizeBytes());
            *commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer( PixelShader,
                                                                           0, passBuffer, 0,
                                                                           passBuffer->
                                                                           getTotalSizeBytes() );

            if( !casterPass )
            {
                size_t texUnit = 1;

                if( mGridBuffer )
                {
                    texUnit = 3;
                    *commandBuffer->addCommand<CbShaderBuffer>() =
                            CbShaderBuffer( PixelShader, 1, mGridBuffer, 0, 0 );
                    *commandBuffer->addCommand<CbShaderBuffer>() =
                            CbShaderBuffer( PixelShader, 2, mGlobalLightListBuffer, 0, 0 );
                }

                if( mIrradianceVolume )
                {
                    const TexturePtr &irradianceTex = mIrradianceVolume->getIrradianceVolumeTexture();
                    const HlmsSamplerblock *samplerblock = mIrradianceVolume->getIrradSamplerblock();

                    *commandBuffer->addCommand<CbTexture>() = CbTexture( texUnit, true,
                                                                         irradianceTex.get(),
                                                                         samplerblock );
                    ++texUnit;
                }

                //We changed HlmsType, rebind the shared textures.
                FastArray<TexturePtr>::const_iterator itor = mPreparedPass.shadowMaps.begin();
                FastArray<TexturePtr>::const_iterator end  = mPreparedPass.shadowMaps.end();
                while( itor != end )
                {
                    *commandBuffer->addCommand<CbTexture>() = CbTexture( texUnit, true, itor->get(),
                                                                         mCurrentShadowmapSamplerblock );
                    ++texUnit;
                    ++itor;
                }

                if( mParallaxCorrectedCubemap )
                {
                    Texture *pccTexture = mParallaxCorrectedCubemap->getBlendCubemap().get();
                    const HlmsSamplerblock *samplerblock =
                            mParallaxCorrectedCubemap->getBlendCubemapTrilinearSamplerblock();
                    *commandBuffer->addCommand<CbTexture>() = CbTexture( texUnit, true, pccTexture,
                                                                         samplerblock );
                    ++texUnit;
                }
            }
            else
            {
                *commandBuffer->addCommand<CbTextureDisableFrom>() = CbTextureDisableFrom( 1 );
            }

            mLastTextureHash = 0;
            mLastBoundPool = 0;

            //layout(binding = 2) uniform InstanceBuffer {} instance
            if( mCurrentConstBuffer < mConstBuffers.size() &&
                (size_t)((mCurrentMappedConstBuffer - mStartMappedConstBuffer) + 4) <=
                    mCurrentConstBufferSize )
            {
                *commandBuffer->addCommand<CbShaderBuffer>() =
                        CbShaderBuffer( VertexShader, 2, mConstBuffers[mCurrentConstBuffer], 0, 0 );
                *commandBuffer->addCommand<CbShaderBuffer>() =
                        CbShaderBuffer( PixelShader, 2, mConstBuffers[mCurrentConstBuffer], 0, 0 );
            }

            rebindTexBuffer( commandBuffer );

            mListener->hlmsTypeChanged( casterPass, commandBuffer, datablock );
        }

        //Don't bind the material buffer on caster passes (important to keep
        //MDI & auto-instancing running on shadow map passes)
        if( mLastBoundPool != datablock->getAssignedPool() &&
            (!casterPass || datablock->getAlphaTest() != CMPF_ALWAYS_PASS) )
        {
            //layout(binding = 1) uniform MaterialBuf {} materialArray
            const ConstBufferPool::BufferPool *newPool = datablock->getAssignedPool();
            *commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer( PixelShader,
                                                                           1, newPool->materialBuffer, 0,
                                                                           newPool->materialBuffer->
                                                                           getTotalSizeBytes() );
            CubemapProbe *manualProbe = datablock->getCubemapProbe();
            if( manualProbe )
            {
                ConstBufferPacked *probeConstBuf = manualProbe->getConstBufferForManualProbes();
                *commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer( PixelShader,
                                                                               3, probeConstBuf,
                                                                               0, 0 );
            }
            mLastBoundPool = newPool;
        }

        uint32 * RESTRICT_ALIAS currentMappedConstBuffer    = mCurrentMappedConstBuffer;
        float * RESTRICT_ALIAS currentMappedTexBuffer       = mCurrentMappedTexBuffer;

        bool hasSkeletonAnimation = queuedRenderable.renderable->hasSkeletonAnimation();

        const Matrix4 &worldMat = queuedRenderable.movableObject->_getParentNodeFullTransform();

        //---------------------------------------------------------------------------
        //                          ---- VERTEX SHADER ----
        //---------------------------------------------------------------------------

        if( !hasSkeletonAnimation )
        {
            //We need to correct currentMappedConstBuffer to point to the right texture buffer's
            //offset, which may not be in sync if the previous draw had skeletal animation.
            const size_t currentConstOffset = (currentMappedTexBuffer - mStartMappedTexBuffer) >>
                                                (2 + !casterPass);
            currentMappedConstBuffer =  currentConstOffset + mStartMappedConstBuffer;
            bool exceedsConstBuffer = (size_t)((currentMappedConstBuffer - mStartMappedConstBuffer) + 4)
                                        > mCurrentConstBufferSize;

            const size_t minimumTexBufferSize = 16 * (1 + !casterPass);
            bool exceedsTexBuffer = (currentMappedTexBuffer - mStartMappedTexBuffer) +
                                         minimumTexBufferSize >= mCurrentTexBufferSize;

            if( exceedsConstBuffer || exceedsTexBuffer )
            {
                currentMappedConstBuffer = mapNextConstBuffer( commandBuffer );

                if( exceedsTexBuffer )
                    mapNextTexBuffer( commandBuffer, minimumTexBufferSize * sizeof(float) );
                else
                    rebindTexBuffer( commandBuffer, true, minimumTexBufferSize * sizeof(float) );

                currentMappedTexBuffer = mCurrentMappedTexBuffer;
            }

            //uint worldMaterialIdx[]
            *currentMappedConstBuffer = datablock->getAssignedSlot() & 0x1FF;

            //mat4x3 world
#if !OGRE_DOUBLE_PRECISION
            memcpy( currentMappedTexBuffer, &worldMat, 4 * 3 * sizeof( float ) );
            currentMappedTexBuffer += 16;
#else
            for( int y = 0; y < 3; ++y )
            {
                for( int x = 0; x < 4; ++x )
                {
                    *currentMappedTexBuffer++ = worldMat[ y ][ x ];
                }
            }
            currentMappedTexBuffer += 4;
#endif

            //mat4 worldView
            Matrix4 tmp = mPreparedPass.viewMatrix.concatenateAffine( worldMat );
    #ifdef OGRE_GLES2_WORKAROUND_1
            tmp = tmp.transpose();
#endif
#if !OGRE_DOUBLE_PRECISION
            memcpy( currentMappedTexBuffer, &tmp, sizeof( Matrix4 ) * !casterPass );
            currentMappedTexBuffer += 16 * !casterPass;
#else
            if( !casterPass )
            {
                for( int y = 0; y < 4; ++y )
                {
                    for( int x = 0; x < 4; ++x )
                    {
                        *currentMappedTexBuffer++ = tmp[ y ][ x ];
                    }
                }
            }
#endif
        }
        else
        {
            bool exceedsConstBuffer = (size_t)((currentMappedConstBuffer - mStartMappedConstBuffer) + 4)
                                        > mCurrentConstBufferSize;

            if( isV1 )
            {
                uint16 numWorldTransforms = queuedRenderable.renderable->getNumWorldTransforms();
                assert( numWorldTransforms <= 256u );

                const size_t minimumTexBufferSize = 12 * numWorldTransforms;
                bool exceedsTexBuffer = (currentMappedTexBuffer - mStartMappedTexBuffer) +
                        minimumTexBufferSize >= mCurrentTexBufferSize;

                if( exceedsConstBuffer || exceedsTexBuffer )
                {
                    currentMappedConstBuffer = mapNextConstBuffer( commandBuffer );

                    if( exceedsTexBuffer )
                        mapNextTexBuffer( commandBuffer, minimumTexBufferSize * sizeof(float) );
                    else
                        rebindTexBuffer( commandBuffer, true, minimumTexBufferSize * sizeof(float) );

                    currentMappedTexBuffer = mCurrentMappedTexBuffer;
                }

                //uint worldMaterialIdx[]
                size_t distToWorldMatStart = mCurrentMappedTexBuffer - mStartMappedTexBuffer;
                distToWorldMatStart >>= 2;
                *currentMappedConstBuffer = (distToWorldMatStart << 9 ) |
                        (datablock->getAssignedSlot() & 0x1FF);

                //vec4 worldMat[][3]
                //TODO: Don't rely on a virtual function + make a direct 4x3 copy
                Matrix4 tmp[256];
                queuedRenderable.renderable->getWorldTransforms( tmp );
                for( size_t i=0; i<numWorldTransforms; ++i )
                {
#if !OGRE_DOUBLE_PRECISION
                    memcpy( currentMappedTexBuffer, &tmp[ i ], 12 * sizeof( float ) );
                    currentMappedTexBuffer += 12;
#else
                    for( int y = 0; y < 3; ++y )
                    {
                        for( int x = 0; x < 4; ++x )
                        {
                            *currentMappedTexBuffer++ = tmp[ i ][ y ][ x ];
                        }
                    }
#endif
                }
            }
            else
            {
                SkeletonInstance *skeleton = queuedRenderable.movableObject->getSkeletonInstance();

#if OGRE_DEBUG_MODE
                assert( dynamic_cast<const RenderableAnimated*>( queuedRenderable.renderable ) );
#endif

                const RenderableAnimated *renderableAnimated = static_cast<const RenderableAnimated*>(
                                                                        queuedRenderable.renderable );

                const RenderableAnimated::IndexMap *indexMap = renderableAnimated->getBlendIndexToBoneIndexMap();

                const size_t minimumTexBufferSize = 12 * indexMap->size();
                bool exceedsTexBuffer = (currentMappedTexBuffer - mStartMappedTexBuffer) +
                                            minimumTexBufferSize >= mCurrentTexBufferSize;

                if( exceedsConstBuffer || exceedsTexBuffer )
                {
                    currentMappedConstBuffer = mapNextConstBuffer( commandBuffer );

                    if( exceedsTexBuffer )
                        mapNextTexBuffer( commandBuffer, minimumTexBufferSize * sizeof(float) );
                    else
                        rebindTexBuffer( commandBuffer, true, minimumTexBufferSize * sizeof(float) );

                    currentMappedTexBuffer = mCurrentMappedTexBuffer;
                }

                //uint worldMaterialIdx[]
                size_t distToWorldMatStart = mCurrentMappedTexBuffer - mStartMappedTexBuffer;
                distToWorldMatStart >>= 2;
                *currentMappedConstBuffer = (distToWorldMatStart << 9 ) |
                        (datablock->getAssignedSlot() & 0x1FF);

                RenderableAnimated::IndexMap::const_iterator itBone = indexMap->begin();
                RenderableAnimated::IndexMap::const_iterator enBone = indexMap->end();

                while( itBone != enBone )
                {
                    const SimpleMatrixAf4x3 &mat4x3 = skeleton->_getBoneFullTransform( *itBone );
                    mat4x3.streamTo4x3( currentMappedTexBuffer );
                    currentMappedTexBuffer += 12;

                    ++itBone;
                }
            }

            //If the next entity will not be skeletally animated, we'll need
            //currentMappedTexBuffer to be 16/32-byte aligned.
            //Non-skeletally animated objects are far more common than skeletal ones,
            //so we do this here instead of doing it before rendering the non-skeletal ones.
            size_t currentConstOffset = (size_t)(currentMappedTexBuffer - mStartMappedTexBuffer);
            currentConstOffset = alignToNextMultiple( currentConstOffset, 16 + 16 * !casterPass );
            currentConstOffset = std::min( currentConstOffset, mCurrentTexBufferSize );
            currentMappedTexBuffer = mStartMappedTexBuffer + currentConstOffset;
        }

        *reinterpret_cast<float * RESTRICT_ALIAS>( currentMappedConstBuffer+1 ) = datablock->
                                                                                    mShadowConstantBias;
        currentMappedConstBuffer += 4;

        //---------------------------------------------------------------------------
        //                          ---- PIXEL SHADER ----
        //---------------------------------------------------------------------------

        if( !casterPass || datablock->getAlphaTest() != CMPF_ALWAYS_PASS )
        {
            if( datablock->mTextureHash != mLastTextureHash )
            {
                //Rebind textures
                size_t texUnit = mTexUnitSlotStart;

                InkBakedTextureArray::const_iterator itor = datablock->mBakedTextures.begin();
                InkBakedTextureArray::const_iterator end  = datablock->mBakedTextures.end();

                while( itor != end )
                {
                    if( itor->texture != mTargetEnvMap )
                    {
                        *commandBuffer->addCommand<CbTexture>() =
                                CbTexture( texUnit++, true, itor->texture.get(), itor->samplerBlock );
                    }
                    ++itor;
                }

                *commandBuffer->addCommand<CbTextureDisableFrom>() = CbTextureDisableFrom( texUnit );

                mLastTextureHash = datablock->mTextureHash;
            }
        }

        mCurrentMappedConstBuffer   = currentMappedConstBuffer;
        mCurrentMappedTexBuffer     = currentMappedTexBuffer;

        return ((mCurrentMappedConstBuffer - mStartMappedConstBuffer) >> 2) - 1;
    }
    //-----------------------------------------------------------------------------------
    void HlmsInk::destroyAllBuffers(void)
    {
        HlmsBufferManager::destroyAllBuffers();

        mCurrentPassBuffer  = 0;

        {
            ConstBufferPackedVec::const_iterator itor = mPassBuffers.begin();
            ConstBufferPackedVec::const_iterator end  = mPassBuffers.end();

            while( itor != end )
            {
                if( (*itor)->getMappingState() != MS_UNMAPPED )
                    (*itor)->unmap( UO_UNMAP_ALL );
                mVaoManager->destroyConstBuffer( *itor );
                ++itor;
            }

            mPassBuffers.clear();
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsInk::frameEnded(void)
    {
        HlmsBufferManager::frameEnded();
        mCurrentPassBuffer  = 0;
    }
    //-----------------------------------------------------------------------------------
    void HlmsInk::setShadowSettings( ShadowFilter filter )
    {
        mShadowFilter = filter;
    }
    //-----------------------------------------------------------------------------------
    void HlmsInk::setAmbientLightMode( AmbientLightMode mode )
    {
        mAmbientLightMode = mode;
    }
#if !OGRE_NO_JSON
    //-----------------------------------------------------------------------------------
    void HlmsInk::_loadJson( const rapidjson::Value &jsonValue, const HlmsJson::NamedBlocks &blocks,
                             HlmsDatablock *datablock ) const
    {
        HlmsJsonInk jsonInk( mHlmsManager );
        jsonInk.loadMaterial( jsonValue, blocks, datablock );
    }
    //-----------------------------------------------------------------------------------
    void HlmsInk::_saveJson( const HlmsDatablock *datablock, String &outString ) const
    {
        HlmsJsonInk jsonInk( mHlmsManager );
        jsonInk.saveMaterial( datablock, outString );
    }
    //-----------------------------------------------------------------------------------
    void HlmsInk::_collectSamplerblocks( set<const HlmsSamplerblock*>::type &outSamplerblocks,
                                         const HlmsDatablock *datablock ) const
    {
        HlmsJsonInk::collectSamplerblocks( datablock, outSamplerblocks );
    }
#endif
    //-----------------------------------------------------------------------------------
    HlmsDatablock* HlmsInk::createDatablockImpl( IdString datablockName,
                                                       const HlmsMacroblock *macroblock,
                                                       const HlmsBlendblock *blendblock,
                                                       const HlmsParamVec &paramVec )
    {
        return OGRE_NEW HlmsInkDatablock( datablockName, this, macroblock, blendblock, paramVec );
    }
}
