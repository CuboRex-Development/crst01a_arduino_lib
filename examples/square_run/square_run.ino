#include <CugoCommon.h>
#include <CugoDiffDriveCtrl.h>



void setup() {
	
	Serial.begin(115200);
	
	// 初期化処理
	cugoDiffDriveCtrl.Init();
	cugoCommon.Init();
	
	cugoCommon.SetSwitchReset(true);	// プロポでRCモード切替時に最初から実行したい場合はtrue、動作を再開する場合はfalse
}

void loop() {

	uint8_t mode;
	
	// cugoCommon.GetControlModeで現在のモード設定を取得
	if(!cugoCommon.GetControlMode(&mode)){
		Serial.println(F("##WARNING::車両コントローラとの通信異常##"));
	}
	
	if(CUGO_CMD_MODE == mode){
		// コマンドモードの処理
		
		//ここから自動走行モードの記述
		//サンプルコード記載
		Serial.println(F("自動走行モード開始"));
		cugoCommon.Wait(1000);

		Serial.println(F("1.0mの正方形移動の実施"));
		cugoDiffDriveCtrl.MoveForward(1.0);
		cugoCommon.Wait(1000);
		cugoDiffDriveCtrl.MoveTurn(90);
		cugoCommon.Wait(1000);
		cugoDiffDriveCtrl.MoveForward(1.0);
		cugoCommon.Wait(1000);
		cugoDiffDriveCtrl.MoveTurn(90);
		cugoCommon.Wait(1000);
		cugoDiffDriveCtrl.MoveForward(1.0);
		cugoCommon.Wait(1000);
		cugoDiffDriveCtrl.MoveTurn(90);
		cugoCommon.Wait(1000);
		cugoDiffDriveCtrl.MoveForward(1.0);
		cugoCommon.Wait(1000);
		cugoDiffDriveCtrl.MoveTurn(90);
		cugoCommon.Wait(1000);
		
		cugoDiffDriveCtrl.Stop();
		Serial.println(F("自動走行モード終了"));
		
		//サンプルコード終了
		
		//自動走行を1回のloopで終了する場合のみ記載、不要の場合コメントアウト
		while(1){
			if(!cugoCommon.GetControlMode(&mode)){
				Serial.println(F("##WARNING::車両コントローラとの通信異常##"));
			}
			if(CUGO_RC_MODE == mode){
				break;
			}
		}
	}
	else if(CUGO_RC_MODE == mode){
		// RCモードの処理
	}
}
