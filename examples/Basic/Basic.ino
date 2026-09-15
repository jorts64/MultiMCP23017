#include <Wire.h>
#include <MultiMCP23017.h>

/*
 * Tres MCP23017:
 *
 *   módulo 0 -> 0x20 -> GPIO virtuales 0..15
 *   módulo 1 -> 0x21 -> GPIO virtuales 16..31
 *   módulo 2 -> 0x22 -> GPIO virtuales 32..47
 */
MultiMCP23017 io(3, 0x20);


void setup()
{
    Serial.begin(115200);

    /*
     * Arduino AVR:
     * Wire.begin();
     *
     * ESP8266:
     * Wire.begin(D2, D1);
     *
     * ESP32:
     * Wire.begin(21, 22);
     */
    Wire.begin();

    if (!io.begin())
    {
        Serial.println("No se ha encontrado ningun MCP23017");
    }

    Serial.println("MultiMCP23017");
    Serial.print("Modulos configurados: ");
    Serial.println(io.moduleCount());

    Serial.print("GPIO virtuales: ");
    Serial.println(io.pinCount());

    for (uint8_t module = 0;
         module < io.moduleCount();
         ++module)
    {
        Serial.print("Modulo ");
        Serial.print(module);

        Serial.print(" direccion 0x");
        Serial.print(io.address(module), HEX);

        Serial.print(" -> ");

        if (io.moduleFound(module))
            Serial.println("OK");
        else
            Serial.println("NO ENCONTRADO");
    }

    /*
     * Primeros 32 GPIO como salidas.
     */
    for (uint8_t pin = 0; pin < 32; ++pin)
    {
        io.pinMode(pin, OUTPUT);
    }

    /*
     * GPIO virtual 32 corresponde a:
     *
     * módulo 2
     * GPIOA0
     */
    io.pinMode(32, INPUT_PULLUP);
}


void loop()
{
    /*
     * GPIO virtual 0 -> MCP23017 0x20 -> GPA0
     */
    io.digitalWrite(0, HIGH);

    delay(500);

    io.digitalWrite(0, LOW);

    delay(500);

    /*
     * Lectura de GPIO virtual 32.
     */
    if (io.digitalRead(32) == LOW)
    {
        Serial.println("Boton pulsado");
    }

    /*
     * Ejemplo de puerto:
     *
     * módulo 0
     * GPA = 10101010
     * GPB = 01010101
     */
    io.writePortA(0, 0xAA);
    io.writePortB(0, 0x55);

    delay(500);

    /*
     * Ejemplo de acceso de 16 bits:
     *
     * bits 0..7  -> GPIOA
     * bits 8..15 -> GPIOB
     */
    io.write16(0, 0x55AA);

    delay(500);
}
