#include <CugoCommon.h>



void setup() {
	
	// 初期化処理
	cugoCommon.Init();
}

void loop() {

	cugoCommon.SetVoltage(18,32);	// 18V以下、32V以上で電圧異常と判断する設定
	
	cugoCommon.SaveParamReq();		// フラッシュメモリに保存
	
	// 処理終了を知らせるLED点滅
	while(1){
		digitalWrite(LED_BUILTIN, HIGH);
		delay(1000);
		digitalWrite(LED_BUILTIN, LOW);
		delay(1000);
	}
}
