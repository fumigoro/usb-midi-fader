# 物理MIDIフェーダー

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
 
 
 
 