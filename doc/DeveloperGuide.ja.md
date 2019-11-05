# GGPO開発者ガイド

GGPOネットワークライブラリ開発者ガイドは、アプリケーションにGGPOネットワークライブラリを実装する開発者向けに用意されたドキュメントです。

## ゲームステートと入力

ゲームは数多くの変化するパートがあると思います。GGPOは次の2つだけに依存します。

- **ゲームステート**はゲーム内で現在の状態全てを表します。シューティングゲームの場合、画面上にある自機と敵機の位置、ショットや敵弾の位置、敵機の残り体力、現在のスコアなどになります。

- **ゲーム入力**はゲームステートを変更する一連のものを指します。プレイヤーが操作したジョイスティックやボタンも間違いなく含みますが、目に見えない入力も含みます。例えば、ゲーム内で何かを計算するために現在時刻を使用したならば、フレームの最初になる現在時刻も、また入力になります。

ゲームエンジンの中にはゲームステートでも入力でもないものが多数あります。例えばオーディオやビデオレンダラーはゲームの結果に影響を与えないので、ゲームステートではありません。ゲームに影響を与えないような、特殊効果を生成する特殊効果エンジンがあれば、ゲームステートから除外することができます。

## 同期にステートと入力を使用する

GGPOを使ったゲームで遊ぶ各プレイヤーは、現在遊んでいるゲームの完全なコピーを持っています。両プレイヤーが同じゲーム内容を遊べるよう、GGPOは両プレイヤーにあるゲームステートのコピーを同期し続ける必要があります。プレイヤー間で各フレームごとにゲームステート全コピーを送信することは大きな負荷になってしまいます。代わりにGGPOはお互いの入力を送信し、互いのゲームを進めています。この機能のために、ゲームエンジンは3つの条件を満たさねばなりません。

- ゲームシミュレーションが完全に決定的であること。与えられたゲームステートと入力があった場合、1フレームゲームステートを進めた際に全プレイヤーのゲームステートが全く同一の結果にならなければいけません。
- ゲームステートが完全にカプセル化され、シリアライズが可能であること。
- ゲームエンジンがそのフレーム時のゲーム内容をレンダリングすることなく、復元、保存、フレームのシミュレーションができること。これはロールバックを実装する際に使用します。

## プログラミングガイド

次のセクションはあなたのアプリケーションをGGPOに移植する一連の流れを紹介します。GGPO APIの詳細な説明は、以下のGGPOリファレンスセクションを参照してください。

### GGPOとのインターフェース

GGPOは新規および既存のゲームエンジンと簡単に繋ぎ込みができるよう設計されています。`GGPOSessionCallbacks`フックを介してアプリケーションに呼び出すことにより、ほとんどのロールバックの実装を行います。

### GGPOSessionオブジェクトの生成

`GGPOSession`オブジェクトはGGPOフレームワークへのインターフェースです。ローカル、IPアドレス、対戦したいプレイヤーをバインドするためのポートを渡す`ggponet_start_session`関数を使用して、作成します。またセッションが1プレイヤー側、2プレイヤー側いずれの場合でも、ゲームステートを管理するコールバック関数で満たされた`GGPOSessionCallbacks`オブジェクトに渡す必要があります。全ての`GGPOSessionCallback`関数を実装しなければなりません。詳細は以下を参照してください。
例として、ポート8001にバインドしている別プレイヤーと同じホストで新しいセッションを開始する場合、次のようになります。

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



`GGPOSession`オブジェクトは単一のゲームセッションだけに使われるべきです。別の相手と接続する必要がある場合、`ggpo_close_session`を使用して既存のオブジェクトを閉じ、新しいオブジェクトを開始します。

```
   /* Close the current session and start a new one */
   ggpo_close_session(ggpo);
```

### プレイヤーの場所を送信

GGPOSessionオブジェクトを作成した時、ゲームに参加しているプレイヤーの数を渡しましたが、どのように連携をするか実際には説明していません。そのためには、それぞれのプレイヤーを表す`GGPOPlayer`オブジェクトを渡して、`ggpo_add_player`を呼び出します。以下は2プレイヤーで遊ぶ時に`ggpo_add_player`を使用する際の例になります。

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

### ローカルと遠隔プレイヤーによる入力の同期

入力の同期は各ゲームフレームの最初に行われます。各ローカルプレイヤーの`ggpo_add_local_input`の呼び出しと、遠隔プレイヤーの入力を取得するために`ggpo_synchronize_input`の呼び出しをすることによって行われます。
`ggpo_synchronize_inputs`の戻り値を必ず確認するようにしてください。`GGPO_OK`以外の値を返す場合、ゲームステートを次に進めるべきではありません。僅かな間、遠隔プレイヤーからパケットを受信していなかったり、内部で行われる予測の限界に達することがあるために、こういったことは頻繁に発生します。

例えば、ローカルゲームのコードがこのような場合、

```
   GameInputs &p1, &p2;
   GetControllerInputs(0, &p1); /* read p1's controller inputs */
   GetControllerInputs(1, &p2); /* read p2's controller inputs */
   AdvanceGameState(&p1, &p2, &gamestate); /* send p1 and p2 to the game */
```

次のように変更するべきです。

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

ロールバック中に発生したものでも、フレームごとに`ggpo_synchronize_inputs`を呼び出す必要があります。ゲームステートを進めるためには、ローカルコントローラーから得られた値を読むのではなく、常に`ggpo_synchronize_inputs`から返された値を使用してください。ロールバック中、`ggpo_synchronize_inputs`は前のフレームに使われた値とともに、`ggpo_add_local_input`に渡された値を置き換えます。また、ロールバックの影響を緩和するためにローカルプレイヤー向けの入力遅延を加えた場合、フレーム遅延が終えるまで`ggpo_add_local_input`に渡される入力は`ggpo_synchronize_inputs`に返されません。

### 保存、復元、解放コールバックの実装

定期的にゲームステートを保存、復元するために、GGPOは`load_game_state`と`save_game_state`コールバックを使用します。`save_game_state`関数はゲームの現在のステートを復元し、`buffer`出力パラメーター内に戻るために、十分な情報を含んだバッファーを作成する必要があります。`load_game_state`関数は以前に保存したバッファーからゲームステートを復元します。例えば、

```
struct GameState gamestate;  // Suppose the authoritative value of our game's state is in here.

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

これ以上必要がなくなった時は、GGPOがあなたの`save_game_state`コールバックにある割り当てたメモリを解放するために、`free_buffer`コールバックを呼び出します。

```
void __cdecl 
ggpo_free_buffer(void *buffer)
{
   free(buffer);
}
```

### 残っているコールバックの実装

前述のように、`GGPOSessionCallbacks`構造体にはオプションのコールバックがありません。少なくとも`return true`である必要がありますが、残りのコールバックはすぐに実装する必要はありません。詳細については`ggponet.h`のコメントを参照してください。

### ggpo_advance_frameとggpo_idle関数の呼び出し

いよいよ終わりに近づいてきています。大丈夫、お約束します。最後のステップはゲームステートが1フレームごとに進んだら、毎回GGPOに通知することです。1フレームを終えた後、次のフレームを開始する前に`ggpo_advance_frame`を呼び出すだけです。

GGPOは内部記録のパケットを送受信するために、一定の時間が必要になります。GGPOに許可したミリ秒単位で、最低でもフレームごとに1回は`ggpo_idle`関数を呼び出す必要があります。

## アプリケーションのチューニング: フレーム遅延 vs 投機的実行

遅延を感じさせないようにするために、GGPOはフレーム遅延と投機的実行の両方を使用します。アプリケーション開発者が入力遅延のフレーム数はどの程度にするか、については選択ができます。もしゲームのフレーム数よりパケットの送信に時間がかかる場合は、GGPOが投機的実行を使って残りの遅延を隠します。もし希望すれば、ゲーム中でもこの数字を変更できます。フレーム遅延の適切な値はゲームによって依存します。以下は役に立つヒントです。

まずはゲームを遊ぶ感覚に影響を与えない範囲で、フレーム遅延を出来るだけ大きく設定してみてください。例えば格闘ゲームであればドット単位の精度を要する操作、寸分違わぬタイミング、極めて素早く動かすアーケードコントローラーといった要素があります。こういったゲームの場合、中級プレイヤーの大半は2フレームの遅延に気付き、上級プレイヤーであれば1フレームの遅延に気付くこともあります。一方、厳密なタイミング操作を必要としないボードゲームやパズルゲームなら、4～5のフレーム遅延であればユーザーが遅延に気付き始める前に上手くゲームを進められるかもしれません。

フレーム遅延を大きく設定するもうひとつの理由は、ロールバック中に発生し得るグリッチ(不具合)を排除することにあります。ロールバックが長くなればなるほど、間違った予測フレームを一時的に実行したことで、本来存在しないシーンを継ぎ接ぎした様子が表示される可能性が高くなるからです。例えば、ユーザーがボタンを押した瞬間にちょうど2フレーム分全画面がフラッシュする仕様のゲームがあったとします。フレーム遅延を1に設定し、パケット送信に4フレームかかったとすると、この場合はロールバックは約3フレーム分(4 - 1 = 3)になります。フラッシュがロールバックの最初のフレームに発生した場合、2フレームのフラッシュはロールバックで消えてしまい、遠隔で遊ぶプレイヤーはフラッシュ演出が見えなくなってしまいます。この場合、さらに大きなフレーム遅延値を設定するか、ロールバック発生後までフラッシュを遅らせるようビデオレンダラーを再設計するか、のどちらかになります。

## サンプルアプリケーション

ソースディレクトリ内のVector Warアプリケーションでは、2つのクライアントを同期するGGPOが搭載されています。コマンドライン引数は以下の通りです。

```
vectorwar.exe  <localport>  <num players> ('local' | <remote ip>:<remote port>) for each player
```

2～4プレイヤーゲーム開始方法についての例は、binディレクトリにある.cmdファイルを参照してください。

## ベストプラクティスとトラブルシューティング

以下はGGPOへアプリケーションを移植する際に検討をしたい、ベストプラクティスの一覧です。最初の段階からゲームを始めなくとも、ここで推奨している多くは簡単に理解することができます。多くのアプリケーションは推薦している以下の手法に準拠しています。

### 非ゲームステートとゲームステートを分ける

GGPOは定期的にゲームにおける全ステートの保存と復元を要求します。多くのゲームにとって、保存が必要となるステートはゲーム全体のなかでも小さな要素にあたります。ビデオやオーディオレンダラー、テーブルの検索、テクスチャー、サウンドデータ、コードセグメントは大半の場合、フレームからフレームまで一定か、ゲームステートの計算には関与しません。これらは保存や復元する必要がありません。

ゲームステートから非ゲームステートを出来る限り分けましょう。例えば、全ゲームステートをC言語の構造体へカプセル化することを考えるかもしれません。この二つは、ゲームステートであるもの、そうでないもの、保存や復元のコールバックの実装に必要でないものをはっきりと分けてくれます(詳細についてはリファレンスガイドを参照してください)。

### ゲームステートを進める際の固定時間を定義する

GGPOは時折、ロールバックやフレームごとにアプリケーションに対してシングルステップ実行をする必要があります。もしあなたのゲームステートが可変ティックレートで進めているのであれば難しい実行です。レンダーループがそうでなくとも、ゲームステートの進行はフレームにつき固定時間でするようにしてください。

### ゲームループでのレンダリングから、ゲームステートの更新を分離する

GGPOはロールバック中、事前フレームコールバックを何度も呼び出します。ロールバックが完了するまで、ロールバック中に発生したエフェクトやサウンドは先延ばしする必要があります。これはレンダーステートからゲームステートを分離することで簡単に実現できます。完了したら、以下のようになるでしょう。

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

言い換えると、ゲームステートは入力のみで決定され、レンダリングコードは現在のゲームステートによって動作をします。レンダリングなしで一連の入力をもとに簡単にゲームステートを進める方法であるべきです。

### ゲームステートの進行は決定的であること

ゲームステートを特定したら、必ず次のゲームステートはゲーム入力のみで計算されるようにしてください。ゲームステートと入力が全て正しく識別できたなら自然と発生しますが、時として手が込む作業です。以下は見落としやすい内容です。

#### 乱数ジェネレーターに気を付ける

次のゲームステートを計算するうえで、多くのゲームは乱数を使用します。もし乱数を使っていたら、それが両プレイヤーがフレーム0の時に乱数ジェネレーターのシードが同一であること、そしてあなたのゲームステート内に乱数ジェネレーターのステートが含まれていること、といった完全に決定的であることを必ず確認してください。この両方を確認すれば、GGPOが特定のフレームでロールバックが何回も必要になろうとも、そのフレームで生成された乱数は必ず同じになります。

#### 外部の時刻情報(壁時計時間)に気を付ける

ゲームステートの計算に現在時刻を使う場合は注意してください。このことでゲームに影響を与える、または他のゲームステートに導く可能性があります(例: タイマーを乱数ジェネレーターのシードに使う)。2つのコンピューターもしくはゲームコンソールの時刻は、ほぼ同期することはなく、ゲームステート計算に時刻を使用すると同期の問題につながります。ゲームステートに時刻の使用する、またはフレームに入力する一部として、プレイヤーの1人のために現在時刻を含める、そして計算に時刻を常に使うといったことは排除するべきです。

非ゲームステート計算における外部の時刻情報の使用は構いません(例: 画面上のエフェクト時間を計算、またはオーディオサンプルの減衰)。

### ダングリングポインターに気を付ける

ゲームステートが動的に割り当てられたメモリを含んでいる場合、データの保存と復元をする際には、ポインターを再配置する保存と復元関数に十分気を付けてください。緩和をするひとつの方法として、ポインターの代わりに割り当てられたメモリを参照するため、ベースとオフセットを使用します。これで再配置が必要なポインターの数が大幅に削減できます。

### 静的変数、またはほかの隠れたステートに気を付ける

あなたのゲームで使われている言語は、全てのステートを追跡するのを難しくさせる機能があるかもしれません。C言語にある静的自動変数はこの挙動の一例にあたります。全ての箇所を探して、保存できる形式へ変換する必要があります。例として、この2つを比較してください。まずはこちらから。

```
   // This will totally get you into trouble.
   int get_next_counter(void) {
      static int counter = 0; /* no way to roll this back... */
      counter++;
      return counter;
   }
```

次にこちらを。
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

### GGPO同期テスト機能をたくさん使用しましょう

GGPOにあなたのアプリケーションを移植したら、漏れが出やすいゲームステートの可能性がある同期問題、この追跡ができる`ggpo_start_synctest`関数を使うことができます。

この同期テストセッションは特別に作られもので、シミュレーション決定論におけるエラーを探すために設計された、シングルプレイヤーセッションです。同期テストセッションを実行すると、GGPOはあなたのゲーム内でフレームごとに1フレームのロールバックを行います。初回に行った時のフレームステートと、ロールバック中に行ったステートを比較して、異なっていればエラーを発生させます。ゲーム実行中に`ggpo_log`関数を使用した場合、エラーを追跡するために初期フレームのログとロールバックフレームのログを比較することができます。

ゲームコードを書いている時に開発システムで継続的に同期テストを実行することで、非同期を発生するバグをすぐに見つけることができます。

## さらに詳しく知りたい方は

このドキュメントはGGPOの基本的な機能を紹介しています。さらに知りたい方は、`ggponet.h`ヘッダーにあるコメント、そしてコードを直接読むことをお勧めします。それではみなさん頑張ってください！
