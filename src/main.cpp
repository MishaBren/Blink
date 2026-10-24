#include <Arduino.h>               // Підключення базової бібліотеки Arduino для ESP32
#include <Wire.h>                  // Підключення бібліотеки для роботи з шиною I2C
#include <Adafruit_GFX.h>          // Підключення графічної бібліотеки Adafruit GFX
#include <Adafruit_LEDBackpack.h>  // Підключення бібліотеки для світлодіодної матриці HT16K33
#include <TM1650.h>                // Підключення бібліотеки для дисплея TM1650

#define LED_PIN_1 2                // Визначення піна 2 для першого світлодіода
#define LED_PIN_2 4                // Визначення піна 4 для другого світлодіода
#define BUTTON_PIN 15              // Визначення піна 15 для тактової кнопки
#define BUZZER_PIN 14              // Визначення піна 14 для звукового випромінювача (бузера)

Adafruit_8x8matrix matrix = Adafruit_8x8matrix(); // Створення об'єкта світлодіодної матриці 8х8
TM1650 disp4;                      // Створення об'єкта дисплея TM1650

int currentMode = 0;               // Змінна поточного режиму (0-вимкнено, 1-поліція, 2-швидка, 3-пожежні)
bool lastButtonState = HIGH;       // Змінна для збереження попереднього стану кнопки (тягнута до HIGH)

char policeWord[] = {'P', 'O', 'L', 'I', 'C', 'E'}; // Масив символів для слова "POLICE"
char ambulanceWord[] = {'A', 'M', 'B', 'U'};         // Масив символів для слова "AMBU"
char fireWord[] = {'F', 'I', 'R', 'E'};              // Масив символів для слова "FIRE"

int currentLetter = 0;             // Індекс поточної літери для виведення на матрицю

unsigned long chaseStartTime = 0;  // Змінна для фіксування часу старту секундоміра
unsigned long previousMillis = 0;  // Змінна для неблокуючого таймера інтервалів
const long interval = 200;         // Інтервал часу для зміни фази сирени та літер (200 мілісекунд)
bool sirenPhase = false;           // Логічний прапорець для перемикання фаз сирени (true/false)

/**
 * Функція setup() викопується один раз при запуску контролера
 */
void setup() {
  Serial.begin(115200);            // Ініціалізація Serial Monitor на швидкості 115200 бод
  pinMode(LED_PIN_1, OUTPUT);      // Налаштування піна першого світлодіода як виходу
  pinMode(LED_PIN_2, OUTPUT);      // Налаштування піна другого світлодіода як виходу
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Налаштування піна кнопки як входу з внутрішньою підтяжкою
  pinMode(BUZZER_PIN, OUTPUT);     // Налаштування піна бузера як виходу

  Wire.begin(21, 22);              // Ініціалізація шини I2C на пінах SDA=21 та SCL=22
  matrix.begin(0x70);              // Ініціалізація світлодіодної матриці за адресою 0x70

  disp4.init();                    // Ініціалізація дисплея TM1650
  disp4.setBrightness(4);          // Встановлення середньої яскравості дисплея (від 0 до 7)
  disp4.displayOn();               // Увімкнення активного режиму дисплея

  char testStr[] = "8888";         // Створення масиву символів для тестового рядка
  disp4.displayString(testStr);    // Виведення тестового рядка на дисплей
}  

/**
 * Функція для виведення окремого символу по центру матриці 8х8
 */
void showChar(char letter) {
  matrix.clear();                  // Очищення буфера матриці перед малюванням
  matrix.setTextSize(1);           // Встановлення стандартного розміру тексту
  matrix.setTextWrap(false);       // Вимкнення перенесення тексту на новий рядок
  matrix.setTextColor(LED_ON);     // Встановлення кольору тексту (увімкнені діоди)
  matrix.setCursor(2, 0);          // Встановлення курсора у позицію (2, 0) для центрування
  matrix.print(letter);            // Запис переданої літери в буфер матриці
  matrix.writeDisplay();           // Відображення буфера на фізичному екрані матриці
}

/**
 * Головний нескінченний цикл програми loop()
 */
void loop() {
  bool currentButtonState = digitalRead(BUTTON_PIN); // Зчитування поточного стану кнопки

  // Перевірка натискання кнопки (перехід з HIGH у LOW через підтяжку PULLUP)
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    currentMode++;                 // Збільшення номера поточного режиму на 1
    if (currentMode > 3) {         // Перевірка виходу за межі останнього режиму
      currentMode = 0;             // Скидання режиму назад у 0 (вимкнено)
    }
    
    chaseStartTime = 0;            // Скидання часу старту секундоміра
    currentLetter = 0;             // Скидання індексу літери
    delay(50);                     // Коротка затримка для програмного усунення дребезгу контактів
  }

  lastButtonState = currentButtonState; // Збереження поточного стану кнопки як попереднього
  
  // Перевірка: якщо режим вимкнено (0)
  if (currentMode == 0) {         
    digitalWrite(LED_PIN_1, LOW);  // Примусове вимикання першого світлодіода
    digitalWrite(LED_PIN_2, LOW);  // Примусове вимикання другого світлодіода
    noTone(BUZZER_PIN);            // Вимикання звуку бузера
    currentLetter = 0;             // Обнулення індексу літери
    chaseStartTime = 0;            // Обнулення часу старту
    matrix.clear();                // Очищення буфера матриці
    matrix.writeDisplay();         // Оновлення фізичного екрана матриці

    char clearStr[] = "    ";      // Створення масиву пробілів для очищення дисплея
    disp4.displayString(clearStr); // Виведення пробілів на 4-розрядний дисплей
    delay(50);                     // Коротка пауза для розвантаження процесора
  } 
  else {                           // Якщо активний один із робочих режимів (1, 2 або 3)
    if (chaseStartTime == 0) {     // Якщо секундомір ще не був запущений
      chaseStartTime = millis();   // Фіксація поточного часу мікроконтролера як часу старту
      previousMillis = millis();   // Ініціалізація неблокуючого таймера подій
    }

    // Обчислення минулого часу для секундоміра без блокування програми
    unsigned long elapsedMillis = millis() - chaseStartTime; // Різниця мілісекунд від старту
    unsigned long totalSeconds = elapsedMillis / 1000;       // Отримання загальної кількості секунд
    int tenths = (elapsedMillis % 1000) / 100;               // Обчислення десятих долей секунди (0-9)

    char timeStr[5];               // Створення масиву для формування символьного рядка часу
    sprintf(timeStr, "%03d%1d", (int)(totalSeconds % 1000), tenths); // Форматування у формат СССД
    disp4.displayString(timeStr);  // Миттєве виведення часу на дисплей TM1650

    // Перевірка неблокуючого таймера (спрацьовує кожні 200 мілісекунд)
    if (millis() - previousMillis >= interval) {
      previousMillis = millis();   // Оновлення міток часу останнього спрацювання таймера
      sirenPhase = !sirenPhase;    // Зміна фази сирени на протилежну (true/false)

      // Конструкція вибору логіки залежно від поточного режиму спецслужби
      switch (currentMode) {
        
        case 1: // РЕЖИМ 1: ПОЛІЦІЯ
          if (sirenPhase) {          // Якщо перша фаза сирени
            digitalWrite(LED_PIN_1, HIGH); // Увімкнути перший світлодіод
            digitalWrite(LED_PIN_2, LOW);  // Вимкнути другий світлодіод
            tone(BUZZER_PIN, 900);         // Увімкнути бузер на високій частоті 900 Гц
          } else {                   // Якщо друга фаза сирени
            digitalWrite(LED_PIN_1, LOW);  // Вимкнути перший світлодіод
            digitalWrite(LED_PIN_2, HIGH); // Увімкнути другий світлодіод
            tone(BUZZER_PIN, 600);         // Увімкнути бузер на нижчій частоті 600 Гц
          }
          showChar(policeWord[currentLetter]); // Виведення поточної літери слова "POLICE"
          currentLetter++;             // Збільшення індексу літери на 1
          if (currentLetter >= 6) {    // Якщо дійшли до кінця слова (6 літер)
            currentLetter = 0;         // Скидання індексу на початок
          }
          break;                       // Вихід із кейсу 1

        case 2: // РЕЖИМ 2: ШВИДКА ДОПОМОГА
          if (sirenPhase) {          // Якщо перша фаза сирени
            digitalWrite(LED_PIN_1, HIGH); // Увімкнути обидва світлодіоди разом
            digitalWrite(LED_PIN_2, HIGH); // Увімкнути обидва світлодіоди разом
            tone(BUZZER_PIN, 1000);        // Увімкнути бузер на частоті 1000 Гц (високий тон)
          } else {                   // Якщо друга фаза сирени
            digitalWrite(LED_PIN_1, LOW);  // Вимкнути обидва світлодіоди
            digitalWrite(LED_PIN_2, LOW);  // Вимкнути обидва світлодіоди
            tone(BUZZER_PIN, 700);         // Увімкнути бузер на частоті 700 Гц (низький тон)
          }
          showChar(ambulanceWord[currentLetter]); // Виведення літери слова "AMBU"
          currentLetter++;             // Збільшення індексу літери на 1
          if (currentLetter >= 4) {    // Якщо дійшли до кінця слова (4 літери)
            currentLetter = 0;         // Скидання індексу на початок
          }
          break;                       // Вихід із кейсу 2

        case 3: // РЕЖИМ 3: ПОЖЕЖНА СЛУЖБА
          if (sirenPhase) {          // Якщо перша фаза сирени
            digitalWrite(LED_PIN_1, HIGH); // Увімкнути перший світлодіод
            digitalWrite(LED_PIN_2, LOW);  // Вимкнути другий світлодіод
            tone(BUZZER_PIN, 500);         // Увімкнути бузер на гулкому тоні 500 Гц
          } else {                   // Якщо друга фаза сирени
            digitalWrite(LED_PIN_1, LOW);  // Вимикати перший світлодіод
            digitalWrite(LED_PIN_2, HIGH); // Увімкнути другий світлодіод
            tone(BUZZER_PIN, 800);         // Увімкнути бузер на тоні 800 Гц
          }
          showChar(fireWord[currentLetter]); // Виведення літери слова "FIRE"
          currentLetter++;             // Збільшення індексу літери на 1
          if (currentLetter >= 4) {    // Якщо дійшли до кінця слова (4 літери)
            currentLetter = 0;         // Скидання індексу на початок
          }
          break;                       // Вихід із кейсу 3
      }
    }
  }
}