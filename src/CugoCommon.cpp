
#include "CugoCommon.h"
#include "Crst01a.h"

#define PC_PRINTLN(str)	if(NULL != l_pSerial){l_pSerial->println(str);}
#define PC_PRINT(str)	if(NULL != l_pSerial){l_pSerial->print(str);}
#define RECV_TIMEOUT	(500)		// 受信した定期送信系データが古いと判断する閾値
#define MIN_VOLTAGE		150
#define MAX_VOLTAGE		520

CugoCommon cugoCommon;


CugoCommon::CugoCommon(void){
	l_switchReset = false;
	l_mode = CUGO_RC_MODE;
}

// 初期化関数
// 引数：pSerial：ログの出力に使用するシリアルのポインタ。NULLを指定するとログを出力しない。
// 戻り値：なし
void CugoCommon::Init(HardwareSerial* pSerial){
	
	l_pSerial = pSerial;
	
	if(!crst01a.Init()){
		PC_PRINTLN(F("##################################"));
		PC_PRINTLN(F("###      CugoCommon Start      ###"));
		PC_PRINTLN(F("##################################"));
	}
}


// モード変更時の動作設定
// 引数：switchReset：プロポでRCモード切替時に最初から実行したい場合はtrue、動作を再開する場合はfalse
// 戻り値：なし
void CugoCommon::SetSwitchReset(bool switchReset){
	l_switchReset = switchReset;
}


// モード変更時の動作設定取得
// 引数：なし
// 戻り値：設定を返す
bool CugoCommon::GetSwitchReset(void){
	return l_switchReset;
}


// エラーの有無を取得する。エラー情報が古くてもエラーとする。
// 引数：なし
// 戻り値：エラー情報
uint32_t CugoCommon::GetErr(void){
	
	uint8_t controllerStatus;
	uint8_t controllerError;
	uint8_t motorDriverError;
	uint16_t driverVoltage;
	uint32_t recvTime;
	uint32_t ret = 0;
	
	crst01a.GetSysStatus(&controllerStatus, &controllerError, &motorDriverError, &driverVoltage, &recvTime);
	ret |= controllerError;
	ret |= motorDriverError << 8;
	
	
	if(RECV_TIMEOUT < (millis() - recvTime)){
		// エラー情報を最後に受信したのが古すぎる
		ret |= CUGO_ERR_CMD_TIMEOUT;
	}
	
	return ret;
}


// エラークリア
// 引数：err：クリアするエラー内容。
// 戻り値：なし
void CugoCommon::ClearErr(uint32_t err){
	
	
	crst01a.ClearControllerError(err & 0xFF);
	crst01a.ClearDriverError((err >> 8) & 0xFF);
}


// 電源電圧の取得
// ※電圧はあくまで目安
// 引数：driverVoltage：値×0.1V単位で現在の電源電圧
// 戻り値：エラー発生でfalse
bool CugoCommon::GetVoltage(uint16_t *driverVoltage){
	
	uint8_t controllerStatus;
	uint8_t controllerError;
	uint8_t motorDriverError;
	uint32_t recvTime;
	
	crst01a.GetSysStatus(&controllerStatus, &controllerError, &motorDriverError, driverVoltage, &recvTime);
	
	
	if(RECV_TIMEOUT < (millis() - recvTime)){
		// エラー情報を最後に受信したのが古すぎる
		return false;
	}
	
	return true;
}


// エラーをチェックしつつ待つ。エラーが発生したら処理を抜けて戻り値で教える。
// モードが変わったらリセットのためにすぐ抜けるかモードが戻るまで待ち続ける。
// 引数：time：待機する時間(ms)
// 戻り値：エラー発生でfalse
bool CugoCommon::Wait(uint32_t time){
	
	uint32_t now = millis();
	uint8_t mode;
	
	while(time > (millis()-now)){
		if(GetErr()){
			// エラー発生
			return false;
		}
		
		GetControlMode(&mode);
		if(CUGO_CMD_MODE != mode){
			if(l_switchReset){
				return true;		// リセットしたい、処理の先頭に戻りたいので待ちから抜ける
			}
			else{
				uint32_t elapsed = millis() - now;
				if(elapsed >= time){
					return true;	// 待ち時間が経過済みのため正常終了
				}
				time -= elapsed;	// 残り待ち時間を更新

				// モードがコマンドモードになるまで待つ
				while(CUGO_CMD_MODE != mode){
					if(GetErr()){
						// エラー発生
						return false;
					}
					GetControlMode(&mode);
					delay(1);
				}

				now = millis();	// 開始時刻をセットして再開
			}
		}
		
		delay(1);
	}
	
	return true;
}


// モード切替
// 引数：mode：CUGO_RC_MODE：RCモード, CUGO_CMD_MODE：コマンドモード
// 戻り値：なし
void CugoCommon::SetControlMode(uint8_t mode){
	crst01a.SetControlMode(mode);
}


// モード取得
// 引数：mode：現在のモード格納先
// 戻り値：モード情報の取得失敗でfalse
bool CugoCommon::GetControlMode(uint8_t *mode){
	
	uint8_t controllerStatus;
	uint8_t controllerError;
	uint8_t motorDriverError;
	uint16_t driverVoltage;
	uint32_t recvTime;
	
	crst01a.GetSysStatus(&controllerStatus, &controllerError, &motorDriverError, &driverVoltage, &recvTime);
	if(CUGO_STS_CMD_MODE & controllerStatus){
		// コマンドモード
		*mode = CUGO_CMD_MODE;
	}
	else{
		// RCモード
		*mode = CUGO_RC_MODE;
	}
	
	if(RECV_TIMEOUT < (millis() - recvTime)){
		// 最後に受信したのが古すぎる
		return false;
	}
	
	return true;
}


// パラメータ保存
// 現在のパラメータをフラッシュメモリに保存する
// 引数：なし
// 戻り値：成功時 true、失敗・タイムアウト時 false
bool CugoCommon::SaveParamReq(void){
	
	return crst01a.SaveParamReq(500);
}


// 電圧上下限設定
// 車両コントローラが電圧異常と検知する電圧の上下限を設定する。
// 引数：driverMinVoltage：電圧異常の下限値 (値×0.1V)
// 　　　driverMaxVoltage：電圧異常の上限値 (値×0.1V)
// 戻り値：設定値が正常時にtrue
bool CugoCommon::SetVoltageConfig(uint16_t driverMinVoltage, uint16_t driverMaxVoltage){

	if(MIN_VOLTAGE > driverMinVoltage){
		return false;
	}
	
	if(MAX_VOLTAGE < driverMaxVoltage){
		return false;
	}
	
	crst01a.GetVoltageConfig(&driverMinVoltage, &driverMaxVoltage);
	return true;
}


// 電圧上下限設定
// 車両コントローラが電圧異常と検知する電圧の上下限を設定する。
// 引数：driverMinVoltage：電圧異常の下限値 (値×0.1V)
// 　　　driverMaxVoltage：電圧異常の上限値 (値×0.1V)
// 戻り値：設定値が正常時にtrue
bool CugoCommon::GetVoltageConfig(uint16_t *pDriverMinVoltage, uint16_t *pDriverMaxVoltage){

	return crst01a.GetVoltageConfig(pDriverMinVoltage, pDriverMaxVoltage);
}


// バージョン読み出し
// 車両コントローラのファームウェアバージョンを取得する。
// 引数：ver0：メジャーバージョン
// 　　　ver1：マイナーバージョン
// 　　　ver2：パッチバージョン
// 戻り値：成功時 true、タイムアウト時 false
bool CugoCommon::GetVersion(uint8_t *pVer0, uint8_t *pVer1, uint8_t *pVer2){
	
	return crst01a.GetVersion(pVer0, pVer1, pVer2, 500);
}


// Raspberry Pi Pico 2 Wに搭載のLEDを操作する
// LEDを点灯したり点滅させたりしてアプリの状態を表示したい。
// ライブラリのサンプル等共通した動作にしたいため実装。
// 引数：blink：点滅させたいときにtrue
// 戻り値：なし
void CugoCommon::LedPrint(bool err){
	
	while(1){
		if(err){
			digitalWrite(LED_BUILTIN, HIGH);
			delay(150);
			digitalWrite(LED_BUILTIN, LOW);
			delay(150);
		}
		else{
			digitalWrite(LED_BUILTIN, HIGH);
			delay(300);
		}
	}
}
