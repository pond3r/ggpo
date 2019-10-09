![](doc/images/ggpo_header.png)

## What's GGPO?
Traditional techniques account for network transmission time by adding delay to a players input, resulting in a sluggish, laggy game-feel.  Rollback networking uses input prediction and speculative execution to send player inputs to the game immediately, providing the illusion of a zero-latency network.  Using rollback, the same timings, reactions visual and audio queues, and muscle memory your players build up playing offline translate directly online.  The GGPO networking SDK is designed to make incorporating rollback networking into new and existing games as easy as possible.  

For more information about the history of GGPO, check out http://ggpo.net/

This repository contains the code, documentation, and sample application for the Windows version of the SDK (requires Visual Studio 2019).

## Licensing
GGPO is available under The MIT License.  This means GGPO is free for commercial and non-commercial use.  I hope making GGPO free will remove all the barriers for Indie and professional game developers alike to investating adding rollback networking to their games.  If you're using GGPO in your game, I'd appreciate it if you could include the GGPO mark somewhere in your attribution screen to help get the word out.  I'm also welcome to pull requests for bug fixes or platform ports, of course.
