#include <Wire.h>
#include <BH1750FVI.h>
#include <EEPROM.h>
#define ideal 70

BH1750FVI lightSensor2;
//BH1750FVI lightSensor2;
int D0 = 3, D1 = 4, D2 = 5, D3 = 6, check = 0, lx;
int stad[16], arr[4] = { 1, 1, 1, 1 }, obt, clevel = 15, pirPin1 = 2, pirPin2 = 2, ledPin = 12;
void setup()
{
	//pir pins
	pinMode(pirPin1, INPUT);
	pinMode(pirPin2, INPUT);
	//led pin
	pinMode(ledPin, OUTPUT);
	// voltage modulator pins
	pinMode(D0, OUTPUT);
	pinMode(13, OUTPUT);
	pinMode(D1, OUTPUT);
	pinMode(D2, OUTPUT);
	pinMode(D3, OUTPUT);

	Serial.begin(9600);
/*	//light sensor2r2r2r2
	lightSensor2.begin();
	lightSensor2.SetAddress(Device_Address_H);
	lightSensor2.SetMode(Continuous_H_resolution_Mode);*/
	// light sensor2
	lightSensor2.begin();
	lightSensor2.SetAddress(Device_Address_L);
	lightSensor2.SetMode(Continuous_H_resolution_Mode);
 
	//reading rom data to standrad array
	for (int i = 0; i < 16; i++)
		stad[i] = EEPROM.read(i) * 256 + EEPROM.read(i + 16);

	setVoltage(15);
       Serial.println("started");
}
void loop()
{
	int16_t lux = getAvgLux();
              

	if (digitalRead(pirPin1) || digitalRead(pirPin2))
	{
		check = 0;
		Serial.println("...");	
            digitalWrite(ledPin,HIGH);
             if (lux < ideal - 5)
		{
			clevel = incLux(0, ideal - lux, clevel);
			delay(500);
			obt = getAvgLux();
		}
		else if (lux > obt + (stad[clevel] - stad[clevel + 1]))
		{
			lux = getAvgLux();
			clevel = decLux(lux, clevel);
			if (clevel == 15)
				obt = 0;
			else
				obt = getAvgLux();
			lx = lightSensor2.GetLightIntensity();
		}
		else if (lux < obt - 10)
		{
			lux = getAvgLux();
			clevel = incLux(obt, lux, clevel);
			obt = getAvgLux();
		}          
	}
      else
      {       
		  check++; 
		  if (check >= 10){
			  
			  setVoltage(15);
			  arr[0] = 1, arr[1] = 1, arr[2] = 1, arr[3] = 1;
			  obt = 0;
			  clevel = 15;
			  check = 0;
		  }
		
        digitalWrite(ledPin,LOW);
      }
        if(Serial.available()>0)
                {
                 if(Serial.peek()=='a')
                    {
                      Serial.println(" Configuring");
                      Serial.println("please make sure lights are turned off");
                      delay(3000);
                      digitalWrite(13,HIGH);
                      configure();
                      digitalWrite(13,LOW);
            		    Serial.println("please reset");      
                    }
                   else if(Serial.peek()=='b')
                    {
                      Serial.read();
                    //  ideal=Serial.parseInt();      
                    } 
                  
                      while(Serial.available()>0)
                       {Serial.read();}
                }
                Serial.print("lux :- ");
                Serial.print(lux);
                Serial.print("  clevel :- ");
				Serial.println(clevel);
				Serial.print(" check :");
				Serial.println(check);
                
              delay(2000);
}

int getLevel(int reqLux)
{
	for (int i = 15; i >= 0; i--)
	{
		if (reqLux <= stad[i])
			return i;
	}
	return 0;
}

int getIntesity(int reqLux)
{

	for (int i = 15; i >= 0; i--)
	{
		if (reqLux <= stad[i])
			return stad[i];
	}
	return 0;
}
void configure()
{
	int value,dflt=0;
	setVoltage(15);
	delay(2000);
	for (int i = 15; i >= 0; i--)
	{
		value = getValue(i); 
		if (i == 15)
			dflt = value;
		value = value - dflt;
		if (value > 256)
		{
			EEPROM.write(i + 16, value % 256);
			EEPROM.write(i, value / 256);
		}
		else
		{
			EEPROM.write(i, 0);
			EEPROM.write(i + 16, value);
		}
	}
	delay(1000);
}
int getValue(int level)
{
	int i = 3, j = 0, value = 0, arr[4] = { 0, 0, 0, 0 };
	int16_t lux;
	while (level)
	{
		arr[i] = level % 2;
		level = level / 2;
		i--;
	}
	digitalWrite(D0, arr[0]);
	digitalWrite(D1, arr[1]);
	digitalWrite(D2, arr[2]);
	digitalWrite(D3, arr[3]);
	delay(100);
	return getAvgLux();

}
int setVoltage(int level)
{
	int i = 3, l = level;
	arr[0] = 0; arr[1] = 0; arr[2] = 0; arr[3] = 0;
	while (level)
	{
		arr[i] = level % 2;
		level = level / 2;
		i--;
	}
	digitalWrite(D0, arr[0]);
	digitalWrite(D1, arr[1]);
	digitalWrite(D2, arr[2]);
	digitalWrite(D3, arr[3]);
	return l;
}
int16_t getAvgLux()
{
	int16_t high = 0, lux = 0, j;
	delay(500);
	high = lightSensor2.GetLightIntensity();
	for (j = 0; j < 5; j++)
	{
		lux = lightSensor2.GetLightIntensity();
		if (lux > high)
			high = lux;
		delay(200);
	}
	return high;
}
int incLux(int16_t obt, int16_t lx, int level)
{
	if (level == 15)
		return(setVoltage(getLevel(lx)));
	else
	{
		int dlux = lx - (obt - stad[level]);//desiredlux
		return(setVoltage(getLevel(dlux)));
	}
}
int decLux(int16_t lx, int level)
{
	int elux = lx - stad[level];//environmental lux
	if (elux < ideal)
		return(setVoltage(getLevel(elux)));
	else
		return(setVoltage(15));
}
