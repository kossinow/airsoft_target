/*
Скетч для страйкбольной мишени, которую можно поразить заданным количеством выстрелов в заданный период времени
TODO устаканить мигания светодиодом, протестировать возрождение после определнного времени
*/

#include <Button.h>
#include <EEPROM.h>

#define mic 14 // вход микрофона (А0)
#define led 2 // выход на светодиод 
#define button_pin 3 // кнопка
#define buzzer 4 // сигнал
#define sens 90 //  настройка чувствительности 0...255 (~100 при фоновом 40)
#define s_delay 100 // задержка для настройки эха микрофона

Button button(button_pin);

long kill_time = 60; // время за которое нужно попасть в секундах
long dead_time = 60; // время когда прибор сигнализирует о смерти
int max_hits_needed = 10; // максимальное количество попаданий для убийства
int hits_needed;

long counting_start; // начало отсчета времени для попаданий
long delay_start; // начало отсчета времени эха
long last_tick; // отсчет времени миганий
int hits_counted = 0; // счетчик попаданий
int hits_blinking_amount; // обратный счтечик для мигания настроек количества попаданий
byte mode = 0; // флаг режимов
bool led_state = true;

void setup() {
  pinMode(mic, INPUT);
  pinMode(led, OUTPUT);
  pinMode(buzzer, OUTPUT);
  button.begin();
  if (EEPROM.read(1) < 0 || EEPROM.read(1) > max_hits_needed) {
    EEPROM.write(1, 3);
  }
  hits_needed = EEPROM.read(1);
  if (button.pressed()){
    mode = 2;
    hits_counted = 0;
    Serial.println("settings");
  }
  Serial.begin(9600);
}

void loop() {

  switch (mode) { // главное меню - переключение между состояниями
    case 0: alive(); break;
    case 1: dead(); break;
    case 2: settings(); break;
  }

}

void alive() { // режим ожидания попаданий

  if (analogRead(mic) > sens & millis() - delay_start > s_delay){ // считываем микрофон с учетом эха
    delay_start = millis();
    hits_counted = hits_counted + 1;
    Serial.println(hits_counted);
  }

  if (hits_counted == 0) { // засекаем время первого попадания
    counting_start = millis();
  }

  // обнуление таймера если не уложились вовремя
  if (hits_counted < hits_needed & millis() - counting_start > kill_time * 1000) {
    hits_counted = 0;
    Serial.println("timeout");
  }

  if (hits_counted >= hits_needed & millis() - counting_start < kill_time * 1000) {
    mode = 1;
    hits_counted = 0;
    counting_start = millis();
    Serial.println("dead");
  }

}

void dead() { // мертвый режим
  digitalWrite(buzzer, 1);
  digitalWrite(led, 1);
  if (button.released() || millis() - counting_start > dead_time * 1000){
    delay_start = millis();
    mode = 0;
    digitalWrite(buzzer, 0);
    digitalWrite(led, 0);
    Serial.println("reborn");
  }

}

void settings() { // режим настройки количества попаданий

  if (button.released()){ // изменение количества попаданий
    hits_needed ++;
    if (hits_needed > 10){
      hits_needed = 1;
    }
    EEPROM.write(1, hits_needed);
    last_tick = millis();
    hits_blinking_amount = hits_needed * 2;
  }

  if (millis() - last_tick > 400 & hits_blinking_amount > 0){ // мигание показывающее количество необходимых попаданий
    last_tick = millis();
    hits_blinking_amount --;
    led_state = !led_state;
    digitalWrite(led, led_state);
  }
  else if(millis() - last_tick > 1200 & hits_blinking_amount == 0) {
    hits_blinking_amount = hits_needed * 2;
    led_state = false;
    digitalWrite(led, led_state);
  }

}
