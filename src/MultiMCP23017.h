#ifndef MULTIMCP23017_H
#define MULTIMCP23017_H

#include <Arduino.h>
#include <Wire.h>

/**
 * MultiMCP23017
 *
 * Controla uno o varios MCP23017 mediante I2C como un único
 * espacio de GPIO virtual.
 *
 * Requisitos:
 *   - Arduino Wire
 *   - MCP23017 con direcciones 0x20 ... 0x27
 *
 * La librería utiliza:
 *   IOCON.BANK = 0
 *   IOCON.SEQOP = 0
 *
 * Por tanto, los registros se mantienen en el mapa BANK=0
 * y permiten operaciones secuenciales A/B.
 */
class MultiMCP23017
{
public:

    static const uint8_t MAX_MODULES = 8;
    static const uint8_t PINS_PER_MODULE = 16;
    static const uint8_t MAX_PINS = MAX_MODULES * PINS_PER_MODULE;

    /*
     * Modos de interrupción.
     */
    static const uint8_t INTERRUPT_CHANGE  = 0;
    static const uint8_t INTERRUPT_COMPARE = 1;

    /*
     * Constructores.
     *
     * MultiMCP23017(3, 0x20);
     *     -> 0x20, 0x21, 0x22
     *
     * MultiMCP23017(3, 0x20, Wire);
     */
    MultiMCP23017(uint8_t moduleCount,
                  uint8_t firstAddress = 0x20);

    MultiMCP23017(uint8_t moduleCount,
                  uint8_t firstAddress,
                  TwoWire &wire);

    /*
     * Inicialización.
     *
     * Devuelve true si al menos un módulo configurado
     * responde correctamente.
     */
    bool begin();

    /*
     * Información de módulos.
     */
    uint8_t moduleCount() const;
    uint8_t pinCount() const;

    uint8_t address(uint8_t module) const;
    bool moduleFound(uint8_t module) const;

    /*
     * Validación.
     */
    bool validPin(uint8_t pin) const;
    bool validModule(uint8_t module) const;

    /*
     * Configuración GPIO.
     */
    bool pinMode(uint8_t pin, uint8_t mode);

    /*
     * GPIO virtual.
     */
    bool digitalWrite(uint8_t pin, uint8_t value);
    int digitalRead(uint8_t pin);

    /*
     * Acceso por puerto.
     *
     * Puerto A = bits 0..7
     * Puerto B = bits 8..15
     */
    bool writePortA(uint8_t module, uint8_t value);
    bool writePortB(uint8_t module, uint8_t value);

    int readPortA(uint8_t module);
    int readPortB(uint8_t module);

    /*
     * Acceso de 16 bits.
     *
     * bits 0..7  = GPIOA
     * bits 8..15 = GPIOB
     */
    bool write16(uint8_t module, uint16_t value);
    int read16(uint8_t module);

    /*
     * Acceso directo a registros.
     *
     * La dirección corresponde al mapa BANK=0.
     *
     * ATENCIÓN:
     * No modificar IOCON.BANK ni IOCON.SEQOP mediante
     * estas funciones mientras se utilice la API de alto nivel.
     */
    bool writeRegister(uint8_t module,
                       uint8_t registerAddress,
                       uint8_t value);

    int readRegister(uint8_t module,
                     uint8_t registerAddress);

    /*
     * Interrupciones.
     *
     * enableInterrupt(pin)
     *     Activa interrupt-on-change utilizando el modo
     *     actualmente configurado.
     *
     * setInterruptMode(pin, INTERRUPT_CHANGE)
     *     Compara con el estado previo.
     *
     * setInterruptMode(pin, INTERRUPT_COMPARE)
     *     Compara con DEFVAL.
     */
    bool enableInterrupt(uint8_t pin);
    bool disableInterrupt(uint8_t pin);

    bool setInterruptMode(uint8_t pin, uint8_t mode);
    bool setInterruptDefault(uint8_t pin, uint8_t value);

    /*
     * Configuración de las salidas de interrupción.
     *
     * mirror:
     *   false = INTA e INTB independientes
     *   true  = ambas salidas reflejan cualquier interrupción
     *
     * openDrain:
     *   true  = salida open-drain
     *   false = salida push-pull
     *
     * activeHigh:
     *   true  = interrupción activa HIGH
     *   false = interrupción activa LOW
     */
    bool configureInterruptOutput(uint8_t module,
                                  bool mirror,
                                  bool openDrain,
                                  bool activeHigh);

    /*
     * Lectura de flags de interrupción.
     */
    int readInterruptFlagsA(uint8_t module);
    int readInterruptFlagsB(uint8_t module);

    /*
     * Lectura de captura de interrupción.
     *
     * La lectura de INTCAP normalmente se utiliza para obtener
     * el estado que produjo la interrupción.
     */
    int readInterruptCaptureA(uint8_t module);
    int readInterruptCaptureB(uint8_t module);

private:

    struct ModuleState
    {
        uint8_t address;
        bool found;

        /*
         * Estado conocido por la librería.
         */
        uint8_t iodirA;
        uint8_t iodirB;

        uint8_t gppuA;
        uint8_t gppuB;

        uint8_t olatA;
        uint8_t olatB;

        uint8_t ipolA;
        uint8_t ipolB;

        uint8_t gpintenA;
        uint8_t gpintenB;

        uint8_t defvalA;
        uint8_t defvalB;

        uint8_t intconA;
        uint8_t intconB;
    };

    TwoWire *_wire;

    ModuleState _modules[MAX_MODULES];
    uint8_t _moduleCount;

    /*
     * Registro IOCON en BANK=0.
     */
    static const uint8_t REG_IODIRA   = 0x00;
    static const uint8_t REG_IODIRB   = 0x01;

    static const uint8_t REG_IPOLA    = 0x02;
    static const uint8_t REG_IPOLB    = 0x03;

    static const uint8_t REG_GPINTENA = 0x04;
    static const uint8_t REG_GPINTENB = 0x05;

    static const uint8_t REG_DEFVALA  = 0x06;
    static const uint8_t REG_DEFVALB  = 0x07;

    static const uint8_t REG_INTCONA  = 0x08;
    static const uint8_t REG_INTCONB  = 0x09;

    static const uint8_t REG_IOCON    = 0x0A;

    static const uint8_t REG_GPPUA    = 0x0C;
    static const uint8_t REG_GPPUB    = 0x0D;

    static const uint8_t REG_INTFA    = 0x0E;
    static const uint8_t REG_INTFB    = 0x0F;

    static const uint8_t REG_INTCAPA  = 0x10;
    static const uint8_t REG_INTCAPB  = 0x11;

    static const uint8_t REG_GPIOA    = 0x12;
    static const uint8_t REG_GPIOB    = 0x13;

    static const uint8_t REG_OLATA    = 0x14;
    static const uint8_t REG_OLATB    = 0x15;

    /*
     * IOCON:
     *
     * BANK  = 0
     * MIRROR = 0
     * SEQOP = 0
     * DISSLW = 0
     * HAEN = 0
     * ODR = 0
     * INTPOL = 0
     *
     * Valor = 0x00
     */
    static const uint8_t IOCON_DEFAULT = 0x00;

    /*
     * Funciones I2C internas.
     */
    bool writeByte(uint8_t module,
                   uint8_t registerAddress,
                   uint8_t value);

    bool writeBytes(uint8_t module,
                    uint8_t registerAddress,
                    const uint8_t *data,
                    uint8_t length);

    int readByte(uint8_t module,
                 uint8_t registerAddress);

    bool readBytes(uint8_t module,
                   uint8_t registerAddress,
                   uint8_t *data,
                   uint8_t length);

    /*
     * Inicialización de un módulo.
     */
    bool initializeModule(uint8_t module);

    /*
     * Traducción de GPIO virtual.
     */
    uint8_t pinModule(uint8_t pin) const;
    uint8_t pinIndex(uint8_t pin) const;

    /*
     * Modificación de bits.
     */
    bool setBitRegister(uint8_t module,
                        uint8_t registerAddress,
                        uint8_t bit,
                        bool state);

    /*
     * Estado cacheado.
     */
    void updateCachedRegister(uint8_t module,
                              uint8_t registerAddress,
                              uint8_t value);

    /*
     * Conversión de pin a puerto/bit.
     */
    uint8_t portRegisterForPin(uint8_t pin,
                               uint8_t regA,
                               uint8_t regB) const;
};

#endif
