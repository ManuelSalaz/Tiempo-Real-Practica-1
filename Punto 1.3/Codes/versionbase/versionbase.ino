/*
 * ==============================================================================
 * VERSIÓN 0: Base estándar de Arduino (attachInterrupt + digitalRead)
 * Latencia estimada: ~99 ciclos de reloj (~6.2 µs a 16 MHz)
 * 
 * Estrategia:
 * Código inicial estándar de Arduino. La latencia se debe a:
 * - Secuencia de hardware por interrupción: 4 ciclos.
 * - Salto de la tabla de vectores: 3 ciclos.
 * - Prólogo de la ISR de Arduino core (guardado de registros en pila): ~15 ciclos.
 * - Búsqueda de callback en attachInterrupt(): ~20 ciclos.
 * - Ejecución de digitalRead() (mapeos de puertos, tablas de pines, timers PWM): ~57 ciclos.
 * ==============================================================================
 */

#define PIN_NUMBER 2
#define MAX_COUNT  200

volatile uint8_t count_edges;          // Conteo de flancos de señal
volatile uint8_t count_high;           // Conteo de niveles en alto
volatile uint16_t ciclos_usados;       // Ciclos totales medidos con Timer1 (como en Punto 1.2)
volatile uint16_t ciclos_digitalRead;  // Ciclos específicos consumidos por digitalRead()

/* Rutina de atención de interrupción */
void read_pin()
{
    uint16_t t_inicio = TCNT1;                // Captura contador Timer1 al entrar (como en Punto 1.2)
    int pin_state = digitalRead(PIN_NUMBER);  // ¡digitalRead al inicio!
    uint16_t t_muestreo = TCNT1;              // Captura tras terminar digitalRead()

    if (count_edges >= MAX_COUNT) return;     // Si ya terminamos, salir
    count_edges++;
    if (pin_state == HIGH) count_high++;

    uint16_t t_fin = TCNT1;                   // Captura al finalizar la ISR
    ciclos_digitalRead = t_muestreo - t_inicio;
    ciclos_usados = t_fin - t_inicio;
}

void setup()
{
    Serial.begin(9600);
    pinMode(PIN_NUMBER, INPUT);

    // --- Configuración de Timer1 para medición de ciclos (como en Punto 1.2) ---
    TCCR1A = 0;           // Init Timer1 en modo normal
    TCCR1B = 0;           // Reset registro
    TCCR1B |= B00000001;  // Prescaler = 1 (1 tick = 1 ciclo de reloj a 16 MHz = 62.5 ns)

    attachInterrupt(digitalPinToInterrupt(PIN_NUMBER), read_pin, CHANGE);
    Serial.println("V0 iniciada: Base (attachInterrupt + digitalRead) [~99 ciclos]");
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
    Serial.print("Ciclos digitalRead(): ");
    Serial.print(ciclos_digitalRead);
    Serial.print(" | Ciclos cuerpo ISR: ");
    Serial.println(ciclos_usados);

    /* Reiniciar conteos */
    count_high = 0;
    count_edges = 0;  // Hacer esto de último para evitar condición de carrera
}