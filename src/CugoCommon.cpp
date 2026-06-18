
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
	
	bool received = crst01a.GetSysStatus(&controllerStatus, &controllerError, &motorDriverError, &driverVoltage, &recvTime);

	if(received){
		// 受信済みの場合のみエラービットを反映する(未受信時はmsgが未初期化でゴミ値のため)
		ret |= controllerError;
		ret |= motorDriverError << 8;
	}

	if(!received || (RECV_TIMEOUT < (millis() - recvTime))){
		// 未受信、またはエラー情報を最後に受信したのが古すぎる
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
	
	if(!crst01a.GetSysStatus(&controllerStatus, &controllerError, &motorDriverError, driverVoltage, &recvTime)){
		// 未受信
		return false;
	}

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
	
	bool received = crst01a.GetSysStatus(&controllerStatus, &controllerError, &motorDriverError, &driverVoltage, &recvTime);
	if(CUGO_STS_CMD_MODE & controllerStatus){
		// コマンドモード
		*mode = CUGO_CMD_MODE;
	}
	else{
		// RCモード
		*mode = CUGO_RC_MODE;
	}

	if(!received || (RECV_TIMEOUT < (millis() - recvTime))){
		// 未受信、または最後に受信したのが古すぎる
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
// 引数：driverMinVoltage：電圧が低いと判定する電圧 (値×0.1V)
// 　　　driverMaxVoltage：電圧が高いと判定する電圧 (値×0.1V)
// 戻り値：設定値が正常時にtrue
bool CugoCommon::SetVoltageConfig(uint16_t driverMinVoltage, uint16_t driverMaxVoltage){

	if((MIN_VOLTAGE > driverMinVoltage) || (MAX_VOLTAGE < driverMinVoltage)){
		return false;
	}
	
	if((MIN_VOLTAGE > driverMaxVoltage) || (MAX_VOLTAGE < driverMaxVoltage)){
		return false;
	}
	
	crst01a.SetVoltageConfig(driverMinVoltage, driverMaxVoltage);
	return true;
}


// 電圧上下限取得
// 車両コントローラが電圧異常と検知する電圧の上下限を取得する。
// 引数：driverMinVoltage：電圧が低いと判定する電圧 (値×0.1V)
// 　　　driverMaxVoltage：電圧が高いと判定する電圧 (値×0.1V)
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


// 緊急減速
// 現在の走行を緊急減速で停止させる。減速のランプはランプ設定(SetSpeedRamp)の緊急減速時の設定に従う。
// 引数：なし
// 戻り値：なし
void CugoCommon::EmergencyDeceleration(void){

	crst01a.SetEmergencyDeceleration();
}


// バンパー、ブレーキ設定
// バンパーの論理・停止有効設定と自動ブレーキの設定を行う。
// 引数：bumperConfig：バンパーの設定。以下のビットの論理和で指定する。
//                     CUGO_BUMPER0_POLARITY：バンパー0の論理反転
//                     CUGO_BUMPER1_POLARITY：バンパー1の論理反転
//                     CUGO_BUMPER_STOP_ENABLE：バンパー接触で停止
// 　　　brakeConfig：自動ブレーキの設定。
//                    CUGO_AUTO_BRAKE_ENABLE：自動ブレーキ有効
// 戻り値：なし
void CugoCommon::SetBumperBrake(uint8_t bumperConfig, uint8_t brakeConfig){

	crst01a.SetBumperBrake(bumperConfig, brakeConfig);
}


// ブレーキの閾値設定
// 自動ブレーキ有効時に停止と判断する速度の閾値を設定する。
// 引数：judgeToStopRpm：停止と判断する閾値 (rpm)
// 戻り値：なし
void CugoCommon::SetBrakeThreshold(float judgeToStopRpm){

	crst01a.SetMdConfig2(judgeToStopRpm);
}


// ランプ設定
// 各減速要因に応じた減速ランプ(RPM/s)とその割り当てを設定する。
// 引数：speedRampA：ランプ設定A (RPM/s)
// 　　　speedRampB：ランプ設定B (RPM/s)
// 　　　speedRampC：ランプ設定C (RPM/s)
// 　　　speedRampSelect：各減速要因でA/B/Cどのランプを使用するかの選択。
//                        CUGO_RAMP_SELECT_x を CUGO_RAMP_SHIFT_xxx でシフトした値の論理和で指定する。
// 戻り値：なし
void CugoCommon::SetSpeedRamp(uint16_t speedRampA, uint16_t speedRampB, uint16_t speedRampC, uint16_t speedRampSelect){

	crst01a.SetMdConfig5(speedRampA, speedRampB, speedRampC, speedRampSelect);
}


// エンコーダリセット
// 車両コントローラが保持しているモータエンコーダのカウント値を0にリセットする。
// 引数：なし
// 戻り値：なし
void CugoCommon::ClearEncoderCount(void){

	crst01a.ClearEncoderCount();
}


// RC設定(0x58)
// SBUS信号のセンター・最小・最大値・不感帯を設定する。
// 引数：rcCenterValue：SBUS中央値
// 　　　rcMinValue：SBUS最小値
// 　　　rcMaxValue：SBUS最大値
// 　　　rcCenterMargin：不感帯幅(例:10の場合は中央値±10の範囲が不感帯)
// 戻り値：なし
void CugoCommon::SetRcConfig0(uint16_t rcCenterValue, uint16_t rcMinValue, uint16_t rcMaxValue, uint16_t rcCenterMargin){

	crst01a.SetRcConfig0(rcCenterValue, rcMinValue, rcMaxValue, rcCenterMargin);
}


// RC設定(0x59)
// SBUS信号でスイッチをON/OFF判定する閾値を設定する。
// 引数：rcLowSwitchingThreshold：スイッチOFFとする閾値
// 　　　rcHighSwitchingThreshold：スイッチONとする閾値
// 戻り値：なし
void CugoCommon::SetRcConfig1(uint16_t rcLowSwitchingThreshold, uint16_t rcHighSwitchingThreshold){

	crst01a.SetRcConfig1(rcLowSwitchingThreshold, rcHighSwitchingThreshold);
}


// RC設定(0x5A,0x5B)
// 各種操作に割り当てるSBUSチャンネルを設定する。
// 引数：movementXChannel：X方向移動のチャンネル (0-15)
// 　　　movementYChannel：Y方向移動のチャンネル (0-15)
// 　　　movementYawChannel：旋回方向移動のチャンネル (0-15)
// 　　　controlModeSwitchChannel：モード切替のチャンネル (0-15)
// 　　　brakeControlChannel：自動ブレーキ制御のチャンネル (0-15)
// 　　　errorAndBumperResetChannel：エラー・バンパー解除のチャンネル (0-15)
// 　　　headlight0Channel：ヘッドライト0制御のチャンネル (0-15)
// 　　　headlight1Channel：ヘッドライト1制御のチャンネル (0-15)
// 戻り値：なし
void CugoCommon::SetRcConfig23(uint8_t movementXChannel, uint8_t movementYChannel, uint8_t movementYawChannel, uint8_t controlModeSwitchChannel, uint8_t brakeControlChannel, uint8_t errorAndBumperResetChannel, uint8_t headlight0Channel, uint8_t headlight1Channel){

	crst01a.SetRcConfig23(movementXChannel, movementYChannel, movementYawChannel, controlModeSwitchChannel, brakeControlChannel, errorAndBumperResetChannel, headlight0Channel, headlight1Channel);
}


// バンパー、ブレーキ設定取得
// SetBumperBrakeで設定したバンパー・ブレーキ設定を取得する。
// 引数：pBumperConfig：バンパー設定の格納先
// 　　　pBrakeConfig：ブレーキ設定の格納先
// 戻り値：成功時 true、タイムアウト時 false
bool CugoCommon::GetBumperBrake(uint8_t *pBumperConfig, uint8_t *pBrakeConfig){

	return crst01a.GetBumperBrake(pBumperConfig, pBrakeConfig);
}


// ブレーキの閾値設定取得
// SetBrakeThresholdで設定した停止判断の閾値を取得する。
// 引数：pJudgeToStopRpm：停止と判断する閾値 (rpm) の格納先
// 戻り値：成功時 true、タイムアウト時 false
bool CugoCommon::GetBrakeThreshold(float *pJudgeToStopRpm){

	return crst01a.GetMdConfig2(pJudgeToStopRpm);
}


// ランプ設定取得
// SetSpeedRampで設定したランプ設定を取得する。
// 引数：pSpeedRampA：ランプ設定A (RPM/s) の格納先
// 　　　pSpeedRampB：ランプ設定B (RPM/s) の格納先
// 　　　pSpeedRampC：ランプ設定C (RPM/s) の格納先
// 　　　pSpeedRampSelect：各減速要因のランプ選択の格納先
// 戻り値：成功時 true、タイムアウト時 false
bool CugoCommon::GetSpeedRamp(uint16_t *pSpeedRampA, uint16_t *pSpeedRampB, uint16_t *pSpeedRampC, uint16_t *pSpeedRampSelect){

	return crst01a.GetMdConfig5(pSpeedRampA, pSpeedRampB, pSpeedRampC, pSpeedRampSelect);
}


// RC設定取得(0xD8)
// SetRcConfig0で設定したSBUSのセンター・最小・最大値・不感帯を取得する。
// 引数：pRcCenterValue：SBUS中央値の格納先
// 　　　pRcMinValue：SBUS最小値の格納先
// 　　　pRcMaxValue：SBUS最大値の格納先
// 　　　pRcCenterMargin：不感帯幅の格納先
// 戻り値：成功時 true、タイムアウト時 false
bool CugoCommon::GetRcConfig0(uint16_t *pRcCenterValue, uint16_t *pRcMinValue, uint16_t *pRcMaxValue, uint16_t *pRcCenterMargin){

	return crst01a.GetRcConfig0(pRcCenterValue, pRcMinValue, pRcMaxValue, pRcCenterMargin);
}


// RC設定取得(0xD9)
// SetRcConfig1で設定したスイッチON/OFF閾値を取得する。
// 引数：pRcLowSwitchingThreshold：スイッチOFFとする閾値の格納先
// 　　　pRcHighSwitchingThreshold：スイッチONとする閾値の格納先
// 戻り値：成功時 true、タイムアウト時 false
bool CugoCommon::GetRcConfig1(uint16_t *pRcLowSwitchingThreshold, uint16_t *pRcHighSwitchingThreshold){

	return crst01a.GetRcConfig1(pRcLowSwitchingThreshold, pRcHighSwitchingThreshold);
}


// RC設定取得(0xDA,0xDB)
// SetRcConfig23で設定した各操作のSBUSチャンネル割当を取得する。
// 引数：pMovementXChannel：X方向移動のチャンネル格納先
// 　　　pMovementYChannel：Y方向移動のチャンネル格納先
// 　　　pMovementYawChannel：旋回方向移動のチャンネル格納先
// 　　　pControlModeSwitchChannel：モード切替のチャンネル格納先
// 　　　pBrakeControlChannel：自動ブレーキ制御のチャンネル格納先
// 　　　pErrorAndBumperResetChannel：エラー・バンパー解除のチャンネル格納先
// 　　　pHeadlight0Channel：ヘッドライト0制御のチャンネル格納先
// 　　　pHeadlight1Channel：ヘッドライト1制御のチャンネル格納先
// 戻り値：成功時 true、タイムアウト時 false
bool CugoCommon::GetRcConfig23(uint8_t *pMovementXChannel, uint8_t *pMovementYChannel, uint8_t *pMovementYawChannel, uint8_t *pControlModeSwitchChannel, uint8_t *pBrakeControlChannel, uint8_t *pErrorAndBumperResetChannel, uint8_t *pHeadlight0Channel, uint8_t *pHeadlight1Channel){

	return crst01a.GetRcConfig23(pMovementXChannel, pMovementYChannel, pMovementYawChannel, pControlModeSwitchChannel, pBrakeControlChannel, pErrorAndBumperResetChannel, pHeadlight0Channel, pHeadlight1Channel, 200);
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
