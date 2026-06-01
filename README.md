# crst01a_arduino_lib

CuGo V4.5等に搭載されたCuboRex製車両コントローラ(CRST01A)を Raspberry Pi Pico 2 W から制御するための Arduino ライブラリです。  

現在の対象商品
- [CuGo V4.5](https://cuborex.com/product/?id=33) 

---

## 目次
- [概要](#概要)
- [動作環境](#動作環境)
- [セットアップ](#セットアップ)
- [サンプルスケッチ](#サンプルスケッチ)
- [API リファレンス](#api-リファレンス)
- [電文フォーマット](#電文フォーマット)
- [ファイル構成](#ファイル構成)
- [クラス構成](#クラス構成)
- [ライセンス](#ライセンス)
- [関連リンク](#関連リンク)

---

## 概要

本リポジトリには車両コントローラ(CRST01A)をRaspberry Pi Pico 2 Wから制御できるArduino ライブラリとサンプルコードが含まれています。  
このArduino ライブラリを使用することでシーケンシャルな動作(前進、後進等の移動)を簡単にプログラムできます。  



---

## 動作環境

| 項目 | 内容 |
|------|------|
| 対応ボード | Raspberry Pi Pico 2 W |
| 通信方式 | UART 115200 bps |
| 通信インターフェース | Serial1（GP0,GP1） |
| Arduino コア | [arduino-pico](https://github.com/earlephilhower/arduino-pico) |
| 依存ライブラリ | [RPi_Pico_TimerInterrupt](https://github.com/khoih-prog/RPI_PICO_TimerInterrupt) |

---

## セットアップ

### 1. Arduino IDE のインストール

1. [公式サイト](https://www.arduino.cc/en/software) からご自身の OS に合ったバージョンを選択
2. **DOWNLOAD** を選択してArduino IDEをダウンロード
3. ダウンロードしたファイルを実行し、指示に従ってインストール

---

### 2. Raspberry Pi Pico 向け設定

1. **ファイル ＞ 環境設定** を選択

   ![環境設定](https://github.com/CuboRex-Development/cugo-beginner-programming/assets/22425319/f08cbb59-36f2-4ba0-80a3-889a7c337e0f)

2. **追加のボードマネージャ URL** に以下を入力

   ```
   https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
   ```

   ![ボードマネージャURL](https://github.com/CuboRex-Development/cugo-beginner-programming/assets/22425319/fc20fa37-3747-4060-8da3-9ae169b9df93)

3. **ツール ＞ ボード ＞ ボードマネージャ…** を選択

   ![ボードマネージャ](https://github.com/CuboRex-Development/cugo-beginner-programming/assets/22425319/320f1434-1135-4788-8b6e-df6194c24781)

4. `pico` で検索し **Raspberry Pi Pico/RP2040/RP2350** を見つけて **INSTALL** を押す

   <img width="649" height="475" alt="image" src="https://github.com/user-attachments/assets/fda6a160-b4bb-496f-9765-3217e786d144" />
   <img width="648" height="720" alt="image" src="https://github.com/user-attachments/assets/591a8995-d30d-4b00-b49a-bc5a4737863e" />


<details>
<summary>Python エラーが発生する場合</summary>

以下の図のようにpythonがうまく実行できないことでコンパイルが通らないことがあります。これはボードマネージャでRP2040(RP2350)をインストールしたのち、 削除してもう一度入れなおした際に発生します。すでに一度RP2040(RP2350)をインストールした後に2回目以降にインストールする際に必要なデータが展開しきれず、必要なものが再配置されないバグが発生していると考えられます。  
<img width="651" height="218" alt="image" src="https://github.com/user-attachments/assets/6c56ae28-4654-4443-b6c0-fd77cca24563" />

以下のフォルダに保存されている内容を削除します。

```
C:\Users\<ユーザー名>\AppData\Local\Arduino15\packages\rp2040\tools\pqt-python3\1.0.1-base-3a57aed-1
```

その後、ボードマネージャから**Raspberry Pi Pico/RP2040/RP2350**を削除(REMOVE)し、再度インストールしてください。

</details>

---

### 3. ライブラリのインストール

#### 3-1. crst01a_arduino_lib のインストール

1. 本リポジトリの **Code ＞ Download ZIP** からダウンロード

   <img width="485" height="371" alt="image" src="https://github.com/user-attachments/assets/3c96f657-b5ac-4787-bcf8-68e500d339bd" />


2. Arduino IDE で **スケッチ ＞ ライブラリをインクルード ＞ .ZIP 形式のライブラリをインストール…** を選択し、ダウンロードした ZIP を指定

   <img width="576" height="390" alt="image" src="https://github.com/user-attachments/assets/f9fdcdf8-1da7-40b7-8b0b-52b968a9cf10" />



#### 3-2. RPi_Pico_TimerInterrupt のインストール

1. **スケッチ ＞ ライブラリをインクルード ＞ ライブラリを管理…** を選択

   <img width="600" height="396" alt="image" src="https://github.com/user-attachments/assets/2968ca2e-ae91-43f9-a636-12a595cb7190" />


2. `RPi_Pico_TimerInterrupt` で検索し **RPI_PICO_TimerInterrupt** を見つけて **INSTALL** を押す

   <img width="811" height="649" alt="image" src="https://github.com/user-attachments/assets/5ca84278-7f1f-48b8-a324-b4094712670c" />
   <img width="810" height="774" alt="image" src="https://github.com/user-attachments/assets/c10c5a4c-8dc9-4e3b-9f6c-a753b425061d" />



---

### 4. Raspberry Pi Pico 2W への書き込み

1. スケッチを Arduino IDE で開く

   [スケッチのサンプルはこちら](#サンプルスケッチ)

2. USB ケーブルで PC と Raspberry Pi Pico 2W を接続する（PC に認識されない場合は **BOOTSEL** ボタンを押しながら挿入）

   <ここにBOOTSELボタンの画像>

3. **ツール ＞ ボード ＞ Raspberry Pi Pico/RP2040/PR2350 ＞ Raspberry Pi Pico 2W** を選択

   <img width="1019" height="471" alt="image" src="https://github.com/user-attachments/assets/8de292ad-0c82-4219-9fad-2628b50500a7" />


4. **ツール ＞ ポート** から Raspberry Pi Pico 2W のポートまたは"UF2 Board"を選択

   <img width="1146" height="657" alt="image" src="https://github.com/user-attachments/assets/ee090fd2-b490-4d5f-a728-6396cf24b6bf" />

   <img width="830" height="723" alt="image" src="https://github.com/user-attachments/assets/3580c55e-eefd-4919-8c35-a76b5250de71" />


5. **→（書き込み）** ボタンを押す

   <img width="630" height="477" alt="image" src="https://github.com/user-attachments/assets/04277e37-5092-4b1e-97e8-93b1ace27879" />


6. **「Done uploading」** と表示されれば完了

   <img width="592" height="187" alt="image" src="https://github.com/user-attachments/assets/ebc80402-d780-4e6f-8ef6-7a015315346e" />

---

## サンプルスケッチ

サンプルスケッチは本リポジトリ内の`examples`フォルダ内にあります。

Arduino IDEからは以下の手順で開きます。

1. Arduino IDE で **ファイル ＞ スケッチ例  ＞ カスタムライブラリのスケッチ例 ＞ crst01a_arduino_lib** から開きたいサンプルを選択します。
<img width="674" height="586" alt="image" src="https://github.com/user-attachments/assets/e5c8e1d6-e6fa-4308-b8df-2bf3c09f03a8" />


### square_run

コマンドモードに入ると1m四方の四角形を描くように走行するスケッチです。  
四角形をなぞった後は停止し続けます。  
RCモードから再度コマンドモードに入ると再度四角形を描くように走行します。

### change_max_speed

最高速度を変更するスケッチです。  
変更後に車両コントローラのフラッシュメモリへ保存します。  
動作終了後はRaspberry Pi Pico 2 W上のLEDが点灯します。  

>[!IMPORTANT]
>スケッチ実行直後に設定がロボットに反映されます。スケッチ実行時にロボットの電源が切れている場合は反映されません。    
>スケッチ書き込み後はロボットの電源を切り、PCとUSBケーブルでの接続を切ります。  
>再度ロボットの電源を入れて確実にスケッチを実行してください。  

cugoDiffDriveCtrl.SetMaxSpeed()で走行速度と旋回速度を設定します。  
第一引数で走行速度(0.001m/s単位)、第二引数で旋回速度(0.001rad/s単位)を指定します。  
【例】
```cpp
cugoDiffDriveCtrl.SetMaxSpeed(1222,1500); // 走行速度を1.222m/s(4.4km/h)と1.5rad/sに設定
```

>[!CAUTION]
>フラッシュメモリへ書き込むため、パラメータ変更後は別のスケッチを書き込んでください。  
>check_read(サンプルスケッチ)等のフラッシュメモリへ書き込みを行わないスケッチを書き込み、運用してください。  

### change_battery_voltage_limits

電圧の上下限設定を変更するスケッチです。  
変更後に車両コントローラのフラッシュメモリへ保存します。  
動作終了後はRaspberry Pi Pico 2 W上のLEDが点灯します。  

>[!IMPORTANT]
>スケッチ実行直後に設定がロボットに反映されます。スケッチ実行時にロボットの電源が切れている場合は反映されません。    
>スケッチ書き込み後はロボットの電源を切り、PCとUSBケーブルでの接続を切ります。  
>再度ロボットの電源を入れて確実にスケッチを実行してください。  

cugoCommon.SetVoltage()で電源電圧の上下限を設定します。  
第一引数で下限電圧(0.1V単位)、第二引数で上限電圧(0.1V単位)を指定します。  
基板における電圧取得精度の特性上、設定値にはある程度の許容幅を持たせることを推奨します。  
【例】
```cpp
cugoCommon.SetVoltage(180,320);	// 18V以下、32V以上で電圧異常と判断する設定
```

>[!CAUTION]
>フラッシュメモリへ書き込むため、パラメータ変更後は別のスケッチを書き込んでください。  
>check_read(サンプルスケッチ)等のフラッシュメモリへ書き込みを行わないスケッチを書き込み、運用してください。

### check_read

以下の情報を読み取りシリアル通信で送信するスケッチです。

- 車両コントローラのバージョン
- 最高速度
- 電圧の上下限設定

読み取りに失敗した場合はRaspberry Pi Pico 2 W上のLEDが点滅します。  

>[!IMPORTANT]
>シリアル通信はRaspberry Pi Pico 2 Wのmicro USBを使用し、ボーレートは115.2kbpsです。  
>ロボットの電源を切り、PCとUSBケーブルでの接続を切ります。ロボットの電源を入れてから5秒以内にPCとUSBケーブルで接続し、シリアルモニタで情報を確認してください。  
>ロボットの電源を入れるより先にPCとつないだ場合はロボットの電源が入ってないため通信が失敗します。  

---

## API リファレンス

| 関数名 | 内容 |
|--------|------|
| cugoCommon.Init() | CugoCommon関連初期化 |
| cugoCommon.SetControlMode() | 動作モード設定 |
| cugoCommon.GetControlMode() | 動作モード取得 |
| cugoCommon.GetErr() | エラー取得 |
| cugoCommon.ClearErr() | エラークリア |
| cugoCommon.GetVoltage() | 電圧取得 |
| cugoCommon.Wait() | 待機 |
| cugoCommon.SetSwitchReset() | RCモード切替時の動作設定 |
| cugoCommon.GetSwitchReset() | RCモード切替時の動作設定取得 |
| cugoCommon.SaveParamReq() | パラメータ保存 |
| cugoCommon.SetVoltageConfig() | 電圧異常の閾値設定 |
| cugoCommon.GetVoltageConfig() | 電圧異常の閾値設定取得 |
| cugoCommon.GetVersion() | バージョン取得 |
| cugoDiffDriveCtrl.Init() | CuGoDiffDriveCtrl関連初期化 |
| cugoDiffDriveCtrl.MoveForward() | 直進走行 |
| cugoDiffDriveCtrl.MoveTurn() | その場旋回 |
| cugoDiffDriveCtrl.MoveCurve() | 曲線走行 |
| cugoDiffDriveCtrl.Stop() | 動作停止 |
| cugoDiffDriveCtrl.SetMoveSpeed() | 速度指定での走行 |
| cugoDiffDriveCtrl.SetKinematics() | 車体情報設定 |
| cugoDiffDriveCtrl.SetMaxSpeed() | 最大速度設定 |
| cugoDiffDriveCtrl.GetMaxSpeed() | 最大速度設定取得 |

---

### CugoCommon
他のクラスでも使用する共通クラスです。

#### CugoCommon関連初期化

**void CugoCommon::Init(HardwareSerial \*pSerial = &Serial)**

- `pSerial`：ログ出力先シリアル。`NULL` を指定するとログを出力しない。デフォルトはSerial
- setup( )の中で一度呼ぶこと

【例】
```cpp
void setup() {
   cugoCommon.Init();  // Serialに対してログ出力
}
```

---

#### 動作モード設定

**void CugoCommon::SetControlMode(uint8_t mode)** 

- `mode`：設定するモード
- 動作モードを設定します。設定できる動作モードは以下となります。

| 定義 | 値 | 意味 |
|------|----|------|
| `CUGO_RC_MODE` | 0x80 | RCモード。プロポから操作するモード |
| `CUGO_CMD_MODE` | 0x81 | コマンドモード。シリアル経由で操作するモード |

【例】
```cpp
cugoCommon.SetControlMode(CUGO_RC_MODE);     // RCモードに設定
cugoCommon.SetControlMode(CUGO_CMD_MODE);    // コマンドモードに設定
```

---

#### 動作モード取得

**bool CugoCommon::GetControlMode(uint8_t \*mode)**

- `mode`：現在のモード格納先。
- 通信タイムアウト時は `false` を返す。

【例】
```cpp
uint8_t nowMode;
cugoCommon.GetControlMode(&nowMode);   // 現在の動作モードがnowModeに入る
```

---

#### エラー取得


**uint32_t CugoCommon::GetErr(void)**  

- 戻り値は現在のエラー状況を返します。


戻り値ビット定義：

| 定義 | ビット | 内容 |
|------|--------|------|
| `CUGO_ERR_CTL_VSYS_UV` | bit0 | システム電源異常 |
| `CUGO_ERR_CTL_VBUS_OV` | bit1 | 電源電圧超過 |
| `CUGO_ERR_CTL_VBUS_UV` | bit2 | 電源電圧低下 |
| `CUGO_ERR_CTL_EMR_DEC` | bit3 | 緊急減速実行後 |
| `CUGO_ERR_CTL_EMR_LATCHED` | bit4 | 非常停止スイッチ押下 |
| `CUGO_ERR_CTL_BUMPER_LATCHED` | bit5 | バンパー接触 |
| `CUGO_ERR_MD_OVERCURRENT` | bit8 | モータドライバ過電流 |
| `CUGO_ERR_MD_VOLTERROR` | bit9 | モータドライバ電圧異常 |
| `CUGO_ERR_MD_OVERTEMP` | bit10 | モータドライバ温度異常 |
| `CUGO_ERR_MD_CAN_TIMEOUTx` | bit12-15 | モータドライバ x との通信エラー |
| `CUGO_ERR_CMD_TIMEOUT` | bit16 | 車両コントローラとの通信タイムアウト |

【例】
```cpp
uint32_t nowErr;
nowErr = cugoCommon.GetErr(); // 現在のエラーがnowErrに入る
```

---

#### エラークリア


**void     CugoCommon::ClearErr(uint32_t err = CUGO_ERR_ALL)**

- `err`：クリアするエラーを選択。デフォルトは全エラークリア
- 現在のエラー状況をクリアします。
- エラーの原因が取り除かれてない場合はクリアしてもエラーが残ります。


【例】
```cpp
cugoCommon.ClearErr();                       // すべてのエラーをクリア
cugoCommon.ClearErr(CUGO_ERR_CTL_VSYS_UV);   // システム電源異常をクリア
```

---

#### 電圧取得


**bool CugoCommon::GetVoltage(uint16_t \*driverVoltage)**

- `driverVoltage`：電源電圧（×0.1 V）
- 通信タイムアウト時は `false` を返す。
> [!NOTE]
> 取得できる電源電圧はあくまで目安です。

【例】
```cpp
uint16_t nowVoltage;
cugoCommon.GetVoltage(&nowVoltage); // 現在の電圧がnowVoltageに入る
```

---

#### 待機


**bool CugoCommon::Wait(uint32_t time)**

- `time`：待機時間（ms）。
- 待機中にエラーが発生した場合は即座に関数から抜けて `false` を返す。
- RCモードに切り替わった場合の挙動は `SetSwitchReset()` の設定に従う。

> [!WARNING]
> スケッチ内では `delay()` の代わりに `Wait()` を使用してください。  
> Arduino 標準の `delay()` はブロッキング動作のため、待機中に RC モードへ切り替わっても `SetSwitchReset()` の設定が反映されません。  
> `Wait()` を使用することで、エラー検知や RC モード切替への対応が正しく動作します。

【例】
```cpp
cugoCommon.Wait(500);   // 500ms待機
```

---

#### RCモード切替時の動作設定


**void CugoCommon::SetSwitchReset(bool switchReset)**  

- `switchReset` を `true` にすると、プロポで RC モードに切り替えた際に `Wait()` / `SetMoveSpeed()` から即座に抜けて `loop()` の先頭に戻る。
- `false` の場合はコマンドモードに戻るまで待機し、再度コマンドモードに戻ると処理を再開する。

【例】
```cpp
cugoCommon.SetSwitchReset(true);    // 動作設定
```

---

#### 現在のRCモード切替時の動作設定取得

**bool CugoCommon::GetSwitchReset(void)**

- 戻り値は現在のRCモード切替時の動作設定を返す。

【例】
```cpp
bool nowSwitchReset = cugoCommon.GetSwitchReset(); // 現在の動作設定を変数に格納する
```

---

#### パラメータ保存

**bool CugoCommon::SaveParamReq(void)**

- 現在のパラメータをフラッシュメモリに保存する。
- 通信タイムアウト時は `false` を返す。

【例】
```cpp
cugoCommon.SaveParamReq();				// フラッシュメモリに保存
```

---

#### 電圧上下限設定

**bool CugoCommon::SetVoltageConfig(uint16_t driverMinVoltage, uint16_t driverMaxVoltage)**

- `driverMinVoltage`：電圧異常の下限値（×0.1 V）
- `driverMaxVoltage`：電圧異常の上限値（×0.1 V）
- 車両コントローラが電圧異常と検知する電圧の上下限を設定する。
- 設定値が範囲外の場合は `false` を返す。
- 設定可能な値は150～520(15V～52V)。
- V4.5のデフォルトは下限値が180(18.0V)、上限値が320(32.0V)。

【例】
```cpp
cugoCommon.SetVoltageConfig(180,320);	// 18V以下、32V以上で電圧異常と判断する設定
```

---

#### 電圧上下限設定取得

**bool CugoCommon::GetVoltageConfig(uint16_t *pDriverMinVoltage, uint16_t *pDriverMaxVoltage)**

- `pDriverMinVoltage`：電圧異常の下限値（×0.1 V）
- `pDriverMaxVoltage`：電圧異常の上限値（×0.1 V）
- 車両コントローラが電圧異常と検知する電圧の設定を取得する。
- 通信タイムアウト時は `false` を返す。

【例】
```cpp
uint16_t driverMinVoltage,driverMaxVoltage;

cugoCommon.GetVoltageConfig(&driverMinVoltage, &driverMaxVoltage);
```

---

#### バージョン取得

**bool CugoCommon::GetVersion(uint8_t *pVer0, uint8_t *pVer1, uint8_t *pVer2)**

- `pVer0`：メジャーバージョン
- `pVer1`：マイナーバージョン
- `pVer2`：パッチバージョン
- 車両コントローラのバージョンを取得する。
- 通信タイムアウト時は `false` を返す。

【例】
```cpp
uint8_t ver0,ver1,ver2;

cugoCommon.GetVersion(&ver0, &ver1, &ver2);
```

---

### CuGoDiffDriveCtrl

差動二輪ロボット（左右独立駆動）向けの走行制御クラスです。

> [!WARNING]
> このクラスは速度制御と経過時間のみにより動作するため正確な自律制御ができません。 
> 指示した値はあくまで目安となります。

#### CuGoDiffDriveCtrl関連初期化


**void CugoDiffDriveCtrl::Init(HardwareSerial \*pSerial = &Serial)**

- `pSerial`：ログ出力先シリアル。`NULL` を指定するとログを出力しない。デフォルトはSerial
- setup( )の中で一度呼ぶこと

【例】
```cpp
void setup() {
   cugoDiffDriveCtrl.Init();  // Serialに対してログ出力
}
```

---

#### 直進走行


**bool CugoDiffDriveCtrl::MoveForward(float targetDistance, float targetSpeed = 0.3)**

- `targetDistance`：目標距離（m）。プラスの値で前進、マイナスの値で後進。
- `targetSpeed`：目標速度（m/s）。常に正の値を指定する。デフォルト 0.3 m/s。
- エラー発生時は `false` を返して停止する。

【例】
```cpp
cugoDiffDriveCtrl.MoveForward(1.0);       // 0.3 m/s で 1.0 m 前進
cugoDiffDriveCtrl.MoveForward(2.0, 0.5);  // 0.5 m/s で 2.0 m 前進
cugoDiffDriveCtrl.MoveForward(-0.5, 0.2); // 0.2 m/s で 0.5 m 後進
```

---

#### その場旋回

**bool CugoDiffDriveCtrl::MoveTurn(float targetDegree, float targetSpeed = 45)**

- `targetDegree`：旋回角度（deg）。プラスの値で反時計回り、マイナスの値で時計回り。
- `targetSpeed`：旋回速度（deg/s）。常に正の値を指定する。デフォルト 45 deg/s。
- エラー発生時は `false` を返して停止する。

【例】
```cpp
cugoDiffDriveCtrl.MoveTurn(90.0);        // 45 deg/s で 90° 反時計回りに旋回
cugoDiffDriveCtrl.MoveTurn(-90.0);       // 45 deg/s で 90° 時計回りに旋回
cugoDiffDriveCtrl.MoveTurn(180.0, 50.0);   // 50 deg/s で 180° 反時計回りに旋回
```

---

#### 曲線走行

**bool CugoDiffDriveCtrl::MoveCurve(float targetRadius, float targetDegree, float targetSpeed = 0.3)**

- `targetRadius`：旋回半径（m）。正の値を指定する。
- `targetDegree`：旋回角度（deg）。プラスの値で反時計回り、マイナスの値で時計回り。
- `targetSpeed`：速度（m/s）。プラスの値で前進方向、マイナスの値で後進方向に円弧を描く。デフォルト 0.3 m/s。
- エラー発生時は `false` を返して停止する。

【例】
```cpp
cugoDiffDriveCtrl.MoveCurve(1.0, 90.0);        // 半径 1.0 m の円弧を 0.3 m/s で 90° 反時計回りに走行
cugoDiffDriveCtrl.MoveCurve(0.5, -180.0, 0.2); // 半径 0.5 m の円弧を 0.2 m/s で 180° 時計回りに走行
cugoDiffDriveCtrl.MoveCurve(2.0, 360.0, 0.5);  // 半径 2.0 m の円を 0.5 m/s で 1 周反時計回りに走行
```

---

#### 動作停止

**bool CugoDiffDriveCtrl::Stop(void)**

- 走行を停止する。エラーが発生している場合は `false` を返す。

【例】
```cpp
cugoDiffDriveCtrl.Stop();  // 現在の走行を停止して速度ゼロにする
```

---

#### 速度指定での走行

**void CugoDiffDriveCtrl::SetMoveSpeed(int16_t xSpeed, int16_t yawSpeed)**

- `xSpeed`：前後速度（mm/s）。プラスの値で前進、マイナスの値で後進。
- `yawSpeed`：旋回速度（mrad/s）。プラスの値で反時計回り、マイナスの値で時計回り。
- 目標距離・角度を指定せず、速度を直接指令する場合に使用する。

【例】
```cpp
cugoDiffDriveCtrl.SetMoveSpeed(300, 0);   // 0.3 m/s で直進前進
cugoDiffDriveCtrl.SetMoveSpeed(-200, 0);  // 0.2 m/s で直進後進
cugoDiffDriveCtrl.SetMoveSpeed(0, 785);   // その場で約 45 deg/s 反時計回りに旋回
cugoDiffDriveCtrl.SetMoveSpeed(200, 500); // 前進しながら反時計回りに旋回（カーブ走行）
```

---

#### 車体情報設定

**void CugoDiffDriveCtrl::SetKinematics(float leftTireDiameter, float rightTireDiameter,
                   float leftGearRatio,    float rightGearRatio,
                   float tread)**

- `leftTireDiameter`：左タイヤ直径（m）
- `rightTireDiameter`：右タイヤ直径（m）
- `leftGearRatio`：左減速比。減速している場合は 1 未満の値を指定する。
- `rightGearRatio`：右減速比。減速している場合は 1 未満の値を指定する。
- `tread`：左右タイヤ間距離（m）
- 指定した値から順運動学・逆運動学行列を計算して CRST01A に書き込む。
> [!IMPORTANT]
> ロボットの足回りを変更した場合は変更に合わせてパラメータを変更する必要があります。  
> 出荷時から変更していない場合は設定する必要がありません。  
> タイヤ径・減速比・トレッドが実機と一致しない場合、`MoveForward()` / `MoveTurn()` 等の距離・角度に誤差が生じます。

【例】
```cpp
// 左右のタイヤ直径が0.07716m、減速比が0.1144、トレッド幅が0.38m
cugoDiffDriveCtrl.SetKinematics(0.07716,0.07716,0.1144,0.1144,0.38);
```

---

#### 最大速度設定

**void CugoDiffDriveCtrl::SetMaxSpeed(uint16_t xSpeed, uint16_t yawSpeed)**

- `xSpeed`：走行速度(0.001m/s単位)
- `yawSpeed`：旋回速度(0.001rad/s単位)
- 走行速度と旋回速度の最大を設定する。
- V4.5のデフォルトは走行速度が1.0m/s、旋回速度が1.5rad/s。
- V4.5の最高速度は走行速度が1.222m/s、旋回速度が6.42rad/s。

【例】
```cpp
cugoDiffDriveCtrl.SetMaxSpeed(1222,1500);	// 走行速度を1.222m/s(4.4km/h)と1.5rad/sに設定
```

---

#### 最大速度設定取得

**void CugoDiffDriveCtrl::GetMaxSpeed(uint16_t *pXSpeed, uint16_t *pYawSpeed)**

- `pXSpeed`：走行速度(0.001m/s単位)
- `pYawSpeed`：旋回速度(0.001rad/s単位)
- 走行速度と旋回速度の最大設定を取得する。

【例】
```cpp
uint16_t x,yaw;

cugoDiffDriveCtrl.GetMaxSpeed(&x, &yaw);
```

---

### Crst01a（上級者向け低レベル API）

> [!WARNING]
> 通常の使用では触る必要はありません。
> `CugoCommon` や `CuGoDiffDriveCtrl`から呼ぶためのクラスです。  
> `CugoCommon` / `CuGoDiffDriveCtrl` で対応できない場合や、CRST01A との通信を細かく制御したい場合のみ参照してください。

#### 基本制御

| 関数 | 概要 |
|------|------|
| `Crst01a::SetControlMode(mode)` | RCモード / コマンドモード切替 |
| `Crst01a::SetEmergencyDeceleration()` | 緊急減速 |
| `Crst01a::ClearControllerError(err)` | 車両コントローラエラー解除 |
| `Crst01a::ClearDriverError(err)` | モータドライバエラー解除 |
| `Crst01a::ClearEncoderCount()` | エンコーダリセット |
| `Crst01a::SetMoveSpeed(x, y, yaw)` | 速度指令（mm/s, mrad/s） |
| `Crst01a::SetLights(headlight, towerlight)` | ライト制御 |
| `Crst01a::SaveParamReq(timeout)` | パラメータをフラッシュに保存 |

#### 定期受信データ取得

| 関数 | 取得内容 |
|------|---------|
| `Crst01a::GetSysStatus(...)` | コントローラステータス・エラー・電圧 |
| `Crst01a::GetReadRunStatus(x, y, yaw, time)` | 現在の走行速度 |
| `Crst01a::GetExtIo(...)` | ヘッドライト・タワーライト・外部入力 |
| `Crst01a::GetEncoder(enc[4], time)` | モータエンコーダ（4軸） |
| `Crst01a::GetMdTemp(temp[4], time)` | モータドライバ温度（4軸） |
| `Crst01a::GetMdStatus(err[4], time)` | モータドライバエラーコード（4軸） |
| `Crst01a::GetMotorOut(speed[4], torque[4], time)` | モータ角速度・トルク（4軸） |
| `Crst01a::GetSbus(val[16], time)` | SBUS 16 チャンネル値 |


#### パラメータ読み出し（要求→応答）

| 関数 | 内容 |
|------|------|
| `Crst01a::GetDataPeriodic(...)` | 定期送信周波数・対象確認 |
| `Crst01a::GetMaxSpeed(x, y, yaw, timeout)` | 最大速度設定値 |
| `Crst01a::GetBumperBrake(bumper, brake, timeout)` | バンパー・ブレーキ設定 |
| `Crst01a::GetVersion(v0, v1, v2, timeout)` | ファームウェアバージョン |
| `Crst01a::GetVoltageConfig(min, max, timeout)` | 電圧設定値 |
| `Crst01a::GetMdConfig0～5(...)` | モータドライバ設定 |
| `Crst01a::GetRcConfig0～3(...)` | RC設定 |
| `Crst01a::GetFwdKinematics(data[12], timeout)` | 順運動学行列（3×4） |
| `Crst01a::GetInvKinematics(data[12], timeout)` | 逆運動学行列（4×3） |

---

## 電文フォーマット


| 位置 | 名前   | 意味|
|--------|----------|----------|
| Byte 0 | Header   | 0xA0固定 |
| Byte 1 | FuncCode | 電文ID |
| Byte 2-9 | Data\[0-7] | データ |
| Byte 10 | DataID | 送信ごとにインクリメントする値 |
| Byte 11 | CheckSum | Byte 0〜10 の合計値（下位 1 バイト） |


---

## ファイル構成

```

crst01a_arduino_lib/
├── src/
│   ├── Crst01a.h / .cpp              # 車両コントローラ通信
│   ├── CugoCommon.h / .cpp           # 共通処理
│   └── CugoDiffDriveCtrl.h / .cpp    # 差動二輪走行制御
├── examples/
│   ├── square_run/                   # 正方形走行サンプル
│   ├── change_max_speed/             # 最高速度変更サンプル
│   ├── change_battery_voltage_limits/# 電圧の上下限設定サンプル
│   └── check_read/                   # 変更確認サンプル
└── library.properties
```

---

## クラス構成

| クラス | ヘッダ | 概要 |
|--------|--------|------|
| `Crst01a` | `Crst01a.h` | CRST01A との低レベル UART 通信 |
| `CugoCommon` | `CugoCommon.h` | モード管理・エラー管理・共通処理 |
| `CuGoDiffDriveCtrl` | `CugoDiffDriveCtrl.h` | 差動二輪ロボット向け走行制御 |

各クラスのグローバルインスタンスがライブラリ内で定義されています。

```cpp
extern Crst01a          crst01a;
extern CugoCommon       cugoCommon;
extern CuGoDiffDriveCtrl cugoDiffDriveCtrl;
```

---

## ライセンス

このプロジェクトはApache License 2.0のもと、公開されています。詳細はLICENSEをご覧ください。

---

## 関連リンク

- [CuboRex 公式サイト](https://cuborex.com)
- [GitHub リポジトリ](https://github.com/CuboRex-Development/crst01a_arduino_lib)
- [CuGo V4.5　商品ページ](https://cuborex.com/product/?id=33)
