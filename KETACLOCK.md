# KETACLOCK clone

これは MIRO/MobileHackerz さんのプロジェクト[KETACLOCK - 桁ごとに独立した時計](http://mobilehackerz.jp/contents/Hardware/KETACLOCK)を模倣したものです。実際の回路やコードは公開されていないためオリジナルとは異なります。

* 時刻情報を送信する一台のマスタと各桁の表示をおこなうスレーブユニットからなる。
* できるだけ電池で動作させる。
* マスタユニットには ESP8266 を用いて NTP 同期による正確な時刻を供給する。
* 学習のためスレーブユニットは異なるアーキテクチャのマイコンを使ってみる。
* マスタ - スレーブの通信には赤外線通信(NECフォーマット)を採用する。

## 赤外線通信

マスタからスレーブには赤外線通信により時刻(時分秒)情報を送信します。
UART データを直接扱うことも考えましたが、既存のライブラリを使って簡単に動かそうと赤外線リモコンに用いられる NEC フォーマットを採用しました。乱暴に時分秒データを次のように割り振っています。このために一般のリモコンと干渉します。リーダー長を変更するなど独自フォーマットとすべきでしょうが実験目的として目をつぶっています。UART データ直接でも良いかと思います。

* 時:アドレス上位8ビット
* 分:アドレス下位8ビット
* 秒:コマンド8ビット

## ESP8266 - master

WiFi に接続し NTP サーバと同期して時刻情報を取得、OLED に表示して毎秒時刻情報を赤外線送信します。

* 赤外線通信ライブラリ https://github.com/Arduino-IRremote/Arduino-IRremote  
使用ピンは PinDefinitainAndMore.h の ESP8266 に関する部分を参照
* I2C 接続 OLED (SSD1306 128x64) Adafruit_SSD1306 ライブラリ  

## AVR - slave

ATmega328P を外付け水晶発振器なしの 3.3V/8MHz 動作とした [Arduino](https://github.com/technoblogy/atmegabreadboard) で動かしています。手持ちの関係で 328P を使いましたが 168(P) でも動くでしょう。

IR 通信には ESP8266 と同じく Arduino-IRremote ライブラリを用いています。7-seg LCD は GPIO による直接駆動で焼付き防止のため 50Hz で極性反転をおこないます。ピン配置についてはソース中のコメントを参照してください。他のスレーブユニットも同様です。

## MSP430 - slave

MSP430G2553 を Energia で動かしています。外付け水晶発振器は使いません。
Energia の IRremote ライブラリは Arduino のものとは異なります。

## CH32V - slave

安価の RISCV マイコン CH32V003F4P6 を外付け水晶発振器無し xxMHz で動かしています。
こちらは Arduino 環境でのライブラリが不十分なため、MounRiver Studio 環境でビルドしました。

赤外線受信については https://github.com/openwch/ch32v003 から InputCapture のコードを参考に書いています。

![KETACLOCK](KETACLOCK.jfif)

