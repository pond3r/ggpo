# GGPO開発者ガイド

GGPOネットワークライブラリ開発者ガイドは、アプリケーションにGGPOネットワークライブラリを実装する開発者向けに用意されたドキュメントです。

## ゲームステートと入力

ゲームには数多くの変化するパートがあるかと思います。GGPOは次の2つだけに依存します。

- **ゲームステート**はゲームでの全ての状態を表します。シューティングゲームの場合、画面上にある自機と敵機の位置、ショットや敵弾の位置、敵機の体力、現在のスコアなどになります。

- **ゲーム入力**はゲームステートを変更する一連のものを指します。言うまでもなく、プレイヤーが操作したジョイスティックやボタンの押下が含まれますが、入力以外のものも含みます。例えば、現在時刻を使って何かを計算した場合、フレームを開始した時の時刻も入力になります。

ゲームエンジンにはゲームステートでも入力でもないものが他にもたくさんあります。例えばオーディオやビデオレンダラーはゲームの結果に影響を与えないため、ゲームステートではありません。ゲームに影響を与えない特殊効果を生成する特殊効果エンジンがあったとしたら、それもゲームステートから除外できます。

## 同期にステートと入力を使用する

GGPOを使ったゲームで遊ぶ各プレイヤーは、プレイしているゲームの完全なコピーを持っています。両プレイヤーが同じゲーム内容で遊べるよう、保持しているゲームステートのコピーを同期し続ける必要があります。フレームが進む度にプレイヤー間でゲームステートの全コピーを送信するのは大きな負荷になります。代わりにGGPOはお互いの入力を送信し、各プレイヤーのゲームを進めます。これが機能するには、ゲームエンジンが3つの条件を満たしている必要があります。

- ゲームのシミュレーションは完全に決定的でなければなりません。つまり、特定のゲームステートと入力があった時に、ゲームステートを1フレーム進めると全プレイヤーのゲームステートが同じにならなければいけません。
- ゲームステートが完全にカプセル化され、シリアライズが可能であること。
- ゲームエンジンはそのフレームのゲーム内容をレンダリングすることなく、復元、保存、フレームのシミュレーションができなくてはなりません。これはロールバックを実装するために使用されます。

## プログラミングガイド

次のセクションではあなたのアプリケーションをGGPO上で動作させるための一連の流れを紹介しています。GGPO APIの詳細な説明については、以下のGGPOリファレンスセクションを参照してください。

### GGPOとの繋ぎ込み

GGPOは新規および既存のゲームエンジンと簡単に繋ぎ込みができるよう設計されています。`GGPOSessionCallbacks`フックを介してアプリケーションを呼び出すことにより、ほとんどのロールバックの実装を行います。

### GGPOSessionオブジェクトの生成

`GGPOSession`オブジェクトはGGPOフレームワークへのインターフェースです。ローカルのポートと、対戦したいプレイヤーのIPアドレスとポートを`ggponet_start_session`関数を渡して作成します。またゲームステートを管理するコールバック関数で満たされた`GGPOSessionCallbacks`オブジェクトと、このセッションで遊ぶプレイヤーの数を渡す必要があります。全ての`GGPOSessionCallback`関数を実装しなければなりません。詳細は以下を参照してください。
例えば、ポート8001にバインドされた別のプレイヤーと同じホストで新しいセッションを開始する場合、次のようになります。

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

### プレイヤーの場所を送信する

GGPOSessionオブジェクトを作成した時、ゲームに参加しているプレイヤーの数を渡しましたが、実際にそれらを連携をする方法について説明していませんでした。これを行うには、各プレイヤーを表す`GGPOPlayer`オブジェクトを`ggpo_add_player`関数に渡して呼び出します。次の例は、2人用のゲームでの`ggpo_add_player`の使い方です。

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

入力の同期は各ゲームフレームの最初に行われます。各ローカルプレイヤーに対する`ggpo_add_local_input`の呼び出しと、遠隔プレイヤーの入力を取得する`ggpo_synchronize_input`の呼び出しによって行われます。
`ggpo_synchronize_inputs`の戻り値は必ず確認するようにしてください。`GGPO_OK`以外の値が返ってきた場合、ゲームステートを進めないでください。これは通常、GGPOがしばらくの間、遠隔プレイヤーからパケットを受信せず、内部の予測制限に達したことで発生します。

例えば、ローカルゲームのコードが次のようになっている場合、

```
   GameInputs &p1, &p2;
   GetControllerInputs(0, &p1); /* read p1's controller inputs */
   GetControllerInputs(1, &p2); /* read p2's controller inputs */
   AdvanceGameState(&p1, &p2, &gamestate); /* send p1 and p2 to the game */
```

次のように変更する必要があります。

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

ロールバック中に発生したものも含め、全てのフレームで`ggpo_synchronize_inputs`を呼び出す必要があります。ゲームステートを進めるためには、ローカルコントローラーから得られた値を読むのではなく、常に`ggpo_synchronize_inputs`から返された値を使用してください。ロールバック中に`ggpo_synchronize_inputs`は`ggpo_add_local_input`に渡された値を前のフレームに使われた値に置き換えます。また、ロールバックの影響を緩和するためにローカルプレイヤー向けの入力遅延を加えた場合、`ggpo_add_local_input`に渡された入力はフレーム遅延が終わるまで`ggpo_synchronize_inputs`に返されません。

### 保存、復元、解放コールバックの実装

GGPOはゲームステートを定期的に保存または復元するために、`load_game_state`と`save_game_state`コールバックを使用します。`save_game_state`関数はゲームの現在のステートを復元し、それを`buffer`出力パラメーターで返すのに十分な情報を含むバッファーを作成する必要があります。`load_game_state`関数は以前に保存したバッファーからゲームステートを復元します。例えば、

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

不要になったら、GGPOは`free_buffer`コールバックを呼び出して、`save_game_state`コールバックで割り当てたメモリを解放します。

```
void __cdecl 
ggpo_free_buffer(void *buffer)
{
   free(buffer);
}
```

### 残っているコールバックの実装

前述のように、`GGPOSessionCallbacks`構造体にはオプション扱いのコールバックはありません。これらは少なくとも`return true`である必要がありますが、残りのコールバックは必ずしもすぐに実装する必要はありません。詳細については`ggponet.h`のコメントを参照してください。

### ggpo_advance_frameとggpo_idle関数の呼び出し

いよいよ終わりに近づいてきました。大丈夫、お約束します。最後のステップはゲームステートを1フレーム進める度にGGPOへ通知することです。1フレームを終えた後、次のフレームを開始する前に`ggpo_advance_frame`を呼び出すだけです。

GGPOは内部記録を行うパケットを送受信するために、一定の時間が必要になります。GGPOに許可したミリ秒単位で、最低でもフレームごとに1回は`ggpo_idle`関数を呼び出す必要があります。

## アプリケーションのチューニング: フレーム遅延 vs 投機的実行

GGPOは遅延を感じさせないようにするために、フレーム遅延と投機的実行の両方を使用します。これは、アプリケーション開発者が入力を遅延させるフレーム数を選択できるようにすることで実現します。もしゲームのフレーム数よりパケットの送信に時間がかかった場合、GGPOは投機的実行を使って残りの遅延を隠します。この数値は、必要に応じてゲーム中でも調整することができます。フレーム遅延の適切な値はゲームに大きく依存します。役に立つヒントをいくつか紹介しましょう。

まずはゲームを遊ぶ感覚に影響を与えない範囲で、フレーム遅延を出来るだけ大きく設定してみてください。例えば格闘ゲームではドット単位の精度、寸分違わぬタイミング、非常に正確なアーケードコントローラーの操作が必要となります。このタイプのゲームでは、ほとんどの中級プレイヤーは2フレームの遅延に気付き、上級プレイヤーであれば1フレームの遅延に気付くこともあります。一方、厳密な操作を必要としないボードゲームやパズルゲームであれば、4～5のフレーム遅延を設定すればユーザーが気付く前に上手くゲームを進められるかもしれません。

フレーム遅延を大きく設定するもうひとつの理由は、ロールバック中に発生し得るグリッチ(不具合)を排除することにあります。ロールバックが長くなればなるほど、間違った予測フレームを一時的に実行したことによって生じた、本来存在しないシーンを継ぎ接ぎした様子が表示される可能性が高くなります。例えば、ユーザーがボタンを押した瞬間に2フレームの画面フラッシュが起きるゲームがあったとします。フレーム遅延を1に設定し、パケット送信に4フレームかかった場合、ロールバックは約3フレーム分(4 - 1 = 3)になります。フラッシュがロールバックの最初のフレームで発生した場合、2フレームのフラッシュはロールバックによって完全に消失してしまい、遠隔で遊ぶプレイヤーはフラッシュ演出を見ることができなくなります。この場合、さらに大きなフレーム遅延値を設定するか、ロールバック発生後までフラッシュを遅らせるようビデオレンダラーを再設計するのが良いでしょう。

## サンプルアプリケーション

ソースディレクトリ内のVector Warには、GGPOを使った2つのクライアントを同期する単純なアプリケーションが含まれています。コマンドライン引数は以下の通りです。

```
vectorwar.exe  <localport>  <num players> ('local' | <remote ip>:<remote port>) for each player
```

2～4プレイヤーでのゲーム開始方法の例については、binディレクトリにある.cmdファイルを参照してください。

## ベストプラクティスとトラブルシューティング

以下はアプリケーションをGGPO上で動作させる際に検討したいベストプラクティスの一覧です。これら推奨事項は、まだゲームを作り初めていない段階でも簡単に理解できます。多くのアプリケーションは既にほとんどの推奨事項を満たしています。

### ゲームステートを非ゲームステートから分離する

GGPOは定期的にゲームステート全体の保存と復元を要求します。ほとんどのゲームにおいて、保存が必要なステートはゲーム全体のごく一部です。通常、ビデオやオーディオレンダラー、テーブルの検索、テクスチャー、サウンドデータ、コードセグメントは、フレームごとに不変であるか、ゲームステートの計算には影響しません。これらを保存または復元する必要はありません。

できるだけゲーム以外の状態をゲームステートから分離する必要があります。例えば、全ゲームステートをC言語の構造体にカプセル化することを考えるかもしれません。これは、ゲームステートであるものとそうでないものが明確に区別され、保存と復元のコールバック実装が簡単になります(詳細についてはリファレンスガイドを参照してください)。

### ゲームステートを進める際の固定時間を定義する

GGPOは、フレームごとにアプリケーションのロールバックとシングルステップ実行を必要とすることがあります。もしゲームステートを可変ティックレートで進めている場合、実行は困難になります。レンダーループがそうでない場合でも、フレームごとに固定時間単位でゲームステートを進めるようにしてください。

### ゲームループ内にあるレンダリングからゲームステートの更新を分離する

GGPOはロールバック中に、advance frameコールバックを何度も呼び出します。ロールバック中に発生するエフェクトやサウンドはロールバックが完了するまで先延ばしする必要があります。これはゲームステートとレンダーステートを分離することで最も簡単に実現できます。分離が出来たら、ゲームループは次のようになるでしょう。

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

言い換えると、ゲームステートは入力のみで決定され、レンダリングは現在のゲームステートによって実行される必要があります。また、レンダリングせずに一連の入力を元にゲームステートを簡単に進める方法が必要です。

### ゲームステートの進行が決定的であることを確認する

ゲームステートを特定したら、次のゲームステートが入力のみから計算されることを確認します。これは、全てのゲームステートと入力を正しく識別できていれば自然とそうなりますが、時には注意が必要です。見落とされがちなことをいくつか紹介します。

#### 乱数ジェネレーターに気を付ける

次のゲームステートを計算するうえで、多くのゲームは乱数を使用します。もし乱数を使う場合、それらが完全に決定的であること、乱数ジェネレーターのシードが両プレイヤーの0フレーム目で同じであること、乱数ジェネレーターの状態がゲームステートに含まれていることを確認してください。これらのことが行われていれば、特定のフレームに対して生成される乱数は、GGPOがそのフレームをロールバックする回数に関係なく、常に同じ値になります。

#### 外部の時刻情報(壁時計時間)に気を付ける

ゲームステートの計算に現在時刻を使う場合は注意してください。ゲームに影響を与えたり、別のゲームステートに導く可能性があります(例: 乱数ジェネレーターのシードにタイマーを使う)。2台のコンピューターまたはゲームコンソールの時刻が同期することはほとんどないため、ゲームステートの計算に時刻を使用すると同期のトラブルに繋がります。ゲームステートに時刻を使うのを止めるか、プレイヤーの現在時刻をフレームへの入力の一部として含め、常にその時刻を使って計算を行う必要があります。

ゲームステート以外の計算に外部の時刻情報を使う分には問題ありません(例: 画面上のエフェクト時間の計算やオーディオサンプルの減衰など)。

### ダングリングポインターに気を付ける

ゲームステートに動的に割り当てられたメモリが含まれる場合、データの保存や復元の際に十分に気を付けながらポインターの再配置を行ってください。これを緩和するひとつの方法は、ポインターの代わりにベースとオフセットを使って割り当てられたメモリを参照することです。これにより再配置が必要なポインターの数を大幅に減らすことができます。

### 静的変数や隠れたステートに気を付ける

ゲームが記述されている言語には、全てのステートの追跡を困難にさせる機能があるかもしれません。C言語の静的自動変数はこの動作の一例です。該当する全ての箇所を探し出し、保存可能な形式に変換する必要があります。例えば、以下を見比べてください。

```
   // This will totally get you into trouble.
   int get_next_counter(void) {
      static int counter = 0; /* no way to roll this back... */
      counter++;
      return counter;
   }
```

次のように書き換えます。
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

### GGPOの同期テスト機能をたくさん使いましょう

あなたのアプリケーションがGGPO上で動作するようになったら、`ggpo_start_synctest`関数を使ってゲームステートの漏れによる同期問題を追跡することができます。

この同期テストセッションは、シミュレーション決定論におけるエラーを探すために設計された特別なシングルプレイヤーセッションです。同期テストセッションで実行すると、GGPOは全てのフレームに対して1フレームのロールバックを行います。フレームが最初に実行されたときのステートとロールバック中に実行されたステートを比較し、それらが異なっていた場合はエラーを発生させます。ゲーム実行中に`ggpo_log`関数を使用すると、初回フレームのログとロールバックフレームのログを比較してエラーを追跡することができます。

ゲームコードを書いている時に開発システム上で同期テストを継続的に実行することで、同期ズレの原因となったバグをすぐに見つけることができます。

## さらに詳しく知りたい方は

このドキュメントではGGPOの基本的な機能について紹介しました。さらに知りたい方は、`ggponet.h`ヘッダーにあるコメント、そしてコードを直接読むことをお勧めします。それではみなさん頑張ってください！
