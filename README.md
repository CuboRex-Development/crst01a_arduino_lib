# crst01a_arduino_lib

CRST01A（CuboRex製 車両コントローラ）を Raspberry Pi Pico 2 W から制御するための Arduino ライブラリです。

---

## 概要

CRST01A と UART（115200 bps）で通信し、走行速度の指令・センサデータの取得・パラメータの設定/読み出しを行います。
20 ms 周期のタイマー割り込みで受信処理を自動実行します。

### クラス構成

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

## 動作環境

| 項目 | 内容 |
|------|------|
| 対応ボード | Raspberry Pi Pico 2 W |
| 通信方式 | UART（Serial1）115200 bps |
| Arduino コア | [arduino-pico](https://github.com/earlephilhower/arduino-pico) |
| 依存ライブラリ | [RPi_Pico_TimerInterrupt](https://github.com/khoih-prog/RPI_PICO_TimerInterrupt) |

---

## セットアップ

### 1. Arduino IDE のインストール

1. [公式サイト](https://www.arduino.cc/en/software) からご自身の OS に合ったバージョンを選択
2. **JUST DOWNLOAD** または **CONTRIBUTE & DOWNLOAD** を選択してダウンロード
3. ダウンロードしたファイルを実行し、指示に従ってインストール

---

### 2. Raspberry Pi Pico の初期設定

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
   <img width="756" height="729" alt="image" src="https://github.com/user-attachments/assets/591a8995-d30d-4b00-b49a-bc5a4737863e" />


<details>
<summary>Python エラーが発生する場合</summary>

以下の図のようにpythonがうまく実行できないことでコンパイルが通らないことがあります。これはボードマネージャでRP2040(RP2350)をインストールしたのち、 削除してもう一度入れなおした際に発生します。すでに一度RP2040(RP2350)をインストールした後に2回目以降にインストールする際に必要なデータが展開しきれず、必要なものが再配置されないバグが発生していると考えられます。
<img width="651" height="218" alt="image" src="https://github.com/user-attachments/assets/6c56ae28-4654-4443-b6c0-fd77cca24563" />


```
C:\Users\<ユーザー名>\AppData\Local\Arduino15\packages\rp2040\tools\pqt-python3\1.0.1-base-3a57aed-1
```

![フォルダ削除](https://private-user-images.githubusercontent.com/22425319/272511907-6f26ac47-6050-43bc-bdfa-2f72e3d013c7.png)

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

### 4. ロボットパラメータの設定

ロボットの足回りを変更した場合は変更に合わせてパラメータを変更する必要があります。  
(出荷時から変更していない場合は設定する必要がありません。)  
スケッチ内でロボットの機構パラメータを `SetKinematics()` で設定してください。
値は CRST01A のフラッシュに書き込まれ、電源を切っても保持されます。

```cpp
cugoDiffDriveCtrl.SetKinematics(
    0.07716,   // 左タイヤ直径（m）
    0.07716,   // 右タイヤ直径（m）
    0.1144,    // 左減速比
    0.1144,    // 右減速比
    0.38       // トレッド（左右タイヤ間距離）（m）
);
```

> **注意：** タイヤ径・減速比・トレッドが実機と一致しない場合、`MoveForward()` / `MoveTurn()` 等の距離・角度に誤差が生じます。

---

### 5. Raspberry Pi Pico 2W への書き込み

1. スケッチが Arduino IDE で開かれていることを確認

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
7. <img width="592" height="187" alt="image" src="https://github.com/user-attachments/assets/ebc80402-d780-4e6f-8ef6-7a015315346e" />


---

### 6. CRST01A のシリアル出力をRaspberry Pi Pico 2Wに変更

電源が切れていることを確認し、CRST01A の DIP スイッチを下図のように変更してください。  
＜ここにDIPスイッチの画像＞

---

## クイックスタート

```cpp
#include <CugoCommon.h>
#include <CugoDiffDriveCtrl.h>

void setup() {
    Serial.begin(115200);

    cugoDiffDriveCtrl.Init();   // タイマー割り込みの起動を含む
    cugoCommon.Init();

    // RCモードからコマンドモードに切り替わったら自動走行を開始
    cugoCommon.SetSwitchReset(true);
}

void loop() {
    uint8_t mode;
    cugoCommon.GetControlMode(&mode);

    if (CUGO_CMD_MODE == mode) {
        // 1辺 1.0 m の正方形走行
        for (int i = 0; i < 4; i++) {
            cugoDiffDriveCtrl.MoveForward(1.0);   // 1.0 m 前進
            cugoCommon.Wait(500);
            cugoDiffDriveCtrl.MoveTurn(90);        // 90° 旋回
            cugoCommon.Wait(500);
        }
        cugoDiffDriveCtrl.Stop();
        while (1);
    }
}
```

---

## API リファレンス

### CugoCommon

#### 初期化

```cpp
void CugoCommon::Init(HardwareSerial *pSerial = &Serial)
```
- `pSerial`：ログ出力先シリアル。`NULL` を指定するとログを出力しない。

---

#### モード管理

```cpp
void SetControlMode(uint8_t mode)
bool GetControlMode(uint8_t *mode)
```

| 定数 | 値 | 意味 |
|------|----|------|
| `CUGO_RC_MODE` | 0x80 | RC（プロポ）モード |
| `CUGO_CMD_MODE` | 0x81 | コマンド（自動走行）モード |

---

#### エラー管理

```cpp
uint32_t GetErr(void)
void     ClearErr(uint32_t err = CUGO_ERR_ALL)
```

`GetErr()` の戻り値ビット定義：

| 定数 | ビット | 内容 |
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

---

#### 電圧取得

```cpp
bool GetVoltage(uint16_t *driverVoltage)
```
- `driverVoltage`：電源電圧（×0.1 V）。通信タイムアウト時は `false` を返す。

---

#### 待機

```cpp
bool Wait(uint32_t time)
```
- `time`：待機時間（ms）。
- 待機中にエラーが発生した場合は即座に `false` を返す。
- RCモードに切り替わった場合の挙動は `SetSwitchReset()` の設定に従う。

> **注意：スケッチ内では `delay()` の代わりに `Wait()` を使用してください。**
> Arduino 標準の `delay()` はブロッキング待機のため、待機中に RC モードへ切り替わっても `SetSwitchReset()` の設定が反映されません。
> `Wait()` を使用することで、エラー検知や RC モード切替への対応が正しく動作します。

---

#### RCモード切替時の動作設定

```cpp
void SetSwitchReset(bool switchReset)
bool GetSwitchReset(void)
```
- `switchReset` を `true` にすると、プロポで RC モードに切り替えた際に `Wait()` / `SetMoveSpeed()` から即座に抜けて `loop()` の先頭に戻る。
- `false` の場合はコマンドモードに戻るまで待機してから処理を再開する。

---

### CuGoDiffDriveCtrl

差動二輪ロボット（左右独立駆動）向けの走行制御クラスです。

#### 初期化

```cpp
void Init(HardwareSerial *pSerial = &Serial)
```

---

#### 走行制御

```cpp
bool MoveForward(float targetDistance, float targetSpeed = 0.3)
```
- `targetDistance`：目標距離（m）。プラスの値で前進、マイナスの値で後進。
- `targetSpeed`：目標速度（m/s）。常に正の値を指定する。デフォルト 0.3 m/s。
- エラー発生時は `false` を返して停止する。

| 例 | 動作 |
|----|------|
| `MoveForward(1.0)` | 0.3 m/s で 1.0 m 前進 |
| `MoveForward(2.0, 0.5)` | 0.5 m/s で 2.0 m 前進 |
| `MoveForward(-0.5, 0.2)` | 0.2 m/s で 0.5 m 後進 |

---

```cpp
bool MoveTurn(float targetDegree, float targetSpeed = 45)
```
- `targetDegree`：旋回角度（deg）。プラスの値で反時計回り、マイナスの値で時計回り。
- `targetSpeed`：旋回速度（deg/s）。常に正の値を指定する。デフォルト 45 deg/s。
- エラー発生時は `false` を返して停止する。

| 例 | 動作 |
|----|------|
| `MoveTurn(90)` | 45 deg/s で 90° 反時計回りに旋回 |
| `MoveTurn(-90)` | 45 deg/s で 90° 時計回りに旋回 |
| `MoveTurn(180, 90)` | 90 deg/s で 180° 反時計回りに旋回 |

---

```cpp
bool MoveCurve(float targetRadius, float targetDegree, float targetSpeed = 0.3)
```
- `targetRadius`：旋回半径（m）。正の値を指定する。
- `targetDegree`：旋回角度（deg）。プラスの値で反時計回り、マイナスの値で時計回り。
- `targetSpeed`：速度（m/s）。プラスの値で前進方向、マイナスの値で後進方向に円弧を描く。デフォルト 0.3 m/s。
- エラー発生時は `false` を返して停止する。

| 例 | 動作 |
|----|------|
| `MoveCurve(1.0, 90)` | 半径 1.0 m の円弧を 0.3 m/s で 90° 反時計回りに走行 |
| `MoveCurve(0.5, -180, 0.2)` | 半径 0.5 m の円弧を 0.2 m/s で 180° 時計回りに走行 |
| `MoveCurve(2.0, 360, 0.5)` | 半径 2.0 m の円を 0.5 m/s で 1 周反時計回りに走行 |

---

```cpp
bool Stop(void)
```
- 走行を停止する。エラーが発生している場合は `false` を返す。

| 例 | 動作 |
|----|------|
| `Stop()` | 現在の走行を停止して速度ゼロにする |

---

```cpp
void SetMoveSpeed(int16_t xSpeed, int16_t yawSpeed)
```
- `xSpeed`：前後速度（mm/s）。プラスの値で前進、マイナスの値で後進。
- `yawSpeed`：旋回速度（mrad/s）。プラスの値で反時計回り、マイナスの値で時計回り。
- 目標距離・角度を指定せず、速度を直接指令する場合に使用する。

| 例 | 動作 |
|----|------|
| `SetMoveSpeed(300, 0)` | 0.3 m/s で直進前進 |
| `SetMoveSpeed(-200, 0)` | 0.2 m/s で直進後進 |
| `SetMoveSpeed(0, 785)` | その場で約 45 deg/s 反時計回りに旋回 |
| `SetMoveSpeed(200, 500)` | 前進しながら反時計回りに旋回（カーブ走行） |

---

#### 運動学設定

```cpp
void SetKinematics(float leftTireDiameter, float rightTireDiameter,
                   float leftGearRatio,    float rightGearRatio,
                   float tread)
```
- `leftTireDiameter`：左タイヤ直径（m）
- `rightTireDiameter`：右タイヤ直径（m）
- `leftGearRatio`：左減速比。減速している場合は 1 未満の値を指定する。
- `rightGearRatio`：右減速比。減速している場合は 1 未満の値を指定する。
- `tread`：左右タイヤ間距離（m）
- 指定した値から順運動学・逆運動学行列を計算して CRST01A に書き込む。

---

### Crst01a（上級者向け低レベル API）

> **通常の使用では触る必要はありません。**
> `CugoCommon` / `CuGoDiffDriveCtrl` で対応できない場合や、CRST01A との通信を細かく制御したい場合のみ参照してください。

#### 基本制御

| 関数 | 概要 |
|------|------|
| `SetControlMode(mode)` | RCモード / コマンドモード切替 |
| `SetEmergencyDeceleration()` | 緊急減速 |
| `ClearControllerError(err)` | 車両コントローラエラー解除 |
| `ClearDriverError(err)` | モータドライバエラー解除 |
| `ClearEncoderCount()` | エンコーダリセット |
| `SetMoveSpeed(x, y, yaw)` | 速度指令（mm/s, mrad/s） |
| `SetLights(headlight, towerlight)` | ライト制御 |
| `SaveParamReq(timeout)` | パラメータをフラッシュに保存 |

#### 定期受信データ取得

定期送信を有効にした後、次の関数でいつでも最新値を取得できます。
`recvTime` に最終受信時刻（ms）が入るため、データの鮮度確認に使用してください。

| 関数 | 取得内容 |
|------|---------|
| `GetSysStatus(...)` | コントローラステータス・エラー・電圧 |
| `GetReadRunStatus(x, y, yaw, time)` | 現在の走行速度 |
| `GetExtIo(...)` | ヘッドライト・タワーライト・外部入力 |
| `GetEncoder(enc[4], time)` | モータエンコーダ（4軸） |
| `GetMdTemp(temp[4], time)` | モータドライバ温度（4軸） |
| `GetMdStatus(err[4], time)` | モータドライバエラーコード（4軸） |
| `GetMotorOut(speed[4], torque[4], time)` | モータ角速度・トルク（4軸） |
| `GetSbus(val[16], time)` | SBUS 16 チャンネル値 |

定期送信の有効化：
```cpp
crst01a.SetCycleReq(0x80);   // システムステータス（0x80）を有効化
crst01a.SetCycleReq(0x81);   // 走行状態（0x81）を有効化
// ... 必要な電文を個別に設定
```

#### パラメータ読み出し（要求→応答）

| 関数 | 内容 |
|------|------|
| `GetDataPeriodic(...)` | 定期送信周波数・対象確認 |
| `GetMaxSpeed(x, y, yaw, timeout)` | 最大速度設定値 |
| `GetBumperBrake(bumper, brake, timeout)` | バンパー・ブレーキ設定 |
| `GetVersion(v0, v1, v2, timeout)` | ファームウェアバージョン |
| `GetVoltageConfig(min, max, timeout)` | 電圧設定値 |
| `GetMdConfig0～5(...)` | モータドライバ設定 |
| `GetRcConfig0～3(...)` | RC設定 |
| `GetFwdKinematics(data[12], timeout)` | 順運動学行列（3×4） |
| `GetInvKinematics(data[12], timeout)` | 逆運動学行列（3×4） |

---

## 電文フォーマット

```
| Byte 0 | Byte 1   | Byte 2-9  | Byte 10 | Byte 11  |
|--------|----------|-----------|---------|----------|
| 0xA0   | FuncCode | Data[0-7] | DataID  | CheckSum |
```

- `CheckSum`：Byte 0〜10 の合計値（下位 1 バイト）

---

## ファイル構成

```
crst01a_arduino_lib/
├── src/
│   ├── Crst01a.h / .cpp          # 車両コントローラ通信
│   ├── CugoCommon.h / .cpp       # 共通処理
│   └── CugoDiffDriveCtrl.h / .cpp# 差動二輪走行制御
├── examples/
│   ├── square_run/               # 正方形走行サンプル
│   └── debug/                    # デバッグ用スケッチ
└── library.properties
```

---

## ライセンス

CuboRex

---

## 関連リンク

- [CuboRex 公式サイト](https://cuborex.com)
- [GitHub リポジトリ](https://github.com/CuboRex-Development/crst01a_arduino_lib)
