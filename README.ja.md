![](doc/images/ggpo_header.png)

&nbsp; _[![Appveyor build status](https://img.shields.io/appveyor/ci/pond3r/ggpo/master.svg?logo=appveyor)](https://ci.appveyor.com/project/pond3r/ggpo/branch/master)_

## GGPOとは

従来の技術はプレイヤーの入力に遅延を織り込んで通信を行っており、その結果反応が遅く、ラグを感じるプレイ感になっていました。ロールバックネットワーキングは入力予測と投機的実行を行って、プレイヤーの入力を即座に送信するため、遅延を感じさせないネット環境をもたらします。ロールバックがあれば、タイミングや相手の動きや効果音に対する反応、指が覚えている入力、これらオフラインで行えた内容が、そのままオンラインでも行えます。GGPOネットワーキングSDKは、ロールバックネットワーキングを新作や発売されているゲームに極力簡単に組み込めるよう作られています。

これまでのGGPOについてさらに知りたい方は、http://ja.ggpo.net/ をご覧ください。

このリポジトリにコードやドキュメント、SDKのサンプルアプリケーションが収められています。

## ビルド

GGPOのビルドは現在のところWindowsのみになりますが、他プラットフォームへのポートも現在行っています。

### Windows

Windowsのビルドは[Visual Studio 2019](https://visualstudio.microsoft.com/downloads/)と[CMake](https://cmake.org/download/)が必要になります。ご使用の前に、どちらもインストールされていることをご確認ください。またインストール時、パスにCMakeを追加してください。

- Visual Studio 2019のソリューションファイルを作成するため、SDKのルートディレクトリで`build_windows.cmd`を実行します。
- コンパイルをするため、Visual Studio 2019で`build/GGPO.sln`ソリューションを開きます。

好みにあわせて`cmake-gui`で実行も出来ます。

## サンプルアプリケーション

ソースディレクトリ内のVector Warには、GGPOを使った2つのクライアントを同期する単純なアプリケーションが含まれています。コマンドライン引数は以下の通りです。

```
vectorwar.exe  <localport>  <num players> ('local' | <remote ip>:<remote port>) for each player
```

2～4プレイヤーでのゲーム開始方法の例については、binディレクトリにある.cmdファイルを参照してください。

## ライセンス

GGPOはMITライセンスの元で利用ができます。つまり、GGPOは商用、非商用のどちらでも無料で利用ができます。クレジットの掲載、帰属は必要ありませんが、してくださると大変うれしく思います。
