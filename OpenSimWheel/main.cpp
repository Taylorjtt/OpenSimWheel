
#include "Hardware/OSWHardware.hh"
#include "Hardware/TMS320F2806.hh"
#include "Scheduler/TaskTable.h"
extern "C"
{
	#include "control/TorqueController.h"
	#include "Math/IQmathLib.h"

}
#include "Scheduler/SerialSendTask.h"


#define ENCODER_CPR 5000

TMS320F2806 processor;
OSWDigital digital;
OSWSerial serial;
QuadratureEncoder encoder;
OSWInverter inverter;
DRV8301 driver;
Spi spi;
CurrentSensor currentSensor;
SerialSendTask serialTask;
TaskTable taskTable;
TorqueControllerHandle torqueController;
float a = 0;
float b = 0;
float c = 0;
float ia = 0;
float ib = 0;
void setupGPIO()
{
	digital.setMode(GPIO_Number_33,GPIO_33_Mode_GeneralPurpose);
	digital.setMode(GPIO_Number_34,GPIO_34_Mode_GeneralPurpose);
	digital.setMode(GPIO_Number_39,GPIO_39_Mode_GeneralPurpose);
	digital.setDirection(GPIO_Number_33,GPIO_Direction_Output);
	digital.setDirection(GPIO_Number_34,GPIO_Direction_Output);
	digital.setDirection(GPIO_Number_39,GPIO_Direction_Output);
}
int main(void)
 {
	torqueController = (TorqueControllerHandle)malloc(sizeof(TorqueController_Obj));
	processor = TMS320F2806();
	digital = OSWDigital();
	processor.setup(PLL_ClkFreq_80_MHz);
	processor.setupTimer0();
	setupGPIO();

	serial = OSWSerial(processor,digital,SCI_BaudRate_115_2_kBaud);
	encoder = QuadratureEncoder(processor,digital,ENCODER_CPR);
	inverter = OSWInverter(processor, digital,80,50,1);
	spi = Spi(processor, digital);
	driver = DRV8301(processor, digital,spi);
	currentSensor = CurrentSensor(processor,digital,inverter);


	torqueController = TorqueController_Constructor((void *)torqueController, sizeof(TorqueController_Obj));

	serialTask =  SerialSendTask(FREQ_100HZ,0,serial,digital);
	taskTable.addTask(serialTask);

	while(true)
	{
		TorqueController_doControl(torqueController, ia, ib);
		inverter.modulate(a,b,c);
		taskTable.execute(processor);
	}
 }
