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
#include "FinjinViewerApplicationSettings.hpp"
#include "finjin/engine/AssetClass.hpp"

using namespace Finjin::Viewer;


//Implementation---------------------------------------------------------------
FinjinViewerApplicationSettings::FinjinViewerApplicationSettings()
{
}

void FinjinViewerApplicationSettings::ReadCommandLineSettings(CommandLineArgsProcessor& argsProcessor, Error& error)
{
    FINJIN_ERROR_METHOD_START(error);

    Utf8String arg;
    for (size_t index = 0; index < argsProcessor.GetCount(); index++)
    {
        arg = argsProcessor[index];
        
        if (arg == "-file" && index < argsProcessor.GetCount() - 1)
        {
            auto& argValue = argsProcessor[index + 1];
            
            Path path(argValue);
            if (path.IsAbsolute())
            {
                if (path.IsFile())
                {
                    path.GetFileName(this->fileName.value);
                    this->fileName.isSet = true;
                    
                    path.GoToParent();
                    if (path.EndsWith(AssetClassUtilities::ToDirectoryName(AssetClass::SCENE)))
                    {
                        path.GetParent(this->additionalReadApplicationAssetsDirectory);
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

            index++;
        }
    }

    ApplicationSettings::ReadCommandLineSettings(argsProcessor, error);
    if (error)
    {
        FINJIN_SET_ERROR_NO_MESSAGE(error);
        return;
    }
}
