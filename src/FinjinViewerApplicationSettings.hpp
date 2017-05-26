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
#include <finjin/engine/ApplicationSettings.hpp>


//Types-------------------------------------------------------------------------
namespace Finjin { namespace Viewer {

    FINJIN_USE_ENGINE_NAMESPACES

    class FinjinViewerApplicationSettings : public ApplicationSettings
    {
    public:
        FinjinViewerApplicationSettings();

        void ReadCommandLineSettings(CommandLineArgsProcessor& argsProcessor, Error& error);

    public:
        Setting<Utf8String> fileName; //Just the file name, such as "test.fstd-scene"
    };

} }
