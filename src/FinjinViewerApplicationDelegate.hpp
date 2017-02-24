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


//Includes---------------------------------------------------------------------
#include "finjin/engine/ApplicationDelegate.hpp"
#include "FinjinViewerApplicationSettings.hpp"


//Classes----------------------------------------------------------------------
namespace Finjin { namespace Viewer {

    FINJIN_USE_ENGINE_NAMESPACES

    class FinjinViewerApplicationDelegate : public ApplicationDelegate
    {   
    public:
        FinjinViewerApplicationDelegate(Allocator* allocator);
        ~FinjinViewerApplicationDelegate();

        const Utf8String& GetName(ApplicationNameFormat format) const override;

        void ReadCommandLineSettings(CommandLineArgsProcessor& argsProcessor, Error& error) override;
        
        const ApplicationSettings& GetApplicationSettings() const override;
        
        void OnStart() override;
        void OnStop() override;

        size_t GetRequestedApplicationViewportDescriptionCount() const override;
        ApplicationViewportDescription& GetApplicationViewportDescription(size_t index) override;

        ApplicationViewportDelegate* CreateApplicationViewportDelegate(Allocator* allocator, size_t index) override;
        
    private:
        FinjinViewerApplicationSettings commandLineSettings;
        
        std::array<ApplicationViewportDescription, 1> appViewportDescriptions;
    };

} }
