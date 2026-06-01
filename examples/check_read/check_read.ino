#include <CugoCommon.h>
#include <CugoDiffDriveCtrl.h>

bool err;

void setup() {
	
	Serial.begin(115200);
	
	// 初期化処理
	cugoDiffDriveCtrl.Init();
	cugoCommon.Init();
	
	delay(5000);	// 少し待つ
	
	uint8_t ver0,ver1,ver2;
	uint16_t x,yaw;
	uint16_t driverMinVoltage,driverMaxVoltage;
	bool ret0,ret1,ret2;
	
	ret0 = cugoCommon.GetVersion(&ver0, &ver1, &ver2);
	ret1 = cugoDiffDriveCtrl.GetMaxSpeed(&x, &yaw);
	ret2 = cugoCommon.GetVoltageConfig(&driverMinVoltage, &driverMaxVoltage);

	Serial.println("===============================");
	if(ret0 && ret1 &&ret2){
		Serial.print("バージョン：");
		Serial.print(ver0);
		Serial.print(".");
		Serial.print(ver1);
		Serial.print(".");
		Serial.println(ver2);
		
		Serial.print("走行速度：");
		Serial.print((float)x/1000, 3);
		Serial.println("m/s");
		Serial.print("旋回速度：");
		Serial.print((float)yaw/1000, 3);
		Serial.println("rad/s");
		
		Serial.print("電圧異常の下限：");
		Serial.print(driverMinVoltage);
		Serial.println("V");
		Serial.print("電圧異常の上限：");
		Serial.print(driverMaxVoltage);
		Serial.println("V");
		
		err = false;
	}
	else{
		Serial.println("読み出しに失敗しました。");
		
		err = true;
	}
	Serial.println("===============================");
}

void loop() {
	cugoCommon.LedPrint(err);
}
