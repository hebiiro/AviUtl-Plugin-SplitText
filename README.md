# AviUtl プラグイン - テキスト分解

テキストオブジェクトをバラバラに分解します。
[最新バージョンをダウンロード](../../releases/latest/)

## 導入方法

以下のファイルを AviUtl の Plugins フォルダに入れてください。
* SplitText.auf
* SplitText.wav

## 使用方法

1. メニューの「表示」->「テキスト分解」を選択してウィンドウを表示します。
1. テキストオブジェクトを選択します。
1. テキスト分解ウィンドウの「テキストを分解する」ボタンを押します。

## 注意事項

* テキストオブジェクトの下にスペースがないとオブジェクトは生成されません。
* 対応しているパラメータは X、Y、サイズ、フォント、B、I、字間、行間のみです。
* アラインには対応していないので注意してください。(左寄せ[上]で実行してください)
* 制御文字にも対応していません。
* 分解後の文字位置は完全には一致しません。手動で微調整が必要です。

## 更新履歴

* 1.0.1 - 2022/07/17 テンポラリファイルをテスト用ドライブに作成していた問題を修正
* 1.0.0 - 2022/07/17 初版

## 動作確認

* (必須) AviUtl 1.10 & 拡張編集 0.92 http://spring-fragrance.mints.ne.jp/aviutl/
* (共存確認) patch.aul r41 https://scrapbox.io/ePi5131/patch.aul

## クレジット

* Microsoft Research Detours Package https://github.com/microsoft/Detours
* aviutl_exedit_sdk https://github.com/ePi5131/aviutl_exedit_sdk
* Common Library https://github.com/hebiiro/Common-Library
* VOICEVOX (青山龍星) https://voicevox.hiroshiba.jp/

## 作成者情報
 
* 作成者 - 蛇色 (へびいろ)
* GitHub - https://github.com/hebiiro
* Twitter - https://twitter.com/io_hebiiro

## 免責事項

この作成物および同梱物を使用したことによって生じたすべての障害・損害・不具合等に関しては、私と私の関係者および私の所属するいかなる団体・組織とも、一切の責任を負いません。各自の責任においてご使用ください。
