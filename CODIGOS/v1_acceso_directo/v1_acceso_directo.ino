/*
 * ==============================================================================
 * VERSIÓN 1: Acceso Directo a Puertos (Direct Port Access)
 * Latencia estimada: ~53 ciclos de reloj (~3.3 µs a 16 MHz)
 * 
 * Estrategia:
 * Se reemplaza la función de biblioteca digitalRead() por la lectura directa
 * del registro de entrada de puertos del microcontrolador ATmega328P (PIND).
 * Se ahorran ~46 ciclos de sobrecarga de digitalRead().
 * ==============================================================================
 */

#define PIN_NUMBER 2
#define PIN_REG    PIND  // El pin digital 2 de Arduino corresponde a PD2 (puerto D, bit 2)
#define PIN_BIT    2
#define MAX_COUNT  200

volatile uint8_t count_edges;  // Conteo de flancos de señal
volatile uint8_t count_high;   // Conteo de niveles en alto

/* Rutina de atención de interrupción */
void read_pin()
{
    uint8_t sampled_pin = PIN_REG;            // ¡Lectura directa del registro PIND primero!
    if (count_edges >= MAX_COUNT) return;     // Si ya terminamos, salir
    count_edges++;
    if (sampled_pin & (1 << PIN_BIT)) count_high++;
}

void setup()
{
    Serial.begin(9600);
    pinMode(PIN_NUMBER, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_NUMBER), read_pin, CHANGE);
    Serial.println("V1 iniciada: Acceso Directo a Puertos (PIND) [~53 ciclos]");
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
