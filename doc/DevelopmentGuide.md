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

Notifying GGPO of the PLayers
When you created the GGPOSession object, you notified it of the number of players you planned to use, but didn't actually describe where those players where.  To do so, call the ggpo_add_player  function with a GGPOPlayer object describing each player.   The following example show how you might use ggpo_add_player in a 2 player game:
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
Synchronizing Input	
You need to notify the GGPO session object of the inputs for a current frame before passing them onto your game engine.  There are two reasons for this:  GGPO needs to send those inputs to the remote player so that they can be used in the game over there and it needs to notify you of the inputs that it receives over the network.  This is done by calling ggpo_add_local_input for each local player and ggpo_synchronize_input to fetch the inputs for remote players.
Be sure to check the return value of ggpo_synchronize_inputs.  If it returns a value other than GGPO_OK, you should not advance your game state.  This usually happens because GGPO has not received packets from the remote player in a while and has reached its internal prediction limit.
For example, if your code looks like this currently for a local game:

   GameInputs &p1, &p2;
   GetControllerInputs(0, &p1); /* read p1’s controller inputs */
   GetControllerInputs(1, &p2); /* read p2’s controller inputs */
   AdvanceGameState(&p1, &p1, &gamestate); /* send p1 and p2 to the game */

You should change it to read as follows:
   GameInputs &p[2];
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

You should call ggpo_synchronize_inputs for all frames of execution, even those that happen during a rollback.  Make sure you always use the values returned from ggpo_synchronize_inputs rather than the values you've read from the local controllers to advance your game state.  During a rollback ggpo_synchronize_inputs will replace the values passed into ggpo_add_local_input with the values used for previous frames.
Implementing your save, load, and free callbacks
GGPO will use the load_game_state and save_game_state callbacks to periodically save and restore the state of your game.  The save_game_state function needs to allocate enough memory to save the current game state.  The load_game_state function needs to take a previously saved state and copy it into your current game state:
struct GameState gamestate;

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


GGPO will also periodically call your free_buffer callback to dispose of the memory you allocated in your save_game_state callback:

void __cdecl 
ggpo_free_buffer(void *buffer)
{
   free(buffer);
}

Implementing Remaining Callbacks
All callbacks in the GGPOSessionCallbacks structure need to be implemented, but the remaining ones they do not need to do anything other than return true.  See the Reference Guide for their descriptions.
Calling the GGPO Advance and IdlE Functions
The last thing you need to do is notify GGPO every time your gamestate finishes advancing by one frame.  Just call ggpo_advance_frame after you’ve finished one frame but before you’ve started the next.  
GGPO also needs some amount of time to send and receive packets do its own internal bookkeeping.  At least once per-frame you should call the ggpo_idle function with the number of milliseconds you’re allowing GGPO to spend. 
Tuning Your Application: Frame Delay vs. Speculative Execution
GGPO uses both frame delay and speculative execution to hide latency.  It does so by allowing the application developer the choice of how many frames that they’d like to delay input by.  If it takes more time to transmit a packet than the number of frames specified by the game, GGPO will use speculative execution to hide the remaining latency.  This number can be tuned by the application at runtime. Choosing a proper value for the frame delay depends very much on your game.   
Here are some helpful hints.
In general you should try to make your frame delay as high as possible without affecting the qualitative experience of the game.  For example, a fighting game requires pixel perfect accuracy, excellent timing, and extremely tightly controlled joystick motions.  For this type of game, any frame delay bigger than 1 can be noticed by most intermediate players, and expert players may even notice a single frame of delay.  On the other hand, board game or puzzle games which do not have very strict timing requirements may get away with setting the frame latency as high as 4 or 5 before users begin to notice.
Another reason to set the frame delay high is to eliminate the glitching that can occur during a rollback. The longer the rollback, the more likely the user is to notice the discontinuities caused by temporarily executing the incorrect prediction frames.  For example, suppose your game has a feature where the entire screen will flash for exactly 2 frames immediately after the user presses a button.  Suppose further that you’ve chosen a value of 1 for the frame latency and the time to transmit a packet is 4 frames.  In this case, a rollback is likely to be around 3 frames (4 – 1 = 3).  If the flash occurs on the first frame of the rollback, your 2-second flash will be entirely consumed by the rollback, and the remote player will never get to see it!  In this case, you’re better off either specifying a higher frame latency value or redesigning your video renderer to delay the flash until after the rollback occurs.
GGPO Samples
The Vector War application in the SDK bin directory contains a simple application which uses GGPO to synchronize the two clients.  The command line arguments are:
vectorwar.exe  <localport>  <num players> ('local' | <remote ip>:<remote port>) for each player
See the .bat files in the bin directory for examples on how to start 2, 3, and 4 player games.  The source for this application is also included in samples\vectorwar. 
Best Practices and Troubleshooting
Below is a list of recommended best practices you should consider while porting your application to GGPO.  Many of these recommendations are easy to follow even if you’re not starting a game from scratch.  Most applications will already conform to most of the recommendations below.
Isolate Game State from Non-Game State
GGPO will periodically request that you save and load the entire state of your game.  For most games the state that needs to be saved is a tiny fraction of the entire game.  Usually the video and audio renderers, look up tables, textures, sound data and your code segments are either constant from frame to frame or not involved in the calculation of game state.  These do not need to be saved or restored.
You should isolate non-game state from the game state as much as possible.  For example, you may consider encapsulating all your game state into a single C structure.  This both clearly delineates what is game state and was is not and makes it trivial to implement the save and load callbacks (see the Reference Guide for more information).
Define a Fixed Time Quanta for Advancing Your Game State
GGPO will occasionally need to rollback and single-step your application frame by frame.  This is difficult to do if your game state advances by a variable frame rate.    You should try to at least make your game state advanced by a fixed time quanta per frame, even if your render loop does not.
Separate Game State from Rendering in Your Game Loop
You don’t want the user to see any intermediate states during a rollback, so you need a way to advance the game engine without rendering anything.  This is most easily accomplished by separating your game state from your render state.  When you’re finished, your game loop may look something like this:
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

In other words, your game state should be determined solely by the inputs, your rendering code should be driven by the current game state, and you should have a way to easily advance the game state forward using a set of inputs without rendering.
Make Sure Your Game State Advances Deterministically
Once you have your game state identified, make sure the next game state is computed solely as from your game inputs.  This should happen naturally if you have correctly identified all the game state and inputs, but it can be tricky sometimes.  Here are some things which are easy to overlook:
Beware OF Random Number Generators
Many games use random numbers in the computing of the next game state.  If you use one, you must ensure that the seed for the random number generator is same at frame 0 for both players and that the state of the random number generator is included in your game state.  Doing both of these will ensure that the random numbers which get generated for a particular frame are always the same, regardless of how many times GGPO needs to rollback to that frame.
Beware of External Time Sources (aka. Wall clock time)
Be careful if you use the current time of day in your game state calculation.  This may be used for an effect on the game or to derive other game state (e.g. using the timer as a seed to the random number generator).  The time on two computers or game consoles is almost never in sync and using time in your game state calculations can lead to synchronization issues.  You should either eliminate the use of time in your game state or include the current time for one of the players as part of the input to a frame and always use that time in your calculations.
The use of external time sources in non-gamestate calculations is fine (e.g. computing the duration of effects on screen, or the attenuation of audio samples).
Beware of dangling references
If your game state contains any dynamically allocated memory be very careful in your save and load functions to rebase your pointers as you save and load your data.  One way to mitigate this is to use a base and offset to reference allocated memory instead of a pointer.  This can greatly reduce the number of pointers you need to rebase.
Beware of static Variables or Other Hidden State
The language your game is written in may have features which make it difficult to track down all your state.   Static automatic variables in C are an example of this behavior.  You need to track down all these locations and convert them to a form which can be saved.  For example:
Wrong:

   int get_next_counter(void) {
      static int counter = 0; /* static stack variables cannot be saved! */
      counter++;
      return counter;
   }

Corrected:
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

Once you’ve ported your application to GGPO, you can use the ggpo_start_synctest function to help track down synchronization issues which may be the result of leaky game state.  
GGPO Reference 

/*
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 * Use of this software is prohibited unless accompanied by a license.
 */

#ifndef _GGPONET_H_
#define _GGPONET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

typedef struct GGPOSession GGPOSession;

typedef void *GGPOPlayerHandle;

typedef enum {
   GGPO_PLAYERTYPE_LOCAL,
   GGPO_PLAYERTYPE_REMOTE,
} GGPOPlayerType;

/*
 * The GGPOPlayer structure used to describe players in ggpo_add_player
 *
 * size: Should be set to the sizeof(GGPOPlayer)
 *
 * type: One of the GGPOPlayerType values describing how inputs should be handled
 *       Local players must have their inputs updated every frame via
 *       ggpo_add_local_inputs.  Remote players values will come over the
 *       network.
 *
 * player_num: The player number.  Should be between 1 and the number of players
 *       In the game (e.g. in a 2 player game, either 1 or 2).
 *
 * If type == GGPO_PLAYERTYPE_REMOTE:
 * 
 * u.remote.ip_address:  The ip address of the ggpo session which will host this
 *       player.
 *
 * u.remote.port: The port where udp packets should be sent to reach this player.
 *       All the local inputs for this session will be sent to this player at
 *       ip_address:port.
 *
 */

typedef struct GGPOPlayer {
   int               size;
   GGPOPlayerType    type;
   int               player_num;
   union {
      struct {
      } local;
      struct {
         char        ip_address[32];
         short       port;
      } remote;
   } u;
} GGPOPlayer;

typedef struct GGPOLocalEndpoint {
   int      player_num;
} GGPOLocalEndpoint;


#define GGPO_ERRORLIST                                               \
   GGPO_ERRORLIST_ENTRY(GGPO_OK,                               0)    \
   GGPO_ERRORLIST_ENTRY(GGPO_ERRORCODE_SUCCESS,                0)    \
   GGPO_ERRORLIST_ENTRY(GGPO_ERRORCODE_GENERAL_FAILURE,        -1)   \
   GGPO_ERRORLIST_ENTRY(GGPO_ERRORCODE_INVALID_SESSION,        1)    \
   GGPO_ERRORLIST_ENTRY(GGPO_ERRORCODE_INVALID_PLAYER_HANDLE,  2)    \
   GGPO_ERRORLIST_ENTRY(GGPO_ERRORCODE_PLAYER_OUT_OF_RANGE,    3)    \
   GGPO_ERRORLIST_ENTRY(GGPO_ERRORCODE_PREDICTION_THRESHOLD,   4)    \
   GGPO_ERRORLIST_ENTRY(GGPO_ERRORCODE_UNSUPPORTED,            5)    \
   GGPO_ERRORLIST_ENTRY(GGPO_ERRORCODE_NOT_SYNCHRONIZED,       6)    \
   GGPO_ERRORLIST_ENTRY(GGPO_ERRORCODE_IN_ROLLBACK,            7)    \
   GGPO_ERRORLIST_ENTRY(GGPO_ERRORCODE_INPUT_DROPPED,          8)


#define GGPO_ERRORLIST_ENTRY(name, value)       name = value,
typedef enum {
   GGPO_ERRORLIST
} GGPOErrorCode;
#undef GGPO_ERRORLIST_ENTRY

#define GGPO_SUCCEEDED(result)      ((result) == GGPO_ERRORCODE_SUCCESS)


#define GGPO_INVALID_HANDLE      ((void*)-1)


/*
 * The GGPOEventCode enumeration describes what type of event just happened.
 *
 * GGPO_EVENTCODE_CONNECTED_TO_PEER - Handshake with the game running on the
 * other side of the network has been completed.
 * 
 * GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER - Beginning the synchronization
 * process with the client on the other end of the networking.  The count
 * and total fields in the u.synchronizing struct of the GGPOEvent
 * object indicate progress.
 *
 * GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER - The synchronziation with this
 * peer has finished.
 *
 * GGPO_EVENTCODE_RUNNING - All the clients have synchronized.  You may begin
 * sending inputs with ggpo_synchronize_inputs.
 *
 * GGPO_EVENTCODE_DISCONNECTED_FROM_PEER - The network connection on 
 * the other end of the network has closed.
 *
 * GGPO_EVENTCODE_TIMESYNC - The time synchronziation code has determined
 * that this client is too far ahead of the other one and should slow
 * down to ensure fairness.  The u.timesync.frames_ahead parameter in
 * the GGPOEvent object indicates how many frames the client is.
 *
 */
typedef enum {
   GGPO_EVENTCODE_CONNECTED_TO_PEER            = 1000,
   GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER      = 1001,
   GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER       = 1002,
   GGPO_EVENTCODE_RUNNING                      = 1003,
   GGPO_EVENTCODE_DISCONNECTED_FROM_PEER       = 1004,
   GGPO_EVENTCODE_TIMESYNC                     = 1005,
} GGPOEventCode;

/*
 * The GGPOEvent structure contains an asynchronous event notification sent
 * by the on_event callback.  See GGPOEventCode, above, for a detailed
 * explanation of each event.
 */
typedef struct {
   GGPOEventCode code;
   union {
      struct {
         GGPOPlayerHandle  player;
      } connected;
      struct {
         GGPOPlayerHandle  player;
         int               count;
         int               total;
      } synchronizing;
      struct {
         GGPOPlayerHandle  player;
      } synchronized;
      struct {
         GGPOPlayerHandle  player;
      } disconnected;
      struct {
         int      frames_ahead;
      } timesync;
   } u;
} GGPOEvent;

/*
 * The GGPOSessionCallbacks structure contains the callback functions that
 * your application must implement.  GGPO.net will periodically call these
 * functions during the game.  All callback functions must be implemented.
 */
typedef struct {
   /*
    * begin_game callback - This callback has been deprecated.  You must
    * implement it, but should ignore the 'game' parameter.
    */
   bool (__cdecl *begin_game)(char *game);

   /*
    * save_game_state - The client should allocate a buffer, copy the
    * entire contents of the current game state into it, and copy the
    * length into the *len parameter.  Optionally, the client can compute
    * a checksum of the data and store it in the *checksum argument.
    */
   bool (__cdecl *save_game_state)(unsigned char **buffer, int *len, int *checksum, int frame);

   /*
    * load_game_state - GGPO.net will call this function at the beginning
    * of a rollback.  The buffer and len parameters contain a previously
    * saved state returned from the save_game_state function.  The client
    * should make the current game state match the state contained in the
    * buffer.
    */
   bool (__cdecl *load_game_state)(unsigned char *buffer, int len);

   /*
    * log_game_state - Used in diagnostic testing.  The client should use
    * the ggpo_log function to write the contents of the specified save
    * state in a human readible form.
    */
   bool (__cdecl *log_game_state)(char *filename, unsigned char *buffer, int len);

   /*
    * free_buffer - Frees a game state allocated in save_game_state.  You
    * should deallocate the memory contained in the buffer.
    */
   void (__cdecl *free_buffer)(void *buffer);

   /*
    * advance_frame - Called during a rollback.  You should advance your game
    * state by exactly one frame.  Before each frame, call ggpo_synchronize_input
    * to retrieve the inputs you should use for that frame.  After each frame,
    * you should call ggpo_advance_frame to notify GGPO.net that you're
    * finished.
    *
    * The flags parameter is reserved.  It can safely be ignored at this time.
    */
   bool (__cdecl *advance_frame)(int flags);

   /* 
    * on_event - Notification that something has happened.  See the GGPOEventCode
    * structure above for more information.
    */
   bool (__cdecl *on_event)(GGPOEvent *info);
} GGPOSessionCallbacks;

/*
 * The GGPONetworkStats function contains some statistics about the current
 * session.
 *
 * network.send_queue_len - The length of the queue containing UDP packets
 * which have not yet been acknowledged by the end client.  The length of
 * the send queue is a rough indication of the quality of the connection.
 * The longer the send queue, the higher the round-trip time between the
 * clients.  The send queue will also be longer than usual during high
 * packet loss situations.
 *
 * network.recv_queue_len - The number of inputs currently buffered by the
 * GGPO.net network layer which have yet to be validated.  The length of
 * the prediction queue is roughly equal to the current frame number
 * minus the frame number of the last packet in the remote queue.
 *
 * network.ping - The roundtrip packet transmission time as calcuated
 * by GGPO.net.  This will be roughly equal to the actual round trip
 * packet transmission time + 2 the interval at which you call ggpo_idle
 * or ggpo_advance_frame.
 *
 * network.kbps_sent - The estimated bandwidth used between the two
 * clients, in kilobits per second.
 *
 * timesync.local_frames_behind - The number of frames GGPO.net calculates
 * that the local client is behind the remote client at this instant in
 * time.  For example, if at this instant the current game client is running
 * frame 1002 and the remote game client is running frame 1009, this value
 * will mostly likely roughly equal 7.
 *
 * timesync.remote_frames_behind - The same as local_frames_behind, but
 * calculated from the perspective of the remote player.
 *
 */
typedef struct GGPONetworkStats {
   struct {
      int   send_queue_len;
      int   recv_queue_len;
      int   ping;
      int   kbps_sent;
   } network;
   struct {
      int   local_frames_behind;
      int   remote_frames_behind;
   } timesync;
} GGPONetworkStats;

/*
 * ggpo_start_session --
 *
 * Used to being a new GGPO.net session.  The ggpo object returned by ggpo_start_session
 * uniquely identifies the state for this session and should be passed to all other
 * functions.
 *
 * session - An out parameter to the new ggpo session object.
 *
 * cb - A GGPOSessionCallbacks structure which contains the callbacks you implement
 * to help GGPO.net synchronize the two games.  You must implement all functions in
 * cb, even if they do nothing but 'return true';
 *
 * game - The name of the game.  This is used internally for GGPO for logging purposes only.
 *
 * num_players - The number of players which will be in this game.  The number of players
 * per session is fixed.  If you need to change the number of players or any player
 * disconnects, you must start a new session.
 *
 * input_size - The size of the game inputs which will be passsed to ggpo_add_local_input.
 *
 * local_port - The port GGPO should bind to for UDP traffic.
 */
__declspec(dllexport) GGPOErrorCode __cdecl ggpo_start_session(GGPOSession **session,
                                                               GGPOSessionCallbacks *cb,
                                                               char *game,
                                                               int num_players,
                                                               int input_size,
                                                               int localport);


/*
 * ggpo_add_player --
 *
 * Must be called for each player in the session (e.g. in a 3 player session, must
 * be called 3 times).
 *
 * player - A GGPOPlayer struct used to describe the player.
 *
 * handle - An out parameter to a handle used to identify this player in the future.
 * (e.g. in the on_event callbacks).
 */
__declspec(dllexport) GGPOErrorCode __cdecl ggpo_add_player(GGPOSession *session,
                                                            GGPOPlayer *player,
                                                            GGPOPlayerHandle *handle);


/*
 * ggpo_start_synctest --
 *
 * Used to being a new GGPO.net sync test session.  During a sync test, every
 * frame of execution is run twice: once in prediction mode and once again to
 * verify the result of the prediction.  If the checksums of your save states
 * do not match, the test is aborted.
 *
 * cb - A GGPOSessionCallbacks structure which contains the callbacks you implement
 * to help GGPO.net synchronize the two games.  You must implement all functions in
 * cb, even if they do nothing but 'return true';
 *
 * game - The name of the game.  This is used internally for GGPO for logging purposes only.
 *
 * num_players - The number of players which will be in this game.  The number of players
 * per session is fixed.  If you need to change the number of players or any player
 * disconnects, you must start a new session.
 *
 * input_size - The size of the game inputs which will be passsed to ggpo_add_local_input.
 *
 * frames - The number of frames to run before verifying the prediction.  The
 * recommended value is 1.
 *
 */
__declspec(dllexport) GGPOErrorCode __cdecl ggpo_start_synctest(GGPOSession **session,
                                                                GGPOSessionCallbacks *cb,
                                                                char *game,
                                                                int num_players,
                                                                int input_size,
                                                                int frames);

/*
 * ggpo_close_session --
 * Used to close a session.  You must call ggpo_close_session to
 * free the resources allocated in ggpo_start_session.
 */
__declspec(dllexport) GGPOErrorCode __cdecl ggpo_close_session(GGPOSession *);


/*
 * ggpo_set_frame_delay --
 *
 * Change the amount of frames ggpo will delay local input.  Must be called
 * before the first call to ggpo_synchronize_input.
 */
__declspec(dllexport) GGPOErrorCode __cdecl ggpo_set_frame_delay(GGPOSession *,
                                                                 GGPOPlayerHandle player,
                                                                 int frame_delay);

/*
 * ggpo_idle --
 * Should be called periodically by your application to give GGPO.net
 * a chance to do some work.  Most packet transmissions and rollbacks occur
 * in ggpo_idle.
 *
 * timeout - The amount of time GGPO.net is allowed to spend in this function,
 * in milliseconds.
 */
__declspec(dllexport) GGPOErrorCode __cdecl ggpo_idle(GGPOSession *,
                                                      int timeout);

/*
 * ggpo_add_local_input --
 *
 * Used to notify GGPO.net of inputs that should be trasmitted to remote
 * players.  ggpo_add_local_input must be called once every frame for
 * all player of type GGPO_PLAYERTYPE_LOCAL.
 *
 * player - The player handle returned for this player when you called
 * ggpo_add_local_player.
 *
 * values - The controller inputs for this player.
 *
 * size - The size of the controller inputs.  This must be exactly equal to the
 * size passed into ggpo_start_session.
 */
__declspec(dllexport) GGPOErrorCode __cdecl ggpo_add_local_input(GGPOSession *,
                                                                 GGPOPlayerHandle player,
                                                                 void *values,
                                                                 int size);

/*
 * ggpo_synchronize_input --
 *
 * You should call ggpo_synchronize_input before every frame of execution,
 * including those frames which happen during rollback.
 *
 * values - When the function returns, the values parameter will contain
 * inputs for this frame for all players.  The values array must be at
 * least (size * players) large.
 *
 * size - The size of the values array.
 */
__declspec(dllexport) GGPOErrorCode __cdecl ggpo_synchronize_input(GGPOSession *,
                                                                   void *values,
                                                                   int size);

/*
 * ggpo_advance_frame --
 *
 * You should call ggpo_advance_frame to notify GGPO.net that you have
 * advanced your gamestate by a single frame.  You should call this everytime
 * you advance the gamestate by a frame, even during rollbacks.  GGPO.net
 * may call your save_state callback before this function returns.
 */
__declspec(dllexport) GGPOErrorCode __cdecl ggpo_advance_frame(GGPOSession *);

/*
 * ggpo_get_network_stats --
 *
 * Used to fetch some statistics about the quality of the network connection.
 *
 * player - The player handle returned from the ggpo_add_player function you used
 * to add the remote player.
 *
 * stats - Out parameter to the network statistics.
 */
__declspec(dllexport) GGPOErrorCode __cdecl ggpo_get_network_stats(GGPOSession *,
                                                                   GGPOPlayerHandle player,
                                                                   GGPONetworkStats *stats);

/*
 * ggpo_log --
 *
 * Used to write to the ggpo.net log.  In the current versions of the
 * SDK, a log file is only generated if the "quark.log" environment
 * variable is set to 1.  This will change in future versions of the
 * SDK.
 */
__declspec(dllexport) void __cdecl ggpo_log(GGPOSession *,
                                            char *fmt, ...);
/*
 * ggpo_logv --
 *
 * A varargs compatible version of ggpo_log.  See ggpo_log for
 * more details.
 */
__declspec(dllexport) void __cdecl ggpo_logv(GGPOSession *,
                                             char *fmt,
                                             va_list args);

#ifdef __cplusplus
};
#endif

#endif