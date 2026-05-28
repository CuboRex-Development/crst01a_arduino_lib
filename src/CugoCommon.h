// 共通ライブラリ
#ifndef CUGO_COMMON_H_
#define CUGO_COMMON_H_

#include "Arduino.h"
#include "hardware/sync.h"

// 各種動作モード定義
#define CUGO_RC_MODE	(0x80)	// RCモード
#define CUGO_CMD_MODE	(0x81)	// コマンドモード

// ステータスフラグ
#define CUGO_STS_CMD_MODE	(0x01)
#define CUGO_STS_BRAKE		(0x02)
#define CUGO_STS_SBUS_TIMEOUT	(0x04)
#define CUGO_STS_EMR_SW		(0x08)
#define CUGO_STS_BUMPER0	(0x10)
#define CUGO_STS_BUMPER1	(0x20)

// エラーフラグ
#define CUGO_ERR_CTL_VSYS_UV		(0x00000001)	// 基板内のシステム電源異常
#define CUGO_ERR_CTL_VBUS_OV		(0x00000002)	// 電源電圧超過
#define CUGO_ERR_CTL_VBUS_UV		(0x00000004)	// 電源電圧低下
#define CUGO_ERR_CTL_EMR_DEC		(0x00000008)	// 緊急減速実行後
#define CUGO_ERR_CTL_EMR_LATCHED	(0x00000010)	// 非常停止スイッチ押下
#define CUGO_ERR_CTL_BUMPER_LATCHED	(0x00000020)	// バンパー接触
#define CUGO_ERR_MD_OVERCURRENT		(0x00000100)	// モータドライバの電流異常
#define CUGO_ERR_MD_VOLTERROR		(0x00000200)	// モータドライバの電圧異常
#define CUGO_ERR_MD_OVERTEMP		(0x00000400)	// モータドライバの温度異常
#define CUGO_ERR_MD_OTHERERROR		(0x00000800)	// モータドライバのその他エラー
#define CUGO_ERR_MD_CAN_TIMEOUT0	(0x00001000)	// モータドライバ0との通信エラー
#define CUGO_ERR_MD_CAN_TIMEOUT1	(0x00002000)	// モータドライバ1との通信エラー
#define CUGO_ERR_MD_CAN_TIMEOUT2	(0x00004000)	// モータドライバ2との通信エラー
#define CUGO_ERR_MD_CAN_TIMEOUT3	(0x00008000)	// モータドライバ3との通信エラー
#define CUGO_ERR_CMD_TIMEOUT		(0x00010000)	// 車両コントローラとの通信エラー

#define CUGO_ERR_ALL				(0xFFFFFFFF)	// 全部のエラー
#define CUGO_ERR_CTL				(0x000000FF)	// 車両コントローラのエラー
#define CUGO_ERR_MD					(0x0000FF00)	// モータドライバのエラー

// シリアル関連
#define SERIAL_PC		Serial		// PCとの通信に使用するシリアル
#define BAUD_RATE		(115200)	// PCとのボーレート


class CugoCommon{

	public:
		CugoCommon(void);
		void Init(HardwareSerial *pSerial = &SERIAL_PC);										// 初期化関数
		void SetSwitchReset(bool switchReset);
		bool GetSwitchReset(void);
		uint32_t GetErr(void);
		void ClearErr(uint32_t err = CUGO_ERR_ALL);
		bool GetVoltage(uint16_t *driverVoltage);
		bool Wait(uint32_t time);
		void SetControlMode(uint8_t mode);
		bool GetControlMode(uint8_t *mode);
		bool SaveParamReq(void);
		bool SetVoltageConfig(uint16_t driverMinVoltage, uint16_t driverMaxVoltage);
		bool GetVoltageConfig(uint16_t *driverMinVoltage, uint16_t *driverMaxVoltage);
		bool GetVersion(uint8_t *ver0, uint8_t *ver1, uint8_t *ver2);
		
		
	private:
		HardwareSerial *l_pSerial;
		bool l_switchReset;
		uint8_t l_mode;
};

extern CugoCommon cugoCommon;

#endif // CUGO_COMMON_H_