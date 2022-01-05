#include "MIDIUSB.h"
#include "MATH.h"

#define MIDI_NOTE_OFF 0x8
#define MIDI_NOTE_ON 0x9
#define MIDI_CC 0xB
#define MIDI_CC_MAX 127
#define MIDI_ON_VELOCITY 0x0F
#define MIDI_OFF_VELOCITY 0x00

#define NUM_OF_FADER 16

void read_all();
unsigned short read_adc(int adc_select, int channel);
void send_midi();
unsigned char convert_10to7(unsigned short adc_value);
void print_adc_value();
void control_change(unsigned char channel, unsigned char control, unsigned char value);

// ADCから読み取った値を保存する配列
unsigned short adc_value[NUM_OF_FADER];
// 読み取った値が前回読んだときと比べて変化しているか否かを保存する配列
bool adc_change_flag[NUM_OF_FADER];

// *******各種設定*******

// 物理フェーダーとMIDIのチャンネル番号の対応を設定
// この配列の要素数とNUM_OF_FADERは一致させる必要があります。
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

void setup()
{
  // put your setup code here, to run once:
  //各IOのモード設定
  pinMode(SPI_SEL_1_8, OUTPUT);
  pinMode(SPI_SEL_9_16, OUTPUT);
  pinMode(SPI_MOSI, OUTPUT);
  pinMode(SPI_MISO, INPUT);
  pinMode(SPI_CLOCK, OUTPUT);

  //SPI用の各IOを初期状態にする
  digitalWrite(SPI_SEL_1_8, HIGH);
  digitalWrite(SPI_SEL_9_16, HIGH);
  digitalWrite(SPI_MOSI, LOW);
  digitalWrite(SPI_CLOCK, LOW);

  // ADCの値と変化フラグ保存用配列を初期化
  for (int i = 0; i < NUM_OF_FADER; i++)
  {
    adc_value[i] = 0;
    adc_change_flag[i] = false;
  }

  Serial.begin(9600);
}

void loop()
{
  // put your main code here, to run repeatedly:
  // 16ch全てのADCの値を読み取り
  read_all();
  // 読み取り結果をArduinoIDEのSerialMonitorに表示する
  // print_adc_value();
  // 値が変化していたフェーダー分のみMIDIを送信
  send_midi();

  delay(50);
}

/**
 * @brief 全フェーダーのADC値を読み取る
 * 読み取り結果と変化フラグはルローバルの配列に代入
 * 
 * 2台の8chADCを用いていること前提のお行儀の良くない書き方になってますスミマセン
 */
void read_all()
{
  for (int i = 0; i < 8; i++)
  {
    // ADC1台目
    // ADC値を読み取り
    unsigned short new_value1 = read_adc(SPI_SEL_1_8, i);
    // MIDI用に7bitへ落とした上で前回読んだ値との変化有無(bool値)を計算
    adc_change_flag[i] = convert_10to7(adc_value[i]) != convert_10to7(new_value1);
    // 読み取った値を配列へ代入
    adc_value[i] = new_value1;

    // ADC2台目
    unsigned short new_value2 = read_adc(SPI_SEL_9_16, i);
    // 2台目のADCからの値と変化フラグは1台目の値に続くように1つの配列へ入れているためIndexがi+8となってます。
    adc_change_flag[i + 8] = convert_10to7(adc_value[i + 8]) != convert_10to7(new_value2);
    adc_value[i + 8] = new_value2;
  }
}

/**
 * @brief ADCのSelectピンとチャンネルを指定してADCの値を読み取る
 * 
 * @param adc_select 読みたいADCのSelectピン番号（SPI_SEL_1_8 or SPI_SEL_9_16）
 * @param channel 読みたいチャンネル(8chの場合0～7で指定)
 * @return unsigned short 読み取ったADCの値(10bit)
 * 
 * SPIの仕様についてはADCのデータシートを参照
 * https://akizukidenshi.com/download/ds/microchip/mcp3008.pdf
 */
unsigned short read_adc(int adc_select, int channel)
{
  unsigned short adcvalue = 0;
  // ADCに読み取りを指示するコマンド
  // 最上位のbitはスタートビットで1固定、2bit目はSingleモードを表す1
  unsigned char commandbits = 0b11000000;
  //commandbitsの左から3,4,5番目(それぞれD2,D1,D0)にチャンネル番号を指定
  commandbits |= (channel << 3);

  // 読み取るADCのSelectをLOWにし選択
  digitalWrite(adc_select, LOW); //Select adc
  // 読み取りを指示するコマンド（全5bit）を上位桁から送信
  for (int i = 7; i >= 3; i--)
  {
    digitalWrite(SPI_MOSI, commandbits << i);
    //cycle clock
    digitalWrite(SPI_CLOCK, HIGH);
    digitalWrite(SPI_CLOCK, LOW);
  }
  // 2クロック分待機
  digitalWrite(SPI_CLOCK, HIGH);
  digitalWrite(SPI_CLOCK, LOW);
  digitalWrite(SPI_CLOCK, HIGH);
  digitalWrite(SPI_CLOCK, LOW);

  // ADCから返ってきたデータ(10bit)を読み取る
  for (int i = 9; i >= 0; i--)
  {
    // （上位桁から）1bitずつ返ってくるので、シフトしながら加算していく
    adcvalue += digitalRead(SPI_MISO) << i;
    //cycle clock
    digitalWrite(SPI_CLOCK, HIGH);
    digitalWrite(SPI_CLOCK, LOW);
  }
  // 読み取るADCのSelectをHIGHにし選択解除（通信終了）
  digitalWrite(adc_select, HIGH);

  return adcvalue;
}

void send_midi()
{
  for (int i = 0; i < NUM_OF_FADER; i++)
  {
    // 値変化時のみMIDI出力
    if (adc_change_flag[i] == true)
    {
      unsigned char midi_value = convert_10to7(adc_value[i]);
      control_change(MIDI_CH - 1, fader_map[i], midi_value);
      MidiUSB.flush();
    }
  }
}

/**
 * @brief ADCは10bitでMIDIは7ビットなので、不要な下3Bitを落とす
 * 
 * @param adc_value 
 * @return unsigned char 
 */
unsigned char convert_10to7(unsigned short adc_value)
{
  return (unsigned char)(adc_value >> 3);
}

/**
 * @brief ADCから返ってきた値をSerialMonitorに表示
 */
void print_adc_value()
{
  for (int i = 0; i < 16; i++)
  {
    // 7bitに落として表示
    Serial.print(convert_10to7(adc_value[i]), DEC);
    Serial.print(" (");
    // 10bitのまま表示
    Serial.print(adc_value[i], DEC);
    Serial.print(")");
  }
  Serial.println("");
}

/**
 * @brief MIDIのCC信号を送信する
 * 
 * @param channel MIDIのチャンネル（0~15）
 * @param control CCのコントロールする番号（フェーダー番号に対応）
 * @param value 値（符号なし7bit 0~127で指定）
 */
void control_change(unsigned char channel, unsigned char control, unsigned char value)
{
  unsigned char status = MIDI_CC << 4 | channel;
  midiEventPacket_t event = {MIDI_CC, status, control, value};
  MidiUSB.sendMIDI(event);
}
