# AviUtl プラグイン - エディットボックス最適化

エディットボックスのサイズやフォントを変更したり、ユニーコード文字列を入力できるようにしたりします。<br>
[最新バージョンをダウンロード](../../releases/latest/)

![綺麗にグラデーション](https://user-images.githubusercontent.com/96464759/152974130-bcda58c8-fdab-43fa-96fa-bfbe091975f2.png)

## IME で UNICODE 文字を入力したい人向け
```usesUnicodeInput``` を ON にすると、IME の日本語入力で UNICODE 文字を入力できるようになります。ただし、サロゲートペアには未対応です。

## タイムラインの描画がおかしいと感じる人向け
高 DPI の設定をしているとタイムラインのグラデーション描画が特におかしくなります。
```usesGradientFill``` を ON にすると、OS の API でグラデーションを描画するようになります。※eclipse_fast.auf と似たような機能です。

## 導入方法

以下のファイルを AviUtl の Plugins フォルダに入れてください。
* OptimizeEditBox.auf
* OptimizeEditBox.ini

## 設定方法

OptimizeEditBox.ini をテキストエディタで編集してから AviUtl を起動します。

```ini
[Settings]
usesUnicodeInput=0 ; テキストオブジェクトで UNICODE 文字を入力したい場合は 1 を指定します。
usesCtrlA=0 ; エディットボックスで Ctrl+A を有効にしたい場合は 1 を指定します。ただし、usesUnicodeInput が 1 のときのみ有効になります。
usesGradientFill=0 ; グラデーション描画を変更する場合は 1 を指定します。ただし、patch.aul のグラデーション描画を無効にしている場合のみ有効になります。
innerColorR=0xff ; 内側の枠の色。
innerColorG=0xff ; 内側の枠の色。
innerColorB=0xff ; 内側の枠の色。
innerEdgeWidth=1 ; 内側の枠の横幅。0以下なら枠の左右は描画しない。
innerEdgeHeight=1 ; 内側の枠の縦幅。0以下なら枠の上下は描画しない。
outerColorR=0x00 ; 外側の枠の色。指定の仕方は内側の枠と同じ。
outerColorG=0x00
outerColorB=0x00
outerEdgeWidth=1
outerEdgeHeight=1
selectionColor=-1 ; 選択領域の色。色は 0x00bbggrr の形式で指定する。-1 の場合は指定なし。
selectionEdgeColor=-1 ; 選択領域端の色。
selectionBkColor=-1 ; 選択領域外の色。
layerBorderLeftColor=-1 ; レイヤー間ボーダーの左側の色。
layerBorderRightColor=-1 ; レイヤー間ボーダーの右側の色。
layerBorderTopColor=-1 ; レイヤー間ボーダーの上側の色。
layerBorderBottomColor=-1 ; レイヤー間ボーダーの下側の色。
layerSeparatorColor=-1 ; レイヤーボタンとレイヤーの間の境界線の色。
addTextEditBoxHeight=0 ; テキストオブジェクトのエディットボックスの高さに加算する値を指定します。例えば、200 を指定するとエディットボックスの高さが通常より 200 ピクセル高くなります。
addScriptEditBoxHeight=0 ; スクリプト制御のエディットボックスの高さに加算する値を指定します。
fontName=Segoe UI ; エディットボックスで使用するフォントのフォント名を指定します。
fontSize=14 ; フォントのサイズを指定します。
fontPitch=1 ; 固定幅を指定する場合は 1 を指定します。
```

## 更新履歴

* 8.0.0 - 2023/05/26 余計な機能を削除
* 7.1.1 - 2022/04/10 設定ダイアログのフックを安全な方法に変更
* 7.1.0 - 2022/03/17 レイヤー左側のセパレータの色を変更する機能を追加
* 7.0.1 - 2022/03/15 レイヤーの枠をデフォルト色に戻せない問題を修正
* 7.0.0 - 2022/03/07 エディットボックスのフォントを変更する機能を追加
* 6.1.0 - 2022/02/28 スクリプト制御の高さも変えられるように修正
* 6.0.0 - 2022/02/28 エディットボックスの高さを変更する機能を追加
* 5.0.0 - 2022/02/12 選択領域外の色などを調整する機能を追加
* 4.0.0 - 2022/02/11 枠線描画の調整機能を追加
* 3.0.0 - 2022/02/08 グラデーションを綺麗にする機能などを追加
* 2.1.1 - 2022/01/08 スピンボタンによる編集が無効になる問題を修正

## 動作確認

* (必須) AviUtl 1.10 & 拡張編集 0.92 http://spring-fragrance.mints.ne.jp/aviutl/
* (共存確認) patch.aul r42 https://scrapbox.io/ePi5131/patch.aul

## クレジット

* Microsoft Research Detours Package https://github.com/microsoft/Detours
* aviutl_exedit_sdk https://github.com/ePi5131/aviutl_exedit_sdk
* Common Library https://github.com/hebiiro/Common-Library

## 作成者情報
 
* 作成者 - 蛇色 (へびいろ)
* GitHub - https://github.com/hebiiro
* Twitter - https://twitter.com/io_hebiiro

## 免責事項

この作成物および同梱物を使用したことによって生じたすべての障害・損害・不具合等に関しては、私と私の関係者および私の所属するいかなる団体・組織とも、一切の責任を負いません。各自の責任においてご使用ください。
