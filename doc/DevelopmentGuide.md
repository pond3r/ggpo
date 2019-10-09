# GGPO Developer Guide

The GGPO Network Library Developers Guide is for developers who are integrating the GGPO Network Library into their applications.

## Game State and Inputs

Your game probably has many moving parts.  GGPO only depends on these two: 

- **Game State** describes the current state of everything in your game.  In a shooter, this would include the position of the ship and all the enemies on the screen, the location of all the bullets, how much health each opponent has, the current score, etc. etc. 

- **Game Inputs** are the set of things which modify the game state.  These obviously include the joystick and button presses done by the player, but can include other non-obvious inputs as well.  For example, if your game uses the current time of day to calculate something in the game, the current time of day at the beginning of a frame is also an input.

There are many other things in your game engine that are neither game state nor inputs.  For example, your audio and video renderers are not game state since they don’t have an effect on the outcome of the game.  If you have a special effects engine that’s generating effects that do not have an impact on the game, they can be excluded from the game state as well.

## Using State and Inputs for Synchronization

Each player in a GGPO networked game has a complete copy of your game running.  GGPO needs to keep both copies of the game state in sync to ensure that both players are experiencing the same game.  It would be much too expensive to send an entire copy of the game state between players every frame.  Instead GGPO sends the players’ inputs to each other and has each player step the game forward.  In order for this to work, your game engine must meet three criteria:

- The game simulation must be fully deterministic.  That is, for any given game state and inputs, advancing the game state by exactly 1 frame must result in identical game states for all players.
- The game state must be fully encapsulated and serializable.  
- Your game engine must be able to load, save, and execute a single simulation frame without rendering the result of that frame.  This will be used to implement rollbacks.

## Programming Guide

The following section contains a walk-through for porting your application to GGPO.  For a detailed description of the GGPO API, please see the GGPO Reference section, below.

### Interfacing with GGPO

GGPO is designed to be easy to interface with new and existing game engines.  It handles most of the implementation of handling rollbacks by calling out to your application via the `GGPOSessionCallbacks` hooks.

### Creating the GGPOSession Object

The `GGPOSession` object is your interface to the GGPO framework.  Create one with the `ggponet_start_session` function passing the port to bind to locally and the IP address and port of the player you’d like to play against.   You should also pass in a `GGPOSessionCallbacks` object filled in with your game’s callback functions for managing game state and whether this session is for player 1 or player 2.  All `GGPOSessionCallback` functions must be implemented.  See the reference for more details. 
For example, to start a new session on the same host with another player bound to port 8001, you would do:

```
   GGPOSession ggpo;
   GGPOErrorCode result;
   GGPOSessionCallbacks cb;

   /* fill in all callback functions */
   cb.begin_game = vw_begin_game_callback;
   cb.advance_frame = vw_advance_frame_callback;
   cb.load_game_state = vw_load_game_state_callback;
   cb.save_game_state = vw_save_game_state_callback;
   cb.free_buffer = vw_free_buffer;
   cb.on_event = vw_on_event_callback;

   /* Start a new session */
   result = ggpo_start_session(&ggpo,         // the new session object
                               &cb,           // our callbacks
                               "test_app",    // application name
                               2,             // 2 players
                               sizeof(int),   // size of an input packet
                               8001);         // our local udp port
```



The `GGPOSession` object should only be used for a single game session.  If you need to connect to another opponent, close your existing object using `ggpo_close_session` and start a new one:

```
   /* Close the current session and start a new one */
   ggpo_close_session(ggpo);
```

### Sending Player Locations
When you created the GGPOSession object passed in the number of players participating in the game, but didn't actually describe how to contact them.  To do so, call the `ggpo_add_player` function with a `GGPOPlayer` object describing each player.   The following example show how you might use ggpo_add_player in a 2 player game:

```
GGPOPlayer p1, p2;
GGPOPlayerHandle player_handles[2];

p1.size = p2.size = sizeof(GGPOPlayer);
p1.type = GGPO_PLAYERTYPE_LOCAL;                // local player
p2.type = GGPO_PLAYERTYPE_REMOTE;               // remote player
strcpy(p2.remote.ip_address, "192.168.0.100");  // ip addess of the player
p2.remote.ip_address.port = 8001;               // port of that player

result = ggpo_add_player(ggpo, &p1,  &player_handles[0]);
...
result = ggpo_add_player(ggpo, &p2,  &player_handles[1]);
```

### Synchronizing Local and Remote Inputs
Input synchronization happens at the top of each game frame.  This is done by calling `ggpo_add_local_input` for each local player and `ggpo_synchronize_input` to fetch the inputs for remote players.
Be sure to check the return value of `ggpo_synchronize_inputs`.  If it returns a value other than `GGPO_OK`, you should not advance your game state.  This usually happens because GGPO has not received packets from the remote player in a while and has reached its internal prediction limit.

For example, if your code looks like this currently for a local game:

```
   GameInputs &p1, &p2;
   GetControllerInputs(0, &p1); /* read p1’s controller inputs */
   GetControllerInputs(1, &p2); /* read p2’s controller inputs */
   AdvanceGameState(&p1, &p1, &gamestate); /* send p1 and p2 to the game */
```

You should change it to read as follows:

```
   GameInputs p[2];
   GetControllerInputs(0, &p[0]); /* read the controller */

   /* notify ggpo of the local player's inputs */
   result = ggpo_add_local_input(ggpo,               // the session object
                                 player_handles[0],  // handle for p1
                                 &p[0],              // p1's inputs
                                 sizeof(p[0]));      // size of p1's inputs

   /* synchronize the local and remote inputs */
   if (GGPO_SUCCEEDED(result)) {
      result = ggpo_synchronize_inputs(ggpo,         // the session object
                                       p,            // array of inputs
                                       sizeof(p));   // size of all inputs
      if (GGPO_SUCCEEDED(result)) {
         /* pass both inputs to our advance function */
         AdvanceGameState(&p[0], &p[1], &gamestate);
      }
   }
```

You should call `ggpo_synchronize_inputs` every frame, even those that happen during a rollback.  Make sure you always use the values returned from `ggpo_synchronize_inputs` rather than the values you've read from the local controllers to advance your game state. During a rollback `ggpo_synchronize_inputs` will replace the values passed into `ggpo_add_local_input` with the values used for previous frames.  Also, if you've manually added input delay for the local player to smooth out the effect of rollbacks, the inputs you pass into `ggpo_add_local_input` won't actually be returned in `ggpo_synchronize_inputs` until after the frame delay.

### Implementing your save, load, and free callbacks
GGPO will use the `load_game_state` and `save_game_state` callbacks to periodically save and restore the state of your game.  The `save_game_state` function should create a buffer containing enough information to restore the current state of the game and return it in the `buffer` out parameter.  The `load_game_state` function should restore the game state from a previously saved buffer.  For example:

```
struct GameState gamestate;  // Suppose the authoratative value of our game's state is in here.

bool __cdecl
ggpo_save_game_state_callback(unsigned char **buffer, int *len,
                               int *checksum, int frame)
{
   *len = sizeof(gamestate);
   *buffer = (unsigned char *)malloc(*len);
   if (!*buffer) {
      return false;
   }
   memcpy(*buffer, &gamestate, *len);
   return true;
}

bool __cdecl
ggpo_load_game_state_callback(unsigned char *buffer, int len)
{
   memcpy(&gamestate, buffer, len);
   return true;
}
```

GGPO will call your `free_buffer` callback to dispose of the memory you allocated in your `save_game_state` callback when it is no longer need.  

```
void __cdecl 
ggpo_free_buffer(void *buffer)
{
   free(buffer);
}
```

### Implementing Remaining Callbacks
As mentioned previously, there are no optional callbacks in the `GGPOSessionCallbacks` structure.  They all need to at least `return true`, but the remaining callbacks do not necessarily need to be implemented right away.  See the comments in `ggponet.h` for more information.

### Calling the GGPO Advance and Idle Functions
We're almost done.  Promise.  The last step is notify GGPO every time your gamestate finishes advancing by one frame.  Just call `ggpo_advance_frame` after you’ve finished one frame but before you’ve started the next.

GGPO also needs some amount of time to send and receive packets do its own internal bookkeeping.  At least once per-frame you should call the `ggpo_idle` function with the number of milliseconds you’re allowing GGPO to spend. 

## Tuning Your Application: Frame Delay vs. Speculative Execution
GGPO uses both frame delay and speculative execution to hide latency.  It does so by allowing the application developer the choice of how many frames that they’d like to delay input by.  If it takes more time to transmit a packet than the number of frames specified by the game, GGPO will use speculative execution to hide the remaining latency.  This number can be tuned by the application mid-game if you so desire.  Choosing a proper value for the frame delay depends very much on your game.  Here are some helpful hints.

In general you should try to make your frame delay as high as possible without affecting the qualitative experience of the game.  For example, a fighting game requires pixel perfect accuracy, excellent timing, and extremely tightly controlled joystick motions.  For this type of game, any frame delay larger than 1 can be noticed by most intermediate players, and expert players may even notice a single frame of delay.  On the other hand, board game or puzzle games which do not have very strict timing requirements may get away with setting the frame latency as high as 4 or 5 before users begin to notice.

Another reason to set the frame delay high is to eliminate the glitching that can occur during a rollback. The longer the rollback, the more likely the user is to notice the discontinuities caused by temporarily executing the incorrect prediction frames.  For example, suppose your game has a feature where the entire screen will flash for exactly 2 frames immediately after the user presses a button.  Suppose further that you’ve chosen a value of 1 for the frame latency and the time to transmit a packet is 4 frames.  In this case, a rollback is likely to be around 3 frames (4 – 1 = 3).  If the flash occurs on the first frame of the rollback, your 2-second flash will be entirely consumed by the rollback, and the remote player will never get to see it!  In this case, you’re better off either specifying a higher frame latency value or redesigning your video renderer to delay the flash until after the rollback occurs.

## Sample Application
The Vector War application in the source directory contains a simple application which uses GGPO to synchronize the two clients.  The command line arguments are:

```
vectorwar.exe  <localport>  <num players> ('local' | <remote ip>:<remote port>) for each player
```

See the .bat files in the bin directory for examples on how to start 2, 3, and 4 player games.

## Best Practices and Troubleshooting
Below is a list of recommended best practices you should consider while porting your application to GGPO.  Many of these recommendations are easy to follow even if you’re not starting a game from scratch.  Most applications will already conform to most of the recommendations below.

### Isolate Game State from Non-Game State
GGPO will periodically request that you save and load the entire state of your game.  For most games the state that needs to be saved is a tiny fraction of the entire game.  Usually the video and audio renderers, look up tables, textures, sound data and your code segments are either constant from frame to frame or not involved in the calculation of game state.  These do not need to be saved or restored.

You should isolate non-game state from the game state as much as possible.  For example, you may consider encapsulating all your game state into a single C structure.  This both clearly delineates what is game state and was is not and makes it trivial to implement the save and load callbacks (see the Reference Guide for more information).

### Define a Fixed Time Quanta for Advancing Your Game State
GGPO will occasionally need to rollback and single-step your application frame by frame.  This is difficult to do if your game state advances by a variable tick rate.  You should try to make your game state advanced by a fixed time quanta per frame, even if your render loop does not.

### Separate Updating Game State from Rendering in Your Game Loop
GGPO will call your advance frame callback many times during a rollback.  Any effects or sounds which are genearted during the rollback need to be deferred until after the rollback is finished.  This is most easily accomplished by separating your game state from your render state.  When you’re finished, your game loop may look something like this:

```
   Bool finished = FALSE;
   GameState state;
   Inputs inputs;

   do {
      GetControllerInputs(&inputs);
      finished = AdvanceGameState(&inputs, &state);
      if (!finished) {
         RenderCurrentFrame(&gamestate);
      }
   while (!finished);
```

In other words, your game state should be determined solely by the inputs, your rendering code should be driven by the current game state, and you should have a way to easily advance the game state forward using a set of inputs without rendering.

### Make Sure Your Game State Advances Deterministically
Once you have your game state identified, make sure the next game state is computed solely as from your game inputs.  This should happen naturally if you have correctly identified all the game state and inputs, but it can be tricky sometimes.  Here are some things which are easy to overlook:

#### Beware OF Random Number Generators
Many games use random numbers in the computing of the next game state.  If you use one, you must ensure that they are fully deterministic, that the seed for the random number generator is same at frame 0 for both players, and that the state of the random number generator is included in your game state.  Doing both of these will ensure that the random numbers which get generated for a particular frame are always the same, regardless of how many times GGPO needs to rollback to that frame.

#### Beware of External Time Sources (aka. Wall clock time)
Be careful if you use the current time of day in your game state calculation.  This may be used for an effect on the game or to derive other game state (e.g. using the timer as a seed to the random number generator).  The time on two computers or game consoles is almost never in sync and using time in your game state calculations can lead to synchronization issues.  You should either eliminate the use of time in your game state or include the current time for one of the players as part of the input to a frame and always use that time in your calculations.

The use of external time sources in non-gamestate calculations is fine (e.g. computing the duration of effects on screen, or the attenuation of audio samples).

### Beware of dangling references
If your game state contains any dynamically allocated memory be very careful in your save and load functions to rebase your pointers as you save and load your data.  One way to mitigate this is to use a base and offset to reference allocated memory instead of a pointer.  This can greatly reduce the number of pointers you need to rebase.

### Beware of static Variables or Other Hidden State
The language your game is written in may have features which make it difficult to track down all your state.   Static automatic variables in C are an example of this behavior.  You need to track down all these locations and convert them to a form which can be saved.  For example, compare:

```
   // This will totally get you into trouble.
   int get_next_counter(void) {
      static int counter = 0; /* no way to roll this back... */
      counter++;
      return counter;
   }
```

To:
```
   // If you must, this is better
   static int global_counter = 0; /* move counter to a global */

   int get_next_counter(void) {
      global_counter++;
      return global_counter; /* use the global value */
   }

   bool __cdecl
   ggpo_load_game_state_callback(unsigned char *buffer, int len)
   {
      ...
      global_counter = *((int *)buffer) /* restore it in load callback */
      ...
      return true;
   }
```

### Use the GGPO SyncTest Feature.  A lot.
Once you’ve ported your application to GGPO, you can use the `ggpo_start_synctest` function to help track down synchronization issues which may be the result of leaky game state.  

The sync test session is a special, single player session which is designed to find errors in your simulation's determinism.  When running in a synctest session, ggpo will execute a 1 frame rollback for every frame if your game.  It compares the state of the frame when it was executed the first time to the state executed during the rollback, and raises an error if they differ.  If you used the `ggpo_log` function during your game's execution, you can diff the log of the initial frame vs the log of the rollback frame to track down errors.  

By running synctest on developer systems continuously when writing game code, you can identify desync causing bugs immediately after they're introduced.

## Where To Go From Here?
This document describes the most basic features of GGPO.  To learn more, I recommend starting with reading the comments in the `ggponet.h` header and just diving into the code.  Good luck!
