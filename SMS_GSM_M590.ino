#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>

#define ONE_WIRE_BUS 10                 // Пин на датчик темпиратуры    
#define BUTTON 4                        // Переменная кнопки (реле)    
#define TEMP  10                        // Темпиратура минимум         

SoftwareSerial gsm(2, 3);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DeviceAddress Thermometer1 = { 0x28, 0xFF, 0xCC, 0xD9, 0x64, 0x16, 0x03, 0x46 };  // Адрес датчика DS18B20 1
DeviceAddress Thermometer2 = { 0x28, 0xFF, 0x49, 0x5E, 0x60, 0x16, 0x05, 0x43 };  // Адрес датчика DS18B20 2

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float temp_1, temp_2;
unsigned long currentTime = 0, loopTime = 0;  // Переменные времени
uint8_t ch = 0;
String val = "";

byte flag = 0; // Статус реле питания
byte status_power = 1; // Статус питания


void setup() 
{    
    // Инициализация GSM
    delay(2000);
    gsm.begin(9600);
    
    gsm.print("AT+CLIP=1\r"); delay(500);  // АОН включен
    gsm.print("AT+CMGF=1\r"); delay(500);  // Настройка формата СМС сообщений, где 1 - текстовый формат
    gsm.print("AT+CSCS=\"GSM\"\r"); delay(500); // Команда кодировки текста  кодировка ASCII
    gsm.print("AT+IFC=1, 1\r"); delay(500);
    gsm.print("AT+CPBS=\"SM\"\r"); delay(500);
    gsm.print("AT+CNMI=1,2,2,1,0\r"); delay(500);
    gsm.print("AT+ENPWRSAVE=0\r"); delay(500);  // Запрещен спящий режим для модема
    gsm.print("ATD+79517956505\r"); delay(500);  // Позвонить, когда все запуститься

    // Инициализация датчиков темпиратуры
    sensors.begin();
    sensors.setResolution(Thermometer1, ONE_WIRE_BUS);
    sensors.setResolution(Thermometer2, ONE_WIRE_BUS);

    sms("OK", String("+79517956505"));  // Отправляем СМС на номер +79517956505

}

void sms(String text, String phone)
{
    gsm.println("AT+CMGS=\"" + phone + "\""); delay(500);
    gsm.print(text); delay(500);
    gsm.print((char)26); delay(2000);
}

void loop() 
{   
    currentTime = millis();  // Получаем текущее время в миллисекундах со старта программы, каждый раз перезаписывая
    
    // Считываем датчики темпиратуры каждые 3 секунды, указываем в миллисекундах
    if(currentTime >= (loopTime + 3000))
    {    
        sensors.requestTemperatures();
        
        temp_1 = sensors.getTempC(Thermometer1); delay(1000);  // Считываем темпиратуру 1
        temp_2  = sensors.getTempC(Thermometer2); delay(1000);   // Считываем темпиратуру 2

        // Если темпиратура батареи ниже 10 градусов
        if(temp_2 < TEMP)
        {
            // Отправляем СМС оповещение
            sms("TEMP < " + String(TEMP), String("+79517956505"));  // Отправляем СМС на номер +79063978854
        }

        loopTime = currentTime; 
    }
    
    // Если GSM модуль что-то послал
    if (gsm.available()) 
    {  
        delay(200);  // Ожидаем заполнения буфера
        
        // Cохраняем входную строку в переменную val
        while (gsm.available())
        {  
            ch = gsm.read();
            val += char(ch);
            delay(10);
        }
        
        // Eсли звонок обнаружен, то проверяем номер
        if (val.indexOf("RING") > -1)
        {
            // Если номер звонящего наш
            if (val.indexOf("79517956505") > -1) 
            {  
                gsm.println("ATH0"); delay(5);  // Разрываем связь
                sms("TEMP 1: " + String(temp_1) + "\n TEMP 2: " + String(temp_2), String("+79517956505"));  // Отправляем СМС на номер +79063978854
            }
        } 

        val = "";  // Очищаем переменную команды
    }

    // Наличие напряжения
    if(digitalRead(BUTTON) == HIGH && flag == 0) 
    {
        flag = 1; 
        status_power = 1;  
        sms("POWER: ON", String("+79517956505"));  // Отправляем СМС на номер +79063978854          
    }
    
    if(digitalRead(BUTTON) == LOW && flag == 1) 
    {
        flag = 0;
        status_power = 0;
        sms("POWER: OFF", String("+79517956505"));  // Отправляем СМС на номер +79063978854          
    }

    
}
