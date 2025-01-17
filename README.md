
[![Build and Test](https://github.com/Synesthesias/libplateau/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/Synesthesias/libplateau/actions/workflows/build-and-test.yml)

# libplateau
libplateauはPLATEAUの3D都市モデルを扱うためのC++ライブラリであり、以下の機能を提供しています。
- CityGMLのパース
- CityGMLのジオメトリのポリゴンメッシュへの変換
- 緯度経度座標の直交座標系への変換
- XYZタイル形式のベースマップへのアクセス
- 3Dファイル形式へのエクスポート
- REST APIを使用したPLATEAUのサーバーへのアクセス

libplateauはロジックの共通化により[PLATEAU SDK for Unity](https://github.com/Synesthesias/PLATEAU-SDK-for-Unity)と[PLATEAU SDK for Unreal](https://github.com/Synesthesias/PLATEAU-SDK-for-Unreal)の開発を加速させるために活用されています。

## 開発環境
- CMake 3.8以降

### Windows
- Visual Studio 2022
  - cmakeを使うためVisual Studio Installerから`C++によるデスクトップ開発`のインストールが必要です。

## セットアップ
### リポジトリクローン
```
git lfs install
git clone https://github.com/Synesthesias/libplateau
cd libplateau
git submodule update --init --recursive
```

## ビルド
ビルドの方法について、全OSで共通の留意点を記したあと、OSごとのビルド方法を記載します。

### 共通

#### 1. fbx_sdkを用意する
お手数をおかけして申し訳ございませんが、libplateau をビルドするには別途 fbx_sdk が必要です。
fbx_sdk は Autodesk社が公開するSDKです。これは自由に製品に組み込んで良いものの、オープンソースではなく、再配布は禁止となっております。    
そのため、libplateau 自体はオープンソースですが、例外的に fbx_sdk のみ別途ご用意頂く形になっております。  
ビルドするためには、Autodesk社が配布しているSDKを指定のディレクトリに配置する必要があります。
- [Autodesk社のWebサイト](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-3) から、各OS向けのFBXSDKをダウンロードしてインストールします。
- インストールして得られるファイルを、次のディレクトリ構成に合うように配置します。
  - `3rdparty/fbx_sdk/2020.3.1` 以下に配置します。
  - `2020.3.1` 以下のディレクトリ構成は、別添のテキストファイル `file_tree_of_fbxsdk.txt` を参照してください。 

#### 2. C++, C# のビルド手順(各OS共通) 
- C++ の libplateau をビルドすると DLL ができます。
  - 詳しくは下記の、各OS向けのビルド手順を参照してください。
- その後 C# の LibPLATEAU.NET をビルドすると自動で上述のDLLがコピーされ、C#側で利用可能になります。
- C++とC#のビルド設定を合わせる必要があります。(C++でRelease 設定でビルドしたなら C# も Release、Debug なら C# も Debug。これを間違うと古いDLLがコピーされます。）
- ユニットテストの実行時、dllがないという旨のエラーが出る場合、C++ビルド結果の out/build/x64-Debug or x64-Release にある  
  libplateau が C#のバイナリのディレクトリにコピーされているか確認してください。  
  コピーコマンドは CSharpPLATEAU.Test.csproj に記載されており、C#のリビルド時に実行されるはずです。  
  このコマンドが正しく動作するか確認してください。

### 3つのOS向けにまとめてビルド
- Github Actions で```Upload Dlls``` というワークフローを手動実行すると、  
  CIの成果として3つのOS向けにビルドしたライブラリをまとめてダウンロードできます。

### Windowsでの手動ビルド
#### C++のビルド
* Visual Studioのローカルフォルダーを開くからcloneしたリポジトリを開きます。
* 一度cmakeこけるので再度cmakeします。(CMakeLists.txt開いてCtrl+S)
* ビルド実行します。(Ctrl+Shift+B)
* `plateau_test`を実行することでユニットテストを実行可能です
#### C#のビルド
* ```wrappers/csharp/LibPLATEAU.NET.sln``` を開きます。
* ビルドする。ただしC++側に変更があった場合、  
  最新のDLLをC#側にコピーするため「ビルド」ではなく「リビルド」を選択します。
* C#ユニットテストも合わせて実行可能です。

### Linuxでの手動ビルド
利用する Linux は、Unityの対応OSに合わせて Ubuntu 18 とします。  
それより新しいバージョンのUbuntuでビルドすると、Ubuntu 18 には存在しないライブラリに依存してしまうのでUnityで実行不能となってしまいます。
#### C++のビルド
* Ubuntuに入っているデフォルトの cmake ではバージョンが古い可能性があります。  
  その場合は新しいcmakeをマシンにインストールします。
* Ubuntu 18 はデフォルトでは git lfs がないので、`sudo apt install git-lfs` します。
* Ubuntu 18 のデフォルトのコンパイラは g++-7 となっていますが、それでは古いので g++-9 を導入します。
* OpenGL API が必要なので、なければ以下のコマンドでインストールします。
```
sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev
```
* glTF-sdk のビルドのために PowerShell が必要なので、[このWebページ](https://learn.microsoft.com/ja-jp/powershell/scripting/install/install-ubuntu?view=powershell-7.3)を参考に Ubuntu向け PowerShell をインストールします。

* 以下のコマンドを実行します。
```
cd (プロジェクトのルートディレクトリ)
cmake -S . -B ./out/build/x64-Release/ -G "Ninja" -D CMAKE_BUILD_TYPE:STRING="RelWithDebInfo" -D CMAKE_INSTALL_PROGRAM="ninja" -D CMAKE_CXX_FLAGS="-w"
cmake --build ./out/build/x64-Release/ --config RelWithDebInfo
```
#### C#のビルド
* マシンに dotnet をインストールします。
* 以下のコマンドを実行します。
```
cd (プロジェクトのルートディレクトリ）
cd ./wrappers/csharp/LibPLATEAU.NET
dotnet build -c Release
```
* 合わせてユニットテストもする場合は以下を実行します。
```
dotnet test -c Release
```

### MacOSでの手動ビルド
#### C++ビルドに必要なもの
- PowerShell をMacにインストールする必要があります。 brew でインストールしてください。
- cmakeのビルドディレクトリを次のように設定します。
  - Debug構成時は out/build/x64-Debug 
  - Release(RelWithDebInfo)構成時は out/build/x64-Release
#### C#ビルドに必要なもの
- dotnet Core 3.1 を利用します。
  - IDEにRiderを利用している場合、デフォルトで dotnet core 7 になっているので 3.1 をインストールしてそちらを利用するように設定を変えます。

## デプロイ
### Unity
PLATEAU SDK for Unityへの導入については、そちらのREADMEを参照してください。

## ディレクトリ構成
- 3rdparty
  - 外部ライブラリはすべてここにsubmoduleで追加します。(fbx_sdkは例外です。)
  - 利用ライブラリについては後述の「ライセンス管理」を参照してください。
- data
  - テスト用のデータを配置します。ビルド時に出力先ディレクトリにコピーされます。
- include
  - ヘッダファイル一式を配置します。
- src
  - 内部実装のソースコードを配置します。
- test
  - ユニットテストを配置します。
- wrappers
  - 他言語向けのwrapper実装を配置します。
- .github/workflows
  - Github Actionsのワークフロー設定を配置します。

## テストデータ
テストデータの詳細については ```data/README.md``` を参照してください。

## モックサーバー
モックサーバーについては [PLATEAU-API-Mock-v2](https://github.com/Synesthesias/PLATEAU-API-Mock-v2) を参照してください。

# 実装上の注意
## 文字コード
- gmlのパース結果はC++の内部では UTF-8 で保持しています。
  - パーサー xerces-c は本家の挙動ではAnsiに変換して保持しますが、その挙動をUTF-8に変えました。Unityから日本語文字を扱う都合上の改変です。
- デバッグTIPS
  - マルチバイト文字を扱う処理で、手元のマシンだと動くけどCIサーバーだと動かないという状況のときは、  
    Windowsのロケール設定を日本語から英語に変えると手元のマシンで状況が再現することがあります。

## CI (継続的インテグレーション)
Github Actions によるCIを導入しています。  
Windows, Mac, Linux でのテストと成果物のダウンロードができます。
- push時、自動でビルドおよびユニットテストが行われます。
- git tagを付けた時、またはgithubサイトから手動で Upload DLLs を実行したときにビルドが走り、 
  成果物となるDLL等を3つのOS向けにダウンロードできます。
  githubサイトから手動で実行するには、 Actions → Workflows から Upload DLLs を選択 → Run workflow からブランチを選んで実行します。
  成果物を ```all-library``` という名前のzipでダウンロードできます。

# コード規約
- 変数名は snake_case , 関数名は lowerCamel
- private static メソッドはヘッダファイルに書かず、.cppの無名名前空間に書く
- 外に見せる必要の無いクラス (C#でいうinternalクラス) のヘッダーファイルの配置ディレクトリはincludeではなくsrc
- `[[nodiscard]]` は書かない
- ファイルの末尾は改行
- 何かの個数を取得する関数名は `get(単数形)Count`
- `*` `&` の位置は左寄せ (`SomeType *foobar` ではなく `SomeType* foobar`)
- Unreal Engine で利用する都合上、ヘッダーファイルで `std::filesystem` は利用しない
  - .cppファイル内での利用は可
  - パスの受け渡しは string(中身はutf8形式) で行い、`.cpp`内で`auto path = std::filesystem::u8path(path_str)`でpathに直す
- コンテナ要素へのアクセスは`[i]`ではなく`.at(i)`を用いる。
  - Unityから利用する際範囲外アクセスでクラッシュしてしまうため。後者は例外として捕捉できる。

# トラブルシューティング
- **Q.** 手元のマシンではユニットテストが通るのに、CIサーバーの自動テストが通りません。
- **A.** 手元のマシンとCIサーバーでの設定の差によるバグを疑ってみましょう。例えば:
  - Windowsの設定で　言語の設定 → 管理用言語の設定 → システムロケールの変更 について、 CIサーバーでは英語（米国）ですが、 
    手元のマシンでは日本語になっているケースです。これはパスなどの文字コードに影響します。 
    パスの文字列をUTF8で扱えていれば、システムロケールが英語でも日本語でも日本語を含むパス文字列に対応します。 
    しかし、パス文字列をUTF8で扱えていなければ、システムロケールが英語のとき、日本語名を含むパス文字列は文字化けします。

# ライセンス
## ライセンス制約
- libcitygmlはLGPLライセンスのため、libplateauにもLGPLライセンスが伝播します。
- 上記により、このリポジトリはLGPLライセンスで公開しなければなりません。
- libplateauと動的リンクするPLATEAU SDK for Unity、PLATEAU SDK for UnrealにはLGPLライセンスは伝播しません。

## ライセンス管理
サードパーティソフトの権利表記をThirdPartyNotices.mdに記載してください。

- libcitygml
  - [https://github.com/Synesthesias/libcitygml.git](https://github.com/Synesthesias/libcitygml.git)
  - 用途: CityGMLのパース
  - ライセンス: GNU Lesser General Public License v2.1

- xerces-c
  - [https://github.com/Synesthesias/xerces-c.git](https://github.com/Synesthesias/xerces-c.git)
  - 用途: libcitygmlの依存ライブラリ
  - ライセンス: Apache-2.0、要 著作権表示、要 改変したことの告知

- libxml2
  - [https://github.com/GNOME/libxml2](https://github.com/GNOME/libxml2)
  - 用途: libcitygmlの依存ライブラリ
  - ライセンス: MIT

- cpp-httplib
  - [https://github.com/yhirose/cpp-httplib.git](https://github.com/yhirose/cpp-httplib.git)
  - 用途: APIサーバーへのアクセス
  - ライセンス: MIT

- glTF-SDK
  - [https://github.com/microsoft/glTF-SDK](https://github.com/microsoft/glTF-SDK)
  - 用途: glTF形式でのエクスポート
  - ライセンス: MIT

- json
  - [https://github.com/nlohmann/json.git](https://github.com/nlohmann/json.git)
  - 用途: APIサーバーから受け取るjsonのパース
  - ライセンス: MIT

- openssl-cmake
  - [https://github.com/janbar/openssl-cmake](https://github.com/janbar/openssl-cmake)
  - ライセンス: dual-license (the OpenSSL license plus the SSLeay license)

- zlib
  - [https://github.com/madler/zlib](https://github.com/madler/zlib)
  - ライセンス: 独自(再配布可)

- pybind11
  - [https://github.com/pybind/pybind11.git](https://github.com/pybind/pybind11.git)
  - 用途: SDKのpython対応(開発停止中)
  - ライセンス: BSD-style license

- FBXSDK
  - [https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-3-1](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-3-1)
  - 用途: fbx形式でのエクスポート
  - ライセンス: 独自
