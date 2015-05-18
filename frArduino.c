#ifdef configUSE_APPLICATION_TASK_TAG
	#undef configUSE_APPLICATION_TASK_TAG
#endif
#define configUSE_APPLICATION_TASK_TAG 1

#include "FreeRTOS_AVR.h"
#include "basic_io_avr.h"

#define traceTASK_SWITCHED_IN() vSetDigitalOutput( (int)pxCurrentTCB->pxTaskTag)

int luminositySensorPin = A0;
int temperatureSensorPin = A1;
int opticalSwicthSensorPin = A2;

int luminosityTaskON = 12;
int temperatureTaskON = 11;
int serialInfoTaskON = 10;
int opticalSwicthTaskON = 9;
int serialMsgTaskON = 8;
int idleTaskON = 13;

unsigned int luminosity = 0;
unsigned int temperature = 0;
unsigned int Counter = 0;
boolean opticalSwitch;

SemaphoreHandle_t xSemaphoreLuminosity;
SemaphoreHandle_t xSemaphoreTemperature;
//SemaphoreHandle_t xMUTEX;

//------------------
//Reading luminositysensor value
static void vLuminosityTask(void *pvParameters) {
	vTaskSetApplicationTaskTag(NULL, (TaskHookFunction_t)1);
	//Create mutex
	while (1) {
		if (xSemaphoreTake(xSemaphoreLuminosity, portMAX_DELAY == pdTRUE))
		{
                  luminosity = analogRead(luminositySensorPin);
		  Serial.println("TASK-A : ----------------------------");
		  xSemaphoreGive(xSemaphoreLuminosity);
		}
		vTaskDelay(configTICK_RATE_HZ / 100); //10ms second waiting
	}
}

//---------------
//Reading void vTemperatureTask(void*pvParameter)
static void vTemperatureTask(void *pvParameters) {
	vTaskSetApplicationTaskTag(NULL, (TaskHookFunction_t) 2);
	while (1) {
		if (xSemaphoreTake(xSemaphoreTemperature, portMAX_DELAY ==pdTRUE))
		{
			temperature = analogRead(temperatureSensorPin);
                        Serial.println("TASK-B : ----------------------------");
			xSemaphoreGive(xSemaphoreTemperature);
		}
		vTaskDelay(configTICK_RATE_HZ / 100); //10ms
	}
}
//---------------
//Reading optocaptor sensor value
static void vOpticalSwitchTask(void *pvParameters) {
	vTaskSetApplicationTaskTag(NULL, (TaskHookFunction_t)3);
	unsigned int temp;
	while (1) {
		Serial.println("TASK-C : ----------------------------");
		if (analogRead(opticalSwicthSensorPin) < 50) {
			opticalSwitch = true;
			Serial.println("Error");
		} else
			opticalSwitch = false;
		vTaskDelay(configTICK_RATE_HZ / 100);  //10ms
	}
}
//---------------
//Send informations via Serial Interface
static void vSerialInfoTask(void *pvParameter) {
	vTaskSetApplicationTaskTag(NULL,(TaskHookFunction_t)4);
	while (1) {
		if ( (xSemaphoreTake (xSemaphoreLuminosity, portMAX_DELAY ==pdTRUE)) and (xSemaphoreTake(xSemaphoreTemperature, portMAX_DELAY ==pdTRUE)) )
		{
			Serial.println("TASK-D : ----------------------------");
			Serial.print("Luminosity:");
			Serial.println(luminosity);
			Serial.print("Temperature:");
			Serial.println(temperature);
			Serial.print("Optical swicth: ");
			Serial.println(opticalSwitch);
			xSemaphoreGive(xSemaphoreLuminosity);
			xSemaphoreGive(xSemaphoreTemperature);
		}
		vTaskDelay(configTICK_RATE_HZ);  //1 second waiting
	}
}
//-----------------
/*
static void vSerialMsgCheck(void *pvParameters) {
	vTaskSetApplicationTaskTag(NULL,(TaskHookFunction_t)5);
	while (1) {
		Serial.println("TASK-E : ----------------------------");
		
                if (Serial.available() > 0) {
			int receivedByte = Serial.read();
			if (receivedByte == 's') {
				delay(3000);  //Force waiting 3 second
			}
		}                
		vTaskDelay(configTICK_RATE_HZ / 200);  //5ms
	}
}
*/

void vIdleTask(void *pvParameters) {
	vTaskSetApplicationTaskTag(NULL, (TaskHookFunction_t) 6);
        //int X=0;
	while (1) 
        {
          Serial.println("IDLE");
          //Serial.println(++Counter);
          //vSetDigitalOutput(6);
        }
}
//------------
void setup() {
	//Initilization of the series communication with 9600kbits as baud
	// Serial.begin(9600);
	Serial.begin(115200);


	pinMode(luminosityTaskON, OUTPUT);
	pinMode(temperatureTaskON, OUTPUT);
	pinMode(serialInfoTaskON, OUTPUT);
	pinMode(opticalSwicthTaskON, OUTPUT);
	pinMode(serialMsgTaskON, OUTPUT);
	pinMode(idleTaskON, OUTPUT);

	/*
	 //Configurating HIGHSPEED
	 bitClear(ADCSRA, ADPS0);
	 bitClear(ADCSRA, ADPS1);
	 bitSet(ADCSRA,ADPS2);
	 analogReference(EXTERNAL);
	 */

	Serial.print("Create MUTEX\r\n");
	delay(1000);

	xSemaphoreLuminosity = xSemaphoreCreateMutex();
	xSemaphoreTemperature = xSemaphoreCreateMutex();
	
	if ((xSemaphoreLuminosity != NULL) && (xSemaphoreTemperature != NULL)) {
		Serial.print("Create TASK\r\n");
		delay(100);

		xTaskCreate(vLuminosityTask, "luminosityTask", configMINIMAL_STACK_SIZE + 50, NULL, 3, NULL);
		xTaskCreate(vTemperatureTask, "temperatureTask", configMINIMAL_STACK_SIZE + 50, NULL, 3, NULL);
		xTaskCreate(vOpticalSwitchTask, "opticalSwitchTask", configMINIMAL_STACK_SIZE + 50, NULL, 2, NULL);
		//xTaskCreate(vSerialMsgCheck, "serialMsgCheckTask", configMINIMAL_STACK_SIZE + 50, NULL, 1, NULL);
		xTaskCreate(vSerialInfoTask, "serialTask", configMINIMAL_STACK_SIZE + 50, NULL, 1, NULL);
		xTaskCreate(vIdleTask, "idleTask", configMINIMAL_STACK_SIZE + 50, NULL, 0, NULL);

		Serial.print("Start SCHEDULER\r\n");
		delay(100);
		//start FreeRTOS
		vTaskStartScheduler();

	}

	Serial.println("Die");
	while (1)
		;
}
void vSetDigitalOutput(int task) {
	digitalWrite(temperatureTaskON, LOW);
	digitalWrite(serialInfoTaskON, LOW);
	digitalWrite(opticalSwicthTaskON, LOW);
	digitalWrite(serialMsgTaskON, LOW);
	digitalWrite(idleTaskON, LOW);
	digitalWrite(luminosityTaskON, LOW);
	switch (task) {
	case 1:
		digitalWrite(luminosityTaskON, HIGH);
		break;
	case 2:
		digitalWrite(temperatureTaskON, HIGH);
		break;
	case 3:
		digitalWrite(serialInfoTaskON, HIGH);
		break;
	case 4:
		digitalWrite(opticalSwicthTaskON, HIGH);
		break;
	case 5:
		digitalWrite(serialMsgTaskON, HIGH);
		break;
	case 6:
		digitalWrite(idleTaskON, HIGH);
		break;
	default:
		break;
	}
}

//#define traceTASK_SWITCHED_IN() vSetDigitalOutput((int) pxCurrentTCB->pxTaskTag);
void loop() {
	//Not use
}

