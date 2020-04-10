# ImpDet_ANA
改良型検出器解析用プログラム

- ImpDet_ANA.cpp
  - すべて同一ファイルに記述されている解析マクロ。rootで動く
- ImpDet_pededtal.cpp
  - ペデスタルを取得するためのマクロ。rootで動く
- ImpDet_ANA_origin.cpp
  - 永田さんが作った解析プログラム
  - 気圧の補正、フォトンピークの自動取得などの機能が入っている。

# idana_build
マクロを分割コンパイルできるようにしたもの
- include
  - ヘッダーファイル
- source
  - ソースファイルとメイン関数
- Makefile
  - インクルードファイルとリンクするライブラリにrootのものを追加してある。
