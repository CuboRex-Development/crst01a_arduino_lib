#include <CugoCommon.h>



void setup() {
	
	// 初期化処理
	cugoCommon.Init();
}

void loop() {

	cugoCommon.SetVoltageConfig(180,320);	// 18V以下、32V以上で電圧異常と判断する設定
	
	cugoCommon.SaveParamReq();		// フラッシュメモリに保存
	
	// 処理終了を知らせるLED点灯
	cugoCommon.LedPrint(false);
}
