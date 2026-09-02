/*
 * ==============================================================================
 * VERSIÓN 2: ISR Propia (Custom ISR)
 * Latencia estimada: ~20 ciclos de reloj (~1.25 µs a 16 MHz)
 * 
 * Estrategia:
 * Se reemplaza la función attachInterrupt() y el envoltorio genérico de Arduino
 * por una rutina de interrupción nativa de AVR (ISR(INT0_vect)).
 * Se configuran los registros EICRA y EIMSK directamente por hardware.
 * Se ahorran ~33 ciclos adicionales al eliminar la tabla de callbacks de Arduino
 * y reducir los registros que el compilador debe respaldar en el prólogo.
 * ==============================================================================
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#define PIN_REG    PIND
#define PIN_BIT    2
#define MAX_COUNT  200

volatile uint8_t count_edges;  // Conteo de flancos de señal
volatile uint8_t count_high;   // Conteo de niveles en alto

/* Rutina nativa de interrupción para INT0 */
ISR(INT0_vect)
{
    uint8_t sampled_pin = PIN_REG;            // Lectura directa de PIND
    if (count_edges >= MAX_COUNT) return;     // Si ya terminamos, salir
    count_edges++;
    if (sampled_pin & (1 << PIN_BIT)) count_high++;
}

void setup()
{
    Serial.begin(9600);
    pinMode(2, INPUT);

    /* 
     * Configuración del registro de control de interrupciones externas A (EICRA)
     * ISC00 = 1, ISC01 = 0 -> Cualquier cambio lógico (flanco de subida o bajada) genera INT0.
     */
    EICRA = (EICRA & ~((1 << ISC01) | (1 << ISC00))) | (1 << ISC00);

    /* Habilitar la interrupción externa INT0 en la máscara EIMSK */
    EIMSK |= (1 << INT0);

    /* Habilitar interrupciones globales */
    sei();

    Serial.println("V2 iniciada: ISR Propia (ISR(INT0_vect) + PIND) [~20 ciclos]");
}

void loop()
{
    /* Espera a que el manejador de interrupciones cuente MAX_COUNT flancos */
    while (count_edges < MAX_COUNT) { /* espera */ }

    /* Reporte de resultados */
    Serial.print("Counted ");
    Serial.print(count_high);
    Serial.print(" HIGH levels for ");
    Serial.print(count_edges);
    Serial.println(" edges");

    /* Reiniciar conteos */
    count_high = 0;
    count_edges = 0;
}
