#include <SPI.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3);    // RX, TX

#define ONE_WIRE_BUS 10   // Пин на управляющие пины датчиков темпиратуры

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DeviceAddress Thermometer1 = { 0x28, 0xFF, 0xCC, 0xD9, 0x64, 0x16, 0x03, 0x46 };  // адрес датчика DS18B20 1
DeviceAddress Thermometer2 = { 0x28, 0xFF, 0x49, 0x5E, 0x60, 0x16, 0x05, 0x43 };  // адрес датчика DS18B20 2
DeviceAddress Thermometer3 = { 0x28, 0xFF, 0x53, 0x8F, 0x60, 0x16, 0x05, 0x0C };  // адрес датчика DS18B20 3
  
#define DEVELOPER             TRUE             // Заставка разработчика

float temp_1, temp_2, temp_3;

//#define OLED_RESET 4
//Adafruit_SSD1306 display(OLED_RESET);

unsigned long currentTime = 0, loopTime = 0;    // Переменные времени

uint8_t pinRelays[] = { 4, 5, 6 };   // Массив переменных пинов реле
uint8_t countPins = 3;               // Колличество реле

uint8_t ch = 0;
String val = "";

//////////////////////////////////////////////////////////////////////////
//  Функция вывода на дисплей   //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/*
void dysplay_update() 
{    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10, 19);
    display.println(temps[0]);  
    display.setCursor(10, 29);
    display.println(temps[1]);  
    display.setCursor(10, 39);
    display.println(temps[2]);  
    display.display();
}
*/

//////////////////////////////////////////////////////////////////////////
//  Функция настройки программы //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void setup() {
    // Инициализация пинов на выходы
    for(int i = 0; i < (countPins - 1); i++)
    {
        pinMode(pinRelays[i], OUTPUT);
        digitalWrite(pinRelays[i], LOW); 
    }
    
    // Инициализация GSM
    delay(2000);
    mySerial.begin(9600);
    mySerial.println("AT+CLIP=1"); delay(100); //включаем АОН
    mySerial.println("AT+CMGF=1"); delay(100); //режим кодировки СМС - обычный (для англ.)
    mySerial.println("AT+CSCS=\"GSM\""); delay(100); //режим кодировки текста

    // Инициализация датчиков темпиратуры
    sensors.begin();
    sensors.setResolution(Thermometer1, 10);
    sensors.setResolution(Thermometer2, 10);
    sensors.setResolution(Thermometer3, 10);

/*
    // Инициализация дисплея
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.display();
    display.clearDisplay();

    #ifdef DEVELOPER
        display.drawCircle (64, 32, 57, 1);
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(32, 19);
        display.println("Developer-RU");
        display.setCursor(14, 29);
        display.println("+7(951) 795-65-05");
        display.setCursor(54, 45);
        display.println("2016");
        display.display();
    #endif
    
    delay(3000);

    #ifdef DEVELOPER
        display.clearDisplay();
    #endif
*/

}

//////////////////////////////////////////////////////////////////////////
//  Функция отправки смс    //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void sms(String text, String phone) {
    mySerial.println("AT+CMGS=\"" + phone + "\""); delay(500);
    mySerial.print(text); delay(500);
    mySerial.print((char)26); delay(2000);
}

//////////////////////////////////////////////////////////////////////////
//  Функция основной цикл   //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void loop() {   
    currentTime = millis(); // получаем текущее время в миллисекундах со старта программы, каждый раз перезаписывая
    
    if(currentTime >= (loopTime + 3000)) {    // считываем датчики темпиратуры каждые 5 секунд, ниже указываем в миллисекундах
        
        // здесь смотрим текущие темпиратуры и делаем действия необходимые - учитываем также ошибочные данные типа 127 градусов это сбой и -48 глюк и т.п.
        sensors.requestTemperatures();
        
        temp_1 = sensors.getTempC(Thermometer1); delay(500);    // считываем темпиратуру 1 во временную переменную
        temp_2  = sensors.getTempC(Thermometer2); delay(500);   // считываем темпиратуру 2 во временную переменную
        temp_3 = sensors.getTempC(Thermometer3); delay(500);    // считываем темпиратуру 3 во временную переменную
        
        loopTime = currentTime; 
    }
    
    //если GSM модуль что-то послал нам, то
    if (mySerial.available()) {  
        delay(200); // Ожидаем заполнения буфера
        
        //сохраняем входную строку в переменную val, посимвольно в строку
        while (mySerial.available()) {  
            ch = mySerial.read();
            val += char(ch);
            delay(10);
        }
        
        //если звонок обнаружен, то проверяем номер
        if (val.indexOf("RING") > -1) {   // возможно необходимо передать перевод каретки (\r) и переход на другую строку (/n)
            //если номер звонящего наш. Укажите свой номер без "+"
            if (val.indexOf("79063978854") > -1) {  
                mySerial.println("ATH0"); delay(500);  //разрываем связь, надо уточнить, возможно необходимо передать перевод каретки (\r) и переход на другую строку (/n)
                sms("ULITSA: " + String(temp_1) + " \n DOM: " + String(temp_2) + " \n KOTEL: " + String(temp_3), String("+79063978854")); delay(500); //отправляем СМС на номер +79063978854
            }
        } 

        //если пришло смс, читаем команду
        if (val.indexOf("+CMT") > -1) {
            if (val.indexOf("1 on") > -1)  { digitalWrite(pinRelays[0], HIGH); delay(500);  }
            
            if (val.indexOf("1 off") > -1) { digitalWrite(pinRelays[0], LOW); delay(500); } 
            
            if (val.indexOf("2 on") > -1)  { digitalWrite(pinRelays[1], HIGH); delay(500);  }
            
            if (val.indexOf("2 off") > -1) { digitalWrite(pinRelays[1], LOW); delay(500); }
            
            if (val.indexOf("3 on") > -1)  { digitalWrite(pinRelays[2], HIGH); delay(500);  }
            
            if (val.indexOf("3 off") > -1) { digitalWrite(pinRelays[2], LOW); delay(500); }
            
            if (val.indexOf("1 restart") > -1) {
                digitalWrite(pinRelays[0], HIGH); delay(60000);
                digitalWrite(pinRelays[0], LOW); delay(500);
            } 
            
            if (val.indexOf("2 restart") > -1) {
                digitalWrite(pinRelays[1], HIGH); delay(60000);
                digitalWrite(pinRelays[1], LOW); delay(500);
            }
        }
        
        val = ""; // Очищаем переменную команды
    }

    //dysplay_update(); 
}

