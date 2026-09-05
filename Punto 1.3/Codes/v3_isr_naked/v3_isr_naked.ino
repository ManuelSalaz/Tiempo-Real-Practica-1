/*
 * ==============================================================================
 * VERSIÓN 3: ISR Desnuda (Naked ISR) con Ensamblador Inline
 * Latencia estimada: ~10 ciclos de reloj (~0.625 µs a 16 MHz)
 * 
 * Estrategia:
 * Se define la ISR con el atributo ISR_NAKED para evitar que el compilador genere
 * el prólogo estándar (que guarda múltiples registros en la pila antes de leer).
 * En ensamblador inline se guarda únicamente el registro temporal r0, se lee PIND
 * de inmediato, se guarda el resultado y se salta a la segunda parte en C.
 * ==============================================================================
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#define PIN_BIT    2
#define MAX_COUNT  200

volatile uint8_t count_edges;          // Conteo de flancos de señal
volatile uint8_t count_high;           // Conteo de niveles en alto
volatile uint8_t sampled_pin;          // Registro muestreado por la parte en ASM
volatile uint16_t ciclos_usados;       // Ciclos medidos con Timer1 (como en Punto 1.2)

/* Parte 1: ISR Desnuda en ensamblador inline (mínima latencia de captura) */
ISR(INT0_vect, ISR_NAKED)
{
    asm volatile(
    "    push r0                \n"  // Guarda r0 en el stack (2 ciclos)
    "    in r0, %[pin]          \n"  // Lee PIND en r0 (1 ciclo) -> ¡MUESTREO AQUÍ!
    "    sts sampled_pin, r0    \n"  // Almacena r0 en variable global (2 ciclos)
    "    pop r0                 \n"  // Restaura valor original de r0 (2 ciclos)
    "    rjmp INT0_vect_part_2  \n"  // Salto a la segunda parte (2 ciclos)
    :: [pin] "I" (_SFR_IO_ADDR(PIND)));
}

/* Parte 2: Lógica de conteo en C gestionada con prólogo/epílogo normal */
ISR(INT0_vect_part_2)
{
    uint16_t t_inicio = TCNT1;        // Captura Timer1 al entrar a parte 2
    if (count_edges < MAX_COUNT) {
        count_edges++;
        if (sampled_pin & (1 << PIN_BIT)) count_high++;
    }
    uint16_t t_fin = TCNT1;           // Captura Timer1 al finalizar
    ciclos_usados = t_fin - t_inicio;
}

void setup()
{
    Serial.begin(9600);
    pinMode(2, INPUT);

    // --- Configuración de Timer1 para medición de ciclos (como en Punto 1.2) ---
    TCCR1A = 0;           // Init Timer1 en modo normal
    TCCR1B = 0;           // Reset registro
    TCCR1B |= B00000001;  // Prescaler = 1 (1 tick = 1 ciclo de reloj a 16 MHz = 62.5 ns)

    /* Configurar INT0 para cualquier flanco lógico (ISC00=1, ISC01=0) */
    EICRA = (EICRA & ~((1 << ISC01) | (1 << ISC00))) | (1 << ISC00);
    EIMSK |= (1 << INT0);
    sei();

    Serial.println("V3 iniciada: ISR Naked con Ensamblador Inline [~10 ciclos]");
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

    /* Reporte de ciclos medidos con Timer1 (como en Punto 1.2) */
    Serial.print("Ciclos logica C (Timer1): ");
    Serial.println(ciclos_usados);

    /* Reiniciar conteos */
    count_high = 0;
    count_edges = 0;
}
