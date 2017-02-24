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


//Includes-----------------------------------------------------------------------------
#include "FinjinPrecompiled.hpp"
#import "ViewerNSApplicationDelegate.h"
#include "FinjinViewerApplicationDelegate.hpp"

using namespace Finjin::Common;
using namespace Finjin::Viewer;


//Implementation--------------------------------------------------------------------------
@implementation ViewerNSApplicationDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification
{
    //Set up Finjin application delegate
    self.engineApplicationDelegate = AllocatedClass::New<FinjinViewerApplicationDelegate>(self.engineAllocator, FINJIN_CALLER_ARGUMENTS);
    
    //Call superclass method
    [super applicationDidFinishLaunching:aNotification];
}

@end
