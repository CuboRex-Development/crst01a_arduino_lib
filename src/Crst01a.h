// CRST01Aライブラリ
#ifndef CRST01A_H_
#define CRST01A_H_

#include "Arduino.h"
#include "hardware/sync.h"
#include "RPi_Pico_TimerInterrupt.h"


#define CRST_DATA_LEN		(8)		// データフィールドのバイト長
// 各種動作モード定義
#define CRST_RC_MODE	(0x80)
#define CRST_CMD_MODE	(0x81)

// --- ファンクションコード一覧 ---
// W: 上位装置 -> 車両コントローラ (書き込み)
#define CRST_FUNC_MODE_CHANGE			(0x00)	// モード変更、ブレーキON/OFF、フォールト解除、エンコーダリセット
#define CRST_FUNC_SET_MOVE_SPEED		(0x01)	// 移動速度設定
#define CRST_FUNC_SET_LIGHTS			(0x04)	// ヘッドライトとタワーライトのON/OFF設定
#define CRST_FUNC_SET_PARAM_SEND		(0x20)	// パラメータ送信設定
#define CRST_FUNC_SAVE_PARAM_REQ		(0x24)	// パラメータ保存要求
#define CRST_FUNC_SET_DATA_PERIODIC		(0x40)	// データ定期送信設定
#define CRST_FUNC_SET_MAX_SPEED 		(0x41)	// 最大速度設定
#define CRST_FUNC_SET_BUMPER_BRAKE		(0x44)	// バンパー、ブレーキ設定
#define CRST_FUNC_SET_VOLTAGE			(0x48)	// 電圧設定
#define CRST_FUNC_SET_MD_CONFIG0		(0x50)	// モータドライバ設定0 (有無)
#define CRST_FUNC_SET_MD_CONFIG1		(0x51)	// モータドライバ設定1 (最大速度, タイヤ径)
#define CRST_FUNC_SET_MD_CONFIG2		(0x52)	// モータドライバ設定2 (停止閾値)
#define CRST_FUNC_SET_MD_CONFIG3		(0x53)	// モータドライバ設定3 (トルク)
#define CRST_FUNC_SET_MD_CONFIG4		(0x54)	// モータドライバ設定4 (トルク)
#define CRST_FUNC_SET_MD_CONFIG5		(0x55)	// モータドライバ設定5 (加減速)
#define CRST_FUNC_SET_RC_CONFIG0		(0x58)	// RC設定0 (センター, 最小, 最大, 不感帯)
#define CRST_FUNC_SET_RC_CONFIG1		(0x59)	// RC設定1 (スイッチ閾値)
#define CRST_FUNC_SET_RC_CONFIG2		(0x5A)	// RC設定2 (移動チャンネル)
#define CRST_FUNC_SET_RC_CONFIG3		(0x5B)	// RC設定3 (ライトチャンネル)
#define CRST_FUNC_SET_FWD_KINEMATICS_0	(0x60)	// 順運動学行列設定 (1行目)
#define CRST_FUNC_SET_FWD_KINEMATICS_1	(0x61)	// 順運動学行列設定 (1行目)
#define CRST_FUNC_SET_FWD_KINEMATICS_2	(0x62)	// 順運動学行列設定 (2行目)
#define CRST_FUNC_SET_FWD_KINEMATICS_3	(0x63)	// 順運動学行列設定 (2行目)
#define CRST_FUNC_SET_FWD_KINEMATICS_4	(0x64)	// 順運動学行列設定 (3行目)
#define CRST_FUNC_SET_FWD_KINEMATICS_5	(0x65)	// 順運動学行列設定 (3行目)
#define CRST_FUNC_SET_INV_KINEMATICS_0	(0x70)	// 逆運動学行列設定
#define CRST_FUNC_SET_INV_KINEMATICS_1	(0x71)	// 逆運動学行列設定
#define CRST_FUNC_SET_INV_KINEMATICS_2	(0x72)	// 逆運動学行列設定
#define CRST_FUNC_SET_INV_KINEMATICS_3	(0x73)	// 逆運動学行列設定
#define CRST_FUNC_SET_INV_KINEMATICS_4	(0x74)	// 逆運動学行列設定
#define CRST_FUNC_SET_INV_KINEMATICS_5	(0x75)	// 逆運動学行列設定

// R: 車両コントローラ -> 上位装置 (読み出し/応答)
#define CRST_FUNC_SAVE_PARAM_RESP		(0x2F)	// パラメータ保存応答
#define CRST_FUNC_READ_SYS_STATUS		(0x80)	// システムステータス読み出し
#define CRST_FUNC_READ_RUN_STATUS		(0x81)	// 走行状態読み出し
#define CRST_FUNC_READ_EXT_IO			(0x84)	// 外部IO読み出し
#define CRST_FUNC_READ_ENCODER_01		(0x88)	// モータエンコーダ読み出し (0, 1)
#define CRST_FUNC_READ_ENCODER_23		(0x89)	// モータエンコーダ読み出し (2, 3)
#define CRST_FUNC_READ_MD_TEMP			(0x8C)	// モータドライバ温度読み出し
#define CRST_FUNC_READ_MD_STATUS		(0x8E)	// モータドライバ状態読み出し
#define CRST_FUNC_READ_MOTOR_OUT_0		(0x90)	// モータ出力読み出し (0)
#define CRST_FUNC_READ_MOTOR_OUT_1		(0x91)	// モータ出力読み出し (1)
#define CRST_FUNC_READ_MOTOR_OUT_2		(0x92)	// モータ出力読み出し (2)
#define CRST_FUNC_READ_MOTOR_OUT_3		(0x93)	// モータ出力読み出し (3)
#define CRST_FUNC_READ_SBUS_0			(0xB0)	// SBUS読み出し (Ch 1-4)
#define CRST_FUNC_READ_SBUS_1			(0xB1)	// SBUS読み出し (Ch 5-8)
#define CRST_FUNC_READ_SBUS_2			(0xB2)	// SBUS読み出し (Ch 9-12)
#define CRST_FUNC_READ_SBUS_3			(0xB3)	// SBUS読み出し (Ch 13-16)
#define CRST_FUNC_READ_DATA_PERIODIC	(0xC0)	// データ定期送信読み出し (0x40)
#define CRST_FUNC_READ_MAX_SPEED		(0xC1)	// 最大速度設定読み出し (0x41)
#define CRST_FUNC_READ_BUMPER_BRAKE 	(0xC4)	// バンパー、ブレーキ設定読み出し (0x44)
#define CRST_FUNC_READ_VERSION			(0xC5)	// バージョン読み出し
#define CRST_FUNC_READ_VOLTAGE			(0xC8)	// 電圧設定読み出し (0x48)
#define CRST_FUNC_READ_MD_CONFIG0		(0xD0)	// モータドライバ設定0読み出し (0x50)
#define CRST_FUNC_READ_MD_CONFIG1		(0xD1)	// モータドライバ設定1読み出し (0x51)
#define CRST_FUNC_READ_MD_CONFIG2		(0xD2)	// モータドライバ設定2読み出し (0x52)
#define CRST_FUNC_READ_MD_CONFIG3		(0xD3)	// モータドライバ設定3読み出し (0x53)
#define CRST_FUNC_READ_MD_CONFIG4		(0xD4)	// モータドライバ設定4読み出し (0x54)
#define CRST_FUNC_READ_MD_CONFIG5		(0xD5)	// モータドライバ設定5読み出し (0x55)
#define CRST_FUNC_READ_RC_CONFIG0		(0xD8)	// RC設定0読み出し (0x58)
#define CRST_FUNC_READ_RC_CONFIG1		(0xD9)	// RC設定1読み出し (0x59)
#define CRST_FUNC_READ_RC_CONFIG2		(0xDA)	// RC設定2読み出し (0x5A)
#define CRST_FUNC_READ_RC_CONFIG3		(0xDB)	// RC設定3読み出し (0x5B)
#define CRST_FUNC_READ_FWD_KINEMATICS_0	(0xE0)	// 順運動学行列読み出し (0x60)
#define CRST_FUNC_READ_FWD_KINEMATICS_1	(0xE1)	// 順運動学行列読み出し (0x61)
#define CRST_FUNC_READ_FWD_KINEMATICS_2	(0xE2)	// 順運動学行列読み出し (0x62)
#define CRST_FUNC_READ_FWD_KINEMATICS_3	(0xE3)	// 順運動学行列読み出し (0x63)
#define CRST_FUNC_READ_FWD_KINEMATICS_4	(0xE4)	// 順運動学行列読み出し (0x64)
#define CRST_FUNC_READ_FWD_KINEMATICS_5	(0xE5)	// 順運動学行列読み出し (0x65)
#define CRST_FUNC_READ_INV_KINEMATICS_0	(0xF0)	// 逆運動学行列読み出し (0x70)
#define CRST_FUNC_READ_INV_KINEMATICS_1	(0xF1)	// 逆運動学行列読み出し (0x71)
#define CRST_FUNC_READ_INV_KINEMATICS_2	(0xF2)	// 逆運動学行列読み出し (0x72)
#define CRST_FUNC_READ_INV_KINEMATICS_3	(0xF3)	// 逆運動学行列読み出し (0x73)
#define CRST_FUNC_READ_INV_KINEMATICS_4	(0xF4)	// 逆運動学行列読み出し (0x74)
#define CRST_FUNC_READ_INV_KINEMATICS_5	(0xF5)	// 逆運動学行列読み出し (0x75)

#define SEND_BUF_SIZE	20						// 送信用バッファサイズ

class Crst01a{

	public:
		Crst01a(void);
		bool Init(void);
	
		void SetControlMode(uint8_t mode);												// モード設定(0x00)
		void SetEmergencyDeceleration(void);											// 緊急減速(0x00)
		void ClearControllerError(uint8_t resetError);									// 車両コントローラエラー解除(0x00)
		void ClearDriverError(uint8_t resetError);										// モータドライバーエラー解除(0x00)
		void ClearEncoderCount(void);													// エンコーダリセット(0x00)
		void SetMoveSpeed(int16_t xSpeed, int16_t ySpeed, int16_t yawSpeed);			// 最大速度設定(0x01)
		void SetLights(uint8_t headlightControl, uint8_t towerlightControl);			// ヘッドライトとタワーライトのON/OFF設定(0x04)
		void SetReq(uint8_t id);														// パラメータ送信設定(0x20) 引数で指定したもののみを要求する
		bool SaveParamReq(uint32_t timeout = 200);										// パラメータ保存要求(0x24)
		void SetCycleReqFrequency(uint8_t frequency);									// 
		void SetCycleReq(uint8_t id);													// データ定期送信設定(0x40)(設定)
		void ClearCycleReq(uint8_t id);													// データ定期送信設定(0x40)(クリア)
		void SetSpeed(uint16_t xSpeed, uint16_t ySpeed, uint16_t yawSpeed);				// 最大速度設定(0x41)
		void SetBumperBrake(uint8_t bumperConfig, uint8_t brakeConfig);					// バンパー、ブレーキ設定(0x44)
		void SetVoltageConfig(uint16_t driverMinVoltage, uint16_t driverMaxVoltage);			// 電圧設定(0x48)
		void SetMdConfig0(uint16_t driveMotorExistFlag);								// モータドライバ設定0(0x50)
		void SetMdConfig1(float motorMaxGbSpeed);										// モータドライバ設定1(0x51)
		void SetMdConfig2(float judgeToStopRPM);										// モータドライバ設定2(0x52)
		void SetMdConfig3(float maxTorque, float startTorque);							// モータドライバ設定3(0x53)
		void SetMdConfig4(float recoveryTorque, float torqueAddRatio);					// モータドライバ設定4(0x54)
		void SetMdConfig5(uint16_t speedRampA, uint16_t speedRampB, uint16_t speedRampC, uint16_t speedRampSelect);			// モータドライバ設定5(0x55)
		void SetRcConfig0(uint16_t rcCenterValue, uint16_t rcMinValue, uint16_t rcMaxValue, uint16_t rcCenterMargin);		// RC設定0(0x58)
		void SetRcConfig1(uint16_t rcLowSwitchingThreshold, uint16_t rcHighSwitchingThreshold);								// RC設定1(0x59)
		// RC設定2,3(0x5A,0x5B)
		void SetRcConfig23(uint8_t movementXChannel, uint8_t movementYChannel, uint8_t movementYawChannel, uint8_t controlModeSwitchChannel, uint8_t brakeControlChannel, uint8_t errorAndBumperResetChannel,uint8_t headlight0Channel, uint8_t headlight1Channel);// 
		void SetFwdKinematics(float *data);												// 順運動学行列設定(0x60-0x65)
		void SetInvKinematics(float *data);												// 逆運動学行列設定(0x70-0x75)
		void GetSysStatus(uint8_t *controllerStatus, uint8_t *controllerError, uint8_t *l_motorDriverError, uint16_t *l_DriverVoltage, uint32_t *recvTime);	// システムステータス読み出し(0x80)
		void GetReadRunStatus(int16_t *xSpeed, int16_t *ySpeed, int16_t *yawSpeed, uint32_t *recvTime);					// 走行状態読み出し(0x81)
		void GetExtIo(uint8_t *headlightControl, uint8_t *towerlightControl, uint8_t *in4Bit, uint32_t *recvTime);			// 外部IO読み出し(0x84)
		void GetEncoder(uint32_t *motorEncoder, uint32_t *recvTime);					// モータエンコーダ読み出し(0x88, 0x89)
		void GetMdTemp(uint16_t *motorTemp, uint32_t *recvTime);						// モータドライバ温度読み出し (0x8C)
		void GetMdStatus(uint16_t *motorErr, uint32_t *recvTime);						// モータドライバ状態読み出し (0x8E)
		void GetMotorOut(float *motorSpeed, float *motorTorque, uint32_t *recvTime);	// モータ出力読み出し (0x90-0x93)
		void GetSbus(uint16_t *sbusVal, uint32_t *recvTime);							// SBUS読み出し (0xB0-0xB3)
		bool GetDataPeriodic(uint8_t *frequency, uint8_t *data, uint32_t timeout = 200);									// データ定期送信読み出し(0xC0)
		bool GetMaxSpeed(uint16_t *xSpeed, uint16_t *ySpeed, uint16_t *yawSpeed, uint32_t timeout = 200);					// 最大速度設定読み出し(0xC1)
		bool GetBumperBrake(uint8_t *bumperConfig, uint8_t *brakeConfig, uint32_t timeout = 200);							// バンパー、ブレーキ設定読み出し(0xC4)
		void GetBumperBrakeReq(void);																						// バンパー、ブレーキ設定読み出し(0xC4)の要求を送るだけ
		void GetBumperBrakeRes(uint8_t *bumperConfig, uint8_t *brakeConfig, uint32_t *time);								// バンパー、ブレーキ設定読み出し(0xC4)の結果を受け取るだけ
		bool GetVersion(uint8_t *ver0, uint8_t *ver1, uint8_t *ver2, uint32_t timeout = 200);								// バージョン読み出し(0xC5)
		bool GetVoltageConfig(uint16_t *minVol, uint16_t *maxVol, uint32_t timeout = 200);	// 電圧設定読み出し(0xC8)
		bool GetMdConfig0(uint16_t *existFlag, uint32_t timeout = 200);					// モータドライバ設定0読み出し(0xD0)
		bool GetMdConfig1(float *maxGbSpeed, uint32_t timeout = 200);					// モータドライバ設定1読み出し(0xD1)
		bool GetMdConfig2(float *stopRpm, uint32_t timeout = 200);						// モータドライバ設定2読み出し(0xD2)
		bool GetMdConfig3(float *maxTorque, float *startTorque, uint32_t timeout = 200);			// モータドライバ設定3読み出し(0xD3)
		bool GetMdConfig4(float *recoveryTorque, float *torqueAddRatio, uint32_t timeout = 200);	// モータドライバ設定4読み出し(0xD4)
		bool GetMdConfig5(uint16_t *rampA, uint16_t *rampB, uint16_t *rampC, uint16_t *rampSelect, uint32_t timeout = 200);	// モータドライバ設定5読み出し(0xD5)
		bool GetRcConfig0(uint16_t *center, uint16_t *min, uint16_t *max, uint16_t *margin, uint32_t timeout = 200);		// RC設定0読み出し(0xD8)
		bool GetRcConfig1(uint16_t *lowTh, uint16_t *highTh, uint32_t timeout = 200);	// RC設定1読み出し(0xD9)
		// RC設定2,3読み出し(0xDA,0xDB)
		bool GetRcConfig23(uint8_t* movementXChannel, uint8_t* movementYChannel, uint8_t* movementYawChannel, uint8_t* controlModeSwitchChannel, uint8_t* brakeControlChannel, uint8_t* errorAndBumperResetChannel,uint8_t* headlight0Channel, uint8_t* headlight1Channel, uint32_t timeout);
		bool GetRcConfig3(uint8_t *headlight0Ch, uint8_t *headlight1Ch, uint32_t timeout = 200);	// RC設定3読み出し(0xDB)
		bool GetFwdKinematics(float *data, uint32_t timeout = 200);						// 順運動学数列読み出し(0xE0-0xE5)
		bool GetInvKinematics(float *data, uint32_t timeout = 200);						// 逆運動学数列読み出し(0xF0-0xF5)

	private:
		
		// 車両コントローラ電文フォーマット
		typedef struct{
			uint8_t startByte;				// 0xA0 固定
			uint8_t funcCode;				// ファンクションコード
			uint8_t data[CRST_DATA_LEN];	// 8バイトのデータ
			uint8_t dataId;					// 電文ごと(送受信別)にインクリメント
			uint8_t checkSum;				// startByteからdataIdまでの和 (下位1バイト)
		}telegram_t;

		// 車両コントローラ電文+タイムスタンプ
		typedef struct{
			uint32_t recvTime;
			telegram_t msg;
		}telegram_time_t;

		static bool TimerHandler0(struct repeating_timer *t);
		void SetCmd(void);
		void GetCmd(void);

		RPI_PICO_Timer iTimer0;
		bool l_initFlg;
		uint16_t l_waitFunkCode;		// 受信待ちをしている電文ID。待ちが無い時は0xFFFF
		telegram_t l_reqCycleMsg;	// 定周期送信要求バッファ

		// 定期送信内容を保持する変数
		telegram_time_t l_recvSysStatus;
		telegram_time_t l_recvRunStatus;
		telegram_time_t l_recvExtIo;
		telegram_time_t l_recvEncoder01;
		telegram_time_t l_recvEncoder23;
		telegram_time_t l_recvMdtemp;
		telegram_time_t l_recvMdStatus;
		telegram_time_t l_recvMotorOut0;
		telegram_time_t l_recvMotorOut1;
		telegram_time_t l_recvMotorOut2;
		telegram_time_t l_recvMotorOut3;
		telegram_time_t l_recvSbus0;
		telegram_time_t l_recvSbus1;
		telegram_time_t l_recvSbus2;
		telegram_time_t l_recvSbus3;
		// 特例として単発応答を保持する変数
		telegram_time_t l_recvBumperBrake;
		// 運動学応答を保持する変数
		telegram_time_t l_recvFwdKinematics[6];	// 順運動学数列 (0xE0-0xE5)
		telegram_time_t l_recvInvKinematics[6];	// 逆運動学数列 (0xF0-0xF5)
		// 単発応答を保持する変数
		telegram_t l_RecvTelegram;
		// data idを保持する変数
		uint8_t l_dataId;
		// 送信電文を保持する変数
		telegram_t l_sendBuf[SEND_BUF_SIZE];	// 送信用バッファ
		uint8_t l_sendBufCount;					// 送信バッファ内のデータ数
		
		uint16_t GetWaitFunkCode(void);				// 受信待ちフラグの取得関数
		void SetWaitFunkCode(uint16_t waitFunkCode);// 受信待ちフラグの設定関数
		uint8_t CalcCheckSum(telegram_t *p);		// チェックサムの計算
		uint8_t CalcAddDataId(void);				// data idのインクリメント
		bool SendData(telegram_t *p);				// 送信用バッファにデータの格納
};

extern Crst01a crst01a;

#endif // CRST01A_H_