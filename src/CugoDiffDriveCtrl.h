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

class CuGoDiffDriveCtrl{

	public:
		CuGoDiffDriveCtrl(void);
		void Init(HardwareSerial* pSerial = &SERIAL_PC);										// 初期化関数
		bool MoveForward(float targetDistance, float targetSpeed = DEF_SPEED_FORWARD);				// 前後進制御
		bool MoveTurn(float targetDegree, float targetSpeed = DEF_SPEED_TURN);						// 旋回制御
		bool MoveCurve(float targetRadius, float targetDegree, float targetSpeed = DEF_SPEED_CURVE);// 円軌道での移動命令
		bool Stop(void);																		// 停止命令
		void SetKinematics(float leftTireDiameter, float rightTireDiameter, float leftGearRatio, float rightGearRatio, float tread);	// 順運動学、逆運運動学設定関数
		void SetMoveSpeed(int16_t xSpeed, int16_t yawSpeed);
		
	private:
		HardwareSerial *l_pSerial;
		int16_t l_xSpeed;
		int16_t l_yawSpeed;
};

extern CuGoDiffDriveCtrl cugoDiffDriveCtrl;

#endif // CUGO_DIFFDRIVER_CTRL_H_