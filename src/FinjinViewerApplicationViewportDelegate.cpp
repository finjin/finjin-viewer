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


//Includes----------------------------------------------------------------------
#include "FinjinViewerPrecompiled.hpp"
#include "FinjinViewerApplicationViewportDelegate.hpp"
#include <finjin/common/Allocator.hpp>
#include <finjin/common/DebugLog.hpp>
#include <finjin/common/JobSystem.hpp>
#include <finjin/engine/AssetCountsSettings.hpp>

using namespace Finjin::Viewer;


//Macros------------------------------------------------------------------------
#define USE_TEST_SCENE_AS_FALLBACK 1


//Local functions---------------------------------------------------------------
static void WriteScreenCapture(const StandardPaths& standardPaths, const ScreenCapture& screenCapture, bool forceRaw = false)
{
    static int screenCaptureCount = 0;
    
    auto standardScreenCapturePath = standardPaths.GetBestSavedScreenCapturePath();
    if (standardScreenCapturePath == nullptr)
        return;
    
    auto filePath = standardScreenCapturePath->path;
    filePath /= "finjin-viewer-screenshot-";
    filePath += Convert::ToString(screenCaptureCount);
    filePath += ".";
    
    ScreenCaptureWriteSettings writeSettings;
    writeSettings.writeUnsupportedFormatAsRaw = true;
    auto writeResult = screenCapture.WriteToFile(filePath, writeSettings);
    if (writeResult == ScreenCapture::WriteResult::SUCCESS)
    {
        FINJIN_DEBUG_LOG_INFO("Saved screenshot to '%1%'.", filePath);

        screenCaptureCount++;
    }
    else
    {
        FINJIN_DEBUG_LOG_INFO("Failed to write screenshot: %1%", screenCapture.GetWriteResultString(writeResult));
    }    
}


//Implementation----------------------------------------------------------------
FinjinViewerApplicationViewportDelegate::FinjinViewerApplicationViewportDelegate(Allocator* allocator, const Utf8String& _loadFileName, bool _startInVR) :
    ApplicationViewportDelegate(allocator),
    sceneReader(allocator),
    tempAssetRef(allocator),
    loadFileName(_loadFileName, allocator),
    startInVR(_startInVR)
{
    this->totalElapsedTime = 0;
    this->runState = RunState::STARTING;
    this->clearColor = MathVector4(0, 0, 0, 1);
    this->moveUnitsPerSecond = 30.0f;
    this->rotateUnitsPerSecond = 1.0f;
    this->allowRelativeMove = true;
    this->allowRelativeLook = true;
    this->lifetimeFrameSequenceIndex = 0;
    this->flyingCameraGameControllerIndex = (size_t)-1;
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
                    FINJIN_SET_ERROR(error, "Failed to read asset counts file 'scene-reader-asset-counts.cfg'.");
                    return UpdateResult::LOGIC_ONLY;
                }
                stateSettings.assetCounts = assetCountsSettings.assetCounts;

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
                this->tempAssetRef.ForLocalFile(FlyingCameraInputBindings::GetDefaultBindingsFileName());
                                
            #if FINJIN_TARGET_VR_SYSTEM != FINJIN_TARGET_VR_SYSTEM_NONE
                if (updateContext.vrContext != nullptr && updateContext.vrContext->GetInitializationStatus() == VRContextInitializationStatus::INITIALIZED && this->startInVR)
                {
                    auto result = this->headsetFlyingCameraInputBindings.GetFromConfiguration(updateContext.inputContext, InputBindingsConfigurationSearchCriteria(InputDeviceClass::HEADSET, Utf8String::GetEmpty(), (size_t)0, InputBindingsConfigurationFlag::CONNECTED_ONLY, InputDeviceSemantic::NONE), this->tempAssetRef, this->tempBuffer);
                    if (result.IsSuccess())
                    {
                        this->allowRelativeMove = false;
                        this->allowRelativeLook = false;

                        FINJIN_DEBUG_LOG_INFO("Headset successfully configured.");
                        this->flyingCameraGameControllerIndex = result.deviceIndex;
                    }
                }
            #endif                
                {
                    auto result = this->flyingCameraInputBindings.GetFromConfiguration(updateContext.inputContext, InputBindingsConfigurationSearchCriteria(InputDeviceClass::GAME_CONTROLLER, Utf8String::GetEmpty(), (size_t)-1, InputBindingsConfigurationFlag::CONNECTED_ONLY, InputDeviceSemantic::NONE), this->tempAssetRef, this->tempBuffer);
                    if (result.IsSuccess())
                    {
                        FINJIN_DEBUG_LOG_INFO("Game controller successfully configured.");
                        this->flyingCameraGameControllerIndex = result.deviceIndex;
                    }
                }
                auto result = this->flyingCameraInputBindings.GetFromConfiguration(updateContext.inputContext, InputBindingsConfigurationSearchCriteria(InputDeviceClass::KEYBOARD, (size_t)-1), this->tempAssetRef, this->tempBuffer);
                if (result.IsSuccess())
                {
                    FINJIN_DEBUG_LOG_INFO("Keyboard successfully configured.");
                }
                result = this->flyingCameraInputBindings.GetFromConfiguration(updateContext.inputContext, InputBindingsConfigurationSearchCriteria(InputDeviceClass::MOUSE, (size_t)-1), this->tempAssetRef, this->tempBuffer);
                if (result.IsSuccess())
                {
                    FINJIN_DEBUG_LOG_INFO("Mouse successfully configured.");
                }
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

        FlyingCameraEvents flyingCameraActions, headsetFlyingCameraActions;
        HandleEventsAndInputs(updateContext, flyingCameraActions, headsetFlyingCameraActions);

        StartFrame(updateContext, flyingCameraActions, headsetFlyingCameraActions);
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
                    FINJIN_DEBUG_LOG_INFO("  Light: %1%, type: %2%, light type: %3%", item.name, item.GetTypeDescription().GetName(), (int)item.lightType);
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
                    FINJIN_DEBUG_LOG_INFO("  Node: %1%, type: %2%", item.name, item.GetTypeDescription().GetName());
                }
            }

            {
                auto result = scene.Get<FinjinSceneObjectEntity>();
                FINJIN_DEBUG_LOG_INFO("Entity count: %1%", result.size());
                for (auto& item : result)
                {
                    FINJIN_DEBUG_LOG_INFO("  Entity: %1%, type: %2%", item.name, item.GetTypeDescription().GetName());
                }
            }

            {
                auto result = scene.Get<FinjinSceneObjectCamera>();
                FINJIN_DEBUG_LOG_INFO("Camera count: %1%", result.size());
                for (auto& item : result)
                {
                    FINJIN_DEBUG_LOG_INFO("  Active camera: %1%, type: %2%", item.name, item.GetTypeDescription().GetName());

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

void FinjinViewerApplicationViewportDelegate::StartFrame(ApplicationViewportUpdateContext& updateContext, FlyingCameraEvents& flyingCameraActions, FlyingCameraEvents& headsetFlyingCameraActions)
{
    //Kick off jobs for the frame
    
    this->totalElapsedTime += updateContext.elapsedTime;

    auto& frameStage = updateContext.gpuContext->StartFrameStage(updateContext.jobPipelineStage->index, updateContext.elapsedTime, this->totalElapsedTime);

    auto frameSequenceIndex = this->lifetimeFrameSequenceIndex++;

    ScreenCapture screenCapture;
    if (updateContext.gpuContext->GetScreenCapture(screenCapture, frameStage) == ScreenCaptureResult::SUCCESS)
    {
        WriteScreenCapture(*updateContext.standardPaths, screenCapture);
    }

    updateContext.jobSystem->StartGroupFromMainThread();
    updateContext.jobPipelineStage->simulateAndRenderFuture = updateContext.jobSystem->Submit([this, &updateContext, flyingCameraActions, headsetFlyingCameraActions, &frameStage, frameSequenceIndex]()
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
            if (headsetFlyingCameraActions.Contains(FlyingCameraEvents::LOCATOR))
            {
                //This handler must come before others that depend on updating the camera
                if (!this->allowRelativeMove)
                {
                    this->camera.SetPosition(headsetFlyingCameraActions.lookHeadset.position);
                    cameraChanged = true;
                }
                if (!this->allowRelativeLook)
                {
                    this->camera.SetOrientationFromColumns(headsetFlyingCameraActions.lookHeadset.orientation);
                    cameraChanged = true;
                }                
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::MOVE))
            {
                if (this->allowRelativeMove)
                {
                    this->camera.Pan(flyingCameraActions.move[0] * updateContext.elapsedTime * this->moveUnitsPerSecond, 0);
                    this->camera.Walk(flyingCameraActions.move[1] * updateContext.elapsedTime * this->moveUnitsPerSecond);

                    cameraChanged = true;

                    FINJIN_DEBUG_LOG_INFO("Move: %1% %2%", flyingCameraActions.move[0], flyingCameraActions.move[1]);
                }
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::PAN_ORBIT))
            {
                if (this->allowRelativeMove)
                {
                    this->camera.Pan(flyingCameraActions.panOrbit[0] * updateContext.elapsedTime * this->moveUnitsPerSecond, flyingCameraActions.panOrbit[1] * updateContext.elapsedTime * this->moveUnitsPerSecond);

                    FINJIN_DEBUG_LOG_INFO("PanOrbit: %1% %2%", flyingCameraActions.panOrbit[0], flyingCameraActions.panOrbit[1]);
                }
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::LOOK))
            {
                if (this->allowRelativeLook)
                {
                    this->camera.RotateY(Radians(flyingCameraActions.look[0] * updateContext.elapsedTime * this->rotateUnitsPerSecond * -1.0f)); //Negate since a left 'look' is negative but represents a positive rotation about Y
                    this->camera.Pitch(Radians(flyingCameraActions.look[1] * updateContext.elapsedTime * this->rotateUnitsPerSecond));

                    cameraChanged = true;

                    FINJIN_DEBUG_LOG_INFO("Look: %1% %2%", flyingCameraActions.look[0], flyingCameraActions.look[1]);
                }
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
                updateContext.gpuCommands.CaptureScreen();
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::ESCAPE))
            {
                FINJIN_DEBUG_LOG_INFO("Escape");
                //updateContext.RequestClose();
                //updateContext.RequestApplicationExit();
            }
            if (flyingCameraActions.Contains(FlyingCameraEvents::MENU))
            {
                FINJIN_DEBUG_LOG_INFO("Menu");
            }

            if (cameraChanged)
                this->camera.Update();

            if (!updateContext.gpuCommands.StartGraphicsCommands())
            {
            }

            //GPU
            if (AnySet(updateContext.gpuContext->GetAssetCreationCapabilities(AssetClass::MESH) & AssetCreationCapability::FRAME_THREAD))
            {
                for (auto& item : updateContext.newMeshes)
                {
                    if (!updateContext.gpuCommands.CreateMesh(&item))
                    {
                    }
                }
            }
            if (AnySet(updateContext.gpuContext->GetAssetCreationCapabilities(AssetClass::TEXTURE) & AssetCreationCapability::FRAME_THREAD))
            {
                for (auto& item : updateContext.newTextures)
                {
                    if (!updateContext.gpuCommands.CreateTexture(&item))
                    {
                    }
                }
            }
            if (AnySet(updateContext.gpuContext->GetAssetCreationCapabilities(AssetClass::MATERIAL) & AssetCreationCapability::FRAME_THREAD))
            {
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
                //updateContext.gpuCommands.SetClearColor(MathVector4(1, 0, 0, 1));

                if (this->sceneData.scene != nullptr)
                {
                    for (auto& subscene : this->sceneData.scene->subscenes)
                    {
                        auto& ambientLightState = subscene.environment.ambientLight.Evaluate(updateContext.jobPipelineStage->index, frameSequenceIndex);

                        for (auto sceneNode : subscene.sceneNodes)
                        {
                            auto& sceneNodeState = sceneNode->Evaluate(updateContext.jobPipelineStage->index, frameSequenceIndex);

                            for (auto obj : sceneNode->objects)
                            {
                                if (obj->IsTypeOf(FINJIN_TYPE_DESCRIPTION(FinjinSceneObjectEntity)))
                                {
                                    auto entity = reinterpret_cast<FinjinSceneObjectEntity*>(obj);

                                    auto shaderFeatureFlags = RenderShaderFeatureFlags::GetDefault();
                                    //shaderFeatureFlags.renderingFlags |= ShaderFeatureFlag::RENDERING_FILL_WIREFRAME;

                                    GpuCommandLights lights;
                                    for (auto light : this->sceneData.lights)
                                    {
                                        if (light->isActive)
                                        {
                                            auto& lightSceneNodeState = light->parentNodePointer->Evaluate(updateContext.jobPipelineStage->index, frameSequenceIndex);
                                            lights.push_back(GpuCommandLight(light, &light->Evaluate(updateContext.jobPipelineStage->index, frameSequenceIndex, lightSceneNodeState)));
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

            if (!updateContext.gpuCommands.FinishGraphicsCommands())
            {
            }

            if (!updateContext.gpuCommands.ExecuteGraphicsCommands())
            {
            }

            updateContext.jobSystem->FinishGroupFromNonMainThread();
        }

        //Render----------------------------
        {
            FINJIN_DECLARE_ERROR(error);

            updateContext.Execute(frameStage, error);
            if (error)
            {
                FINJIN_DEBUG_LOG_ERROR("Failed to execute frame stage: %1%", error.ToString());
            }
            else
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
            renderContext.gpuContext->PresentFrameStage(frameStage, renderContext.renderStatus, renderContext.presentSyncIntervalOverride, error);
            if (error)
            {
                FINJIN_SET_ERROR(error, "Failed to present back buffer.");
                FINJIN_DEBUG_LOG_ERROR("Present frame error: %1%", error.ToString());
            }
        }
    }
}

void FinjinViewerApplicationViewportDelegate::HandleEventsAndInputs(ApplicationViewportUpdateContext& updateContext, FlyingCameraEvents& flyingCameraActions, FlyingCameraEvents& headsetFlyingCameraActions)
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

    //Poll subsystems that require it
    updateContext.StartPoll();

    //If no game controller has ever been detected, try to detect one
#if FINJIN_TARGET_PLATFORM_IS_APPLE || FINJIN_TARGET_PLATFORM_IS_LINUX
    if (this->flyingCameraGameControllerIndex == (size_t)-1)
    {
        for (size_t i = 0; i < updateContext.inputContext->GetGameControllerCount(); i++)
        {
            auto gameController = updateContext.inputContext->GetGameController(i);
            if (gameController->IsNewConnection())
            {
                this->tempAssetRef.ForLocalFile(FlyingCameraInputBindings::GetDefaultBindingsFileName());
                auto result = this->flyingCameraInputBindings.GetFromConfiguration(updateContext.inputContext, InputBindingsConfigurationSearchCriteria(InputDeviceClass::GAME_CONTROLLER, Utf8String::GetEmpty(), i, InputBindingsConfigurationFlag::CONNECTED_ONLY, InputDeviceSemantic::NONE), this->tempAssetRef, this->tempBuffer);
                if (result.IsSuccess())
                {
                    FINJIN_DEBUG_LOG_INFO("Game controller successfully configured.");
                    this->flyingCameraGameControllerIndex = i;
                }
            }
        }
    }
#endif

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
    updateContext.inputContext->GetActions(flyingCameraActions, this->flyingCameraInputBindings, updateContext.elapsedTime);
    updateContext.inputContext->GetActions(headsetFlyingCameraActions, this->headsetFlyingCameraInputBindings, updateContext.elapsedTime);

    updateContext.FinishPoll();
}
