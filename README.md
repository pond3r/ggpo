# GGPOUE4

A port of [GGPO](http://ggpo.net) to an Unreal Engine 4 plugin. This was branched from BwdYeti's great initial work on this repo [here](https://github.com/BwdYeti/GGPOUE4).

## Setup & Usage

Add to the Plugins folder of your Unreal project.

See [doc/README.md](doc/README.md), [doc/DeveloperGuide.md](doc/DeveloperGuide.md), and the [GGPO GitHub](https://github.com/pond3r/ggpo) for more information.

### Sample Application

Use branched repo of BwdYeti's [VectorWar UE4](https://github.com/erebuswolf/VectorWarUE4) which is a port of the GGPO sample game VectorWar, using GGPOUE4 for netcode.

### Issues

Currently the Connection Manager abstraction interface only has ip based connections implimented which only work on windows. Adding support for Steam and Epic network communication libraries is completely possible with this codebase, but requries setting up a project with those backends and testing which I do not currently have time for.

## Licensing

GGPO is available under The MIT License. This means GGPO is free for commercial and non-commercial use. Attribution is not required, but appreciated. 
