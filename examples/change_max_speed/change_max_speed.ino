#include <CugoCommon.h>
#include <CugoDiffDriveCtrl.h>



void setup() {
	
	Serial.begin(115200);
	
	// 初期化処理
	cugoDiffDriveCtrl.Init();
	cugoCommon.Init();
}

void loop() {

//	cugoDiffDriveCtrl.SetMaxSpeed(1000,1500);	// 3.6km/h
	cugoDiffDriveCtrl.SetMaxSpeed(1222,1500);	// 4.4km/h
	
	cugoCommon.SaveParamReq();				// フラッシュメモリに保存
	
	// 処理終了
	while(1){
		digitalWrite(LED_BUILTIN, HIGH);
		delay(1000);
		digitalWrite(LED_BUILTIN, LOW);
		delay(1000);
	}
}
