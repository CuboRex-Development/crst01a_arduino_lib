#include <CugoCommon.h>
#include <CugoDiffDriveCtrl.h>



void setup() {
	
	// 初期化処理
	cugoDiffDriveCtrl.Init();
	cugoCommon.Init();
}

void loop() {

	cugoDiffDriveCtrl.SetKinematics(0.07716,0.07716,0.1144064068,0.1144064068,0.38);	// V4.5デフォルト設定。トレッド幅は0.38m
//	cugoDiffDriveCtrl.SetKinematics(0.07716,0.07716,0.1144064068,0.1144064068,0.5);		// トレッド幅を0.5mに設定する場合
	
//	cugoCommon.SaveParamReq();				// フラッシュメモリに保存
	
	// 処理終了を知らせるLED点灯
	cugoCommon.LedPrint(false);
}
