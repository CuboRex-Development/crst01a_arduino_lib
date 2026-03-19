// 対象ボード：Raspberry Pi Pico 2 W
#include <string.h>
#include <stdint.h>
#include "hardware/sync.h"
#include "crst01a.h"

// シリアル関連
#define SERIAL_CRST01A		Serial1		// 車両コントローラとの通信に使用するシリアル
#define BAUD_RATE	115200		// 車両コントローラとのボーレート

#define TIMER_CYCLE			(20 * 1000)		// 周期処理を行う周期を設定(us)

#define CRST_START_BYTE		(0xA0)	// CRSTの電文ヘッダー
#define CRST_PACKET_LEN		(12)	// CRSTの電文長


#define CRST_FUNC_NONE					0xFFFF	// ファンクションコード未設定時に内部処理でのみ使用

#define MOTOR_MAX	4			// 接続可能なモータ数
#define SBUS_MAX	16			// SBUSの最大チャンネル数

#define RESULT_OK	(0x01)		// パラメータ保存成功
#define RESULT_NG	(0x02)		// パラメータ保存失敗


Crst01a crst01a;


// コンストラクタ（メンバ変数の初期化）
// 引数：なし
// 戻り値：なし
Crst01a::Crst01a(void):iTimer0(0){
	
	l_initFlg = false;
	l_waitFunkCode = CRST_FUNC_NONE;
	
	l_reqCycleMsg.startByte = CRST_START_BYTE;
	l_reqCycleMsg.funcCode = CRST_FUNC_SET_DATA_PERIODIC;
	l_reqCycleMsg.data[0] = 0x02;
	l_reqCycleMsg.data[1] = 0x00;
	l_reqCycleMsg.data[2] = 0x00;
	l_reqCycleMsg.data[3] = 0x00;
	l_reqCycleMsg.data[4] = 0x00;
	l_reqCycleMsg.data[5] = 0x00;
	l_reqCycleMsg.data[6] = 0x00;
	l_reqCycleMsg.data[7] = 0x00;
	
	
	l_recvSysStatus.recvTime = 0;
	l_recvRunStatus.recvTime = 0;
	l_recvExtIo.recvTime = 0;
	l_recvEncoder01.recvTime = 0;
	l_recvEncoder23.recvTime = 0;
	l_recvMdtemp.recvTime = 0;
	l_recvMdStatus.recvTime = 0;
	l_recvMotorOut0.recvTime = 0;
	l_recvMotorOut1.recvTime = 0;
	l_recvMotorOut2.recvTime = 0;
	l_recvMotorOut3.recvTime = 0;
	l_recvSbus0.recvTime = 0;
	l_recvSbus1.recvTime = 0;
	l_recvSbus2.recvTime = 0;
	l_recvSbus3.recvTime = 0;
	for(int j = 0; j < 6; j++){
		l_recvFwdKinematics[j].recvTime = 0;
		l_recvInvKinematics[j].recvTime = 0;
	}
	
}

// 初期化関数
// 引数：なし
// 戻り値：定期割り込みの設定に失敗したら戻り値がfalse
bool Crst01a::Init(void){
	
	if(false == l_initFlg){
	
		SERIAL_CRST01A.begin(BAUD_RATE, SERIAL_8N1);		// 車両コントローラとの通信
	
		// 定期割り込み設定
		if(!(iTimer0.attachInterruptInterval(TIMER_CYCLE, TimerHandler0))){
			return false;
		}
		
		l_initFlg = true;
	}
	
	return true;
}

// モード設定(0x00)
// 引数：mode：0x80：RCモード, 0x81：コマンドモード
// 戻り値：なし
void Crst01a::SetControlMode(uint8_t mode){

	telegram_t sendTelegram;
	
	sendTelegram.startByte = CRST_START_BYTE;
	sendTelegram.funcCode = CRST_FUNC_MODE_CHANGE;	// モード変更、ブレーキON/OFF、フォールト解除、エンコーダリセット
	memset(sendTelegram.data, 0, CRST_DATA_LEN);
	sendTelegram.data[0] = mode;
	sendTelegram.checkSum = CalcCheckSum(&sendTelegram);
	SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
}

// 緊急減速(0x00)
// 引数：なし
// 戻り値：なし
void Crst01a::SetEmergencyDeceleration(void){

	telegram_t sendTelegram;
	
	sendTelegram.startByte = CRST_START_BYTE;
	sendTelegram.funcCode = CRST_FUNC_MODE_CHANGE;	// モード変更、ブレーキON/OFF、フォールト解除、エンコーダリセット
	memset(sendTelegram.data, 0, CRST_DATA_LEN);
	sendTelegram.data[1] = 0x01;
	sendTelegram.checkSum = CalcCheckSum(&sendTelegram);
	SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
}

// 車両コントローラエラー解除(0x00)
// 引数：resetError：解除するエラーのみ
// 戻り値：なし
void Crst01a::ClearControllerError(uint8_t resetError){

	telegram_t sendTelegram;
	
	sendTelegram.startByte = CRST_START_BYTE;
	sendTelegram.funcCode = CRST_FUNC_MODE_CHANGE;	// モード変更、ブレーキON/OFF、フォールト解除、エンコーダリセット
	memset(sendTelegram.data, 0, CRST_DATA_LEN);
	sendTelegram.data[2] = resetError;
	sendTelegram.checkSum = CalcCheckSum(&sendTelegram);
	SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
}

// モータドライバーエラー解除(0x00)
// 引数：resetError：解除するエラーのみ
// 戻り値：なし
void Crst01a::ClearDriverError(uint8_t resetError){

	telegram_t sendTelegram;
	
	sendTelegram.startByte = CRST_START_BYTE;
	sendTelegram.funcCode = CRST_FUNC_MODE_CHANGE;	// モード変更、ブレーキON/OFF、フォールト解除、エンコーダリセット
	memset(sendTelegram.data, 0, CRST_DATA_LEN);
	sendTelegram.data[3] = resetError;
	sendTelegram.checkSum = CalcCheckSum(&sendTelegram);
	SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
}

// エンコーダリセット(0x00)
// 引数：なし
// 戻り値：なし
void Crst01a::ClearEncoderCount(void){

	telegram_t sendTelegram;
	
	sendTelegram.startByte = CRST_START_BYTE;
	sendTelegram.funcCode = CRST_FUNC_MODE_CHANGE;	// モード変更、ブレーキON/OFF、フォールト解除、エンコーダリセット
	memset(sendTelegram.data, 0, CRST_DATA_LEN);
	sendTelegram.data[4] = 0x01;
	sendTelegram.checkSum = CalcCheckSum(&sendTelegram);
	SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
}

// 移動速度設定(0x01)
// 移送速度、旋回速度を設定する電文です。
// 引数：xSpeed：X(forward)速度 (値×0.001m/s)
// 　　　ySpeed：Y(left)速度 (値×0.001m/s)
// 　　　yawSpeed：ωθ(left rotation)速度 (値×0.001rad/s)
// 戻り値：なし
void Crst01a::SetMoveSpeed(int16_t xSpeed, int16_t ySpeed, int16_t yawSpeed){

	telegram_t sendTelegram;

	sendTelegram.startByte = CRST_START_BYTE;
	sendTelegram.funcCode = CRST_FUNC_SET_MOVE_SPEED;	// 移動速度設定
	memset(sendTelegram.data, 0, CRST_DATA_LEN);
	*(int16_t*)(&sendTelegram.data[0]) = xSpeed;
	*(int16_t*)(&sendTelegram.data[2]) = ySpeed;
	*(int16_t*)(&sendTelegram.data[4]) = yawSpeed;
	sendTelegram.checkSum = CalcCheckSum(&sendTelegram);
	SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
}

// ヘッドライトとタワーライトのON/OFF設定(0x04)
// ヘッドライトとタワーライトのON/OFFを設定する電文です。
// 引数：headlightControl：bit0：ヘッドライト0のON(1)/OFF(0) bit1：ヘッドライト1のON/OFF
// 　　　towerlightControl：bit0-1：タワーライト0の点灯設定(0：消灯、1：点灯、2：遅い点滅、3：早い点滅)
//                          bit2-3：タワーライト1の点灯設定
//                          bit4-5：タワーライト2の点灯設定
// 戻り値：なし
void Crst01a::SetLights(uint8_t headlightControl, uint8_t towerlightControl){

	telegram_t sendTelegram;
	
	sendTelegram.startByte = CRST_START_BYTE;
	sendTelegram.funcCode = CRST_FUNC_SET_LIGHTS;	// ヘッドライトとタワーライトのON/OFF設定
	memset(sendTelegram.data, 0, CRST_DATA_LEN);
	sendTelegram.data[0] = headlightControl;
	sendTelegram.data[1] = towerlightControl;
	sendTelegram.checkSum = CalcCheckSum(&sendTelegram);
	SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
}


// パラメータ送信設定(0x20)
// 各電文の定期送信の切り替え送信要求を行う電文です。
// 引数で指定したもののみを要求します。
// 引数：id：要求するファンクションコード
// 戻り値：なし
void Crst01a::SetReq(uint8_t id){
	telegram_t reqMsg = {CRST_START_BYTE, CRST_FUNC_SET_PARAM_SEND, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	switch(id){
		case CRST_FUNC_READ_DATA_PERIODIC:
			reqMsg.data[0] |= 0x01;
			break;
		case CRST_FUNC_READ_MAX_SPEED:
			reqMsg.data[0] |= 0x02;
			break;
		case CRST_FUNC_READ_BUMPER_BRAKE:
			reqMsg.data[0] |= 0x10;
			break;
		case CRST_FUNC_READ_VERSION:
			reqMsg.data[0] |= 0x20;
			break;
		case CRST_FUNC_READ_VOLTAGE:
			reqMsg.data[1] |= 0x01;
			break;
		case CRST_FUNC_READ_MD_CONFIG0:
			reqMsg.data[2] |= 0x01;
			break;
		case CRST_FUNC_READ_MD_CONFIG1:
			reqMsg.data[2] |= 0x02;
			break;
		case CRST_FUNC_READ_MD_CONFIG2:
			reqMsg.data[2] |= 0x04;
			break;
		case CRST_FUNC_READ_MD_CONFIG3:
			reqMsg.data[2] |= 0x08;
			break;
		case CRST_FUNC_READ_MD_CONFIG4:
			reqMsg.data[2] |= 0x10;
			break;
		case CRST_FUNC_READ_MD_CONFIG5:
			reqMsg.data[2] |= 0x20;
			break;
		case CRST_FUNC_READ_RC_CONFIG0:
			reqMsg.data[3] |= 0x01;
			break;
		case CRST_FUNC_READ_RC_CONFIG1:
			reqMsg.data[3] |= 0x02;
			break;
		case CRST_FUNC_READ_RC_CONFIG2:
			reqMsg.data[3] |= 0x04;
			break;
		case CRST_FUNC_READ_RC_CONFIG3:
			reqMsg.data[3] |= 0x08;
			break;
		case CRST_FUNC_READ_FWD_KINEMATICS_0:
			reqMsg.data[4] |= 0x01;
			break;
		case CRST_FUNC_READ_FWD_KINEMATICS_1:
			reqMsg.data[4] |= 0x02;
			break;
		case CRST_FUNC_READ_FWD_KINEMATICS_2:
			reqMsg.data[4] |= 0x04;
			break;
		case CRST_FUNC_READ_FWD_KINEMATICS_3:
			reqMsg.data[4] |= 0x08;
			break;
		case CRST_FUNC_READ_FWD_KINEMATICS_4:
			reqMsg.data[4] |= 0x10;
			break;
		case CRST_FUNC_READ_FWD_KINEMATICS_5:
			reqMsg.data[4] |= 0x20;
			break;
		case CRST_FUNC_READ_INV_KINEMATICS_0:
			reqMsg.data[6] |= 0x01;
			break;
		case CRST_FUNC_READ_INV_KINEMATICS_1:
			reqMsg.data[6] |= 0x02;
			break;
		case CRST_FUNC_READ_INV_KINEMATICS_2:
			reqMsg.data[6] |= 0x04;
			break;
		case CRST_FUNC_READ_INV_KINEMATICS_3:
			reqMsg.data[6] |= 0x08;
			break;
		case CRST_FUNC_READ_INV_KINEMATICS_4:
			reqMsg.data[6] |= 0x10;
			break;
		case CRST_FUNC_READ_INV_KINEMATICS_5:
			reqMsg.data[6] |= 0x20;
			break;
		default:
			break;
	}
	
	reqMsg.checkSum = CalcCheckSum(&reqMsg);			// チェックサムをセット
	SERIAL_CRST01A.write((uint8_t*)&reqMsg, CRST_PACKET_LEN);	// 送信
}

// パラメータ保存要求(0x24)
// 本体のパラメータをフラッシュメモリに保存する電文です。
// 引数：timeout：タイムアウト時間 (ms)
// 戻り値：成功時 true、失敗・タイムアウト時 false
bool Crst01a::SaveParamReq(uint32_t timeout){

	uint32_t now;
	bool ret = true;
	telegram_t sendTelegram;
	uint32_t irq_state;

	sendTelegram.startByte = CRST_START_BYTE;
	sendTelegram.funcCode = CRST_FUNC_SAVE_PARAM_REQ;	// 
	memset(sendTelegram.data, 0, CRST_DATA_LEN);
	sendTelegram.checkSum = CalcCheckSum(&sendTelegram);		// チェックサムをセット
	SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
	
	SetWaitFunkCode(CRST_FUNC_SAVE_PARAM_RESP);	// 応答待ち
	now = millis();
	
	while(1){
		
		if(CRST_FUNC_NONE == GetWaitFunkCode()){
			// 電文を受信した
			irq_state = save_and_disable_interrupts();	// 割り込み禁止
			if(RESULT_OK == l_RecvTelegram.data[0]){
				ret = true;
			}
			else{
				ret = false;
			}
			restore_interrupts(irq_state);				// 割り込み許可
			return ret;
		}
		
		if(millis() > (now + timeout)){
			return false;		// タイムアウト
		}
	}
}

// データ定期送信設定(0x40)(設定)
// 送信周期の設定と、各電文の定期送信の切り替えを行う電文です。
// 引数：id：定期送信を有効にするファンクションコード
// 戻り値：なし
void Crst01a::SetCycleReq(uint8_t id){
	switch(id){
		case CRST_FUNC_READ_SYS_STATUS:
			l_reqCycleMsg.data[1] |= 0x01;
			break;
		case CRST_FUNC_READ_RUN_STATUS:
			l_reqCycleMsg.data[1] |= 0x02;
			break;
		case CRST_FUNC_READ_EXT_IO:
			l_reqCycleMsg.data[1] |= 0x10;
			break;
		case CRST_FUNC_READ_SBUS_0:
			l_reqCycleMsg.data[3] |= 0x01;
			break;
		case CRST_FUNC_READ_SBUS_1:
			l_reqCycleMsg.data[3] |= 0x02;
			break;
		case CRST_FUNC_READ_SBUS_2:
			l_reqCycleMsg.data[3] |= 0x04;
			break;
		case CRST_FUNC_READ_SBUS_3:
			l_reqCycleMsg.data[3] |= 0x08;
			break;
		case CRST_FUNC_READ_ENCODER_01:
			l_reqCycleMsg.data[4] |= 0x01;
			break;
		case CRST_FUNC_READ_ENCODER_23:
			l_reqCycleMsg.data[4] |= 0x02;
			break;
		case CRST_FUNC_READ_MD_TEMP:
			l_reqCycleMsg.data[4] |= 0x10;
			break;
		case CRST_FUNC_READ_MD_STATUS:
			l_reqCycleMsg.data[4] |= 0x40;
			break;
		case CRST_FUNC_READ_MOTOR_OUT_0:
			l_reqCycleMsg.data[5] |= 0x01;
			break;
		case CRST_FUNC_READ_MOTOR_OUT_1:
			l_reqCycleMsg.data[5] |= 0x02;
			break;
		case CRST_FUNC_READ_MOTOR_OUT_2:
			l_reqCycleMsg.data[5] |= 0x04;
			break;
		case CRST_FUNC_READ_MOTOR_OUT_3:
			l_reqCycleMsg.data[5] |= 0x08;
			break;
		default:
			break;
	}
	
	l_reqCycleMsg.checkSum = CalcCheckSum(&l_reqCycleMsg);			// チェックサムをセット
	SERIAL_CRST01A.write((uint8_t*)&l_reqCycleMsg, CRST_PACKET_LEN);	// 送信
}

// データ定期送信設定(0x40)(クリア)
// 送信周期の設定と、各電文の定期送信の解除を行う電文です。
// 引数：id：定期送信を無効にするファンクションコード
// 戻り値：なし
void Crst01a::ClearCycleReq(uint8_t id){
	switch(id){
		case CRST_FUNC_READ_SYS_STATUS:
			l_reqCycleMsg.data[1] &= ~(0x01);
			break;
		case CRST_FUNC_READ_RUN_STATUS:
			l_reqCycleMsg.data[1] &= ~(0x02);
			break;
		case CRST_FUNC_READ_EXT_IO:
			l_reqCycleMsg.data[1] &= ~(0x10);
			break;
		case CRST_FUNC_READ_SBUS_0:
			l_reqCycleMsg.data[3] &= ~(0x01);
			break;
		case CRST_FUNC_READ_SBUS_1:
			l_reqCycleMsg.data[3] &= ~(0x02);
			break;
		case CRST_FUNC_READ_SBUS_2:
			l_reqCycleMsg.data[3] &= ~(0x04);
			break;
		case CRST_FUNC_READ_SBUS_3:
			l_reqCycleMsg.data[3] &= ~(0x08);
			break;
		case CRST_FUNC_READ_ENCODER_01:
			l_reqCycleMsg.data[4] &= ~(0x01);
			break;
		case CRST_FUNC_READ_ENCODER_23:
			l_reqCycleMsg.data[4] &= ~(0x02);
			break;
		case CRST_FUNC_READ_MD_TEMP:
			l_reqCycleMsg.data[4] &= ~(0x10);
			break;
		case CRST_FUNC_READ_MD_STATUS:
			l_reqCycleMsg.data[4] &= ~(0x40);
			break;
		case CRST_FUNC_READ_MOTOR_OUT_0:
			l_reqCycleMsg.data[5] &= ~(0x01);
			break;
		case CRST_FUNC_READ_MOTOR_OUT_1:
			l_reqCycleMsg.data[5] &= ~(0x02);
			break;
		case CRST_FUNC_READ_MOTOR_OUT_2:
			l_reqCycleMsg.data[5] &= ~(0x04);
			break;
		case CRST_FUNC_READ_MOTOR_OUT_3:
			l_reqCycleMsg.data[5] &= ~(0x08);
			break;
		default:
			break;
	}
	
	l_reqCycleMsg.checkSum = CalcCheckSum(&l_reqCycleMsg);			// チェックサムをセット
	SERIAL_CRST01A.write((uint8_t*)&l_reqCycleMsg, CRST_PACKET_LEN);	// 送信
}

// 最大速度設定(0x41)
// 車体の最大速度を設定する電文です。
// 引数：xSpeed：X方向の最大速度 (値×0.001m/s)
// 　　　ySpeed：Y方向の最大速度 (値×0.001m/s)
// 　　　yawSpeed：最大旋回速度 (値×0.001rad/s)
// 戻り値：なし
void Crst01a::SetSpeed(uint16_t xSpeed, uint16_t ySpeed, uint16_t yawSpeed){
	
	telegram_t sendTelegram;
	
	sendTelegram.startByte = CRST_START_BYTE;
	sendTelegram.funcCode = CRST_FUNC_SET_MAX_SPEED;	// 最大速度設定
	memset(sendTelegram.data, 0, CRST_DATA_LEN);
	*(uint16_t*)(&sendTelegram.data[0]) = xSpeed;
	*(uint16_t*)(&sendTelegram.data[2]) = ySpeed;
	*(uint16_t*)(&sendTelegram.data[4]) = yawSpeed;
	sendTelegram.checkSum = CalcCheckSum(&sendTelegram);
	SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
}

// バンパー、ブレーキ設定(0x44)
// バンパーとブレーキの動作設定する電文です。
// 引数：bumperConfig：バンパーの設定 (bit0-1:論理設定, bit7:停止有効設定)
// 　　　brakeConfig：自動ブレーキの設定 (bit0:自動ブレーキ有効設定)
// 戻り値：なし
void Crst01a::SetBumperBrake(uint8_t bumperConfig, uint8_t brakeConfig){
	
	telegram_t sendTelegram;
	
	sendTelegram.startByte = CRST_START_BYTE;
	sendTelegram.funcCode = CRST_FUNC_SET_BUMPER_BRAKE;
	memset(sendTelegram.data, 0, CRST_DATA_LEN);
	sendTelegram.data[0] = bumperConfig;
	sendTelegram.data[1] = brakeConfig;
	sendTelegram.checkSum = CalcCheckSum(&sendTelegram);
	SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
}

// 電圧設定(0x48)
// 車両コントローラが電圧異常と検知する電圧の上下限を設定する電文です。
// 引数：driverMinVoltage：電圧異常の下限値 (値×0.1V)
// 　　　driverMaxVoltage：電圧異常の上限値 (値×0.1V)
// 戻り値：なし
void Crst01a::SetVoltage(uint16_t driverMinVoltage, uint16_t driverMaxVoltage){

	telegram_t sendTelegram;

	sendTelegram.startByte = CRST_START_BYTE;
	sendTelegram.funcCode = CRST_FUNC_SET_VOLTAGE;
	memset(sendTelegram.data, 0, CRST_DATA_LEN);
	*(uint16_t*)(&sendTelegram.data[0]) = driverMinVoltage;
	*(uint16_t*)(&sendTelegram.data[2]) = driverMaxVoltage;
	sendTelegram.checkSum = CalcCheckSum(&sendTelegram);
	SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
}

// モータドライバ設定0(0x50)
// 接続されているモータドライバの有無を設定する電文です。
// 引数：driveMotorExistFlag：モータドライバの接続有無。bit0:モータドライバ0有無、…bit3:モータドライバ3有無
// 戻り値：なし
void Crst01a::SetMdConfig0(uint16_t driveMotorExistFlag){
	
	telegram_t sendTelegram;
	
	sendTelegram.startByte = CRST_START_BYTE;
	sendTelegram.funcCode = CRST_FUNC_SET_MD_CONFIG0;
	memset(sendTelegram.data, 0, CRST_DATA_LEN);
	*(uint16_t*)(&sendTelegram.data[0]) = driveMotorExistFlag;
	sendTelegram.checkSum = CalcCheckSum(&sendTelegram);
	SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
}

// モータドライバ設定1(0x51)
// ギアボックス込みのモータ最大回転数を設定する電文です。
// 引数：motorMaxGbSpeed：モータ最大回転数 (rpm)
// 戻り値：なし
void Crst01a::SetMdConfig1(float motorMaxGbSpeed){

	telegram_t sendTelegram;

	sendTelegram.startByte = CRST_START_BYTE;
	sendTelegram.funcCode = CRST_FUNC_SET_MD_CONFIG1;
	memset(sendTelegram.data, 0, CRST_DATA_LEN);
	*(float*)(&sendTelegram.data[0]) = motorMaxGbSpeed;
	sendTelegram.checkSum = CalcCheckSum(&sendTelegram);
	SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
}

// モータドライバ設定2(0x52)
// 自動ブレーキ有効時のブレーキを掛ける閾値等を設定する電文です。
// 引数：judgeToStopRPM：停止判断の閾値 (rpm)
// 戻り値：なし
void Crst01a::SetMdConfig2(float judgeToStopRPM){

	telegram_t sendTelegram;

	sendTelegram.startByte = CRST_START_BYTE;
	sendTelegram.funcCode = CRST_FUNC_SET_MD_CONFIG2;
	memset(sendTelegram.data, 0, CRST_DATA_LEN);
	*(float*)(&sendTelegram.data[0]) = judgeToStopRPM;
	sendTelegram.checkSum = CalcCheckSum(&sendTelegram);
	SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
}

// モータドライバ設定3(0x53)
// モータドライバへ指示する最大トルクを設定する電文です。
// 引数：maxTorque：最大トルク (A)
// 　　　startTorque：8割到達までの最大トルク (A)
// 戻り値：なし
void Crst01a::SetMdConfig3(float maxTorque, float startTorque){

	telegram_t sendTelegram;

	sendTelegram.startByte = CRST_START_BYTE;
	sendTelegram.funcCode = CRST_FUNC_SET_MD_CONFIG3;
	memset(sendTelegram.data, 0, CRST_DATA_LEN);
	*(float*)(&sendTelegram.data[0]) = maxTorque;
	*(float*)(&sendTelegram.data[4]) = startTorque;
	sendTelegram.checkSum = CalcCheckSum(&sendTelegram);
	SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
}

// モータドライバ設定4(0x54)
// 停止指示中の最大トルク量やトルクの増減量を設定する電文です。
// 引数：recoveryTorque：実際に止まるまでの最大トルク量 (A)
// 　　　torqueAddRatio：50Hz周期で1回に上げ下げするトルク量 (A)
// 戻り値：なし
void Crst01a::SetMdConfig4(float recoveryTorque, float torqueAddRatio){

	telegram_t sendTelegram;

	sendTelegram.startByte = CRST_START_BYTE;
	sendTelegram.funcCode = CRST_FUNC_SET_MD_CONFIG4;
	memset(sendTelegram.data, 0, CRST_DATA_LEN);
	*(float*)(&sendTelegram.data[0]) = recoveryTorque;
	*(float*)(&sendTelegram.data[4]) = torqueAddRatio;
	sendTelegram.checkSum = CalcCheckSum(&sendTelegram);
	SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
}

// モータドライバ設定5(0x55)
// それぞれの減速要因に応じたランプ設定を設定する電文です。
// 引数：speedRampA：ランプ設定A (RPM/s)
// 　　　speedRampB：ランプ設定B (RPM/s)
// 　　　speedRampC：ランプ設定C (RPM/s)
// 　　　speedRampSelect：各要因でどのランプを使用するかの選択
//                        bit0-1：速度変更時の設定
//                        bit2-3：緊急減速時の設定
//                        bit4-5：電圧異常時の設定
//                        bit6-7：モータドライバエラー時の設定
//                        bit8-9：バンパー停止時の設定
//                        bit10-11：非常停止スイッチ押下時の設定
//                        bit14-15：その他エラー
// 戻り値：なし
void Crst01a::SetMdConfig5(uint16_t speedRampA, uint16_t speedRampB, uint16_t speedRampC, uint16_t speedRampSelect){

	telegram_t sendTelegram;

	sendTelegram.startByte = CRST_START_BYTE;
	sendTelegram.funcCode = CRST_FUNC_SET_MD_CONFIG5;
	memset(sendTelegram.data, 0, CRST_DATA_LEN);
	*(uint16_t*)(&sendTelegram.data[0]) = speedRampA;
	*(uint16_t*)(&sendTelegram.data[2]) = speedRampB;
	*(uint16_t*)(&sendTelegram.data[4]) = speedRampC;
	*(uint16_t*)(&sendTelegram.data[6]) = speedRampSelect;
	sendTelegram.checkSum = CalcCheckSum(&sendTelegram);
	SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
}

// RC設定0(0x58)
// SBUS信号のセンター、最小、最大値、不感帯を設定する電文です。
// 引数：rcCenterValue：SBUS中央値
// 　　　rcMinValue：SBUS最小値
// 　　　rcMaxValue：SBUS最大値
// 　　　rcCenterMargin：不感帯幅(例:10の場合は+-10の範囲が不感帯)
// 戻り値：なし
void Crst01a::SetRcConfig0(uint16_t rcCenterValue, uint16_t rcMinValue, uint16_t rcMaxValue, uint16_t rcCenterMargin){

	telegram_t sendTelegram;

	sendTelegram.startByte = CRST_START_BYTE;
	sendTelegram.funcCode = CRST_FUNC_SET_RC_CONFIG0;
	memset(sendTelegram.data, 0, CRST_DATA_LEN);
	*(uint16_t*)(&sendTelegram.data[0]) = rcCenterValue;
	*(uint16_t*)(&sendTelegram.data[2]) = rcMinValue;
	*(uint16_t*)(&sendTelegram.data[4]) = rcMaxValue;
	*(uint16_t*)(&sendTelegram.data[6]) = rcCenterMargin;
	sendTelegram.checkSum = CalcCheckSum(&sendTelegram);
	SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
}

// RC設定1(0x59)
// SBUS信号のスイッチON/OFF閾値を設定する電文です。
// 引数：rcLowSwitchingThreshold：スイッチOFFとする閾値
// 　　　rcHighSwitchingThreshold：スイッチONとする閾値
// 戻り値：なし
void Crst01a::SetRcConfig1(uint16_t rcLowSwitchingThreshold, uint16_t rcHighSwitchingThreshold){

	telegram_t sendTelegram;

	sendTelegram.startByte = CRST_START_BYTE;
	sendTelegram.funcCode = CRST_FUNC_SET_RC_CONFIG1;
	memset(sendTelegram.data, 0, CRST_DATA_LEN);
	*(uint16_t*)(&sendTelegram.data[4]) = rcLowSwitchingThreshold;
	*(uint16_t*)(&sendTelegram.data[6]) = rcHighSwitchingThreshold;
	sendTelegram.checkSum = CalcCheckSum(&sendTelegram);
	SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
}

// RC設定2,3(0x5A,0x5B)
// 各種移動やモード切替等に使用するSBUSチャンネルを設定する電文です。
// 引数：movementXChannel：X方向移動のチャンネル (0-15)
// 　　　movementYChannel：Y方向移動のチャンネル (0-15)
// 　　　movementYawChannel：旋回方向移動のチャンネル (0-15)
// 　　　controlModeSwitchChannel：モード切替のチャンネル (0-15)
// 　　　brakeControlChannel：自動ブレーキ制御のチャンネル (0-15)
// 　　　errorAndBumperResetChannel：エラー解除のチャンネル (0-15)
// 　　　headlight0Channel：ヘッドライト0制御のチャンネル (0-15)
// 　　　headlight1Channel：ヘッドライト1制御のチャンネル (0-15)
// 戻り値：なし
void Crst01a::SetRcConfig23(uint8_t movementXChannel, uint8_t movementYChannel, uint8_t movementYawChannel, uint8_t controlModeSwitchChannel, uint8_t brakeControlChannel, uint8_t errorAndBumperResetChannel,uint8_t headlight0Channel, uint8_t headlight1Channel){

	telegram_t sendTelegram;

	sendTelegram.startByte = CRST_START_BYTE;
	sendTelegram.funcCode = CRST_FUNC_SET_RC_CONFIG2;
	memset(sendTelegram.data, 0, CRST_DATA_LEN);
	sendTelegram.data[0] = movementXChannel;
	sendTelegram.data[1] = movementYChannel;
	sendTelegram.data[2] = movementYawChannel;
	sendTelegram.data[4] = controlModeSwitchChannel;
	sendTelegram.data[5] = brakeControlChannel;
	sendTelegram.data[6] = errorAndBumperResetChannel;
	sendTelegram.checkSum = CalcCheckSum(&sendTelegram);
	SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
	
	sendTelegram.startByte = CRST_START_BYTE;
	sendTelegram.funcCode = CRST_FUNC_SET_RC_CONFIG3;
	memset(sendTelegram.data, 0, CRST_DATA_LEN);
	sendTelegram.data[0] = headlight0Channel;
	sendTelegram.data[1] = headlight1Channel;
	sendTelegram.checkSum = CalcCheckSum(&sendTelegram);
	SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
}

// 順運動学行列設定(0x60-0x65)
// 順運動学行列の設定を行う電文です。
// 引数：data：行列内の要素(3行4列)を格納したポインタ (12要素)
// 戻り値：なし
void Crst01a::SetFwdKinematics(float *data){

	telegram_t sendTelegram;
	int i;

	sendTelegram.startByte = CRST_START_BYTE;
	
	for(i = 0;i < 6;i++){
		sendTelegram.funcCode = CRST_FUNC_SET_FWD_KINEMATICS_0 + i;
		*(float*)(&sendTelegram.data[0]) = data[0+i*2];
		*(float*)(&sendTelegram.data[4]) = data[1+i*2];
		sendTelegram.checkSum = CalcCheckSum(&sendTelegram);
		SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
	}
}


// 逆運動学行列設定(0x70-0x75)
// 逆運動学行列の設定を行う電文です。
// 引数：data：行列内の要素(4行3列)を格納したポインタ (12要素)
// 戻り値：なし
void Crst01a::SetInvKinematics(float *data){
	
	telegram_t sendTelegram;
	int i;
	
	sendTelegram.startByte = CRST_START_BYTE;
	
	for(i = 0;i < 6;i++){
		sendTelegram.funcCode = CRST_FUNC_SET_INV_KINEMATICS_0 + i;
		*(float*)(&sendTelegram.data[0]) = data[0+i*2];
		*(float*)(&sendTelegram.data[4]) = data[1+i*2];
		sendTelegram.checkSum = CalcCheckSum(&sendTelegram);
		SERIAL_CRST01A.write((uint8_t*)&sendTelegram, CRST_PACKET_LEN);	// 送信
	}
}

// システムステータス読み出し(0x80)
// 車両コントローラのステータスを取得します。
// 引数：controllerStatus：コントローラのステータス格納先
// 　　　controllerError：エラー状態の格納先
// 　　　motorDriverError：モータドライバエラー状態の格納先
// 　　　DriverVoltage：現在の電源電圧 (0.1V単位)
// 　　　recvTime：受信時刻(ms)
// 戻り値：なし
void Crst01a::GetSysStatus(uint8_t *controllerStatus, uint8_t *controllerError, uint8_t *motorDriverError, uint16_t *DriverVoltage, uint32_t *recvTime){

	*controllerStatus = l_recvSysStatus.msg.data[0];
	*controllerError = l_recvSysStatus.msg.data[1];
	*motorDriverError = l_recvSysStatus.msg.data[2];
	*DriverVoltage = *(uint16_t*)(&l_recvSysStatus.msg.data[6]);
	*recvTime = l_recvSysStatus.recvTime;
}

// 走行状態読み出し(0x81)
// 現在の走行速度(X, Y, Yaw)を取得します。
// 引数：xSpeed：X方向速度格納先
// 　　　ySpeed：Y方向速度格納先
// 　　　yawSpeed：旋回速度格納先
// 　　　recvTime：受信時刻(ms)
// 戻り値：なし
void Crst01a::GetReadRunStatus(int16_t *xSpeed, int16_t *ySpeed, int16_t *yawSpeed, uint32_t *recvTime){

	*xSpeed = *(int16_t*)(&l_recvRunStatus.msg.data[0]);
	*ySpeed = *(int16_t*)(&l_recvRunStatus.msg.data[2]);
	*yawSpeed = *(int16_t*)(&l_recvRunStatus.msg.data[4]);
	*recvTime = l_recvRunStatus.recvTime;
}

// 外部IO読み出し(0x84)
// ライトの状態および4bit入力の状態を取得します。
// 引数：headlightControl：ヘッドライト状態格納先
// 　　　towerlightControl：タワーライト状態格納先
// 　　　in4Bit：4bit入力状態格納先
// 　　　recvTime：受信時刻(ms)
// 戻り値：なし
void Crst01a::GetExtIo(uint8_t *headlightControl, uint8_t *towerlightControl, uint8_t *in4Bit, uint32_t *recvTime){

	*headlightControl = l_recvExtIo.msg.data[0];
	*towerlightControl = l_recvExtIo.msg.data[0];
	*in4Bit = l_recvExtIo.msg.data[0];
	*recvTime = l_recvExtIo.recvTime;
}

// モータエンコーダ読み出し(0x88, 0x89)
// モータ0～3のエンコーダカウント値を取得します。
// 引数：motorEncoder：4つのエンコーダ値を格納する配列
// 　　　recvTime：受信時刻(ms)
// 戻り値：なし
void Crst01a::GetEncoder(uint32_t *motorEncoder, uint32_t *recvTime){

	uint32_t now = millis();

	motorEncoder[0] = *(uint32_t*)(&l_recvEncoder01.msg.data[0]);
	motorEncoder[1] = *(uint32_t*)(&l_recvEncoder01.msg.data[4]);
	motorEncoder[2] = *(uint32_t*)(&l_recvEncoder23.msg.data[0]);
	motorEncoder[3] = *(uint32_t*)(&l_recvEncoder23.msg.data[4]);
	
	// 一番遅いタイムスタンプを返す
	*recvTime = l_recvEncoder01.recvTime;
	if((now - *recvTime) < (now - l_recvEncoder23.recvTime)){
		*recvTime = l_recvEncoder23.recvTime;
	}
}

// モータドライバ温度読み出し (0x8C)
// モータドライバ0～3の温度を取得します。
// 引数：motorTemp：4つの温度値を格納する配列 (℃)
// 　　　recvTime：受信時刻(ms)
// 戻り値：なし
void Crst01a::GetMdTemp(uint16_t *motorTemp, uint32_t *recvTime){

	motorTemp[0] = *(uint16_t*)(&l_recvMdtemp.msg.data[0]);
	motorTemp[1] = *(uint16_t*)(&l_recvMdtemp.msg.data[2]);
	motorTemp[2] = *(uint16_t*)(&l_recvMdtemp.msg.data[4]);
	motorTemp[3] = *(uint16_t*)(&l_recvMdtemp.msg.data[6]);
	*recvTime = l_recvMdtemp.recvTime;
}

// モータドライバ状態読み出し (0x8E)
// モータドライバから受信したエラーコードを取得します。
// 引数：motorErr：4つのエラーコードを格納する配列
// 　　　recvTime：受信時刻(ms)
// 戻り値：なし
void Crst01a::GetMdStatus(uint16_t *motorErr, uint32_t *recvTime){

	motorErr[0] = *(uint16_t*)(&l_recvMdStatus.msg.data[0]);
	motorErr[1] = *(uint16_t*)(&l_recvMdStatus.msg.data[2]);
	motorErr[2] = *(uint16_t*)(&l_recvMdStatus.msg.data[4]);
	motorErr[3] = *(uint16_t*)(&l_recvMdStatus.msg.data[6]);
	*recvTime = l_recvMdStatus.recvTime;
}

// モータ出力読み出し (0x90-0x93)
// モータ0～3の角速度とトルクを取得します。
// 引数：motorSpeed：4つの角速度(rpm)を格納する配列
// 　　　motorTorque：4つのトルク(A)を格納する配列
// 　　　recvTime：受信時刻(ms)
// 戻り値：なし
void Crst01a::GetMotorOut(float *motorSpeed, float *motorTorque, uint32_t *recvTime){

	uint32_t now = millis();

	motorSpeed[0] = *(float*)(&l_recvMotorOut0.msg.data[0]);
	motorTorque[0] = *(float*)(&l_recvMotorOut0.msg.data[4]);
	motorSpeed[1] = *(float*)(&l_recvMotorOut1.msg.data[0]);
	motorTorque[1] = *(float*)(&l_recvMotorOut1.msg.data[4]);
	motorSpeed[2] = *(float*)(&l_recvMotorOut2.msg.data[0]);
	motorTorque[2] = *(float*)(&l_recvMotorOut2.msg.data[4]);
	motorSpeed[3] = *(float*)(&l_recvMotorOut3.msg.data[0]);
	motorTorque[3] = *(float*)(&l_recvMotorOut3.msg.data[4]);
	
	// 一番遅いタイムスタンプを返す
	*recvTime = l_recvMotorOut0.recvTime;
	if((now - *recvTime) < (now - l_recvMotorOut1.recvTime)){
		*recvTime = l_recvMotorOut1.recvTime;
	}
	if((now - *recvTime) < (now - l_recvMotorOut2.recvTime)){
		*recvTime = l_recvMotorOut2.recvTime;
	}
	if((now - *recvTime) < (now - l_recvMotorOut3.recvTime)){
		*recvTime = l_recvMotorOut3.recvTime;
	}
}

// SBUS読み出し (0xB0-0xB3)
// SBUS 16チャンネル分の信号値を取得します。
// 引数：sbusVal：16チャンネルの信号値を格納する配列
// 　　　recvTime：受信時刻(ms)
// 戻り値：なし
void Crst01a::GetSbus(uint16_t *sbusVal, uint32_t *recvTime){

	uint32_t now = millis();

	sbusVal[0] = *(uint16_t*)(&l_recvSbus0.msg.data[0]);
	sbusVal[1] = *(uint16_t*)(&l_recvSbus0.msg.data[2]);
	sbusVal[2] = *(uint16_t*)(&l_recvSbus0.msg.data[4]);
	sbusVal[3] = *(uint16_t*)(&l_recvSbus0.msg.data[6]);
	sbusVal[4] = *(uint16_t*)(&l_recvSbus1.msg.data[0]);
	sbusVal[5] = *(uint16_t*)(&l_recvSbus1.msg.data[2]);
	sbusVal[6] = *(uint16_t*)(&l_recvSbus1.msg.data[4]);
	sbusVal[7] = *(uint16_t*)(&l_recvSbus1.msg.data[6]);
	sbusVal[8] = *(uint16_t*)(&l_recvSbus2.msg.data[0]);
	sbusVal[9] = *(uint16_t*)(&l_recvSbus2.msg.data[2]);
	sbusVal[10] = *(uint16_t*)(&l_recvSbus2.msg.data[4]);
	sbusVal[11] = *(uint16_t*)(&l_recvSbus2.msg.data[6]);
	sbusVal[12] = *(uint16_t*)(&l_recvSbus3.msg.data[0]);
	sbusVal[13] = *(uint16_t*)(&l_recvSbus3.msg.data[2]);
	sbusVal[14] = *(uint16_t*)(&l_recvSbus3.msg.data[4]);
	sbusVal[15] = *(uint16_t*)(&l_recvSbus3.msg.data[6]);
	
	// 一番遅いタイムスタンプを返す
	*recvTime = l_recvSbus0.recvTime;
	if((now - *recvTime) < (now - l_recvSbus1.recvTime)){
		*recvTime = l_recvSbus1.recvTime;
	}
	if((now - *recvTime) < (now - l_recvSbus2.recvTime)){
		*recvTime = l_recvSbus2.recvTime;
	}
	if((now - *recvTime) < (now - l_recvSbus3.recvTime)){
		*recvTime = l_recvSbus3.recvTime;
	}
}

// データ定期送信読み出し(0xC0)
// データの定期送信設定を取得する電文です。
// 引数：frequency：送信周期の設定値
// 　　　data：各電文の定期送信有効/無効ビットを格納する配列 (4要素)
//             data[0]：bit0：0x80の定期送信有効(1)無効(0)
//                      bit1：0x81の定期送信有効(1)無効(0)
//                      bit4：0x84の定期送信有効(1)無効(0)
//             data[1]：bit0：0x88の定期送信有効(1)無効(0)
//                      bit1：0x89の定期送信有効(1)無効(0)
//                      bit4：0x8Cの定期送信有効(1)無効(0)
//                      bit6：0x8Eの定期送信有効(1)無効(0)
//             data[2]：bit0：0x90の定期送信有効(1)無効(0)
//                      bit1：0x91の定期送信有効(1)無効(0)
//                      bit2：0x92の定期送信有効(1)無効(0)
//                      bit3：0x93の定期送信有効(1)無効(0)
//             data[3]：bit0：0xB0の定期送信有効(1)無効(0)
//                      bit1：0xB1の定期送信有効(1)無効(0)
//                      bit2：0xB2の定期送信有効(1)無効(0)
//                      bit3：0xB3の定期送信有効(1)無効(0)
// 　　　timeout：タイムアウト時間 (ms)
// 戻り値：成功時 true、タイムアウト時 false
bool Crst01a::GetDataPeriodic(uint8_t *frequency, uint8_t *data,uint32_t timeout){

	uint32_t now;
	uint32_t irq_state;
	
	SetReq(CRST_FUNC_READ_DATA_PERIODIC);			// 送信要求
	SetWaitFunkCode(CRST_FUNC_READ_DATA_PERIODIC);	// 応答待ち
	now = millis();
	
	while(1){
		
		if(CRST_FUNC_NONE == GetWaitFunkCode()){
			// 電文を受信した
			irq_state = save_and_disable_interrupts();	// 割り込み禁止
			*frequency = l_RecvTelegram.data[0];
			data[0] = l_RecvTelegram.data[1];
			data[1] = l_RecvTelegram.data[4];
			data[2] = l_RecvTelegram.data[5];
			data[3] = l_RecvTelegram.data[3];
			restore_interrupts(irq_state);				// 割り込み許可
			return true;
		}
		
		if(millis() > (now + timeout)){
			return false;		// タイムアウト
		}
	}
}

// 最大速度設定読み出し(0xC1)
// 現在設定されている最大速度を取得します。
// 引数：xSpeed：X方向最大速度格納先
// 　　　ySpeed：Y方向最大速度格納先
// 　　　yawSpeed：最大旋回速度格納先
// 　　　timeout：タイムアウト時間 (ms)
// 戻り値：成功時 true、タイムアウト時 false
bool Crst01a::GetMaxSpeed(uint16_t *xSpeed, uint16_t *ySpeed, uint16_t *yawSpeed, uint32_t timeout){

	uint32_t now;
	uint32_t irq_state;
	
	SetReq(CRST_FUNC_READ_MAX_SPEED);			// 送信要求
	SetWaitFunkCode(CRST_FUNC_READ_MAX_SPEED);	// 応答待ち
	now = millis();
	
	while(1){
		
		if(CRST_FUNC_NONE == GetWaitFunkCode()){
			// 電文を受信した
			irq_state = save_and_disable_interrupts();	// 割り込み禁止
			*xSpeed = *(uint16_t*)(&l_RecvTelegram.data[0]);
			*ySpeed = *(uint16_t*)(&l_RecvTelegram.data[2]);
			*yawSpeed = *(uint16_t*)(&l_RecvTelegram.data[4]);
			restore_interrupts(irq_state);				// 割り込み許可
			return true;
		}
		
		if(millis() > (now + timeout)){
			return false;		// タイムアウト
		}
	}
}

// バンパー、ブレーキ設定読み出し(0xC4)
// バンパーとブレーキの設定を取得します。
// 引数：bumperConfig：バンパー設定格納先
// 　　　brakeConfig：ブレーキ設定格納先
// 　　　timeout：タイムアウト時間 (ms)
// 戻り値：成功時 true、タイムアウト時 false
bool Crst01a::GetBumperBrake(uint8_t *bumperConfig, uint8_t *brakeConfig, uint32_t timeout){

	uint32_t now;
	uint32_t irq_state;
	
	SetReq(CRST_FUNC_READ_BUMPER_BRAKE);			// 送信要求
	SetWaitFunkCode(CRST_FUNC_READ_BUMPER_BRAKE);	// 応答待ち
	now = millis();
	
	while(1){
		
		if(CRST_FUNC_NONE == GetWaitFunkCode()){
			// 電文を受信した
			irq_state = save_and_disable_interrupts();	// 割り込み禁止
			*bumperConfig = l_RecvTelegram.data[0];
			*brakeConfig = l_RecvTelegram.data[1];
			restore_interrupts(irq_state);				// 割り込み許可
			return true;
		}
		
		if(millis() > (now + timeout)){
			return false;		// タイムアウト
		}
	}
}


// バージョン読み出し(0xC5)
// 車両コントローラのファームウェアバージョンを取得します。
// 引数：ver0：メジャーバージョン
// 　　　ver1：マイナーバージョン
// 　　　ver2：パッチバージョン
// 　　　timeout：タイムアウト時間 (ms)
// 戻り値：成功時 true、タイムアウト時 false
bool Crst01a::GetVersion(uint8_t *ver0, uint8_t *ver1, uint8_t *ver2, uint32_t timeout){

	uint32_t now;
	uint32_t irq_state;
	
	SetReq(CRST_FUNC_READ_VERSION);					// 送信要求
	SetWaitFunkCode(CRST_FUNC_READ_VERSION);		// 応答待ち
	now = millis();
	
	while(1){
		
		if(CRST_FUNC_NONE == GetWaitFunkCode()){
			// 電文を受信した
			irq_state = save_and_disable_interrupts();	// 割り込み禁止
			*ver0 = l_RecvTelegram.data[0];
			*ver1 = l_RecvTelegram.data[1];
			*ver2 = l_RecvTelegram.data[2];
			restore_interrupts(irq_state);				// 割り込み許可
			return true;
		}
		
		if(millis() > (now + timeout)){
			return false;		// タイムアウト
		}
	}
}


// 電圧設定読み出し(0xC8)
// 電圧異常検知の上下限設定を取得します。
// 引数：minVol：下限電圧 (0.1V単位)
// 　　　maxVol：上限電圧 (0.1V単位)
// 　　　timeout：タイムアウト時間 (ms)
// 戻り値：成功時 true、タイムアウト時 false
bool Crst01a::GetVoltageConfig(uint16_t *minVol, uint16_t *maxVol, uint32_t timeout){

	uint32_t now;
	uint32_t irq_state;
	
	SetReq(CRST_FUNC_READ_VOLTAGE);					// 送信要求
	SetWaitFunkCode(CRST_FUNC_READ_VOLTAGE);		// 応答待ち
	now = millis();
	
	while(1){
		
		if(CRST_FUNC_NONE == GetWaitFunkCode()){
			// 電文を受信した
			irq_state = save_and_disable_interrupts();	// 割り込み禁止
			*minVol = *(uint16_t*)(&l_RecvTelegram.data[0]);
			*maxVol = *(uint16_t*)(&l_RecvTelegram.data[2]);
			restore_interrupts(irq_state);				// 割り込み許可
			return true;
		}
		
		if(millis() > (now + timeout)){
			return false;		// タイムアウト
		}
	}
}


// モータドライバ設定0読み出し(0xD0)
// モータドライバの接続有無設定を取得します。
// 引数：existFlag：接続有無ビットフラグ
// 　　　timeout：タイムアウト時間 (ms)
// 戻り値：成功時 true、タイムアウト時 false
bool Crst01a::GetMdConfig0(uint16_t *existFlag, uint32_t timeout){

	uint32_t now;
	uint32_t irq_state;
	
	SetReq(CRST_FUNC_READ_MD_CONFIG0);				// 送信要求
	SetWaitFunkCode(CRST_FUNC_READ_MD_CONFIG0);		// 応答待ち
	now = millis();
	
	while(1){
		
		if(CRST_FUNC_NONE == GetWaitFunkCode()){
			// 電文を受信した
			irq_state = save_and_disable_interrupts();	// 割り込み禁止
			*existFlag = *(uint16_t*)(&l_RecvTelegram.data[0]);
			restore_interrupts(irq_state);				// 割り込み許可
			return true;
		}
		
		if(millis() > (now + timeout)){
			return false;		// タイムアウト
		}
	}
}


// モータドライバ設定1読み出し(0xD1)
// ギアボックス込みのモータ最大回転数設定を取得します。
// 引数：maxGbSpeed：最大回転数格納先 (rpm)
// 　　　timeout：タイムアウト時間 (ms)
// 戻り値：成功時 true、タイムアウト時 false
bool Crst01a::GetMdConfig1(float *maxGbSpeed, uint32_t timeout){

	uint32_t now;
	uint32_t irq_state;
	
	SetReq(CRST_FUNC_READ_MD_CONFIG1);				// 送信要求
	SetWaitFunkCode(CRST_FUNC_READ_MD_CONFIG1);		// 応答待ち
	now = millis();
	
	while(1){
		
		if(CRST_FUNC_NONE == GetWaitFunkCode()){
			// 電文を受信した
			irq_state = save_and_disable_interrupts();	// 割り込み禁止
			*maxGbSpeed = *(float*)(&l_RecvTelegram.data[0]);
			restore_interrupts(irq_state);				// 割り込み許可
			return true;
		}
		
		if(millis() > (now + timeout)){
			return false;		// タイムアウト
		}
	}
}


// モータドライバ設定2読み出し(0xD2)
// 停止判断の閾値設定を取得します。
// 引数：stopRpm：閾値格納先 (rpm)
// 　　　timeout：タイムアウト時間 (ms)
// 戻り値：成功時 true、タイムアウト時 false
bool Crst01a::GetMdConfig2(float *stopRpm, uint32_t timeout){

	uint32_t now;
	uint32_t irq_state;
	
	SetReq(CRST_FUNC_READ_MD_CONFIG2);				// 送信要求
	SetWaitFunkCode(CRST_FUNC_READ_MD_CONFIG2);		// 応答待ち
	now = millis();
	
	while(1){
		
		if(CRST_FUNC_NONE == GetWaitFunkCode()){
			// 電文を受信した
			irq_state = save_and_disable_interrupts();	// 割り込み禁止
			*stopRpm = *(float*)(&l_RecvTelegram.data[0]);
			restore_interrupts(irq_state);				// 割り込み許可
			return true;
		}
		
		if(millis() > (now + timeout)){
			return false;		// タイムアウト
		}
	}
}


// モータドライバ設定3読み出し(0xD3)
// トルク制限設定を取得します。
// 引数：maxTorque：最大トルク (A)
// 　　　startTorque：起動時トルク (A)
// 　　　timeout：タイムアウト時間 (ms)
// 戻り値：成功時 true、タイムアウト時 false
bool Crst01a::GetMdConfig3(float *maxTorque, float *startTorque, uint32_t timeout){

	uint32_t now;
	uint32_t irq_state;
	
	SetReq(CRST_FUNC_READ_MD_CONFIG3);				// 送信要求
	SetWaitFunkCode(CRST_FUNC_READ_MD_CONFIG3);		// 応答待ち
	now = millis();
	
	while(1){
		
		if(CRST_FUNC_NONE == GetWaitFunkCode()){
			// 電文を受信した
			irq_state = save_and_disable_interrupts();	// 割り込み禁止
			*maxTorque = *(float*)(&l_RecvTelegram.data[0]);
			*startTorque = *(float*)(&l_RecvTelegram.data[4]);
			restore_interrupts(irq_state);				// 割り込み許可
			return true;
		}
		
		if(millis() > (now + timeout)){
			return false;		// タイムアウト
		}
	}
}


// モータドライバ設定4読み出し(0xD4)
// リカバリトルクとトルク加算設定を取得します。
// 引数：recoveryTorque：リカバリトルク (A)
// 　　　torqueAddRatio：トルク加算量 (A)
// 　　　timeout：タイムアウト時間 (ms)
// 戻り値：成功時 true、タイムアウト時 false
bool Crst01a::GetMdConfig4(float *recoveryTorque, float *torqueAddRatio, uint32_t timeout){

	uint32_t now;
	uint32_t irq_state;
	
	SetReq(CRST_FUNC_READ_MD_CONFIG4);				// 送信要求
	SetWaitFunkCode(CRST_FUNC_READ_MD_CONFIG4);		// 応答待ち
	now = millis();
	
	while(1){
		
		if(CRST_FUNC_NONE == GetWaitFunkCode()){
			// 電文を受信した
			irq_state = save_and_disable_interrupts();	// 割り込み禁止
			*recoveryTorque = *(float*)(&l_RecvTelegram.data[0]);
			*torqueAddRatio = *(float*)(&l_RecvTelegram.data[4]);
			restore_interrupts(irq_state);				// 割り込み許可
			return true;
		}
		
		if(millis() > (now + timeout)){
			return false;		// タイムアウト
		}
	}
}


// モータドライバ設定5読み出し(0xD5)
// ランプ設定と選択設定を取得します。
// 引数：rampA：ランプ設定A
// 　　　rampB：ランプ設定B
// 　　　rampC：ランプ設定C
// 　　　rampSelect：ランプ選択フラグ
// 　　　timeout：タイムアウト時間 (ms)
// 戻り値：成功時 true、タイムアウト時 false
bool Crst01a::GetMdConfig5(uint16_t *rampA, uint16_t *rampB, uint16_t *rampC, uint16_t *rampSelect, uint32_t timeout){

	uint32_t now;
	uint32_t irq_state;
	
	SetReq(CRST_FUNC_READ_MD_CONFIG5);				// 送信要求
	SetWaitFunkCode(CRST_FUNC_READ_MD_CONFIG5);		// 応答待ち
	now = millis();
	
	while(1){
		
		if(CRST_FUNC_NONE == GetWaitFunkCode()){
			// 電文を受信した
			irq_state = save_and_disable_interrupts();	// 割り込み禁止
			*rampA = *(uint16_t*)(&l_RecvTelegram.data[0]);
			*rampB = *(uint16_t*)(&l_RecvTelegram.data[2]);
			*rampC = *(uint16_t*)(&l_RecvTelegram.data[4]);
			*rampSelect = *(uint16_t*)(&l_RecvTelegram.data[6]);
			restore_interrupts(irq_state);				// 割り込み許可
			return true;
		}
		
		if(millis() > (now + timeout)){
			return false;		// タイムアウト
		}
	}
}


// RC設定0読み出し(0xD8)
// SBUS信号の基本設定(センター、最小、最大、不感帯)を取得します。
// 引数：center：中央値
// 　　　min：最小値
// 　　　max：最大値
// 　　　margin：不感帯
// 　　　timeout：タイムアウト時間 (ms)
// 戻り値：成功時 true、タイムアウト時 false
bool Crst01a::GetRcConfig0(uint16_t *center, uint16_t *min, uint16_t *max, uint16_t *margin, uint32_t timeout){

	uint32_t now;
	uint32_t irq_state;
	
	SetReq(CRST_FUNC_READ_RC_CONFIG0);				// 送信要求
	SetWaitFunkCode(CRST_FUNC_READ_RC_CONFIG0);		// 応答待ち
	now = millis();
	
	while(1){
		
		if(CRST_FUNC_NONE == GetWaitFunkCode()){
			// 電文を受信した
			irq_state = save_and_disable_interrupts();	// 割り込み禁止
			*center = *(uint16_t*)(&l_RecvTelegram.data[0]);
			*min = *(uint16_t*)(&l_RecvTelegram.data[2]);
			*max = *(uint16_t*)(&l_RecvTelegram.data[4]);
			*margin = *(uint16_t*)(&l_RecvTelegram.data[6]);
			restore_interrupts(irq_state);				// 割り込み許可
			return true;
		}
		
		if(millis() > (now + timeout)){
			return false;		// タイムアウト
		}
	}
}


// RC設定1読み出し(0xD9)
// SBUS信号のON/OFF閾値設定を取得します。
// 引数：lowTh：スイッチOFF閾値
// 　　　highTh：スイッチON閾値
// 　　　timeout：タイムアウト時間 (ms)
// 戻り値：成功時 true、タイムアウト時 false
bool Crst01a::GetRcConfig1(uint16_t *lowTh, uint16_t *highTh, uint32_t timeout){

	uint32_t now;
	uint32_t irq_state;
	
	SetReq(CRST_FUNC_READ_RC_CONFIG1);				// 送信要求
	SetWaitFunkCode(CRST_FUNC_READ_RC_CONFIG1);		// 応答待ち
	now = millis();
	
	while(1){
		
		if(CRST_FUNC_NONE == GetWaitFunkCode()){
			// 電文を受信した
			irq_state = save_and_disable_interrupts();	// 割り込み禁止
			*lowTh = *(uint16_t*)(&l_RecvTelegram.data[4]);
			*highTh = *(uint16_t*)(&l_RecvTelegram.data[6]);
			restore_interrupts(irq_state);				// 割り込み許可
			return true;
		}
		
		if(millis() > (now + timeout)){
			return false;		// タイムアウト
		}
	}
}


// RC設定2,3読み出し(0xDA,0xDB)
// 移動やモード切替等に割り当てられたSBUSチャンネルを取得します。
// 引数：movementXChannel：X方向移動のチャンネル
// 　　　movementYChannel：Y方向移動のチャンネル
// 　　　movementYawChannel：旋回方向移動のチャンネル)
// 　　　controlModeSwitchChannel：モード切替のチャンネル
// 　　　brakeControlChannel：自動ブレーキ制御のチャンネル
// 　　　errorAndBumperResetChannel：エラー解除のチャンネル
// 　　　headlight0Channel：ヘッドライト0制御のチャンネル
// 　　　headlight1Channel：ヘッドライト1制御のチャンネル
// 　　　timeout：タイムアウト時間 (ms)
// 戻り値：成功時 true、タイムアウト時 false
bool Crst01a::GetRcConfig23(uint8_t* movementXChannel, uint8_t* movementYChannel, uint8_t* movementYawChannel, uint8_t* controlModeSwitchChannel, uint8_t* brakeControlChannel, uint8_t* errorAndBumperResetChannel,uint8_t* headlight0Channel, uint8_t* headlight1Channel, uint32_t timeout){

	uint32_t now;
	uint32_t irq_state;
	
	SetReq(CRST_FUNC_READ_RC_CONFIG2);				// 送信要求
	SetWaitFunkCode(CRST_FUNC_READ_RC_CONFIG2);		// 応答待ち
	now = millis();
	
	while(1){
		
		if(CRST_FUNC_NONE == GetWaitFunkCode()){
			// 電文を受信した
			irq_state = save_and_disable_interrupts();	// 割り込み禁止
			*movementXChannel = l_RecvTelegram.data[0];			// movement X
			*movementYChannel = l_RecvTelegram.data[1];			// movement Y
			*movementYawChannel = l_RecvTelegram.data[2];			// movement Yaw
			*controlModeSwitchChannel = l_RecvTelegram.data[4];			// mode switch
			*brakeControlChannel = l_RecvTelegram.data[5];			// Brake control
			*errorAndBumperResetChannel = l_RecvTelegram.data[6];			// reset
			restore_interrupts(irq_state);				// 割り込み許可
			break;;
		}
		
		if(millis() > (now + timeout)){
			return false;		// タイムアウト
		}
	}
	
	SetReq(CRST_FUNC_READ_RC_CONFIG3);				// 送信要求
	SetWaitFunkCode(CRST_FUNC_READ_RC_CONFIG3);		// 応答待ち
	now = millis();
	
	while(1){
		
		if(CRST_FUNC_NONE == GetWaitFunkCode()){
			// 電文を受信した
			irq_state = save_and_disable_interrupts();	// 割り込み禁止
			*headlight0Channel = l_RecvTelegram.data[0];
			*headlight1Channel = l_RecvTelegram.data[1];
			restore_interrupts(irq_state);				// 割り込み許可
			return true;
		}
		
		if(millis() > (now + timeout)){
			return false;		// タイムアウト
		}
	}
}


// 順運動学数列読み出し(0xE0-0xE5)
// 順運動学行列の全要素を取得します。
// 引数：data：行列要素を格納する配列 (12要素)
// 　　　timeout：タイムアウト時間 (ms)
// 戻り値：成功時 true、タイムアウト時 false
bool Crst01a::GetFwdKinematics(float *data, uint32_t timeout){

	int i;
	uint32_t irq_state;
	uint32_t now;
	
	// 専用ストレージをクリア
	irq_state = save_and_disable_interrupts();
	for(i = 0; i < 6; i++){
		l_recvFwdKinematics[i].recvTime = 0;
	}
	restore_interrupts(irq_state);
	
	// 0xE0-0xE5を一括要求 (data[4] bit0-5をすべてセット)
	telegram_t reqMsg = {CRST_START_BYTE, CRST_FUNC_SET_PARAM_SEND, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	reqMsg.data[4] = 0x3F;
	reqMsg.checkSum = CalcCheckSum(&reqMsg);
	SERIAL_CRST01A.write((uint8_t*)&reqMsg, CRST_PACKET_LEN);
	
	now = millis();
	
	// 6電文すべての受信を待つ
	for(i = 0; i < 6; i++){
		while(1){
			irq_state = save_and_disable_interrupts();
			uint32_t recvTime = l_recvFwdKinematics[i].recvTime;
			restore_interrupts(irq_state);
			if(recvTime != 0){
				irq_state = save_and_disable_interrupts();
				data[i*2+0] = *(float*)(&l_recvFwdKinematics[i].msg.data[0]);
				data[i*2+1] = *(float*)(&l_recvFwdKinematics[i].msg.data[4]);
				restore_interrupts(irq_state);
				break;
			}
			if(millis() > (now + timeout)){
				return false;		// タイムアウト
			}
		}
	}
	return true;
}


// 逆運動学数列読み出し(0xF0-0xF5)
// 逆運動学行列の全要素を取得します。
// 引数：data：行列要素を格納する配列 (12要素)
// 　　　timeout：タイムアウト時間 (ms)
// 戻り値：成功時 true、タイムアウト時 false
bool Crst01a::GetInvKinematics(float *data, uint32_t timeout){

	int i;
	uint32_t irq_state;
	uint32_t now;
	
	// 専用ストレージをクリア
	irq_state = save_and_disable_interrupts();
	for(i = 0; i < 6; i++){
		l_recvInvKinematics[i].recvTime = 0;
	}
	restore_interrupts(irq_state);
	
	// 0xF0-0xF5を一括要求 (data[6] bit0-5をすべてセット)
	telegram_t reqMsg = {CRST_START_BYTE, CRST_FUNC_SET_PARAM_SEND, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	reqMsg.data[6] = 0x3F;
	reqMsg.checkSum = CalcCheckSum(&reqMsg);
	SERIAL_CRST01A.write((uint8_t*)&reqMsg, CRST_PACKET_LEN);
	
	now = millis();
	
	// 6電文すべての受信を待つ
	for(i = 0; i < 6; i++){
		while(1){
			irq_state = save_and_disable_interrupts();
			uint32_t recvTime = l_recvInvKinematics[i].recvTime;
			restore_interrupts(irq_state);
			if(recvTime != 0){
				irq_state = save_and_disable_interrupts();
				data[i*2+0] = *(float*)(&l_recvInvKinematics[i].msg.data[0]);
				data[i*2+1] = *(float*)(&l_recvInvKinematics[i].msg.data[4]);
				restore_interrupts(irq_state);
				break;
			}
			if(millis() > (now + timeout)){
				return false;		// タイムアウト
			}
		}
	}
	return true;
}


// 定期処理
// 引数：t：タイマー構造体ポインタ
// 戻り値：true (継続)
bool Crst01a::TimerHandler0(struct repeating_timer *t){
	crst01a.GetCmd();		// 車両コントローラから電文の読み出し
	return true;
}



// 電文読み出し処理
// 車両コントローラからのシリアルデータを解析し、メンバ変数へ格納します。
// 引数：なし
// 戻り値：なし
void Crst01a::GetCmd(void){
	telegram_t buf;
	uint8_t *pBuf = (uint8_t*)&buf;
	int i;
	
	while(1){
		// ヘッダー以外を削除
		while(SERIAL_CRST01A.available()){
			if(CRST_START_BYTE != SERIAL_CRST01A.peek()){
				SERIAL_CRST01A.read();	// 読み飛ばす
			}
			else{
				break;			// ヘッダーが見つかった
			}
		}
		
		if(CRST_PACKET_LEN <= SERIAL_CRST01A.available()){
			for(i = 0;i < CRST_PACKET_LEN;i++){
				pBuf[i] = SERIAL_CRST01A.read();
			}
			
			if(buf.checkSum == CalcCheckSum(&buf)){
				// チェックサムが合っている
				
				switch(buf.funcCode){
					case CRST_FUNC_READ_SYS_STATUS:		// システムステータス読み出し
						l_recvSysStatus.msg = buf;
						l_recvSysStatus.recvTime = millis();
						break;
					case CRST_FUNC_READ_RUN_STATUS:		// 走行状態読み出し
						l_recvRunStatus.msg = buf;
						l_recvRunStatus.recvTime = millis();
						break;
					case CRST_FUNC_READ_EXT_IO:			// 外部IO読み出し
						l_recvExtIo.msg = buf;
						l_recvExtIo.recvTime = millis();
						break;
					case CRST_FUNC_READ_ENCODER_01:		// モータエンコーダ読み出し (0, 1)
						l_recvEncoder01.msg = buf;
						l_recvEncoder01.recvTime = millis();
						break;
					case CRST_FUNC_READ_ENCODER_23:		// モータエンコーダ読み出し (2, 3)
						l_recvEncoder23.msg = buf;
						l_recvEncoder23.recvTime = millis();
						break;
					case CRST_FUNC_READ_MD_TEMP:		// モータドライバ温度読み出し
						l_recvMdtemp.msg = buf;
						l_recvMdtemp.recvTime = millis();
						break;
					case CRST_FUNC_READ_MD_STATUS:		// モータドライバ状態読み出し
						l_recvMdStatus.msg = buf;
						l_recvMdStatus.recvTime = millis();
						break;
					case CRST_FUNC_READ_MOTOR_OUT_0:	// モータ出力読み出し (0)
						l_recvMotorOut0.msg = buf;
						l_recvMotorOut0.recvTime = millis();
						break;
					case CRST_FUNC_READ_MOTOR_OUT_1:	// モータ出力読み出し (1)
						l_recvMotorOut1.msg = buf;
						l_recvMotorOut1.recvTime = millis();
						break;
					case CRST_FUNC_READ_MOTOR_OUT_2:	// モータ出力読み出し (2)
						l_recvMotorOut2.msg = buf;
						l_recvMotorOut2.recvTime = millis();
						break;
					case CRST_FUNC_READ_MOTOR_OUT_3:	// モータ出力読み出し (3)
						l_recvMotorOut3.msg = buf;
						l_recvMotorOut3.recvTime = millis();
						break;
					case CRST_FUNC_READ_SBUS_0:			// SBUS読み出し (Ch 1-4)
						l_recvSbus0.msg = buf;
						l_recvSbus0.recvTime = millis();
						break;
					case CRST_FUNC_READ_SBUS_1:			// SBUS読み出し (Ch 5-8)
						l_recvSbus1.msg = buf;
						l_recvSbus1.recvTime = millis();
						break;
					case CRST_FUNC_READ_SBUS_2:			// SBUS読み出し (Ch 9-12)
						l_recvSbus2.msg = buf;
						l_recvSbus2.recvTime = millis();
						break;
					case CRST_FUNC_READ_SBUS_3:			// SBUS読み出し (Ch 13-16)
						l_recvSbus3.msg = buf;
						l_recvSbus3.recvTime = millis();
						break;
					case CRST_FUNC_READ_FWD_KINEMATICS_0:
					case CRST_FUNC_READ_FWD_KINEMATICS_1:
					case CRST_FUNC_READ_FWD_KINEMATICS_2:
					case CRST_FUNC_READ_FWD_KINEMATICS_3:
					case CRST_FUNC_READ_FWD_KINEMATICS_4:
					case CRST_FUNC_READ_FWD_KINEMATICS_5:
						l_recvFwdKinematics[buf.funcCode - CRST_FUNC_READ_FWD_KINEMATICS_0].msg = buf;
						l_recvFwdKinematics[buf.funcCode - CRST_FUNC_READ_FWD_KINEMATICS_0].recvTime = millis();
						break;
					case CRST_FUNC_READ_INV_KINEMATICS_0:
					case CRST_FUNC_READ_INV_KINEMATICS_1:
					case CRST_FUNC_READ_INV_KINEMATICS_2:
					case CRST_FUNC_READ_INV_KINEMATICS_3:
					case CRST_FUNC_READ_INV_KINEMATICS_4:
					case CRST_FUNC_READ_INV_KINEMATICS_5:
						l_recvInvKinematics[buf.funcCode - CRST_FUNC_READ_INV_KINEMATICS_0].msg = buf;
						l_recvInvKinematics[buf.funcCode - CRST_FUNC_READ_INV_KINEMATICS_0].recvTime = millis();
						break;
					default:
						break;
				}
				
				// 受信待ちなら処理
				if((CRST_FUNC_NONE != l_waitFunkCode) && (buf.funcCode == (uint8_t)l_waitFunkCode)){
					l_RecvTelegram = buf;
					l_waitFunkCode = CRST_FUNC_NONE;
				}
			}
		}
		else{
			break;	// 1電文受信していないので処理をしない
		}
	}
}

// 受信待ちフラグの取得関数
// 引数：なし
// 戻り値：現在待機中のファンクションコード
uint16_t Crst01a::GetWaitFunkCode(void){
	
	uint32_t irq_state;
	uint16_t waitFunkCode;
	
	irq_state = save_and_disable_interrupts();	// 割り込み禁止
	waitFunkCode = l_waitFunkCode;				// フラグのコピー
	restore_interrupts(irq_state);				// 割り込み許可
	
	return waitFunkCode;
}

// 受信待ちフラグの設定関数
// 引数：waitFunkCode：待機するファンクションコード
// 戻り値：なし
void Crst01a::SetWaitFunkCode(uint16_t waitFunkCode){
	
	uint32_t irq_state;
	
	irq_state = save_and_disable_interrupts();	// 割り込み禁止
	l_waitFunkCode = waitFunkCode;				// フラグのコピー
	restore_interrupts(irq_state);				// 割り込み許可
}

// チェックサムの計算
// start byteからdata idまでの値を合算します。
// 引数：p：電文構造体ポインタ
// 戻り値：計算されたチェックサム (下位1バイト)
uint8_t Crst01a::CalcCheckSum(telegram_t *p){
	uint8_t sum = 0;
	int i;
	
	sum += p->startByte;
	sum += p->funcCode;
	
	for(i = 0;i < CRST_DATA_LEN;i++){
		sum += p->data[i];
	}
	
	sum += p->dataId;
	
	return sum;
}