![](doc/images/ggpo_header.png)

## What's GGPO?
Traditional techniques account for network transmission time by adding delay to a players input, resulting in a sluggish, laggy game-feel.  Rollback networking uses input prediction and speculative execution to send player inputs to the game immediately, providing the illusion of a zero-latency network.  Using rollback, the same timings, reactions visual and audio queues, and muscle memory your players build up playing offline translate directly online.  The GGPO networking SDK is designed to make incorporating rollback networking into new and existing games as easy as possible.  

For more information about the history of GGPO, check out http://ggpo.net/

This repository contains the code, documentation, and sample applications for the SDK.

## Building
Building GGPO is currently only available on Windows, however efforts are being made to port it to other platforms. To get started, clone the repository using either git or svn:
```git clone https://github.com/pond3r/ggpo.git```
**OR**
```svn checkout https://github.com/pond3r/ggpo```

### Windows 
On windows, it's recommended to use Visual Studio 2019 and cmake-gui (which can be downloaded [here](https://cmake.org/download/)). Once everything has been setup, open cmake-gui and select the root of the repository under ```Where is the source code:``` followed by the build folder under ```Where to build the binaries:```. Next, hit ```Configure``` and change any options that appear in the main window before hitting ```Generate``` which will generate the VS2019 solution. Once the solution has been generated, hit ```Open Project``` and build with the appropriate settings (Debug/Release). The built binaries can then be found where they were configured to be built.

## Licensing
GGPO is available under The MIT License. This means GGPO is free for commercial and non-commercial use. Attribution is not required, but appreciated. 
