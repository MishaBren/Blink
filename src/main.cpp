#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

#define LED_PIN_1 2
#define LED_PIN_2 4
#define BUTTON_PIN 15 // Оголошуємо номер піна, до якого підключена кнопка
#define BUZZER_PIN 14

Adafruit_8x8matrix matrix = Adafruit_8x8matrix(); // Створюємо об'єкт для матриці 8x8 на базі VK16K33 / HT16K33
bool isRunning = false;       // Прапор стану: true — мигалка працює, false — вимкнена
bool lastButtonState = HIGH;  // Змінна для збереження попереднього стану кнопки (HIGH = не натиснута)

// Масив букв для нашої матриці 8х8 (виводитимемо по одній)
char policeWord[] = {'P', 'O', 'L', 'I', 'C', 'E'};
int currentLetter = 0;

void setup() {
  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  // Явно ініціалізуємо I2C шину для ESP32 (SDA = 21, SCL = 22)
  Wire.begin(21, 22);
  // Ініціалізація матриці (адреса за замовчуванням 0x70)
  matrix.begin(0x70); 
  matrix.setBrightness(8); 
  matrix.clear();
  matrix.writeDisplay();
}

// Функція для виведення однієї букви по центру матриці
void showChar(char letter) {
  matrix.clear();
  matrix.setTextSize(1);      
  matrix.setTextWrap(false);  
  matrix.setTextColor(LED_ON);
  
  matrix.setCursor(2, 0);     // Центруємо букву на екрані 8х8
  matrix.print(letter);
  matrix.writeDisplay();      
}

void loop() {
  bool currentButtonState = digitalRead(BUTTON_PIN); // Зчитуємо поточний стан кнопки (LOW або HIGH)

  // Перевіряємо фронт натискання: кнопка була відпущена (HIGH), а стала натиснута (LOW)
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    isRunning = !isRunning; // Змінюємо стан режиму на протилежний (увімкнути/вимкнути)
    delay(50);              // Затримка 50 мс для захисту від дрижання контактів (антидребезг)
  }

  lastButtonState = currentButtonState; // Зберігаємо поточний стан кнопки для наступного циклу
                                  // Якщо режим увімкнений — виконуємо мигання
  
  if (isRunning) {

    // Виводимо чергову букву з нашого слова
    showChar(policeWord[currentLetter]);
    currentLetter++; // Збільшуємо номер букви на 1 для наступного разу

    // Фаза 1 сирени
    digitalWrite(LED_PIN_1, HIGH); // Вмикаємо перший світлодіод
    digitalWrite(LED_PIN_2, LOW);  // Вимикаємо другий світлодіод
    tone(BUZZER_PIN, 900);
    delay(200);                    // Чекаємо 200 мілісекунд

    digitalWrite(LED_PIN_1, LOW);  // Вимикаємо перший світлодіод
    digitalWrite(LED_PIN_2, HIGH); // Вмикаємо другий світлодіод
    tone(BUZZER_PIN, 600);
    delay(200);                    // Чекаємо 200 мілісекунд


    if (currentLetter >= 6) {
      currentLetter = 0; // Скидаємо лічильник на початок слова ('P')

      for (int i = 0; i < 3; i++) {
        matrix.fillRect(0, 0, 8, 8, LED_ON); // Увімкнути абсолютно всі світлодіоди
        matrix.writeDisplay();
        delay(150);

        matrix.clear(); // Вимкнути все
        matrix.writeDisplay();
        delay(150);
      } 


  } else {
    // Якщо режим вимкнений — погасити обидва світлодіоди
    digitalWrite(LED_PIN_1, LOW);  // Вимикаємо перший світлодіод
    digitalWrite(LED_PIN_2, LOW);  // Вимикаємо другий світлодіод
    noTone(BUZZER_PIN);
    matrix.clear();
    matrix.writeDisplay();
    delay(50);                     // Коротка затримка, щоб не перевантажувати процесор
  }

}
}