/*
 * ==============================================================================
 * GENERADOR DE PULSOS CON ESP32 PARA PRUEBAS DE LATENCIA DE INTERRUPCIÓN
 * 
 * Conexión física:
 * - ESP32 GPIO 4 (o el pin que elijas) ----> Arduino Uno Pin 2 (INT0)
 * - ESP32 GND                         ----> Arduino Uno GND  (¡TIERRA COMÚN!)
 * 
 * Compatibilidad eléctrica:
 * - ESP32 entrega 3.3V en nivel HIGH.
 * - El ATmega328P a 5V requiere mínimo 3.0V (0.6 * Vcc) para nivel HIGH.
 * - 3.3V es reconocido de forma segura y directa por el pin de Arduino Uno.
 * ==============================================================================
 */

#include <Arduino.h>
#include "soc/gpio_reg.h"

#define PULSE_PIN 4      // Pin de salida del ESP32 hacia el Pin 2 de Arduino Uno
#define NUM_PULSES 100   // 100 pulsos = 200 flancos (subida + bajada) -> MAX_COUNT

// Genera un tren de pulsos con un ancho en alto específico
void send_pulse_train(uint32_t high_duration_ns) {
    // A 240 MHz, 1 ciclo de CPU = ~4.166 ns -> ciclos = (ns * 240) / 1000
    uint32_t high_cycles = (high_duration_ns * 240) / 1000;
    
    // Espaciado entre pulsos (50 µs) para que el Arduino procese cada interrupción
    uint32_t low_cycles = (50000 * 240) / 1000;

    Serial.printf("\n--- Enviando ráfaga de %d pulsos (%d flancos) ---\n", NUM_PULSES, NUM_PULSES * 2);
    Serial.printf("Ancho de pulso en HIGH: %u ns (aprox. %.2f µs)\n", high_duration_ns, high_duration_ns / 1000.0);

    // Sección crítica para evitar que el RTOS del ESP32 interrumpa la temporización
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&mux);

    for (int i = 0; i < NUM_PULSES; i++) {
        // Poner en ALTO usando registro directo de hardware
        REG_WRITE(GPIO_OUT_W1TS_REG, (1 << PULSE_PIN));
        
        if (high_cycles > 0) {
            uint32_t start = esp_cpu_get_cycle_count();
            while ((esp_cpu_get_cycle_count() - start) < high_cycles) {
                __asm__ __volatile__("nop");
            }
        }

        // Poner en BAJO usando registro directo de hardware
        REG_WRITE(GPIO_OUT_W1TC_REG, (1 << PULSE_PIN));

        // Tiempo en bajo suficiente para dar holgura a la ISR del Arduino
        uint32_t start_low = esp_cpu_get_cycle_count();
        while ((esp_cpu_get_cycle_count() - start_low) < low_cycles) {
            __asm__ __volatile__("nop");
        }
    }

    portEXIT_CRITICAL(&mux);
    Serial.println("Ráfaga completada. Revisa el Monitor Serial del Arduino Uno.");
}

void print_menu() {
    Serial.println("\n==================================================");
    Serial.println("   GENERADOR DE PULSOS ESP32 - TEST DE LATENCIA   ");
    Serial.println("==================================================");
    Serial.println("Selecciona el ancho del pulso (escribe el numero):");
    Serial.println(" 1 -> 10.0 µs  (10000 ns) - Debería detectarlo V0, V1, V2, V3, V4");
    Serial.println(" 2 ->  5.0 µs  (5000 ns)  - V0 empieza a fallar, V1 a V4 pasan");
    Serial.println(" 3 ->  2.5 µs  (2500 ns)  - V0 falla, V1 empieza a fallar, V2-V4 pasan");
    Serial.println(" 4 ->  1.0 µs  (1000 ns)  - Solo V2, V3 y V4 detectan correctamente");
    Serial.println(" 5 ->  500 ns  (0.5 µs)   - Solo V3 y V4 detectan");
    Serial.println(" 6 ->  250 ns  (0.25 µs)  - Al límite de V3 (~10 ciclos Uno), pasa V4");
    Serial.println(" 7 ->  125 ns  (0.125 µs) - Pulso ultracorto (2 ciclos a 16MHz)");
    Serial.println("==================================================");
}

void setup() {
    Serial.begin(115200);
    pinMode(PULSE_PIN, OUTPUT);
    digitalWrite(PULSE_PIN, LOW);
    delay(1000);
    print_menu();
}

void loop() {
    if (Serial.available() > 0) {
        char opt = Serial.read();
        while (Serial.available()) Serial.read(); // Limpiar buffer de entrada

        switch (opt) {
            case '1': send_pulse_train(10000); break;
            case '2': send_pulse_train(5000);  break;
            case '3': send_pulse_train(2500);  break;
            case '4': send_pulse_train(1000);  break;
            case '5': send_pulse_train(500);   break;
            case '6': send_pulse_train(250);   break;
            case '7': send_pulse_train(125);   break;
            default:
                print_menu();
                break;
        }
    }
}
