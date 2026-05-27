#include <CugoCommon.h>



void setup() {
	
	Serial.begin(115200);
	
	// 初期化処理
	cugoCommon.Init();
}

void loop() {

	cugoCommon.SetVoltage(20,33);	// 18V以下、32V以上で電圧異常と判断
	
	cugoCommon.SaveParamReq();				// フラッシュメモリに保存
	
	// 処理終了
	while(1){
		digitalWrite(LED_BUILTIN, HIGH);
		delay(1000);
		digitalWrite(LED_BUILTIN, LOW);
		delay(1000);
	}
}
