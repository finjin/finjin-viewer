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
#import "FinjinViewerUIApplicationDelegate.h"
#include "FinjinViewerApplicationDelegate.hpp"

FINJIN_USE_ENGINE_NAMESPACES
using namespace Finjin::Viewer;


//Implementation----------------------------------------------------------------
@implementation FinjinViewerUIApplicationDelegate

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    //Set up Finjin application delegate
    self.engineApplicationDelegate = AllocatedClass::New<FinjinViewerApplicationDelegate>(self.engineAllocator, FINJIN_CALLER_ARGUMENTS);

    //Call superclass method
    return [super application:application didFinishLaunchingWithOptions:launchOptions];
}

@end
