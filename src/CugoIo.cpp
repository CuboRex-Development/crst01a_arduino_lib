
#include "CugoIo.h"
#include "CugoCommon.h"
#include "Crst01a.h"

#define PC_PRINTLN(str)	if(NULL != l_pSerial){l_pSerial->println(str);}
#define PC_PRINT(str)	if(NULL != l_pSerial){l_pSerial->print(str);}
#define RECV_TIMEOUT	(500)		// 受信した定期送信系データが古いと判断する閾値

CugoIo cugoIo;


// コンストラクタ
// 引数：なし
CugoIo::CugoIo(void){
}


// 初期化関数
// 入出力管理で使用する定期送信(外部IO、SBUS)をまとめて有効化する。
// 定期送信設定(0x40)は車両コントローラ側で全置換となるため、既定で有効なシステムステータス(0x80)・
// 走行状態(0x81)も合わせて有効化し、他クラスのモード取得・エラー検知・走行制御が壊れないようにする。
// 引数：pSerial：ログの出力に使用するシリアルのポインタ。NULLを指定するとログを出力しない。
// 戻り値：なし
void CugoIo::Init(HardwareSerial* pSerial){

	l_pSerial = pSerial;

	if(!crst01a.Init()){
		PC_PRINTLN(F("##################################"));
		PC_PRINTLN(F("###         CugoIo Start       ###"));
		PC_PRINTLN(F("##################################"));
	}

	// 入出力管理で使用する定期送信を全て有効化する
	crst01a.SetCycleReq(CRST_FUNC_READ_SYS_STATUS);	// システムステータス(0x80)：既定で有効、保持のため再設定
	crst01a.SetCycleReq(CRST_FUNC_READ_RUN_STATUS);	// 走行状態(0x81)：既定で有効、保持のため再設定
	crst01a.SetCycleReq(CRST_FUNC_READ_EXT_IO);		// 外部IO(0x84)
	crst01a.SetCycleReq(CRST_FUNC_READ_SBUS_0);		// SBUS(0xB0)
	crst01a.SetCycleReq(CRST_FUNC_READ_SBUS_1);		// SBUS(0xB1)
	crst01a.SetCycleReq(CRST_FUNC_READ_SBUS_2);		// SBUS(0xB2)
	crst01a.SetCycleReq(CRST_FUNC_READ_SBUS_3);		// SBUS(0xB3)
}


// 4bit入力状況の読み出し
// 外部IO(0x84)の定期送信から4bit入力の状態を取得する。
// 0x84のbit0-3が4bit入力1、bit4-7が4bit入力2の2セット構成のため引数を分けて返す。
// 引数：in4bit1：4bit入力1の状態格納先(下位4bit、値0-15)
// 　　　in4bit2：4bit入力2の状態格納先(上位4bit、値0-15)
// 戻り値：データが新しい場合にtrue、古い(取得失敗)場合にfalse
bool CugoIo::GetInput4bit(uint8_t *in4bit1, uint8_t *in4bit2){

	uint8_t headlightControl;
	uint8_t towerlightControl;
	uint8_t in4bit;
	uint32_t recvTime;

	bool received = crst01a.GetExtIo(&headlightControl, &towerlightControl, &in4bit, &recvTime);

	*in4bit1 = in4bit & 0x0F;			// bit0-3：4bit入力1
	*in4bit2 = (in4bit >> 4) & 0x0F;	// bit4-7：4bit入力2

	if(!received || (RECV_TIMEOUT < (millis() - recvTime))){
		// 未受信、または最後に受信したのが古すぎる
		return false;
	}

	return true;
}


// バンパー状態の読み出し
// システムステータス(0x80、既定で有効)からバンパー0・1の接触状態を取得する。
// 引数：bumper0：バンパー0の状態格納先(接触で1、非接触で0)
// 　　　bumper1：バンパー1の状態格納先(接触で1、非接触で0)
// 戻り値：データが新しい場合にtrue、古い(取得失敗)場合にfalse
bool CugoIo::GetBumper(uint8_t *bumper0, uint8_t *bumper1){

	uint8_t controllerStatus;
	uint8_t controllerError;
	uint8_t motorDriverError;
	uint16_t driverVoltage;
	uint32_t recvTime;

	bool received = crst01a.GetSysStatus(&controllerStatus, &controllerError, &motorDriverError, &driverVoltage, &recvTime);

	*bumper0 = (CUGO_STS_BUMPER0 & controllerStatus) ? 1 : 0;
	*bumper1 = (CUGO_STS_BUMPER1 & controllerStatus) ? 1 : 0;

	if(!received || (RECV_TIMEOUT < (millis() - recvTime))){
		// 未受信、または最後に受信したのが古すぎる
		return false;
	}

	return true;
}


// SBUS状態の読み出し
// SBUS(0xB0-0xB3)の定期送信から16チャンネル分の信号値を取得する。
// 引数：sbusVal：16チャンネルの信号値を格納する配列(要素数16)
// 戻り値：データが新しい場合にtrue、古い(取得失敗)場合にfalse
bool CugoIo::GetSbus(uint16_t *sbusVal){

	uint32_t recvTime;

	if(!crst01a.GetSbus(sbusVal, &recvTime)){
		// 未受信
		return false;
	}

	if(RECV_TIMEOUT < (millis() - recvTime)){
		// 最後に受信したのが古すぎる
		return false;
	}

	return true;
}


// 非常停止スイッチ状態の読み出し
// システムステータス(0x80、既定で有効)から非常停止スイッチの状態を取得する。
// 引数：emergencySwitch：非常停止スイッチの状態格納先(押下で1、非押下で0)
// 戻り値：データが新しい場合にtrue、古い(取得失敗)場合にfalse
bool CugoIo::GetEmergencySwitch(uint8_t *emergencySwitch){

	uint8_t controllerStatus;
	uint8_t controllerError;
	uint8_t motorDriverError;
	uint16_t driverVoltage;
	uint32_t recvTime;

	bool received = crst01a.GetSysStatus(&controllerStatus, &controllerError, &motorDriverError, &driverVoltage, &recvTime);

	*emergencySwitch = (CUGO_STS_EMR_SW & controllerStatus) ? 1 : 0;

	if(!received || (RECV_TIMEOUT < (millis() - recvTime))){
		// 未受信、または最後に受信したのが古すぎる
		return false;
	}

	return true;
}


// ヘッドライト、タワーライト出力設定
// ヘッドライトとタワーライトのON/OFF(点灯モード)を設定する。
// 引数：headlightControl：ヘッドライトの設定。CUGO_HEADLIGHT0 / CUGO_HEADLIGHT1 の論理和で指定する。
// 　　　towerlightControl：タワーライトの設定。各タワーライト2bitで点灯モードを指定する。
//                          CUGO_TOWERLIGHT_x を CUGO_TOWERLIGHTn_SHIFT でシフトした値の論理和で指定する。
// 戻り値：なし
void CugoIo::SetLight(uint8_t headlightControl, uint8_t towerlightControl){

	crst01a.SetLights(headlightControl, towerlightControl);
}


// ヘッドライト、タワーライト出力状態の読み出し
// 外部IO(0x84)の定期送信から現在のライト出力状態を取得する。
// 引数：headlightControl：ヘッドライトの状態格納先(ビット構成はSetLightと同じ)
// 　　　towerlightControl：タワーライトの状態格納先(ビット構成はSetLightと同じ)
// 戻り値：データが新しい場合にtrue、古い(取得失敗)場合にfalse
bool CugoIo::GetLight(uint8_t *headlightControl, uint8_t *towerlightControl){

	uint8_t in4bit;
	uint32_t recvTime;

	if(!crst01a.GetExtIo(headlightControl, towerlightControl, &in4bit, &recvTime)){
		// 未受信
		return false;
	}

	if(RECV_TIMEOUT < (millis() - recvTime)){
		// 最後に受信したのが古すぎる
		return false;
	}

	return true;
}
