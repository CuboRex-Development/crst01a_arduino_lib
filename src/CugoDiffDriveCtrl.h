// 走行関連ライブラリ
#ifndef CUGO_DIFFDRIVER_CTRL_H_
#define CUGO_DIFFDRIVER_CTRL_H_

#include "Arduino.h"
#include "hardware/sync.h"

// シリアル関連
#define SERIAL_PC		Serial		// PCとの通信に使用するシリアルのデフォルト

#define DEF_SPEED_FORWARD	(0.3)	// 未設定時のデフォルトスピード(m/s)
#define DEF_SPEED_TURN		(45)	// 未設定時のデフォルトスピード(deg/s)
#define DEF_SPEED_CURVE		(0.3)	// 未設定時のデフォルトスピード(m/s)

class CugoDiffDriveCtrl{

	public:
		CugoDiffDriveCtrl(void);
		void Init(HardwareSerial* pSerial = &SERIAL_PC);										// 初期化関数
		bool MoveForward(float targetDistance, float targetSpeed = DEF_SPEED_FORWARD);				// 前後進制御
		bool MoveTurn(float targetDegree, float targetSpeed = DEF_SPEED_TURN);						// 旋回制御
		bool MoveCurve(float targetRadius, float targetDegree, float targetSpeed = DEF_SPEED_CURVE);// 円軌道での移動命令
		bool Stop(void);																		// 停止命令
		void SetKinematics(float leftTireDiameter, float rightTireDiameter, float leftGearRatio, float rightGearRatio, float tread);	// 順運動学、逆運運動学設定関数
		void SetMoveSpeed(int16_t xSpeed, int16_t yawSpeed);
		void SetMaxSpeed(uint16_t xSpeed, uint16_t yawSpeed);		// 最大速度変更
		bool GetMaxSpeed(uint16_t *pXSpeed, uint16_t *pYawSpeed);	// 最大速度取得
		bool GetEncoder(uint32_t *pRightEncoder, uint32_t *pLeftEncoder);							// エンコーダ読み出し(0x88,0x89)
		bool GetMotorTemp(uint16_t *pRightTemp, uint16_t *pLeftTemp);								// モータドライバ温度読み出し(0x8C)
		bool GetMotorError(uint16_t *pRightError, uint16_t *pLeftError);								// モータドライバ状態読み出し(0x8E)
		bool GetMotorOut(float *pRightSpeed, float *pRightTorque, float *pLeftSpeed, float *pLeftTorque);	// モータ出力読み出し(0x90-0x93)

	private:
		HardwareSerial *l_pSerial;
		int16_t l_xSpeed;
		int16_t l_yawSpeed;
};

extern CugoDiffDriveCtrl cugoDiffDriveCtrl;

#endif // CUGO_DIFFDRIVER_CTRL_H_