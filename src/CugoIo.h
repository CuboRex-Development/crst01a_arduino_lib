// 入出力管理ライブラリ
#ifndef CUGO_IO_H_
#define CUGO_IO_H_

#include "Arduino.h"
#include "hardware/sync.h"

// シリアル関連
#define SERIAL_PC		Serial		// PCとの通信に使用するシリアルのデフォルト

// ヘッドライト制御ビット (SetLightのheadlightControl)
#define CUGO_HEADLIGHT0		(0x01)	// ヘッドライト0のON
#define CUGO_HEADLIGHT1		(0x02)	// ヘッドライト1のON

// タワーライト点灯モード (SetLightのtowerlightControl。各タワーライト2bitで指定する)
#define CUGO_TOWERLIGHT_OFF			(0x00)	// 消灯
#define CUGO_TOWERLIGHT_ON			(0x01)	// 点灯
#define CUGO_TOWERLIGHT_SLOW_BLINK	(0x02)	// 遅い点滅
#define CUGO_TOWERLIGHT_FAST_BLINK	(0x03)	// 早い点滅
// タワーライトのビットシフト量 (towerlightControlへ CUGO_TOWERLIGHT_x << CUGO_TOWERLIGHTn_SHIFT で指定)
#define CUGO_TOWERLIGHT0_SHIFT	(0)		// タワーライト0
#define CUGO_TOWERLIGHT1_SHIFT	(2)		// タワーライト1
#define CUGO_TOWERLIGHT2_SHIFT	(4)		// タワーライト2

class CugoIo{

	public:
		CugoIo(void);
		void Init(HardwareSerial *pSerial = &SERIAL_PC);					// 初期化関数
		bool GetInput4bit(uint8_t *in4bit1, uint8_t *in4bit2);				// 4bit入力状況の読み出し(0x84) 入力1=bit0-3, 入力2=bit4-7
		bool GetBumper(uint8_t *bumper0, uint8_t *bumper1);					// バンパー状態の読み出し(0x80)
		bool GetSbus(uint16_t *sbusVal);									// SBUS状態の読み出し(0xB0-0xB3)
		bool GetEmergencySwitch(uint8_t *emergencySwitch);					// 非常停止スイッチ状態の読み出し(0x80)
		void SetLight(uint8_t headlightControl, uint8_t towerlightControl);	// ヘッドライト、タワーライト出力設定(0x04)
		bool GetLight(uint8_t *headlightControl, uint8_t *towerlightControl);	// ヘッドライト、タワーライト出力状態の読み出し(0x84)

	private:
		HardwareSerial *l_pSerial;
};

extern CugoIo cugoIo;

#endif // CUGO_IO_H_
