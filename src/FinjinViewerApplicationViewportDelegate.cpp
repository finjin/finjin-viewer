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


//Includes---------------------------------------------------------------------
#include "FinjinPrecompiled.hpp"
#include "FinjinViewerApplicationViewportDelegate.hpp"
#include "finjin/common/Allocator.hpp"
#include "finjin/common/DebugLog.hpp"
#include "finjin/common/JobSystem.hpp"
#include "finjin/engine/InputBindingsSerializer.hpp"
#include "finjin/engine/AssetCountsSettings.hpp"

#define USE_TEST_SCENE_AS_FALLBACK 0

using namespace Finjin::Viewer;


//Implementation---------------------------------------------------------------
FinjinViewerApplicationViewportDelegate::FinjinViewerApplicationViewportDelegate(Allocator* allocator, const Utf8String& loadFileName) :
    ApplicationViewportDelegate(allocator),
    sceneReader(allocator),
    tempAssetRef(allocator),
    loadFileName(allocator)
{
    this->totalElapsedTime = 0;
    this->runState = RunState::STARTING;
    this->loadFileName = loadFileName;    
    this->clearColor = MathVector4(0, 0, 0, 1);
    this->moveUnitsPerSecond = 30.0f;
    this->rotateUnitsPerSecond = 1.0f;
}

ApplicationViewportDelegate::UpdateResult FinjinViewerApplicationViewportDelegate::Update(ApplicationViewportUpdateContext& updateContext, Error& error)
{
    FINJIN_ERROR_METHOD_START(error);

    if (this->sceneReader.GetReadStatus().IsStartedOrSuccess())
    {
        FINJIN_DEBUG_LOG_INFO("Scene progress: %1%", this->sceneReader.GetReadStatus().GetProgress());
    }

    switch (this->runState)
    {
        case RunState::STARTING:
        {
            if (!this->tempBuffer.Create(EngineConstants::DEFAULT_CONFIGURATION_BUFFER_SIZE, GetAllocator()))
            {
                FINJIN_SET_ERROR(error, "Failed to allocate temp buffer.");
                return UpdateResult::LOGIC_ONLY;
            }

            this->camera.SetFovY(Degrees(90.0f));
            this->camera.SetPosition(0, 35, 0);

            {
                const size_t fileCount = 100;
                
                FinjinSceneReader::State::Settings stateSettings;
                stateSettings.maxFileCount = fileCount;
                stateSettings.allocator = GetAllocator();

                this->tempAssetRef.ForLocalFile("scene-reader-asset-counts.cfg");
                AssetCountsSettings assetCountsSettings;
                auto readResult = (*updateContext.assetClassFileReaders)[AssetClass::SETTINGS].ReadAndParseSettingsFile(assetCountsSettings, this->tempBuffer, this->tempAssetRef, error);
                if (error)
                {
                    FINJIN_SET_ERROR(error, "Failed to read asset counts.");
                    return UpdateResult::LOGIC_ONLY;
                }
                else if (readResult == FileOperationResult::NOT_FOUND)
                {          
                    FINJIN_SET_ERROR(error, "Failed to read asset counts file 'asset-counts.cfg'.");
                    return UpdateResult::LOGIC_ONLY;
                }
                stateSettings.assetCountsByClass = assetCountsSettings.countsByClass;
                
                this->sceneReaderState.Create(stateSettings, error);
                if (error)
                {
                    FINJIN_SET_ERROR(error, "Failed to create scene reader state.");
                    return UpdateResult::LOGIC_ONLY;
                }

                FinjinSceneReader::Settings readerSettings;
                readerSettings.setupAllocator = GetAllocator();
                readerSettings.state = &this->sceneReaderState;
                readerSettings.assetReadQueue = updateContext.assetReadQueue;
                readerSettings.assetClassFileReaders = updateContext.assetClassFileReaders;
                readerSettings.maxQueuedFiles = fileCount;
                
                updateContext.gpuContext->GetExternalAssetFileExtensions(readerSettings.externalTextureExtensions, AssetClass::TEXTURE, error);
                if (error)
                {
                    FINJIN_SET_ERROR(error, "Failed to get supported texture extensions.");
                    return UpdateResult::LOGIC_ONLY;
                }
                
                updateContext.soundContext->GetExternalAssetFileExtensions(readerSettings.externalSoundExtensions, AssetClass::SOUND, error);
                if (error)
                {
                    FINJIN_SET_ERROR(error, "Failed to get supported sound extensions.");
                    return UpdateResult::LOGIC_ONLY;
                }
                
                this->sceneReader.Create(readerSettings, error);
                if (error)
                {
                    FINJIN_SET_ERROR(error, "Failed to create scene reader.");
                    return UpdateResult::LOGIC_ONLY;
                }
            }

            {
                this->tempAssetRef.ForLocalFile(FlyingCameraInputBindings::DEFAULT_BINDINGS_FILE_NAME);
                auto result = this->inputBindings.GetFromConfiguration(updateContext.inputContext, InputBindingsConfigurationSearchCriteria(InputDeviceClass::GAME_CONTROLLER, Utf8String::Empty(), 0, InputBindingsConfigurationFlag::CONNECTED_ONLY, InputDeviceSemantic::NONE), this->tempAssetRef, this->tempBuffer);
                if (result.IsSuccess())
                {
                    this->inputBindings.SetInputBindingsDeviceIndex(result.deviceIndex);
                    FINJIN_DEBUG_LOG_INFO("Game controller successfully configured.")
                }                
                result = this->inputBindings.GetFromConfiguration(updateContext.inputContext, InputBindingsConfigurationSearchCriteria(InputDeviceClass::KEYBOARD, 0), this->tempAssetRef, this->tempBuffer);
                result = this->inputBindings.GetFromConfiguration(updateContext.inputContext, InputBindingsConfigurationSearchCriteria(InputDeviceClass::MOUSE, 0), this->tempAssetRef, this->tempBuffer);
            }

            if (!this->loadFileName.empty())
            {
                this->sceneReader.RequestReadLocalFiles(error, this->loadFileName);
                if (error)
                {
                    FINJIN_SET_ERROR(error, "Failed to add scene read request.");
                    return UpdateResult::LOGIC_ONLY;
                }

                this->runState = RunState::LOADING_SCENE;
            }
            else
            {
            #if USE_TEST_SCENE_AS_FALLBACK
                this->sceneReader.RequestReadLocalFiles(error, "test.fstd-scene");
                if (error)
                {
                    FINJIN_SET_ERROR(error, "Failed to add scene read request.");
                    return UpdateResult::LOGIC_ONLY;
                }
                
                this->runState = RunState::LOADING_SCENE;
            #else
                this->runState = RunState::RUNNING;
            #endif
            }

            break;
        }
        case RunState::LOADING_SCENE:
        {
            if (this->sceneReader.GetReadStatus().IsSuccess())
            {
                this->runState = RunState::RUNNING;

                this->sceneReader.GetReadStatus().Reset();

                FINJIN_DEBUG_LOG_INFO("Finished scene reading.");
            }
            break;
        }
        default: break;
    }
    
    if (this->runState == RunState::RUNNING)
    {   
        HandleNewAssets(updateContext, error);
        if (error)
        {
            FINJIN_SET_ERROR(error, "Failed to handle new assets.");
            return UpdateResult::LOGIC_ONLY;
        }

        FlyingCameraEvents flyingCameraActions;
        HandleEventsAndInputs(updateContext, flyingCameraActions);

        StartFrame(updateContext, flyingCameraActions);
        return UpdateResult::STARTED_FRAME;
    }   
    else
        return UpdateResult::LOGIC_ONLY;
}

void FinjinViewerApplicationViewportDelegate::HandleNewAssets(ApplicationViewportUpdateContext& updateContext, Error& error)
{
    FINJIN_ERROR_METHOD_START(error);

    //GPU
    {
        updateContext.newScenes = this->sceneReaderState.GetNewAssets<FinjinScene>();
        if (!updateContext.newScenes.empty())
        {
            auto& scene = *updateContext.newScenes.begin();
            
            auto result = scene.Get<FinjinSceneObjectLight>();
            if (!result.empty())
            {  
                this->sceneData.lights.CreateEmpty(result.size(), GetAllocator());
                FINJIN_DEBUG_LOG_INFO("Light count: %1%", result.size());
                for (auto& item : result)
                {
                    FINJIN_DEBUG_LOG_INFO("  Light: %1%, type: %2%, light type: %3%", item.name, item.GetClassDescription().GetName(), (int)item.lightType);
                    updateContext.gpuContext->CreateLightFromMainThread(item, error);
                    if (error)
                    {
                    }

                    this->sceneData.lights.push_back(&item);
                }
            }

            {
                auto result = scene.Get<FinjinSceneNode>();
                FINJIN_DEBUG_LOG_INFO("Node count: %1%", result.size());
                for (auto& item : result)
                {
                    FINJIN_DEBUG_LOG_INFO("  Node: %1%, type: %2%", item.name, item.GetClassDescription().GetName());
                }
            }

            {
                auto result = scene.Get<FinjinSceneObjectEntity>();
                FINJIN_DEBUG_LOG_INFO("Entity count: %1%", result.size());
                for (auto& item : result)
                {
                    FINJIN_DEBUG_LOG_INFO("  Entity: %1%, type: %2%", item.name, item.GetClassDescription().GetName());
                }
            }

            {
                auto result = scene.Get<FinjinSceneObjectCamera>();
                FINJIN_DEBUG_LOG_INFO("Camera count: %1%", result.size());
                for (auto& item : result)
                {
                    FINJIN_DEBUG_LOG_INFO("  Active camera: %1%, type: %2%", item.name, item.GetClassDescription().GetName());

                    auto& sceneNodeState = item.parentNodePointer->Evaluate(0, 0);
                    auto& cameraState = item.Evaluate(0, 0, sceneNodeState);
                    this->camera.Set(cameraState);
                    this->camera.Update();
                    
                    break;
                }
            }

            this->clearColor = scene.subscenes[0].environment.backgroundColor;

            this->sceneData.scene = &scene;
        }

        auto commandBytesFree = updateContext.gpuCommands.GetBytesFree();

        if (AnySet(updateContext.gpuContext->GetAssetCreationCapabilities(AssetClass::MESH) & AssetCreationCapability::FRAME_THREAD))
        {
            updateContext.newMeshes = this->sceneReaderState.GetNewAssets<FinjinMesh>(updateContext.gpuCommands.ByteCountToObjectCount<CreateMeshGpuCommand>(commandBytesFree));
            commandBytesFree -= updateContext.newMeshes.size() * updateContext.gpuCommands.GetObjectSize<CreateMeshGpuCommand>();
        }
        if (AnySet(updateContext.gpuContext->GetAssetCreationCapabilities(AssetClass::MESH) & AssetCreationCapability::MAIN_THREAD))
        {
            if (NoneSet(updateContext.gpuContext->GetAssetCreationCapabilities(AssetClass::MESH) & AssetCreationCapability::FRAME_THREAD))
                updateContext.newMeshes = this->sceneReaderState.GetNewAssets<FinjinMesh>();
            
            for (auto& mesh : updateContext.newMeshes)
            {
                updateContext.gpuContext->CreateMeshFromMainThread(mesh, error);
                if (error)
                {
                    FINJIN_SET_ERROR(error, "Failed to create mesh.");
                    return;
                }
            }
        }

        if (AnySet(updateContext.gpuContext->GetAssetCreationCapabilities(AssetClass::TEXTURE) & AssetCreationCapability::FRAME_THREAD))
        {
            updateContext.newTextures = this->sceneReaderState.GetNewAssets<FinjinTexture>(updateContext.gpuCommands.ByteCountToObjectCount<CreateTextureGpuCommand>(commandBytesFree));
            commandBytesFree -= updateContext.newTextures.size() * updateContext.gpuCommands.GetObjectSize<CreateTextureGpuCommand>();
        }
        if (AnySet(updateContext.gpuContext->GetAssetCreationCapabilities(AssetClass::TEXTURE) & AssetCreationCapability::MAIN_THREAD))
        {
            if (NoneSet(updateContext.gpuContext->GetAssetCreationCapabilities(AssetClass::TEXTURE) & AssetCreationCapability::FRAME_THREAD))
                updateContext.newTextures = this->sceneReaderState.GetNewAssets<FinjinTexture>();
            
            for (auto& texture : updateContext.newTextures)
            {
                updateContext.gpuContext->CreateTextureFromMainThread(texture, error);
                if (error)
                {
                    FINJIN_SET_ERROR(error, "Failed to create texture.");
                    return;
                }
            }
        }        

        if (AnySet(updateContext.gpuContext->GetAssetCreationCapabilities(AssetClass::MATERIAL) & AssetCreationCapability::FRAME_THREAD))
        {
            updateContext.newMaterials = this->sceneReaderState.GetNewAssets<FinjinMaterial>(updateContext.gpuCommands.ByteCountToObjectCount<CreateMaterialGpuCommand>(commandBytesFree));
            commandBytesFree -= updateContext.newMaterials.size() * updateContext.gpuCommands.GetObjectSize<CreateMaterialGpuCommand>();
        }
        if (AnySet(updateContext.gpuContext->GetAssetCreationCapabilities(AssetClass::MATERIAL) & AssetCreationCapability::MAIN_THREAD))
        {
            if (NoneSet(updateContext.gpuContext->GetAssetCreationCapabilities(AssetClass::TEXTURE) & AssetCreationCapability::FRAME_THREAD))
                updateContext.newMaterials = this->sceneReaderState.GetNewAssets<FinjinMaterial>();
            
            for (auto& material : updateContext.newMaterials)
            {
                updateContext.gpuContext->CreateMaterialFromMainThread(material, error);
                if (error)
                {
                    FINJIN_SET_ERROR(error, "Failed to create material.");
                    return;
                }
            }
        }        
    }
}

void FinjinViewerApplicationViewportDelegate::StartFrame(ApplicationViewportUpdateContext& updateContext, FlyingCameraEvents& flyingCameraActions)
{
    this->totalElapsedTime += updateContext.elapsedTime;
    
    //Kick off jobs for the frame
    auto& frameStage = updateContext.gpuContext->StartFrameStage(updateContext.jobPipelineStage->index, updateContext.elapsedTime, this->totalElapsedTime, &updateContext.jobPipelineStage->frameSequenceIndex);
    
    updateContext.jobSystem->StartGroupFromMainThread();
    updateContext.jobPipelineStage->simulateAndRenderFuture = updateContext.jobSystem->Submit([this, &updateContext, flyingCameraActions, &frameStage]()
    {
        //Simulate----------------------------
        auto cameraChanged = false;
        {                
            if (flyingCameraActions.Contains(FlyingCameraEvents::TAP))
            {
                FINJIN_DEBUG_LOG_INFO("Tap");
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::SWIPE))
            {
                FINJIN_DEBUG_LOG_INFO("Swipe");
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::MOUSE_X))
            {
                FINJIN_DEBUG_LOG_INFO("MouseX");
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::MOUSE_Y))
            {
                FINJIN_DEBUG_LOG_INFO("MouseY");
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::MOUSE_LEFT_BUTTON_DOWN))
            {
                FINJIN_DEBUG_LOG_INFO("MouseLeftButtonDown");
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::MOUSE_LEFT_BUTTON_UP))
            {
                FINJIN_DEBUG_LOG_INFO("MouseLeftButtonUp");
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::MOUSE_RIGHT_BUTTON_DOWN))
            {
                FINJIN_DEBUG_LOG_INFO("MouseRightButtonDown");
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::MOUSE_RIGHT_BUTTON_UP))
            {
                FINJIN_DEBUG_LOG_INFO("MouseRightButtonUp");
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::MOVE))
            {
                this->camera.Pan(flyingCameraActions.move[0] * updateContext.elapsedTime * this->moveUnitsPerSecond, 0);
                this->camera.Walk(flyingCameraActions.move[1] * updateContext.elapsedTime * this->moveUnitsPerSecond);

                cameraChanged = true;

                FINJIN_DEBUG_LOG_INFO("Move: %1% %2%", flyingCameraActions.move[0], flyingCameraActions.move[1]);
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::PAN_ORBIT))
            {
                this->camera.Pan(flyingCameraActions.panOrbit[0] * updateContext.elapsedTime * this->moveUnitsPerSecond, flyingCameraActions.panOrbit[1] * updateContext.elapsedTime * this->moveUnitsPerSecond);

                FINJIN_DEBUG_LOG_INFO("PanOrbit: %1% %2%", flyingCameraActions.panOrbit[0], flyingCameraActions.panOrbit[1]);
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::LOOK))
            {
                this->camera.RotateY(Radians(flyingCameraActions.look[0] * updateContext.elapsedTime * this->rotateUnitsPerSecond * -1.0f)); //Negate since a left 'look' is negative but represents a positive rotation about Y
                this->camera.Pitch(Radians(flyingCameraActions.look[1] * updateContext.elapsedTime * this->rotateUnitsPerSecond));

                cameraChanged = true;

                FINJIN_DEBUG_LOG_INFO("Look: %1% %2%", flyingCameraActions.look[0], flyingCameraActions.look[1]);
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::SELECT_OBJECT))
            {
                FINJIN_DEBUG_LOG_INFO("SelectObject");
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::DESELECT_OBJECT))
            {
                FINJIN_DEBUG_LOG_INFO("DeselectObject");
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::CHANGE_CAMERA))
            {
                FINJIN_DEBUG_LOG_INFO("ChangeCamera");
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::TOGGLE_VIEW_CAMERA_LOCK))
            {
                FINJIN_DEBUG_LOG_INFO("ToggleViewCameraLock");
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::SET_DOLLY_MODE))
            {
                FINJIN_DEBUG_LOG_INFO("SetDollyMode");
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::POP_DOLLY))
            {
                FINJIN_DEBUG_LOG_INFO("PopDolly");
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::SET_PAN_OR_ORBIT_MODE))
            {
                FINJIN_DEBUG_LOG_INFO("SetPanOrOrbitMode");
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::POP_PAN_OR_ORBIT))
            {
                FINJIN_DEBUG_LOG_INFO("PopPanOrOrbit");
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::SET_FREE_LOOK_MODE))
            {
                FINJIN_DEBUG_LOG_INFO("SetFreeLookMode");
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::POP_FREE_LOOK))
            {
                FINJIN_DEBUG_LOG_INFO("PopFreeLook");
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::ZOOM_TO_FIT))
            {
                FINJIN_DEBUG_LOG_INFO("ZoomToFit");
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::SCREENSHOT))
            {
                FINJIN_DEBUG_LOG_INFO("Screenshot");
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::ESCAPE))
            {
                FINJIN_DEBUG_LOG_INFO("Escape");
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::MENU))
            {
                FINJIN_DEBUG_LOG_INFO("Menu");
            }

            if (cameraChanged)
                this->camera.Update();
            
            if (!updateContext.gpuCommands.StartGraphicsCommandList())
            {
            }

            //GPU
            if (AnySet(updateContext.gpuContext->GetAssetCreationCapabilities(AssetClass::MESH) & AssetCreationCapability::FRAME_THREAD))
            {
                size_t meshIndex = 0;
                for (auto& item : updateContext.newMeshes)
                {
                    if (!updateContext.gpuCommands.CreateMesh(&item))
                    {
                    }
                }
            }
            if (AnySet(updateContext.gpuContext->GetAssetCreationCapabilities(AssetClass::TEXTURE) & AssetCreationCapability::FRAME_THREAD))
            {
                size_t textureIndex = 0;
                for (auto& item : updateContext.newTextures)
                {
                    if (!updateContext.gpuCommands.CreateTexture(&item))
                    {
                    }
                }
            }
            if (AnySet(updateContext.gpuContext->GetAssetCreationCapabilities(AssetClass::MATERIAL) & AssetCreationCapability::FRAME_THREAD))
            {
                size_t materialIndex = 0;
                for (auto& item : updateContext.newMaterials)
                {
                    if (!updateContext.gpuCommands.CreateMaterial(&item))
                    {
                    }
                }
            }

            if (this->sceneData.scene != nullptr || this->loadFileName.empty())
            {
                if (!updateContext.gpuCommands.StartRenderTarget(nullptr, nullptr))
                {
                }

                updateContext.gpuCommands.SetCamera(this->camera);
                updateContext.gpuCommands.SetClearColor(this->clearColor);

                if (this->sceneData.scene != nullptr)
                {
                    for (auto& subscene : this->sceneData.scene->subscenes)
                    {
                        auto& ambientLightState = subscene.environment.ambientLight.Evaluate(updateContext.jobPipelineStage->index, updateContext.jobPipelineStage->frameSequenceIndex);

                        for (auto sceneNode : subscene.sceneNodes)
                        {
                            auto& sceneNodeState = sceneNode->Evaluate(updateContext.jobPipelineStage->index, updateContext.jobPipelineStage->frameSequenceIndex);

                            for (auto obj : sceneNode->objects)
                            {
                                if (obj->IsTypeOf(FINJIN_CLASS_DESCRIPTION(FinjinSceneObjectEntity)))
                                {
                                    auto entity = (FinjinSceneObjectEntity*)obj;

                                    auto shaderFeatureFlags = RenderShaderFeatureFlags::GetDefault();
                                    //shaderFeatureFlags.renderingFlags |= ShaderFeatureFlag::RENDERING_FILL_WIREFRAME;

                                    GpuCommandLights lights;
                                    for (auto light : this->sceneData.lights)
                                    {
                                        if (light->isActive)
                                        {
                                            auto& lightSceneNodeState = light->parentNodePointer->Evaluate(updateContext.jobPipelineStage->index, updateContext.jobPipelineStage->frameSequenceIndex);
                                            lights.push_back(GpuCommandLight(light, &light->Evaluate(updateContext.jobPipelineStage->index, updateContext.jobPipelineStage->frameSequenceIndex, lightSceneNodeState)));
                                            if (lights.full())
                                            {
                                                break;
                                            }
                                        }
                                    }

                                    std::stable_sort(lights.begin(), lights.end(), [](const GpuCommandLight& a, const GpuCommandLight& b)
                                    {
                                        return a.light->GetLightTypePriority() < b.light->GetLightTypePriority();
                                    });

                                    if (!updateContext.gpuCommands.RenderEntity(sceneNodeState, entity, shaderFeatureFlags, lights, ambientLightState.color))
                                    {
                                    }

                                    break;
                                }
                            }
                        }
                    }
                }

                if (!updateContext.gpuCommands.FinishRenderTarget())
                {
                }
            }

            if (!updateContext.gpuCommands.FinishGraphicsCommandList())
            {
            }

            updateContext.jobSystem->FinishGroupFromNonMainThread();
        }

        //Render----------------------------
        {
            FINJIN_DECLARE_ERROR(error);
            
            updateContext.gpuContext->StartBackFrameBufferRender(frameStage);

            updateContext.Execute(frameStage, error);
            if (error)
            {
            }
        
            updateContext.gpuContext->FinishFrameStage(frameStage);
        }
    });
}

void FinjinViewerApplicationViewportDelegate::FinishFrame(ApplicationViewportRenderContext& renderContext, Error& error)
{
    FINJIN_ERROR_METHOD_START(error);

    auto& frameStage = renderContext.gpuContext->GetFrameStage(renderContext.jobPipelineStage->index);

    if (renderContext.jobPipelineStage->simulateAndRenderFuture.valid()) //Will fail during the final iteration of waiting for pipeline to run out for full screen toggle
    {
        try
        {
            renderContext.jobPipelineStage->simulateAndRenderFuture.get();
        }
        catch (...)
        {
            FINJIN_SET_ERROR(error, "An exception was thrown while getting simulation/render future.");
        }

        if (!error)
        {
            renderContext.gpuContext->FinishBackFrameBufferRender(frameStage, renderContext.continueRendering, renderContext.presentSyncIntervalOverride, error);
            if (error)
            {
                FINJIN_SET_ERROR(error, "Failed to present back buffer.");
                FINJIN_DEBUG_LOG_ERROR("Present frame error: %1%", error.ToString());
            }
        }
    }
}

void FinjinViewerApplicationViewportDelegate::HandleEventsAndInputs(ApplicationViewportUpdateContext& updateContext, FlyingCameraEvents& flyingCameraActions)
{
#if FINJIN_TARGET_VR_SYSTEM != FINJIN_TARGET_VR_SYSTEM_NONE
    //VR events
    /*for (auto& event : updateContext.vrEvents)
    {
    }*/
#endif
    
    //Input events
    /*for (auto& event : updateContext.inputEvents)
    {
    }*/

    //Sound events
    /*for (auto& event : updateContext.soundEvents)
    {        
    }*/

    //GPU events
    /*for (auto& event : updateContext.gpuEvents)
    {        
    }*/

    updateContext.ClearEvents();
    
    //Poll subsystems that require it
    updateContext.Poll();

#if FINJIN_DEBUG
    ChangedInputSources<1> pressedItems;
    ChangedInputSourceFilter filter;
    filter.deviceClasses = InputDeviceClass::GAME_CONTROLLER | InputDeviceClass::KEYBOARD;
    updateContext.inputContext->GetChangedInputSources(pressedItems, filter);
    for (size_t i = 0; i < pressedItems.size(); i++)
    {
        auto& pressedItem = pressedItems[i];
        FINJIN_DEBUG_LOG_INFO("Pressed item %1%: %2%", i, pressedItem.ToString());
    }
#endif

    //Update input    
    updateContext.inputContext->GetActions(flyingCameraActions, this->inputBindings, updateContext.elapsedTime);    
}
