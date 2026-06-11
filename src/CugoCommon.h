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

// バンパー設定 (SetBumperBrakeのbumperConfig)
#define CUGO_BUMPER_STOP_DISABLE	(0x00)	// バンパー無効(接触で停止しない)
#define CUGO_BUMPER0_POLARITY		(0x01)	// バンパー0の論理設定(セットで接触検知の論理反転)
#define CUGO_BUMPER1_POLARITY		(0x02)	// バンパー1の論理設定(セットで接触検知の論理反転)
#define CUGO_BUMPER_STOP_ENABLE		(0x80)	// バンパー接触で停止する設定(セットで有効)

// ブレーキ設定 (SetBumperBrakeのbrakeConfig)
#define CUGO_AUTO_BRAKE_DISABLE		(0x00)	// 自動ブレーキ無効
#define CUGO_AUTO_BRAKE_ENABLE		(0x01)	// 自動ブレーキ設定(セットで有効)

// ランプ選択 (SetSpeedRampのspeedRampSelectで使用する値。各減速要因ごとに2bitで選択)
#define CUGO_RAMP_SELECT_A		(0x00)	// ランプ設定Aを使用
#define CUGO_RAMP_SELECT_B		(0x01)	// ランプ設定Bを使用
#define CUGO_RAMP_SELECT_C		(0x02)	// ランプ設定Cを使用
// ランプ選択のビットシフト量 (speedRampSelectへ CUGO_RAMP_SELECT_x << CUGO_RAMP_SHIFT_xxx で指定)
#define CUGO_RAMP_SHIFT_SPEED		(0)		// 速度変更時
#define CUGO_RAMP_SHIFT_EMR_DEC		(2)		// 緊急減速時
#define CUGO_RAMP_SHIFT_VOLT_ERR	(4)		// 電圧異常時
#define CUGO_RAMP_SHIFT_MD_ERR		(6)		// モータドライバエラー時
#define CUGO_RAMP_SHIFT_BUMPER		(8)		// バンパー停止時
#define CUGO_RAMP_SHIFT_EMR_SW		(10)	// 非常停止スイッチ押下時
#define CUGO_RAMP_SHIFT_OTHER_ERR	(14)	// その他エラー時

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
		bool GetVoltageConfig(uint16_t *pDriverMinVoltage, uint16_t *pDriverMaxVoltage);
		bool GetVersion(uint8_t *pVer0, uint8_t *pVer1, uint8_t *pVer2);
		void EmergencyDeceleration(void);																					// 緊急減速(0x00)
		void SetBumperBrake(uint8_t bumperConfig, uint8_t brakeConfig);														// バンパー、ブレーキ設定(0x44)
		void SetBrakeThreshold(float judgeToStopRpm);																		// ブレーキの閾値設定(0x52)
		void SetSpeedRamp(uint16_t speedRampA, uint16_t speedRampB, uint16_t speedRampC, uint16_t speedRampSelect);			// ランプ設定(0x55)
		void ClearEncoderCount(void);																						// エンコーダリセット(0x00)
		void SetRcConfig0(uint16_t rcCenterValue, uint16_t rcMinValue, uint16_t rcMaxValue, uint16_t rcCenterMargin);		// RC設定(0x58)
		void SetRcConfig1(uint16_t rcLowSwitchingThreshold, uint16_t rcHighSwitchingThreshold);								// RC設定(0x59)
		void SetRcConfig23(uint8_t movementXChannel, uint8_t movementYChannel, uint8_t movementYawChannel, uint8_t controlModeSwitchChannel, uint8_t brakeControlChannel, uint8_t errorAndBumperResetChannel, uint8_t headlight0Channel, uint8_t headlight1Channel);	// RC設定(0x5A,0x5B)
		bool GetBumperBrake(uint8_t *pBumperConfig, uint8_t *pBrakeConfig);													// バンパー、ブレーキ設定取得(0xC4)
		bool GetBrakeThreshold(float *pJudgeToStopRpm);																		// ブレーキの閾値設定取得(0xD2)
		bool GetSpeedRamp(uint16_t *pSpeedRampA, uint16_t *pSpeedRampB, uint16_t *pSpeedRampC, uint16_t *pSpeedRampSelect);	// ランプ設定取得(0xD5)
		bool GetRcConfig0(uint16_t *pRcCenterValue, uint16_t *pRcMinValue, uint16_t *pRcMaxValue, uint16_t *pRcCenterMargin);	// RC設定取得(0xD8)
		bool GetRcConfig1(uint16_t *pRcLowSwitchingThreshold, uint16_t *pRcHighSwitchingThreshold);							// RC設定取得(0xD9)
		bool GetRcConfig23(uint8_t *pMovementXChannel, uint8_t *pMovementYChannel, uint8_t *pMovementYawChannel, uint8_t *pControlModeSwitchChannel, uint8_t *pBrakeControlChannel, uint8_t *pErrorAndBumperResetChannel, uint8_t *pHeadlight0Channel, uint8_t *pHeadlight1Channel);	// RC設定取得(0xDA,0xDB)
		void LedPrint(bool err);
		
		
	private:
		HardwareSerial *l_pSerial;
		bool l_switchReset;
		uint8_t l_mode;
};

extern CugoCommon cugoCommon;

#endif // CUGO_COMMON_H_