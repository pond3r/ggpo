# GGPOとは何か？

2009年に作成されたGGPOネットワーキングSDKは、ピアツーピアゲームでのロールバックネットワーキングの使用を開拓しました。 非常に正確な入力とフレームの完全な実行を必要とする速いペースの単収縮スタイルのゲームでネットワーク遅延を隠すために特別に設計されています。

従来の手法では、プレーヤーの入力に遅延を追加することでネットワークの送信時間を考慮し、結果としてゲームの動きが鈍くなります。 ロールバックネットワーキングは、入力予測と投機的実行を使用して、プレーヤーの入力を直ちにゲームに送信し、ゼロ遅延ネットワークの錯覚を与えます。ロールバックネットコードを用いて、プレイヤーがオフラインでプレイすることで蓄積した筋肉のメモリ、タイミング、リアクション、およびをビジュアルキューとオーディオキューオンラインでも直接に利用できます。 GGPOネットワーキングSDKは、ロールバックネットワーキングをできるだけ簡単に、新規および既存のゲームに組み込むことができるように設計されています。


# 仕組みは？

ロールバックネットワーキングは、完全に決定的なピアツーピアエンジンに統合されるように設計されています。完全な決定論により、単に同じ入力をフィードするだけで、すべてのプレイヤーコンピューターで同じ方法でゲームがプレイされることが保証されます。これを実現する1つの方法は、ネットワーク経由ですべてのプレーヤーの入力を交換し、すべてのプレーヤーがピアからすべての入力を受け取ったときにのみゲームプレイロジックのフレームを実行することです。そうすると、多くの場合、結果としてゲームプレイの反応が遅く、応答が遅くなります。ネットワークを介して入力を取得するのに時間がかかるほど、ゲームは遅くなります。


## インプットディレイによるネットワーキング使用の場合

### 理論的に…

以下の系統図をご覧ください。統計図に理想的な0msの遅延のネットワークで２つのクライエントが同期されています。 プレヤー１のインプットとゲームステートは青い色で、プレヤー２のは赤い色で、ネットワークレイヤが緑色で記述されています。黒い矢はシステムを経由しているインプットとステート遷移を表します。フレームずつは破線で分けられています。統計図はプレヤー１の視点しか見せていませんが、プレヤー２は同一のステップに従っています。

![](images/overview_image1.png)

ゲームエンジンに放送される前、プレヤー１のインプットはネットワークレイヤでプレヤー２のインプットと併合されます。エンジンは現在のフレームのゲームステートをそのインプットを使って異動します。プレヤー２は同様、自分のインプットとプレヤー１のインプットを併合して、組み合わせたインプットをゲームエンジンに放送します。ゲームはそのふうに毎フレームをプレヤーインプットによって前のフレームを加工して、続きます。プレヤー１とプレヤー２は同じゲームステートから始まりまして、両方のゲームエンジンに放送したインプットが一致していますので、二人のプレヤーのゲームステートは自動的にフレームづつに同期化されています。

### In Practice..

The Ideal Network example assumes that packets are transmitted over the network instantaneously.  Reality isn’t quite so rosy.  Typical broadband connections take anywhere between 5 and 150 milliseconds to transmit a packet, depending on the distance between the players and the quality of the infrastructure where the players live.  That could be anywhere between 1 and 9 frames If your game runs at 60 frames per seconds.

Since the game cannot process the frame until it has received the inputs from both players, it must apply 1 to 9 frame delay, or “lag”, on each player’s inputs.  Let’s modify the previous diagram to take latency into account:

![](images/overview_image3.png)

In this example it takes 3 frames to transmit a packet.  This means the remote inputs sent by player 2 at frame 1 don’t arrive at player 1’s game console until 3 frames later.  The game engine for player 1 cannot advance until it receives the input, so it’s forced to delay the frame 1 for 3 frames.  All subsequent frames are delayed by 3 frames as well.  The network layer is generally forced to delay all merged inputs by the maximum one way transit time of the packets sent between the two players.  This lag is enough to substantially affect the quality of the game play experience for many game types in all but the most ideal networking conditions.

## Removing Input Delay with Rollback Networking

### Speculative Execution

GGPO prevents the input lag by hiding the latency required to send a packet using speculative execution.  Let’s see another diagram:

![](images/overview_image2.png)

Instead of waiting for the input to arrive from the remote player, the GGPO predicts what the other player is likely to do based on past inputs.  It combines the predicted input with player 1’s local input and immediately passes the merged inputs to your game engine so it can proceed executing the next frame, even though you have not yet received the packet containing the inputs from the other player.  
If GGPO’s prediction were perfect, the user experience playing online would be identical to playing offline.  Of course, no one can predict the future!  GGPO will occasionally incorrectly predict player 2’s inputs.  Take another look at the diagram above.  What happens if GGPO send the wrong inputs for player 2 at frame 1?  The inputs for player 2 would be different on player 1’s game than in player 2’s.  The two games will lose synchronization and the players will be left interacting with different versions of reality.  The synchronization loss cannot possibly be discovered until frame 4 when player 1 receives the correct inputs for player 2, but by then it’s too late.  
This is why GGPO’s method is called “speculative execution”.  What the current player sees at the current frame may be correct, but it may not be.  When GGPO incorrectly predicts the inputs for the remote player, it needs correct that error before proceeding on to the next frame.  The next example explains how that happens.

### Correcting Speculative Execution Errors with Rollbacks

GGPO uses rollbacks to resynchronize the clients whenever it incorrectly predicts what the remote player will do.  The term "rollback" refers to the process of rewinding state and predicting new outcomes based on new, more correct information about a player's input.  In the previous section we wondered what would happen if the predicted frame for remote input 1 was incorrect.  Let’s see how GGPO corrects the error:

![](images/overview_image5.png)

GGPO checks the quality of its prediction for previous frames every time receives a remote input.  As mentioned earlier, GGPO doesn’t receive the inputs for player 2’s first frame until player 1’s fourth.  At frame 4, GGPO notices that the inputs received from the network do not match the predicted inputs sent at earlier.  To resynchronize the two games, GGPO needs undo the damage caused by running the game with incorrect inputs for 3 frames.  It does this by asking the game engine to go back in time to a frame before the erroneously speculated inputs were sent (i.e. to "rollback" to a previous state).   Once the previous state has been restored, GGPO asks the engine to move forward one frame at a time with the corrected input stream.  These frames are shown in light blue.  Your game engine should advance through these frames as quickly as possible with no visible effect to the user.  For example, your video renderer should not draw these frames to the screen.  Your audio renderer should ideally continue to generate audio, but it should not be rendered it until after the rollback, at which point samples should start playing n frames in, where n is the current frame minus the frame where the sample was generated.
Once your engine reaches the frame it was on before GGPO discovered the error, GGPO drops out of rollback mode and allows the game to proceed as normal.  Frames 5 and 6 in the diagram show what happens when GGPO predicts correctly.  Since the game state is correct, there’s no reason to rollback.

# Code Structure

The following diagram shows the major moving parts in the GGPO session object and their relationship to each other.  Each component is describe in detail below.

![](images/overview_image4.png)

## GGPO Interface

The GGPO interface abstracts away the implementation details between the P2P and the Sync Test backends.  The proper backend is created automatically when you call the ggpo_start_session or ggpo_start_synctest entry points.

## P2P Backend

The P2P backend orchestrates a game between players.  It is created by the ggpo_start_session API call.  Most of the heavy lifting is done by the contained helper classes.

## Poll Object

(not pictured).  The poll object is a registration mechanism used by the other objects in the code.  It delivers timers and notifications when waitable objects become ready.  For example, the UDP backend uses the Poll object to receive notifications when new packets arrive. 

## Sync Object

The sync object is used to keep track of the last n-frames of game state.  When its embedded prediction object notifies it of a prediction error, the Sync backend rewinds the game to the more-correct state and single-steps forward to correct the prediction error.

## Input Queue Object

The InputQueue object keeps track of all the inputs received for a local or remote player.  When asked for an input which it doesn't have, the input queue predicts the next input, and keeps track of this information for later so they sync object will know where to rollback to if the prediction was incorrect.  The input queue also implements the frame-delay if requested.

## UDP Protocol Object

The UDP protocol object handles the synchronization and input exchange protocols between any two players.  It also implements the game input compression and reliable-UDP layer.  Each UDP Protocol object has a contained TimeSync object which is uses to approximate the wall clock time skew between two players.

## UDP Object

The UDP object is simply a dumb UDP packet sender/receiver.  It’s divorced from UDP protocol to ease ports to other platforms.

## Sync Test Backend

(not pictured) The Sync Test backend uses the same Sync object as the P2P backend to verify your application’s save state and stepping functionally execute deterministically.  For more information on sync test uses, consult the Programmers Guide.
