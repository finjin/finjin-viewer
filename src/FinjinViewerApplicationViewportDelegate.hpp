//Copyright (c) 2017 Finjin
//
//This file is part of Finjin Viewer (finjin-viewer).
//
//Finjin Viewer is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//This Source Code Form is subject to the terms of the Mozilla Public
//License, v. 2.0. If a copy of the MPL was not distributed with this
//file, You can obtain one at http://mozilla.org/MPL/2.0/.


#pragma once


//Includes----------------------------------------------------------------------
#include <finjin/common/ByteBuffer.hpp>
#include <finjin/engine/ApplicationViewport.hpp>
#include <finjin/engine/Camera.hpp>
#include <finjin/engine/FlyingCameraInputBindings.hpp>
#include <finjin/engine/FinjinSceneReader.hpp>


//Types-------------------------------------------------------------------------
namespace Finjin { namespace Viewer {

    FINJIN_USE_ENGINE_NAMESPACES

    class FinjinViewerApplicationViewportDelegate : public ApplicationViewportDelegate
    {
    public:
        FinjinViewerApplicationViewportDelegate(Allocator* allocator, const Utf8String& loadFileName, bool startInVR);

        UpdateResult Update(ApplicationViewportUpdateContext& updateContext, Error& error) override;
        void FinishFrame(ApplicationViewportRenderContext& renderContext, Error& error) override;

    private:
        void HandleEventsAndInputs(ApplicationViewportUpdateContext& updateContext, FlyingCameraEvents& flyingCameraActions, FlyingCameraEvents& headsetFlyingCameraActions);
        void HandleNewAssets(ApplicationViewportUpdateContext& updateContext, Error& error);
        void StartFrame(ApplicationViewportUpdateContext& updateContext, FlyingCameraEvents& flyingCameraActions, FlyingCameraEvents& headsetFlyingCameraActions);

    private:
        Utf8String loadFileName;
        bool startInVR;

        FinjinSceneReader::State sceneReaderState;
        FinjinSceneReader sceneReader;

        ByteBuffer tempBuffer;

        AssetReference tempAssetRef;

        SimpleTimeCounter totalElapsedTime;

        enum class RunState
        {
            STARTING,
            LOADING_SCENE,
            RUNNING
        };
        RunState runState;

        FlyingCameraInputBindings headsetFlyingCameraInputBindings; //Overkill since only the locator is used
        FlyingCameraInputBindings flyingCameraInputBindings;
        size_t flyingCameraGameControllerIndex;

        Camera camera;
        MathVector4 clearColor;

        struct SceneData
        {
            SceneData()
            {
                this->scene = nullptr;
            }

            FinjinScene* scene;
            DynamicVector<FinjinSceneObjectLight*> lights;
        };
        SceneData sceneData;

        float moveUnitsPerSecond;
        float rotateUnitsPerSecond;

        bool allowRelativeMove;
        bool allowRelativeLook;

        size_t lifetimeFrameSequenceIndex;
    };

} }
