# Pro Microで作る自作MIDIフェーダー

![image](https://user-images.githubusercontent.com/51395778/147743945-fbfe72d6-1a85-4328-8d8c-9cd47f2d0f0b.png)

## 仕様

 - 16本の物理フェーダー
 - USBを介し MIDI 信号を送信
 - マイコンに Arduino Micro 互換の Pro Micro を使用
 - ArduinoでMIDIを扱うライブラリ MIDIUSB を使用

## 使用部品

### 主要なもの

|名称|型番|数量|仕様|
|--|--|--|--|
|Pro Micro||1|Arduino Micro 互換ボード|
|ADC|MCP3008-I/P|2|10bit 8ch ADC|
|フェーダー|SL4515N-B103L15CM|16|10kΩ Bカーブ|
 
 
 ### Pro Micro
 
 Pro Micro は Aliexpress で購入しました。 僕が買ったものは概ね4\~5ドル程度に日本への送料2\~3ドルで購入できます。
 複数買っても送料は変わらないようなので、念の為2,3個買うと良いと思います。 僕が買ったのは一番安いMicroUSBのモデルですが、Type-Cの物もあるようです。
 
 Standard Shipping の場合、注文から到着まで概ね3週間程かかります。
 海外通販が不安であれば値段は上がりますが日本のAmazonなどでも互換ボードが売っていますし、純正の Arduino Micro を買ってもよいでしょう。
 単純にクレカ支払いが不安な方は [PayPal](https://www.paypal.com/jp/home) を使うのがおすすめです。
 
 ATmega32u4 搭載の Arduino (LeonardやMicro) やその互換ボードあればおそらくなんでも大丈夫なので、別に全く同じところで同じものを買う必要はありません。
 

 https://ja.aliexpress.com/item/32887074671.html

 純正Arduino Microは秋月電子でも販売されています。
 
 https://akizukidenshi.com/catalog/g/gM-08286/
 
 ### ADC MCP3008-I/P
 
 ProMicroにもADCは載っていますが4chのみで足りないため、SPI接続のADCを別で用意しました。
 
 秋月電子で手に入るADCの中で一番チャンネル数が多いのが8chのこれだったため選択。
 読み取り精度が12bitのMCP3208-CI/Pもありましたが、どうせMIDIにする段階で7bitへ落としてしまうため10bitのMCP3008-I/Pを使用しました。
 今回はこれを2個接続し16chとしています。
 
 https://akizukidenshi.com/catalog/g/gI-00238/
 
 ### フェーダー
 
 今回作ったMIDIフェーダーは、フェーダーの位置に応じた電圧をADCで読み取り7bitに変換してMIDI出力しています。
 フェーダーの位置をデジタル化するので、フェーダーの移動量と抵抗値の変化量が常に一定のBカーブ（移動量と抵抗値が線形変化するやつ）を選択しました。
 抵抗値はプルアップ抵抗でよく使われているイメージのある10kΩを選択。この辺はよくわかってません。
 Vccの電圧が5Vでフェーダーが並列で16本あるので、Vccからフェーダーへ引き出される電流は5V / 10k * 16 = 8mA だから大丈夫なハズ！
 
 
 ### その他部品
 
 主要部品を含めた一通りの部品（Pro Micro、底面のアクリル板、ネジ類除く）はこちらにまとめています。
 
 https://partscabi.net/list/7173e061-19dd-4b44-9ed1-14f1cda44222
 
 ## 回路
 
 16本のMIDIフェーダーがADCに接続され、ADCとProMicroがSPIで接続されている非常にシンプルな回路です。
 
 SPIのMISO,MOSI,CLICKは複数のADCに対して並列に接続します。SELECTはADCごとに個別に適当なIOに接続してください。
 
 フェーダーは下げたときが0V、上げたときが5Vとなるように配線してください。
 今回の用途では特にチャタリングが気になることもなかったため、ソフト・ハードともこれといった対策はしていません。
 
 ![回路図](https://user-images.githubusercontent.com/51395778/147753377-edf2ae95-63f7-48a4-be94-9bce4aa18aa7.png)


 
 ## プログラム

このリポジトリを[ダウンロード](https://github.com/fumigoro/usb-midi-fader/archive/refs/heads/master.zip)して解凍後、Arduino IDEで開いてください。

プログラム内には下記のように設定項目がありますので、必要に応じて編集してください。

あとはPro Microに書き込めば完了です。

```c

// *******各種設定*******

// 物理フェーダーとMIDIのチャンネル番号の対応を設定
// この配列の要素数とNUM_OF_FADERは一致させる必要があります。
// 例：ADC1台目のch0に接続したフェーダーをMIDIのCC15番で出力したい場合は、この配列の0番目に15と記入
// 例：ADC2台目のch4に接続したフェーダーをMIDIのCC5番で出力したい場合は、この配列の(8+4=)12番目に15と記入
unsigned char fader_map[] = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};

// SPIのSignalSelectピンを接続したIOの番号
// デジタル出力(HIGH or LOW)ができるIOならどこでもいいです。
// ADC1台目
#define SPI_SEL_1_8 10
// ADC2台目
#define SPI_SEL_9_16 7

// SPIのMOSI,MISO,CLOCKのピンは用いるマイコンごとに1つに決められています。使用するマイコンのデータシートを要確認。
// SPIのMOSI(MasterOut SlaveIn)ピンを接続したIOの番号
#define SPI_MOSI 16
// SPIのMISO(MasterOut SlaveIn)ピンを接続したIOの番号
#define SPI_MISO 14
// SPIのClockピンを接続したIOの番号
#define SPI_CLOCK 15

// USBから出力するMIDIのチャンネル番号
// 0~15で指定する。
#define MIDI_CH 1

// *******ここまで*******

```
