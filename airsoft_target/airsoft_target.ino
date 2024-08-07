/*
Скетч для страйкольной мишени, которую можно поразить заданным количеством выстрелов в заданный период времени

*/

#define mic 14 // вход микрофона (А0)
#define sens 100 //  настройка чувствительности 0...255 (~100 при фоновом 40)
#define s_delay 100 // задержка для настройки эха микрофона

long kill_time = 60; // время за которое нужно попасть в секундах
int hits_needed = 5; // количество попаданий для убийства

long counting_start; // начало отсчета времени для попаданий
long delay_start; // начало отсчета времени эха
int hits_counted = 0; // счетчик попаданий
byte mode = 0; // флаг режимов

void setup() {

  pinMode(mic, INPUT);
  Serial.begin(9600);
}

void loop() {

  switch (mode) { // главное меню - переключение между состояниями
    case 0: alive(); break;
    case 1: dead(); break;
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
    Serial.println("dead");
  }

}

void dead() { // мертвый режим

  if (analogRead(mic) > sens & millis() - delay_start > s_delay){ // считываем микрофон с учетом эха
    delay_start = millis();
    mode = 0;
    Serial.println("reborn");
  }
  
}
