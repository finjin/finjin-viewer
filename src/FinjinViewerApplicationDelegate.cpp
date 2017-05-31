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
#include "FinjinViewerApplicationDelegate.hpp"
#include "FinjinViewerApplicationViewportDelegate.hpp"

using namespace Finjin::Viewer;


//Implementation----------------------------------------------------------------
FinjinViewerApplicationDelegate::FinjinViewerApplicationDelegate(Allocator* allocator) : ApplicationDelegate(allocator)
{
}

FinjinViewerApplicationDelegate::~FinjinViewerApplicationDelegate()
{
}

const Utf8String& FinjinViewerApplicationDelegate::GetName(ApplicationNameFormat format) const
{
    static const Utf8String displayName("Finjin Viewer");
    static const Utf8String upperCamelCaseName("FinjinViewer");
    static const Utf8String lowerCaseName("finjin-viewer");
    static const Utf8String dottedWithOrganizationPrefix("com.finjin.viewer");
    switch (format)
    {
        case ApplicationNameFormat::DISPLAY: return displayName;
        case ApplicationNameFormat::UPPER_CAMEL_CASE: return upperCamelCaseName;
        case ApplicationNameFormat::LOWER_CASE: return lowerCaseName;
        case ApplicationNameFormat::DOTTED_WITH_ORGANIZATION_PREFIX: return dottedWithOrganizationPrefix;
        default:
        {
            assert(0 && "Invalid application name format.");
            return upperCamelCaseName;
        }
    }
}

ReadCommandLineResult FinjinViewerApplicationDelegate::ReadCommandLineSettings(CommandLineArgsProcessor& argsProcessor, Error& error)
{
    FINJIN_ERROR_METHOD_START(error);

    this->commandLineSettings.ReadCommandLineSettings(argsProcessor, error);
    if (error)
    {
        FINJIN_SET_ERROR(error, "Failed to read application command line settings.");
        return ReadCommandLineResult::FAILURE;
    }

    return ReadCommandLineResult::SUCCESS;
}

const ApplicationSettings& FinjinViewerApplicationDelegate::GetApplicationSettings() const
{
    return this->commandLineSettings;
}

void FinjinViewerApplicationDelegate::OnDestroyStart()
{
}

void FinjinViewerApplicationDelegate::OnDestroyFinish()
{
}

size_t FinjinViewerApplicationDelegate::GetRequestedApplicationViewportDescriptionCount() const
{
    return this->appViewportDescriptions.size();
}

ApplicationViewportDescription& FinjinViewerApplicationDelegate::GetApplicationViewportDescription(size_t index)
{
    return this->appViewportDescriptions[index];
}

ApplicationViewportDelegate* FinjinViewerApplicationDelegate::CreateApplicationViewportDelegate(Allocator* allocator, size_t index)
{
    return AllocatedClass::New<FinjinViewerApplicationViewportDelegate>
        (
        allocator,
        FINJIN_CALLER_ARGUMENTS,
        this->commandLineSettings.fileName,
        this->commandLineSettings.startInVR
        );
}
