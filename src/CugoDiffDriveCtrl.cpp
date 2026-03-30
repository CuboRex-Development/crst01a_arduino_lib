
#include <math.h>
#include "CuGoDiffDriveCtrl.h"
#include "CugoCommon.h"
#include "Crst01a.h"


#define PC_PRINTLN(str)	if(NULL != l_pSerial){l_pSerial->println(str);}
#define PC_PRINT(str)	if(NULL != l_pSerial){l_pSerial->print(str);}
#define RECV_TIMEOUT	(500)		// 受信した定期送信系データが古いと判断する閾値


CuGoDiffDriveCtrl cugoDiffDriveCtrl;


// コンストラクタ
// 引数：なし
CuGoDiffDriveCtrl::CuGoDiffDriveCtrl(void){
	l_xSpeed = 0;
	l_yawSpeed = 0;
}


// 初期化関数
// 引数：pSerial：ログの出力に使用するシリアルのポインタ。NULLを指定するとログを出力しない。
// 戻り値：なし
void CuGoDiffDriveCtrl::Init(HardwareSerial* pSerial){
	
	l_pSerial = pSerial;
	
	if(!crst01a.Init()){
		PC_PRINTLN(F("##################################"));
		PC_PRINTLN(F("###   CuGoDiffDriveCtrl Start  ###"));
		PC_PRINTLN(F("##################################"));
	}
}



// 前後進制御
// 引数：targetDistance：目標走行距離(m、-で後進)
// 　　　targetSpeed：目標速度(m/s、-で後進)
// 戻り値：エラー発生や速度の取得失敗でfalse
bool CuGoDiffDriveCtrl::MoveForward(float targetDistance, float targetSpeed){
	
	float runTime;
	int16_t xSpeed;
	int16_t ySpeed;
	int16_t yawSpeed;
	uint32_t recvTime;

	// エラーの確認
	if(cugoCommon.GetErr()){
		PC_PRINTLN(F("##WARNING::すでにエラーが発生しています。##"));
		return false;
	}

	// 時間の計算
	runTime = abs(targetDistance / targetSpeed);

	// 走行開始
	if(0 < targetDistance){
		SetMoveSpeed((int16_t)(targetSpeed * 1000), 0);
	}
	else{
		SetMoveSpeed((int16_t)(-targetSpeed * 1000), 0);
	}
	
	if(!cugoCommon.Wait((uint32_t)(runTime * 1000))){
		PC_PRINTLN(F("##WARNING::エラーが発生しました。走行を中断します。##"));
		return false;
	}
	
	// 減速開始
	SetMoveSpeed(0, 0);
	
	// 停止までの待ち
	while(1){
		crst01a.GetReadRunStatus(&xSpeed, &ySpeed, &yawSpeed, &recvTime);
		if(RECV_TIMEOUT < (millis() - recvTime)){
			// 最後に受信したのが古すぎる
			PC_PRINTLN(F("##WARNING::走行速度が正常に取得できませんでした。走行速度の定期送信を有効にしてください。##"));
			return false;
		}
		
		// エラーの確認
		if(cugoCommon.GetErr()){
			PC_PRINTLN(F("##WARNING::エラーが発生しました。##"));
			return false;
		}
		
		if(10 > xSpeed){
			// 停止したと判断
			return true;
		}
	}
	
	return true;
}


// 旋回制御
// 引数：targetDegree：目標角度(deg、+で半時計周り、-で時計回り)
// 　　　targetSpeed：目標速度(deg/s、-で反転)
// 戻り値：エラー発生や速度の取得失敗でfalse
bool CuGoDiffDriveCtrl::MoveTurn(float targetDegree, float targetSpeed){

	float runTime;
	int16_t xSpeed;
	int16_t ySpeed;
	int16_t yawSpeed;
	uint32_t recvTime;

	// エラーの確認
	if(cugoCommon.GetErr()){
		PC_PRINTLN(F("##WARNING::すでにエラーが発生しています。##"));
		return false;
	}

	// 時間の計算
	runTime = abs(targetDegree / targetSpeed);

	// 走行開始
	if(0 < targetDegree){
		SetMoveSpeed(0, (int16_t)(targetSpeed * 1000 * (2*M_PI) / 360));
	}
	else{
		SetMoveSpeed(0, (int16_t)(-targetSpeed * 1000 * (2*M_PI) / 360));
	}
	
	if(!cugoCommon.Wait((uint32_t)(runTime * 1000))){
		PC_PRINTLN(F("##WARNING::エラーが発生しました。走行を中断します。##"));
		return false;
	}
	
	// 減速開始
	SetMoveSpeed(0, 0);
	
	// 停止までの待ち
	while(1){
		crst01a.GetReadRunStatus(&xSpeed, &ySpeed, &yawSpeed, &recvTime);
		if(RECV_TIMEOUT < (millis() - recvTime)){
			// 最後に受信したのが古すぎる
			PC_PRINTLN(F("##WARNING::走行速度が正常に取得できませんでした。走行速度の定期送信を有効にしてください。##"));
			return false;
		}
		
		// エラーの確認
		if(cugoCommon.GetErr()){
			PC_PRINTLN(F("##WARNING::エラーが発生しました。##"));
			return false;
		}
		
		if(10 > yawSpeed){
			// 停止したと判断
			return true;
		}
	}
	
	return true;
}


// 円軌道での移動命令
// 引数：targetRadius：目標円軌道半径(m)
// 　　　targetDegree：目標円軌道角度(deg、+で半時計周り、-で時計回り)
// 　　　targetSpeed：目標速度(m/s、-で後進)
// 戻り値：エラー発生や速度の取得失敗でfalse
bool CuGoDiffDriveCtrl::MoveCurve(float targetRadius, float targetDegree, float targetSpeed){
	
	float runTime;
	int16_t xSpeed;
	int16_t ySpeed;
	int16_t yawSpeed;
	uint32_t recvTime;
	float targetDegreeRad = targetDegree * 2 * M_PI / 360;		// degをradに変換

	// エラーの確認
	if(cugoCommon.GetErr()){
		PC_PRINTLN(F("##WARNING::すでにエラーが発生しています。##"));
		return false;
	}

	// 時間の計算
	runTime = abs(targetDegreeRad / (targetSpeed / targetRadius));		// 角度/角速度

	// 走行開始
	if(0 < targetSpeed){
		SetMoveSpeed((int16_t)(targetSpeed * 1000), (int16_t)(targetDegreeRad / runTime * 1000));
	}
	else{
		SetMoveSpeed((int16_t)(targetSpeed * 1000), (int16_t)(-targetDegreeRad / runTime * 1000));
	}
	
	if(!cugoCommon.Wait((uint32_t)(runTime * 1000))){
		PC_PRINTLN(F("##WARNING::エラーが発生しました。走行を中断します。##"));
		return false;
	}
	
	// 減速開始
	SetMoveSpeed(0,0);
	
	// 停止までの待ち
	while(1){
		crst01a.GetReadRunStatus(&xSpeed, &ySpeed, &yawSpeed, &recvTime);
		if(RECV_TIMEOUT < (millis() - recvTime)){
			// 最後に受信したのが古すぎる
			PC_PRINTLN(F("##WARNING::走行速度が正常に取得できませんでした。走行速度の定期送信を有効にしてください。##"));
			return false;
		}
		
		// エラーの確認
		if(cugoCommon.GetErr()){
			PC_PRINTLN(F("##WARNING::エラーが発生しました。##"));
			return false;
		}
		
		if(10 > yawSpeed){
			// 停止したと判断
			return true;
		}
	}
	
	return true;
}


// 停止命令
// 引数：なし
// 戻り値：エラー発生でfalse
bool CuGoDiffDriveCtrl::Stop(void){

	if(cugoCommon.GetErr()){
		PC_PRINTLN(F("##WARNING::すでにエラーが発生しています。##"));
		return false;
	}
	
	crst01a.SetMoveSpeed(0, 0, 0);	// ストップはコマンドモードに依存しないでほしいので直接
	
	return true;
}



// 順運動学、逆運運動学設定関数
// 引数：leftTireDiameter：左タイヤ直径 (m)
// 　　　rightTireDiameter：右タイヤ直径 (m)
// 　　　leftGearRatio：左減速比 (減速時に1未満)
// 　　　rightGearRatio：右減速比 (減速時に1未満)
// 　　　tread：トレッド (m)
// 戻り値：なし
void CuGoDiffDriveCtrl::SetKinematics(float leftTireDiameter, float rightTireDiameter, float leftGearRatio, float rightGearRatio, float tread){
	float fwdKinematics[12];
	float InvKinematics[12];
	
	fwdKinematics[0] = 0;
	fwdKinematics[1] = M_PI * leftTireDiameter * leftGearRatio / 120;
	fwdKinematics[2] = -M_PI * rightTireDiameter * rightGearRatio / 120;
	fwdKinematics[3] = 0;
	fwdKinematics[4] = 0;
	fwdKinematics[5] = 0;
	fwdKinematics[6] = 0;
	fwdKinematics[7] = 0;
	fwdKinematics[8] = 0;
	fwdKinematics[9] = -M_PI * leftTireDiameter * leftGearRatio / 60 / tread;
	fwdKinematics[10] = -M_PI * rightTireDiameter * rightGearRatio / 60 / tread;
	fwdKinematics[11] = 0;
	
	InvKinematics[0] = 0;
	InvKinematics[1] = 0;
	InvKinematics[2] = 0;
	InvKinematics[3] = 60 / (M_PI * leftTireDiameter * leftGearRatio);
	InvKinematics[4] = 0;
	InvKinematics[5] = -30 * tread / (M_PI * leftTireDiameter * leftGearRatio);
	InvKinematics[6] = -60 / (M_PI * rightTireDiameter * rightGearRatio);
	InvKinematics[7] = 0;
	InvKinematics[8] = -30 * tread / (M_PI * rightTireDiameter * rightGearRatio);
	InvKinematics[9] = 0;
	InvKinematics[10] = 0;
	InvKinematics[11] = 0;
	
	crst01a.SetFwdKinematics(fwdKinematics);
	crst01a.SetInvKinematics(InvKinematics);
}


void CuGoDiffDriveCtrl::SetMoveSpeed(int16_t xSpeed, int16_t yawSpeed){
	
	uint8_t mode;
	
	if(!cugoCommon.GetControlMode(&mode)){
		PC_PRINTLN(F("##WARNING::モードの取得に失敗。##"));
		return;
	}
	
	if(CUGO_CMD_MODE == mode){
		
		l_xSpeed = xSpeed;
		l_yawSpeed = yawSpeed;
		
		crst01a.SetMoveSpeed(l_xSpeed, 0, l_yawSpeed);
	}
	else{
		if(cugoCommon.GetSwitchReset()){
			return;			// リセットを実行すために処理を飛ばす
		}
		else{
			l_xSpeed = xSpeed;
			l_yawSpeed = yawSpeed;
		}
	}
}