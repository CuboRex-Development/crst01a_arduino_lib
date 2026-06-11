
#include <math.h>
#include "CugoDiffDriveCtrl.h"
#include "CugoCommon.h"
#include "Crst01a.h"


#define PC_PRINTLN(str)	if(NULL != l_pSerial){l_pSerial->println(str);}
#define PC_PRINT(str)	if(NULL != l_pSerial){l_pSerial->print(str);}
#define RECV_TIMEOUT	(500)		// 受信した定期送信系データが古いと判断する閾値


CugoDiffDriveCtrl cugoDiffDriveCtrl;


// コンストラクタ
// 引数：なし
CugoDiffDriveCtrl::CugoDiffDriveCtrl(void){
	l_xSpeed = 0;
	l_yawSpeed = 0;
}


// 初期化関数
// 引数：pSerial：ログの出力に使用するシリアルのポインタ。NULLを指定するとログを出力しない。
// 戻り値：なし
void CugoDiffDriveCtrl::Init(HardwareSerial* pSerial){

	l_pSerial = pSerial;

	if(!crst01a.Init()){
		PC_PRINTLN(F("##################################"));
		PC_PRINTLN(F("###   CugoDiffDriveCtrl Start  ###"));
		PC_PRINTLN(F("##################################"));
	}

	// 走行制御で使用する定期送信を有効化する
	// 定期送信設定(0x40)は車両コントローラ側で全置換となるため、既定で有効なシステムステータス(0x80)・
	// 走行状態(0x81)も合わせて有効化する。
	// モータ出力(0x90-0x93)・エンコーダ(0x88,0x89)は受信時刻を全モータ分で評価するため、全て有効化する。
	crst01a.SetCycleReq(CRST_FUNC_READ_SYS_STATUS);		// システムステータス(0x80)
	crst01a.SetCycleReq(CRST_FUNC_READ_RUN_STATUS);		// 走行状態(0x81)
	crst01a.SetCycleReq(CRST_FUNC_READ_ENCODER_01);		// エンコーダ(0x88)
	crst01a.SetCycleReq(CRST_FUNC_READ_ENCODER_23);		// エンコーダ(0x89)
	crst01a.SetCycleReq(CRST_FUNC_READ_MD_TEMP);		// モータドライバ温度(0x8C)
	crst01a.SetCycleReq(CRST_FUNC_READ_MD_STATUS);		// モータドライバ状態(0x8E)
	crst01a.SetCycleReq(CRST_FUNC_READ_MOTOR_OUT_0);	// モータ出力(0x90)
	crst01a.SetCycleReq(CRST_FUNC_READ_MOTOR_OUT_1);	// モータ出力(0x91)
	crst01a.SetCycleReq(CRST_FUNC_READ_MOTOR_OUT_2);	// モータ出力(0x92)
	crst01a.SetCycleReq(CRST_FUNC_READ_MOTOR_OUT_3);	// モータ出力(0x93)
}



// 前後進制御
// 引数：targetDistance：目標走行距離(m、-で後進)
// 　　　targetSpeed：目標速度(m/s、-で後進)
// 戻り値：エラー発生や速度の取得失敗でfalse
bool CugoDiffDriveCtrl::MoveForward(float targetDistance, float targetSpeed){
	
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

	// 引数チェック
	if(0.0f == targetSpeed){
		PC_PRINTLN(F("##WARNING::targetSpeed が0です。##"));
		return false;
	}

	// 時間の計算
	runTime = fabsf(targetDistance / targetSpeed);

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
		if(!crst01a.GetReadRunStatus(&xSpeed, &ySpeed, &yawSpeed, &recvTime) || (RECV_TIMEOUT < (millis() - recvTime))){
			// 未受信、または最後に受信したのが古すぎる
			PC_PRINTLN(F("##WARNING::走行速度が正常に取得できませんでした。走行速度の定期送信を有効にしてください。##"));
			return false;
		}
		
		// エラーの確認
		if(cugoCommon.GetErr()){
			PC_PRINTLN(F("##WARNING::エラーが発生しました。##"));
			return false;
		}
		
		if(10 > abs(xSpeed)){
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
bool CugoDiffDriveCtrl::MoveTurn(float targetDegree, float targetSpeed){

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

	// 引数チェック
	if(0.0f == targetSpeed){
		PC_PRINTLN(F("##WARNING::targetSpeed が0です。##"));
		return false;
	}

	// 時間の計算
	runTime = fabsf(targetDegree / targetSpeed);

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
		if(!crst01a.GetReadRunStatus(&xSpeed, &ySpeed, &yawSpeed, &recvTime) || (RECV_TIMEOUT < (millis() - recvTime))){
			// 未受信、または最後に受信したのが古すぎる
			PC_PRINTLN(F("##WARNING::走行速度が正常に取得できませんでした。走行速度の定期送信を有効にしてください。##"));
			return false;
		}
		
		// エラーの確認
		if(cugoCommon.GetErr()){
			PC_PRINTLN(F("##WARNING::エラーが発生しました。##"));
			return false;
		}
		
		if(10 > abs(yawSpeed)){
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
bool CugoDiffDriveCtrl::MoveCurve(float targetRadius, float targetDegree, float targetSpeed){
	
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

	// 引数チェック
	if(0.0f == targetSpeed || 0.0f == targetRadius){
		PC_PRINTLN(F("##WARNING::targetSpeed または targetRadius が0です。##"));
		return false;
	}

	// 時間の計算
	runTime = fabsf(targetDegreeRad / (targetSpeed / targetRadius));		// 角度/角速度

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
		if(!crst01a.GetReadRunStatus(&xSpeed, &ySpeed, &yawSpeed, &recvTime) || (RECV_TIMEOUT < (millis() - recvTime))){
			// 未受信、または最後に受信したのが古すぎる
			PC_PRINTLN(F("##WARNING::走行速度が正常に取得できませんでした。走行速度の定期送信を有効にしてください。##"));
			return false;
		}
		
		// エラーの確認
		if(cugoCommon.GetErr()){
			PC_PRINTLN(F("##WARNING::エラーが発生しました。##"));
			return false;
		}
		
		if(10 > abs(yawSpeed)){
			// 停止したと判断
			return true;
		}
	}

	return true;
}


// 停止命令
// 引数：なし
// 戻り値：エラー発生でfalse
bool CugoDiffDriveCtrl::Stop(void){

	if(cugoCommon.GetErr()){
		PC_PRINTLN(F("##WARNING::すでにエラーが発生しています。##"));
		return false;
	}
	
	crst01a.SetMoveSpeed(0, 0, 0);	// ストップはコマンドモードに依存しないでほしいので直接
	
	return true;
}



// 順運動学、逆運動学設定関数
// 引数：leftTireDiameter：左タイヤ直径 (m)
// 　　　rightTireDiameter：右タイヤ直径 (m)
// 　　　leftGearRatio：左減速比 (減速時に1未満)
// 　　　rightGearRatio：右減速比 (減速時に1未満)
// 　　　tread：トレッド (m)
// 戻り値：なし
void CugoDiffDriveCtrl::SetKinematics(float leftTireDiameter, float rightTireDiameter, float leftGearRatio, float rightGearRatio, float tread){
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


void CugoDiffDriveCtrl::SetMoveSpeed(int16_t xSpeed, int16_t yawSpeed){
	
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


// 最大速度変更
// 引数：xSpeed：前進方向の最大速度 (値×0.001m/s)
// 　　　yawSpeed：最大旋回速度 (値×0.001rad/s)
// 戻り値：なし
void CugoDiffDriveCtrl::SetMaxSpeed(uint16_t xSpeed, uint16_t yawSpeed){
	crst01a.SetSpeed(xSpeed, 0, yawSpeed);
}


// 最大速度取得
// 引数：xSpeed：進行方向最大速度格納先
// 　　　yawSpeed：最大旋回速度格納先
// 戻り値：成功時 true、タイムアウト時 false
bool CugoDiffDriveCtrl::GetMaxSpeed(uint16_t *pXSpeed, uint16_t *pYawSpeed){

	uint16_t ySpeed;

	return crst01a.GetMaxSpeed(pXSpeed, &ySpeed, pYawSpeed);
}


// エンコーダ読み出し
// モータエンコーダ(0x88,0x89)のカウント値を取得する。右モータ=モータ2、左モータ=モータ1。
// 引数：pRightEncoder：右モータ(モータ2)のエンコーダ値格納先
// 　　　pLeftEncoder：左モータ(モータ1)のエンコーダ値格納先
// 戻り値：データが新しい場合にtrue、古い(取得失敗)場合にfalse
bool CugoDiffDriveCtrl::GetEncoder(uint32_t *pRightEncoder, uint32_t *pLeftEncoder){

	uint32_t motorEncoder[4];
	uint32_t recvTime;

	bool received = crst01a.GetEncoder(motorEncoder, &recvTime);

	*pRightEncoder = motorEncoder[2];
	*pLeftEncoder = motorEncoder[1];

	if(!received || (RECV_TIMEOUT < (millis() - recvTime))){
		// 未受信、または最後に受信したのが古すぎる
		return false;
	}

	return true;
}


// モータドライバ温度読み出し
// モータドライバ温度(0x8C)を取得する。右モータ=モータ2、左モータ=モータ1。
// 引数：pRightTemp：右モータ(モータ2)の温度格納先 (℃)
// 　　　pLeftTemp：左モータ(モータ1)の温度格納先 (℃)
// 戻り値：データが新しい場合にtrue、古い(取得失敗)場合にfalse
bool CugoDiffDriveCtrl::GetMotorTemp(uint16_t *pRightTemp, uint16_t *pLeftTemp){

	uint16_t motorTemp[4];
	uint32_t recvTime;

	bool received = crst01a.GetMdTemp(motorTemp, &recvTime);

	*pRightTemp = motorTemp[2];
	*pLeftTemp = motorTemp[1];

	if(!received || (RECV_TIMEOUT < (millis() - recvTime))){
		// 未受信、または最後に受信したのが古すぎる
		return false;
	}

	return true;
}


// モータドライバ状態読み出し
// モータドライバから受信したエラーコード(0x8E)を取得する。右モータ=モータ2、左モータ=モータ1。
// 引数：pRightError：右モータ(モータ2)のエラーコード格納先
// 　　　pLeftError：左モータ(モータ1)のエラーコード格納先
// 戻り値：データが新しい場合にtrue、古い(取得失敗)場合にfalse
bool CugoDiffDriveCtrl::GetMotorError(uint16_t *pRightError, uint16_t *pLeftError){

	uint16_t motorErr[4];
	uint32_t recvTime;

	bool received = crst01a.GetMdStatus(motorErr, &recvTime);

	*pRightError = motorErr[2];
	*pLeftError = motorErr[1];

	if(!received || (RECV_TIMEOUT < (millis() - recvTime))){
		// 未受信、または最後に受信したのが古すぎる
		return false;
	}

	return true;
}


// モータ出力読み出し
// モータの角速度とトルク(0x90-0x93)を取得する。右モータ=モータ2、左モータ=モータ1。
// 引数：pRightSpeed：右モータ(モータ2)の角速度格納先 (rpm)
// 　　　pRightTorque：右モータ(モータ2)のトルク格納先 (A)
// 　　　pLeftSpeed：左モータ(モータ1)の角速度格納先 (rpm)
// 　　　pLeftTorque：左モータ(モータ1)のトルク格納先 (A)
// 戻り値：データが新しい場合にtrue、古い(取得失敗)場合にfalse
bool CugoDiffDriveCtrl::GetMotorOut(float *pRightSpeed, float *pRightTorque, float *pLeftSpeed, float *pLeftTorque){

	float motorSpeed[4];
	float motorTorque[4];
	uint32_t recvTime;

	bool received = crst01a.GetMotorOut(motorSpeed, motorTorque, &recvTime);

	*pRightSpeed = motorSpeed[2];
	*pRightTorque = motorTorque[2];
	*pLeftSpeed = motorSpeed[1];
	*pLeftTorque = motorTorque[1];

	if(!received || (RECV_TIMEOUT < (millis() - recvTime))){
		// 未受信、または最後に受信したのが古すぎる
		return false;
	}

	return true;
}