#include <Arduino.h>

#define LED_PIN_1 2
#define LED_PIN_2 4
#define BUTTON_PIN 15 // Оголошуємо номер піна, до якого підключена кнопка

bool isRunning = false;       // Прапор стану: true — мигалка працює, false — вимкнена
bool lastButtonState = HIGH;  // Змінна для збереження попереднього стану кнопки (HIGH = не натиснута)



void setup() {
  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
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
    digitalWrite(LED_PIN_1, HIGH); // Вмикаємо перший світлодіод
    digitalWrite(LED_PIN_2, LOW);  // Вимикаємо другий світлодіод
    delay(200);                    // Чекаємо 200 мілісекунд

    digitalWrite(LED_PIN_1, LOW);  // Вимикаємо перший світлодіод
    digitalWrite(LED_PIN_2, HIGH); // Вмикаємо другий світлодіод
    delay(200);                    // Чекаємо 200 мілісекунд
  } else {
    // Якщо режим вимкнений — погасити обидва світлодіоди
    digitalWrite(LED_PIN_1, LOW);  // Вимикаємо перший світлодіод
    digitalWrite(LED_PIN_2, LOW);  // Вимикаємо другий світлодіод
    delay(50);                     // Коротка затримка, щоб не перевантажувати процесор
  }

}
