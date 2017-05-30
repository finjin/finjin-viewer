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
#include "FinjinViewerApplicationSettings.hpp"
#include <finjin/engine/AssetClass.hpp>

using namespace Finjin::Viewer;


//Implementation----------------------------------------------------------------
FinjinViewerApplicationSettings::FinjinViewerApplicationSettings()
{
    this->checkSystemMemoryFree = false;
    //this->startInVR = true;    
}

void FinjinViewerApplicationSettings::ReadCommandLineSettings(CommandLineArgsProcessor& argsProcessor, Error& error)
{
    FINJIN_ERROR_METHOD_START(error);

    for (size_t argIndex = 0; argIndex < argsProcessor.GetCount(); argIndex++)
    {
        auto& arg = argsProcessor[argIndex];

        if (arg == "-file" && argIndex < argsProcessor.GetCount() - 1)
        {
            auto& argValue = argsProcessor[argIndex + 1];

            Path path(argValue);
            if (path.IsAbsolute())
            {
                if (path.IsFile())
                {
                    //Hold onto just the file name
                    path.GetFileName(this->fileName.value);
                    this->fileName.isSet = true;

                    path.GoToParent();
                    if (AssetClassUtilities::IsDirectoryName(path, AssetClass::SCENE))
                    {
                        //Hold onto the path that contains the scene directory
                        path.GetParent(this->additionalReadApplicationAssetsDirectory.value);
                        this->additionalReadApplicationAssetsDirectory.isSet = true;
                    }
                }
                else
                {
                    FINJIN_SET_ERROR(error, FINJIN_FORMAT_ERROR_MESSAGE("Specified '-file' argument '%1%' is not a file.", argValue));
                    return;
                }
            }
            else
                this->fileName = argValue;

            argIndex++;
        }
    }

    ApplicationSettings::ReadCommandLineSettings(argsProcessor, error);
    if (error)
    {
        FINJIN_SET_ERROR_NO_MESSAGE(error);
        return;
    }
}
