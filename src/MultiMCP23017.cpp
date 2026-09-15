#include "MultiMCP23017.h"


MultiMCP23017::MultiMCP23017(uint8_t moduleCount,
                             uint8_t firstAddress)
    : _wire(&Wire),
      _moduleCount(0)
{
    if (moduleCount > MAX_MODULES)
        moduleCount = MAX_MODULES;

    /*
     * Las direcciones válidas del MCP23017 son 0x20..0x27.
     */
    for (uint8_t i = 0; i < moduleCount; ++i)
    {
        uint8_t addr = firstAddress + i;

        if (addr < 0x20 || addr > 0x27)
            break;

        _modules[i].address = addr;
        _modules[i].found = false;

        _modules[i].iodirA = 0xFF;
        _modules[i].iodirB = 0xFF;

        _modules[i].gppuA = 0x00;
        _modules[i].gppuB = 0x00;

        _modules[i].olatA = 0x00;
        _modules[i].olatB = 0x00;

        _modules[i].ipolA = 0x00;
        _modules[i].ipolB = 0x00;

        _modules[i].gpintenA = 0x00;
        _modules[i].gpintenB = 0x00;

        _modules[i].defvalA = 0x00;
        _modules[i].defvalB = 0x00;

        _modules[i].intconA = 0x00;
        _modules[i].intconB = 0x00;

        ++_moduleCount;
    }
}


MultiMCP23017::MultiMCP23017(uint8_t moduleCount,
                             uint8_t firstAddress,
                             TwoWire &wire)
    : _wire(&wire),
      _moduleCount(0)
{
    if (moduleCount > MAX_MODULES)
        moduleCount = MAX_MODULES;

    for (uint8_t i = 0; i < moduleCount; ++i)
    {
        uint8_t addr = firstAddress + i;

        if (addr < 0x20 || addr > 0x27)
            break;

        _modules[i].address = addr;
        _modules[i].found = false;

        _modules[i].iodirA = 0xFF;
        _modules[i].iodirB = 0xFF;

        _modules[i].gppuA = 0x00;
        _modules[i].gppuB = 0x00;

        _modules[i].olatA = 0x00;
        _modules[i].olatB = 0x00;

        _modules[i].ipolA = 0x00;
        _modules[i].ipolB = 0x00;

        _modules[i].gpintenA = 0x00;
        _modules[i].gpintenB = 0x00;

        _modules[i].defvalA = 0x00;
        _modules[i].defvalB = 0x00;

        _modules[i].intconA = 0x00;
        _modules[i].intconB = 0x00;

        ++_moduleCount;
    }
}


bool MultiMCP23017::begin()
{
    bool anyFound = false;

    for (uint8_t i = 0; i < _moduleCount; ++i)
    {
        _modules[i].found = false;

        /*
         * Primero comprobamos presencia.
         */
        _wire->beginTransmission(_modules[i].address);

        uint8_t error = _wire->endTransmission();

        if (error != 0)
            continue;

        /*
         * El dispositivo responde.
         */
        if (initializeModule(i))
        {
            _modules[i].found = true;
            anyFound = true;
        }
    }

    return anyFound;
}


uint8_t MultiMCP23017::moduleCount() const
{
    return _moduleCount;
}


uint8_t MultiMCP23017::pinCount() const
{
    return _moduleCount * PINS_PER_MODULE;
}


uint8_t MultiMCP23017::address(uint8_t module) const
{
    if (!validModule(module))
        return 0;

    return _modules[module].address;
}


bool MultiMCP23017::moduleFound(uint8_t module) const
{
    if (!validModule(module))
        return false;

    return _modules[module].found;
}


bool MultiMCP23017::validModule(uint8_t module) const
{
    return module < _moduleCount;
}


bool MultiMCP23017::validPin(uint8_t pin) const
{
    return pin < pinCount();
}


uint8_t MultiMCP23017::pinModule(uint8_t pin) const
{
    return pin / PINS_PER_MODULE;
}


uint8_t MultiMCP23017::pinIndex(uint8_t pin) const
{
    return pin % PINS_PER_MODULE;
}


uint8_t MultiMCP23017::portRegisterForPin(uint8_t pin,
                                          uint8_t regA,
                                          uint8_t regB) const
{
    return (pinIndex(pin) < 8) ? regA : regB;
}


bool MultiMCP23017::initializeModule(uint8_t module)
{
    if (!validModule(module))
        return false;

    /*
     * Establecemos primero IOCON.
     *
     * BANK  = 0
     * SEQOP = 0
     *
     * Esto es fundamental para que el resto de la librería
     * pueda utilizar el mapa BANK=0 y las operaciones
     * secuenciales A/B.
     */
    if (!writeByte(module, REG_IOCON, IOCON_DEFAULT))
        return false;

    /*
     * Estado seguro y determinista:
     *
     * - todos los GPIO como entradas
     * - pull-up desactivados
     * - polaridad normal
     * - interrupciones desactivadas
     * - DEFVAL = 0
     * - INTCON = 0
     * - OLAT = 0
     */
    if (!writeByte(module, REG_IODIRA, 0xFF))
        return false;

    if (!writeByte(module, REG_IODIRB, 0xFF))
        return false;

    if (!writeByte(module, REG_IPOLA, 0x00))
        return false;

    if (!writeByte(module, REG_IPOLB, 0x00))
        return false;

    if (!writeByte(module, REG_GPINTENA, 0x00))
        return false;

    if (!writeByte(module, REG_GPINTENB, 0x00))
        return false;

    if (!writeByte(module, REG_DEFVALA, 0x00))
        return false;

    if (!writeByte(module, REG_DEFVALB, 0x00))
        return false;

    if (!writeByte(module, REG_INTCONA, 0x00))
        return false;

    if (!writeByte(module, REG_INTCONB, 0x00))
        return false;

    if (!writeByte(module, REG_GPPUA, 0x00))
        return false;

    if (!writeByte(module, REG_GPPUB, 0x00))
        return false;

    /*
     * OLAT no controla entradas, por lo que podemos dejar
     * un valor conocido para cuando un pin pase a OUTPUT.
     */
    if (!writeByte(module, REG_OLATA, 0x00))
        return false;

    if (!writeByte(module, REG_OLATB, 0x00))
        return false;

    _modules[module].iodirA = 0xFF;
    _modules[module].iodirB = 0xFF;

    _modules[module].gppuA = 0x00;
    _modules[module].gppuB = 0x00;

    _modules[module].olatA = 0x00;
    _modules[module].olatB = 0x00;

    _modules[module].ipolA = 0x00;
    _modules[module].ipolB = 0x00;

    _modules[module].gpintenA = 0x00;
    _modules[module].gpintenB = 0x00;

    _modules[module].defvalA = 0x00;
    _modules[module].defvalB = 0x00;

    _modules[module].intconA = 0x00;
    _modules[module].intconB = 0x00;

    return true;
}


bool MultiMCP23017::pinMode(uint8_t pin, uint8_t mode)
{
    if (!validPin(pin))
        return false;

    uint8_t module = pinModule(pin);

    if (!_modules[module].found)
        return false;

    uint8_t localPin = pinIndex(pin);

    bool output = false;
    bool pullup = false;

    if (mode == OUTPUT)
    {
        output = true;
        pullup = false;
    }
    else if (mode == INPUT)
    {
        output = false;
        pullup = false;
    }
    else if (mode == INPUT_PULLUP)
    {
        output = false;
        pullup = true;
    }
    else
    {
        return false;
    }

    bool portB = localPin >= 8;
    uint8_t bit = localPin & 0x07;
    uint8_t mask = (uint8_t)(1U << bit);

    uint8_t iodir;
    uint8_t gppu;

    if (portB)
    {
        iodir = _modules[module].iodirB;
        gppu  = _modules[module].gppuB;
    }
    else
    {
        iodir = _modules[module].iodirA;
        gppu  = _modules[module].gppuA;
    }

    /*
     * IODIR:
     *   1 = input
     *   0 = output
     */
    if (output)
        iodir &= (uint8_t)~mask;
    else
        iodir |= mask;

    if (pullup)
        gppu |= mask;
    else
        gppu &= (uint8_t)~mask;

    if (portB)
    {
        if (!writeByte(module, REG_IODIRB, iodir))
            return false;

        if (!writeByte(module, REG_GPPUB, gppu))
            return false;

        _modules[module].iodirB = iodir;
        _modules[module].gppuB = gppu;
    }
    else
    {
        if (!writeByte(module, REG_IODIRA, iodir))
            return false;

        if (!writeByte(module, REG_GPPUA, gppu))
            return false;

        _modules[module].iodirA = iodir;
        _modules[module].gppuA = gppu;
    }

    return true;
}


bool MultiMCP23017::digitalWrite(uint8_t pin, uint8_t value)
{
    if (!validPin(pin))
        return false;

    uint8_t module = pinModule(pin);

    if (!_modules[module].found)
        return false;

    uint8_t localPin = pinIndex(pin);

    bool portB = localPin >= 8;
    uint8_t bit = localPin & 0x07;
    uint8_t mask = (uint8_t)(1U << bit);

    uint8_t output;

    if (portB)
        output = _modules[module].olatB;
    else
        output = _modules[module].olatA;

    if (value == LOW)
        output &= (uint8_t)~mask;
    else
        output |= mask;

    if (portB)
    {
        if (!writeByte(module, REG_OLATB, output))
            return false;

        _modules[module].olatB = output;
    }
    else
    {
        if (!writeByte(module, REG_OLATA, output))
            return false;

        _modules[module].olatA = output;
    }

    return true;
}


int MultiMCP23017::digitalRead(uint8_t pin)
{
    if (!validPin(pin))
        return -1;

    uint8_t module = pinModule(pin);

    if (!_modules[module].found)
        return -1;

    uint8_t localPin = pinIndex(pin);

    uint8_t reg = (localPin < 8) ? REG_GPIOA : REG_GPIOB;
    uint8_t bit = localPin & 0x07;

    int value = readByte(module, reg);

    if (value < 0)
        return -1;

    return (value & (1U << bit)) ? HIGH : LOW;
}


bool MultiMCP23017::writePortA(uint8_t module,
                                uint8_t value)
{
    if (!validModule(module))
        return false;

    if (!_modules[module].found)
        return false;

    if (!writeByte(module, REG_OLATA, value))
        return false;

    _modules[module].olatA = value;

    return true;
}


bool MultiMCP23017::writePortB(uint8_t module,
                                uint8_t value)
{
    if (!validModule(module))
        return false;

    if (!_modules[module].found)
        return false;

    if (!writeByte(module, REG_OLATB, value))
        return false;

    _modules[module].olatB = value;

    return true;
}


int MultiMCP23017::readPortA(uint8_t module)
{
    if (!validModule(module))
        return -1;

    if (!_modules[module].found)
        return -1;

    return readByte(module, REG_GPIOA);
}


int MultiMCP23017::readPortB(uint8_t module)
{
    if (!validModule(module))
        return -1;

    if (!_modules[module].found)
        return -1;

    return readByte(module, REG_GPIOB);
}


bool MultiMCP23017::write16(uint8_t module,
                            uint16_t value)
{
    if (!validModule(module))
        return false;

    if (!_modules[module].found)
        return false;

    uint8_t data[2];

    /*
     * bits 0..7  -> GPIOA / OLATA
     * bits 8..15 -> GPIOB / OLATB
     */
    data[0] = (uint8_t)(value & 0xFF);
    data[1] = (uint8_t)((value >> 8) & 0xFF);

    if (!writeBytes(module,
                    REG_OLATA,
                    data,
                    2))
    {
        return false;
    }

    _modules[module].olatA = data[0];
    _modules[module].olatB = data[1];

    return true;
}


int MultiMCP23017::read16(uint8_t module)
{
    if (!validModule(module))
        return -1;

    if (!_modules[module].found)
        return -1;

    uint8_t data[2];

    if (!readBytes(module,
                   REG_GPIOA,
                   data,
                   2))
    {
        return -1;
    }

    uint16_t result = data[0];

    result |= ((uint16_t)data[1] << 8);

    return (int)result;
}


bool MultiMCP23017::writeRegister(uint8_t module,
                                  uint8_t registerAddress,
                                  uint8_t value)
{
    if (!validModule(module))
        return false;

    if (!_modules[module].found)
        return false;

    /*
     * BANK=0 expone 0x00..0x15.
     *
     * 0x0B es también IOCON en BANK=0.
     */
    if (registerAddress > 0x15)
        return false;

    /*
     * Protegemos la configuración estructural de IOCON.
     *
     * La librería depende de:
     *   BANK  = 0
     *   SEQOP = 0
     *
     * Permitimos modificar MIRROR/ODR/INTPOL,
     * pero no BANK/SEQOP.
     */
    if (registerAddress == REG_IOCON ||
        registerAddress == 0x0B)
    {
        uint8_t safeValue = value;

        /*
         * BANK = 0
         * SEQOP = 0
         */
        safeValue &= (uint8_t)~(1U << 7);
        safeValue &= (uint8_t)~(1U << 5);

        return writeByte(module,
                         REG_IOCON,
                         safeValue);
    }

    if (!writeByte(module,
                   registerAddress,
                   value))
    {
        return false;
    }

    updateCachedRegister(module,
                          registerAddress,
                          value);

    return true;
}


int MultiMCP23017::readRegister(uint8_t module,
                                 uint8_t registerAddress)
{
    if (!validModule(module))
        return -1;

    if (!_modules[module].found)
        return -1;

    if (registerAddress > 0x15)
        return -1;

    return readByte(module,
                    registerAddress);
}


bool MultiMCP23017::enableInterrupt(uint8_t pin)
{
    if (!validPin(pin))
        return false;

    uint8_t module = pinModule(pin);

    if (!_modules[module].found)
        return false;

    uint8_t localPin = pinIndex(pin);

    bool portB = localPin >= 8;
    uint8_t bit = localPin & 0x07;
    uint8_t mask = (uint8_t)(1U << bit);

    if (portB)
    {
        uint8_t value = _modules[module].gpintenB;
        value |= mask;

        if (!writeByte(module, REG_GPINTENB, value))
            return false;

        _modules[module].gpintenB = value;
    }
    else
    {
        uint8_t value = _modules[module].gpintenA;
        value |= mask;

        if (!writeByte(module, REG_GPINTENA, value))
            return false;

        _modules[module].gpintenA = value;
    }

    return true;
}


bool MultiMCP23017::disableInterrupt(uint8_t pin)
{
    if (!validPin(pin))
        return false;

    uint8_t module = pinModule(pin);

    if (!_modules[module].found)
        return false;

    uint8_t localPin = pinIndex(pin);

    bool portB = localPin >= 8;
    uint8_t bit = localPin & 0x07;
    uint8_t mask = (uint8_t)(1U << bit);

    if (portB)
    {
        uint8_t value = _modules[module].gpintenB;
        value &= (uint8_t)~mask;

        if (!writeByte(module, REG_GPINTENB, value))
            return false;

        _modules[module].gpintenB = value;
    }
    else
    {
        uint8_t value = _modules[module].gpintenA;
        value &= (uint8_t)~mask;

        if (!writeByte(module, REG_GPINTENA, value))
            return false;

        _modules[module].gpintenA = value;
    }

    return true;
}


bool MultiMCP23017::setInterruptMode(uint8_t pin,
                                      uint8_t mode)
{
    if (!validPin(pin))
        return false;

    if (mode != INTERRUPT_CHANGE &&
        mode != INTERRUPT_COMPARE)
    {
        return false;
    }

    uint8_t module = pinModule(pin);

    if (!_modules[module].found)
        return false;

    uint8_t localPin = pinIndex(pin);

    bool portB = localPin >= 8;
    uint8_t bit = localPin & 0x07;
    uint8_t mask = (uint8_t)(1U << bit);

    if (portB)
    {
        uint8_t value = _modules[module].intconB;

        if (mode == INTERRUPT_COMPARE)
            value |= mask;
        else
            value &= (uint8_t)~mask;

        if (!writeByte(module, REG_INTCONB, value))
            return false;

        _modules[module].intconB = value;
    }
    else
    {
        uint8_t value = _modules[module].intconA;

        if (mode == INTERRUPT_COMPARE)
            value |= mask;
        else
            value &= (uint8_t)~mask;

        if (!writeByte(module, REG_INTCONA, value))
            return false;

        _modules[module].intconA = value;
    }

    return true;
}


bool MultiMCP23017::setInterruptDefault(uint8_t pin,
                                         uint8_t value)
{
    if (!validPin(pin))
        return false;

    uint8_t module = pinModule(pin);

    if (!_modules[module].found)
        return false;

    uint8_t localPin = pinIndex(pin);

    bool portB = localPin >= 8;
    uint8_t bit = localPin & 0x07;

    /*
     * DEFVAL trabaja por puerto, no por pin individual.
     *
     * Por ello esta función modifica únicamente el bit
     * correspondiente y conserva los otros siete.
     */
    uint8_t mask = (uint8_t)(1U << bit);

    if (portB)
    {
        uint8_t defval = _modules[module].defvalB;

        if (value == LOW)
            defval &= (uint8_t)~mask;
        else
            defval |= mask;

        if (!writeByte(module, REG_DEFVALB, defval))
            return false;

        _modules[module].defvalB = defval;
    }
    else
    {
        uint8_t defval = _modules[module].defvalA;

        if (value == LOW)
            defval &= (uint8_t)~mask;
        else
            defval |= mask;

        if (!writeByte(module, REG_DEFVALA, defval))
            return false;

        _modules[module].defvalA = defval;
    }

    return true;
}


bool MultiMCP23017::configureInterruptOutput(uint8_t module,
                                             bool mirror,
                                             bool openDrain,
                                             bool activeHigh)
{
    if (!validModule(module))
        return false;

    if (!_modules[module].found)
        return false;

    uint8_t value = 0;

    /*
     * IOCON:
     *
     * bit 6 MIRROR
     * bit 2 ODR
     * bit 1 INTPOL
     *
     * BANK y SEQOP permanecen en 0.
     */
    if (mirror)
        value |= (1U << 6);

    if (openDrain)
        value |= (1U << 2);

    if (activeHigh)
        value |= (1U << 1);

    return writeByte(module,
                     REG_IOCON,
                     value);
}


int MultiMCP23017::readInterruptFlagsA(uint8_t module)
{
    if (!validModule(module))
        return -1;

    if (!_modules[module].found)
        return -1;

    return readByte(module, REG_INTFA);
}


int MultiMCP23017::readInterruptFlagsB(uint8_t module)
{
    if (!validModule(module))
        return -1;

    if (!_modules[module].found)
        return -1;

    return readByte(module, REG_INTFB);
}


int MultiMCP23017::readInterruptCaptureA(uint8_t module)
{
    if (!validModule(module))
        return -1;

    if (!_modules[module].found)
        return -1;

    return readByte(module, REG_INTCAPA);
}


int MultiMCP23017::readInterruptCaptureB(uint8_t module)
{
    if (!validModule(module))
        return -1;

    if (!_modules[module].found)
        return -1;

    return readByte(module, REG_INTCAPB);
}


bool MultiMCP23017::writeByte(uint8_t module,
                              uint8_t registerAddress,
                              uint8_t value)
{
    if (!validModule(module))
        return false;

    _wire->beginTransmission(_modules[module].address);

    _wire->write(registerAddress);
    _wire->write(value);

    return _wire->endTransmission() == 0;
}


bool MultiMCP23017::writeBytes(uint8_t module,
                               uint8_t registerAddress,
                               const uint8_t *data,
                               uint8_t length)
{
    if (!validModule(module))
        return false;

    if (data == NULL || length == 0)
        return false;

    _wire->beginTransmission(_modules[module].address);

    _wire->write(registerAddress);

    for (uint8_t i = 0; i < length; ++i)
        _wire->write(data[i]);

    return _wire->endTransmission() == 0;
}


int MultiMCP23017::readByte(uint8_t module,
                            uint8_t registerAddress)
{
    if (!validModule(module))
        return -1;

    _wire->beginTransmission(_modules[module].address);

    _wire->write(registerAddress);

    if (_wire->endTransmission(false) != 0)
        return -1;

    uint8_t received = _wire->requestFrom(
        (int)_modules[module].address,
        1
    );

    if (received != 1)
        return -1;

    if (!_wire->available())
        return -1;

    return _wire->read();
}


bool MultiMCP23017::readBytes(uint8_t module,
                               uint8_t registerAddress,
                               uint8_t *data,
                               uint8_t length)
{
    if (!validModule(module))
        return false;

    if (data == NULL || length == 0)
        return false;

    _wire->beginTransmission(_modules[module].address);

    _wire->write(registerAddress);

    if (_wire->endTransmission(false) != 0)
        return false;

    uint8_t received = _wire->requestFrom(
        (int)_modules[module].address,
        (int)length
    );

    if (received != length)
    {
        while (_wire->available())
            _wire->read();

        return false;
    }

    for (uint8_t i = 0; i < length; ++i)
    {
        if (!_wire->available())
            return false;

        data[i] = _wire->read();
    }

    return true;
}


void MultiMCP23017::updateCachedRegister(uint8_t module,
                                          uint8_t registerAddress,
                                          uint8_t value)
{
    if (!validModule(module))
        return;

    switch (registerAddress)
    {
        case REG_IODIRA:
            _modules[module].iodirA = value;
            break;

        case REG_IODIRB:
            _modules[module].iodirB = value;
            break;

        case REG_IPOLA:
            _modules[module].ipolA = value;
            break;

        case REG_IPOLB:
            _modules[module].ipolB = value;
            break;

        case REG_GPINTENA:
            _modules[module].gpintenA = value;
            break;

        case REG_GPINTENB:
            _modules[module].gpintenB = value;
            break;

        case REG_DEFVALA:
            _modules[module].defvalA = value;
            break;

        case REG_DEFVALB:
            _modules[module].defvalB = value;
            break;

        case REG_INTCONA:
            _modules[module].intconA = value;
            break;

        case REG_INTCONB:
            _modules[module].intconB = value;
            break;

        case REG_GPPUA:
            _modules[module].gppuA = value;
            break;

        case REG_GPPUB:
            _modules[module].gppuB = value;
            break;

        case REG_OLATA:
            _modules[module].olatA = value;
            break;

        case REG_OLATB:
            _modules[module].olatB = value;
            break;

        default:
            break;
    }
}


bool MultiMCP23017::setBitRegister(uint8_t module,
                                   uint8_t registerAddress,
                                   uint8_t bit,
                                   bool state)
{
    if (bit > 7)
        return false;

    int current = readByte(module, registerAddress);

    if (current < 0)
        return false;

    uint8_t value = (uint8_t)current;
    uint8_t mask = (uint8_t)(1U << bit);

    if (state)
        value |= mask;
    else
        value &= (uint8_t)~mask;

    if (!writeByte(module,
                   registerAddress,
                   value))
    {
        return false;
    }

    updateCachedRegister(module,
                         registerAddress,
                         value);

    return true;
}
