/*
 * ==============================================================================
 * VERSIÓN 4: Uso del Registro Especial de E/S GPIOR0
 * Latencia estimada: ~8 ciclos de reloj (~0.5 µs a 16 MHz)
 * 
 * Estrategia:
 * Utiliza el registro de E/S de propósito general GPIOR0 direccionable a nivel de bit.
 * Con la instrucción ensambladora 'sbic' (Skip if Bit in I/O is Clear), se lee PIND
 * en el PRIMER CICLO de la ISR sin necesidad de guardar ningún registro en la pila.
 * El bit se copia en GPIOR0 con 'sbi'.
 * ==============================================================================
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#define PIN_BIT    2
#define MAX_COUNT  200

volatile uint8_t count_edges;  // Conteo de flancos de señal
volatile uint8_t count_high;   // Conteo de niveles en alto

/* ISR Desnuda con lectura en el ciclo 1 usando sbic */
ISR(INT0_vect, ISR_NAKED)
{
    asm volatile(
    "    sbic %[pin], %[bit]    \n"  // Salta la siguiente instrucción si el bit 2 de PIND es 0 (1-2 ciclos) -> ¡MUESTREO EN CICLO 1!
    "    sbi %[gpio], 0         \n"  // Coloca en 1 el bit 0 de GPIOR0 si estaba en alto (2 ciclos)
    "    rjmp INT0_vect_part_2  \n"  // Salta a la segunda parte (2 ciclos)
    :: [pin]  "I" (_SFR_IO_ADDR(PIND)),
       [bit]  "I" (PIN_BIT),
       [gpio] "I" (_SFR_IO_ADDR(GPIOR0)));
}

/* Parte 2: Procesamiento y acumulación de conteo */
ISR(INT0_vect_part_2)
{
    if (count_edges < MAX_COUNT) {
        count_edges++;
        if (GPIOR0 & (1 << 0)) count_high++;
    }
    GPIOR0 = 0;  // Resetear el registro para la siguiente interrupción
}

void setup()
{
    Serial.begin(9600);
    pinMode(2, INPUT);
    GPIOR0 = 0;  // Asegurar valor inicial en cero

    /* Configurar INT0 para cualquier flanco lógico (ISC00=1, ISC01=0) */
    EICRA = (EICRA & ~((1 << ISC01) | (1 << ISC00))) | (1 << ISC00);
    EIMSK |= (1 << INT0);
    sei();

    Serial.println("V4 iniciada: Registro GPIOR0 + Ensamblador [~8 ciclos]");
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
