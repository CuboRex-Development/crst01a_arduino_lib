#include <CugoCommon.h>
#include <CugoDiffDriveCtrl.h>



void setup() {
	
	// 初期化処理
	cugoDiffDriveCtrl.Init();
	cugoCommon.Init();
}

void loop() {

//	cugoDiffDriveCtrl.SetMaxSpeed(1000,1500);	// 走行速度を1.0m/s(3.6km/h)と1.5rad/sに設定
	cugoDiffDriveCtrl.SetMaxSpeed(1222,1500);	// 走行速度を1.222m/s(4.4km/h)と1.5rad/sに設定
	
	cugoCommon.SaveParamReq();				// フラッシュメモリに保存
	
	// 処理終了を知らせるLED点灯
	cugoCommon.LedPrint(false);
}
