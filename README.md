# MultiMCP23017

<p align="center">
  <img src="images/MultiMCP23017.jpg" alt="MultiMCP23017" width="600">
</p>

**MultiMCP23017** es una librería Arduino / ESP8266 / ESP32 autónoma para controlar uno o varios expansores GPIO **MCP23017 mediante I²C** como si formaran un único sistema de GPIO virtuales.

La librería no depende de ninguna librería externa específica del MCP23017. Utiliza directamente `Wire.h` y contiene la lógica necesaria para comunicarse con el dispositivo.

---

## Características

- Control de hasta 8 MCP23017 en un mismo bus I²C.
- Hasta 128 GPIO virtuales.
- Direcciones `0x20` a `0x27`.
- API GPIO similar a Arduino.
- `pinMode()`.
- `digitalWrite()`.
- `digitalRead()`.
- `INPUT`.
- `OUTPUT`.
- `INPUT_PULLUP`.
- Acceso directo a GPIOA y GPIOB.
- Acceso de 16 bits.
- Acceso directo a registros.
- Detección individual de módulos.
- Caché interno de registros controlados por la librería.
- Soporte para `TwoWire`.
- Compatible con Arduino AVR, ESP8266, ESP32 y otros cores Arduino compatibles con `Wire`.
- Sin dependencias externas.
- Soporte inicial para interrupciones del MCP23017.

---

# ¿Qué es el MCP23017?

El MCP23017 es un expansor de entradas/salidas digitales de 16 bits con interfaz I²C fabricado por Microchip.

Los 16 GPIO están divididos en dos puertos:

```text
GPIOA
  GPA0
  GPA1
  GPA2
  GPA3
  GPA4
  GPA5
  GPA6
  GPA7

GPIOB
  GPB0
  GPB1
  GPB2
  GPB3
  GPB4
  GPB5
  GPB6
  GPB7
```

El dispositivo permite configurar individualmente cada GPIO como entrada o salida y proporciona pull-ups internos, inversión de polaridad e interrupciones.

Microchip especifica tres entradas de dirección A0/A1/A2, permitiendo hasta ocho dispositivos MCP23017 en el mismo bus I²C. 

---

# Arquitectura de MultiMCP23017

La librería convierte los GPIO de todos los MCP23017 en un único espacio virtual.

Por ejemplo:

```cpp
MultiMCP23017 io(3, 0x20);
```

produce:

```text
Módulo 0
0x20
GPIO virtuales 0 - 15

Módulo 1
0x21
GPIO virtuales 16 - 31

Módulo 2
0x22
GPIO virtuales 32 - 47
```

Por tanto:

```cpp
io.digitalWrite(0, HIGH);
```

actúa sobre:

```text
0x20 -> GPA0
```

Mientras:

```cpp
io.digitalWrite(20, HIGH);
```

actúa sobre:

```text
0x21 -> GPA4
```

---

# Direcciones I²C

El MCP23017 dispone de tres entradas de dirección:

```text
A2
A1
A0
```

Esto permite utilizar ocho direcciones:

| Módulo | Dirección | GPIO virtuales |
|---:|---:|---:|
| 0 | `0x20` | 0 - 15 |
| 1 | `0x21` | 16 - 31 |
| 2 | `0x22` | 32 - 47 |
| 3 | `0x23` | 48 - 63 |
| 4 | `0x24` | 64 - 79 |
| 5 | `0x25` | 80 - 95 |
| 6 | `0x26` | 96 - 111 |
| 7 | `0x27` | 112 - 127 |

Por ejemplo:

```cpp
MultiMCP23017 io(8, 0x20);
```

proporciona:

```text
8 × 16 = 128 GPIO virtuales
```

Las direcciones A0/A1/A2 son hardware del MCP23017; no se deben confundir con los pines SDA/SCL del bus I²C. 

---

# Instalación

Copia la carpeta:

```text
MultiMCP23017/
```

en:

```text
Arduino/libraries/
```

La estructura es:

```text
MultiMCP23017/
├── src/
│   ├── MultiMCP23017.h
│   └── MultiMCP23017.cpp
├── examples/
│   └── Basic/
│       └── Basic.ino
├── images/
│   └── MultiMCP23017.jpg
├── library.properties
└── README.md
```

No es necesario instalar ninguna librería adicional.

La única dependencia es:

```cpp
#include <Wire.h>
```

---

# Ejemplo básico

```cpp
#include <Wire.h>
#include <MultiMCP23017.h>

MultiMCP23017 io(3, 0x20);

void setup()
{
    Serial.begin(115200);

    Wire.begin();

    if (!io.begin())
    {
        Serial.println("Error inicializando MCP23017");
    }

    io.pinMode(0, OUTPUT);
    io.pinMode(20, INPUT_PULLUP);
}

void loop()
{
    io.digitalWrite(0, HIGH);

    delay(500);

    io.digitalWrite(0, LOW);

    delay(500);

    if (io.digitalRead(20) == LOW)
    {
        Serial.println("Boton pulsado");
    }
}
```

---

# Bus I²C

La librería recibe una referencia a `TwoWire`, por lo que no presupone que el sistema utilice un determinado bus o unos determinados pines.

Uso habitual:

```cpp
MultiMCP23017 io(3, 0x20, Wire);
```

## Arduino AVR

```cpp
Wire.begin();
```

## ESP8266

Por ejemplo:

```cpp
Wire.begin(D2, D1);
```

donde:

```text
D2 -> SDA
D1 -> SCL
```

## ESP32

Por ejemplo:

```cpp
Wire.begin(21, 22);
```

donde:

```text
GPIO21 -> SDA
GPIO22 -> SCL
```

Los pines concretos dependen de la placa y del core utilizado. La librería no configura automáticamente SDA/SCL.

---

# API

## begin()

Inicializa los MCP23017 configurados.

```cpp
if (!io.begin())
{
    Serial.println("No se ha encontrado ningun MCP23017");
}
```

`begin()` comprueba individualmente cada dirección.

Si uno de los módulos no responde, los demás pueden continuar funcionando.

El valor devuelto es:

```text
true
```

si se encontró al menos un módulo.

```text
false
```

si ninguno respondió.

---

## moduleCount()

Devuelve el número de módulos configurados.

```cpp
uint8_t count = io.moduleCount();
```

---

## pinCount()

Devuelve el número total de GPIO virtuales.

```cpp
uint8_t count = io.pinCount();
```

Por ejemplo:

```text
3 módulos × 16 GPIO = 48
```

---

## address()

Devuelve la dirección I²C de un módulo.

```cpp
uint8_t addr = io.address(1);
```

Resultado:

```text
0x21
```

---

## moduleFound()

Permite comprobar individualmente un módulo.

```cpp
if (io.moduleFound(2))
{
    Serial.println("Modulo encontrado");
}
```

---

## validPin()

Comprueba si un GPIO virtual es válido.

```cpp
if (io.validPin(pin))
{
    io.digitalWrite(pin, HIGH);
}
```

---

# GPIO virtuales

Cada MCP23017 proporciona 16 GPIO.

La asignación es:

```text
Virtual 0  -> GPA0
Virtual 1  -> GPA1
Virtual 2  -> GPA2
Virtual 3  -> GPA3
Virtual 4  -> GPA4
Virtual 5  -> GPA5
Virtual 6  -> GPA6
Virtual 7  -> GPA7

Virtual 8  -> GPB0
Virtual 9  -> GPB1
Virtual 10 -> GPB2
Virtual 11 -> GPB3
Virtual 12 -> GPB4
Virtual 13 -> GPB5
Virtual 14 -> GPB6
Virtual 15 -> GPB7
```

Para el módulo siguiente se suma 16.

---

# pinMode()

Configura un GPIO virtual.

## OUTPUT

```cpp
io.pinMode(0, OUTPUT);
```

## INPUT

```cpp
io.pinMode(0, INPUT);
```

## INPUT_PULLUP

```cpp
io.pinMode(0, INPUT_PULLUP);
```

En el MCP23017 `INPUT_PULLUP` se implementa mediante dos registros:

```text
IODIR = 1
GPPU  = 1
```

Es decir, el pin se configura realmente como entrada y se activa el pull-up interno.

---

# digitalWrite()

```cpp
io.digitalWrite(0, HIGH);
```

o:

```cpp
io.digitalWrite(0, LOW);
```

La librería mantiene el estado conocido de `OLATA` y `OLATB`.

Al modificar un único GPIO, sólo se modifica el bit correspondiente.

Por ejemplo:

```cpp
io.digitalWrite(3, HIGH);
```

no requiere reescribir los otros siete bits con valores obtenidos mediante una lectura del puerto.

El registro utilizado es `OLAT`.

Esto permite separar el estado del latch de salida del estado físico leído mediante `GPIO`.

---

# digitalRead()

```cpp
int value = io.digitalRead(20);
```

Devuelve:

```text
HIGH
LOW
```

o:

```text
-1
```

si la operación I²C falla o el GPIO no es válido.

Las lecturas se realizan sobre los registros `GPIOA`/`GPIOB`.

---

# Acceso por puerto

También es posible trabajar directamente con los dos puertos de ocho bits.

## Puerto A

```cpp
io.writePortA(0, 0xAA);
```

Esto escribe:

```text
GPA7..GPA0 = 10101010
```

## Puerto B

```cpp
io.writePortB(0, 0x55);
```

Esto escribe:

```text
GPB7..GPB0 = 01010101
```

Lectura:

```cpp
uint8_t a = io.readPortA(0);
uint8_t b = io.readPortB(0);
```

Las escrituras de puerto utilizan una única transferencia de datos I²C.

---

# Acceso de 16 bits

MultiMCP23017 utiliza la siguiente representación:

```text
bit 0  - bit 7   -> GPIOA
bit 8  - bit 15  -> GPIOB
```

Por ejemplo:

```cpp
io.write16(0, 0x55AA);
```

equivale a:

```text
GPIOA = 0xAA
GPIOB = 0x55
```

Lectura:

```cpp
uint16_t value = io.read16(0);
```

La operación de 16 bits aprovecha el direccionamiento secuencial del MCP23017 para transferir ambos registros dentro de una misma transacción.

---

# Registros internos

La librería utiliza `IOCON.BANK = 0`.

El mapa relevante es:

| Registro | Dirección |
|---|---:|
| IODIRA | `0x00` |
| IODIRB | `0x01` |
| IPOLA | `0x02` |
| IPOLB | `0x03` |
| GPINTENA | `0x04` |
| GPINTENB | `0x05` |
| DEFVALA | `0x06` |
| DEFVALB | `0x07` |
| INTCONA | `0x08` |
| INTCONB | `0x09` |
| IOCON | `0x0A` |
| GPPUA | `0x0C` |
| GPPUB | `0x0D` |
| INTFA | `0x0E` |
| INTFB | `0x0F` |
| INTCAPA | `0x10` |
| INTCAPB | `0x11` |
| GPIOA | `0x12` |
| GPIOB | `0x13` |
| OLATA | `0x14` |
| OLATB | `0x15` |

Estas direcciones corresponden al mapa BANK=0 especificado por Microchip.
---

# BANK y SEQOP

La librería establece:

```text
IOCON.BANK = 0
IOCON.SEQOP = 0
```

Esto produce un mapa de registros intercalado A/B y permite que el puntero interno avance automáticamente durante una transferencia secuencial.

Por ejemplo:

```text
OLATA = 0x14
OLATB = 0x15
```

y:

```text
GPIOA = 0x12
GPIOB = 0x13
```

Por ello:

```cpp
io.write16(0, 0x55AA);
```

puede realizarse como:

```text
START
ADDRESS + WRITE
0x14
0xAA
0x55
STOP
```

El MCP23017 incrementa el puntero de dirección entre ambos bytes cuando `SEQOP=0`.

---

# Acceso directo a registros

Para usuarios avanzados:

```cpp
io.writeRegister(0, 0x02, 0xFF);
```

y:

```cpp
int value = io.readRegister(0, 0x12);
```

Estas funciones trabajan con el mapa:

```text
BANK = 0
```

La librería protege los bits:

```text
IOCON.BANK
IOCON.SEQOP
```

porque la implementación depende de:

```text
BANK = 0
SEQOP = 0
```

Sí permite utilizar los demás bits de configuración de `IOCON`.

---

# Interrupciones

El MCP23017 dispone de dos salidas de interrupción:

```text
INTA
INTB
```

La interrupción puede configurarse de diferentes maneras.

El dispositivo permite generar una interrupción cuando una entrada cambia respecto a su estado anterior o cuando se compara con el valor configurado en `DEFVAL`. Microchip también proporciona `INTCAP` para capturar el estado del puerto que produjo la interrupción. 

## Activar interrupción

```cpp
io.enableInterrupt(20);
```

Desactivar:

```cpp
io.disableInterrupt(20);
```

## Interrupt-on-change

```cpp
io.setInterruptMode(
    20,
    MultiMCP23017::INTERRUPT_CHANGE
);
```

## Comparación contra DEFVAL

```cpp
io.setInterruptMode(
    20,
    MultiMCP23017::INTERRUPT_COMPARE
);
```

Después:

```cpp
io.setInterruptDefault(20, HIGH);
```

y:

```cpp
io.enableInterrupt(20);
```

---

# INTA / INTB

Por defecto las interrupciones de los dos puertos son independientes.

Es posible configurar:

```cpp
io.configureInterruptOutput(
    0,
    true,
    false,
    false
);
```

donde:

```text
true  -> MIRROR
false -> push-pull
false -> activo LOW
```

Los parámetros corresponden a:

```text
MIRROR
ODR
INTPOL
```

del registro `IOCON`.

`MIRROR` permite asociar lógicamente las interrupciones de ambos puertos a ambas salidas. `ODR` permite utilizar la salida como open-drain y `INTPOL` selecciona la polaridad.
---

# INTFA / INTFB

Los registros:

```text
INTFA
INTFB
```

indican qué GPIO provocaron una interrupción.

Se pueden leer mediante:

```cpp
int flagsA = io.readInterruptFlagsA(0);
int flagsB = io.readInterruptFlagsB(0);
```

---

# INTCAPA / INTCAPB

Los registros:

```text
INTCAPA
INTCAPB
```

capturan el estado del puerto en el momento asociado a la interrupción.

Por ejemplo:

```cpp
uint8_t captured =
    io.readInterruptCaptureA(0);
```

La captura resulta especialmente útil cuando el programa no puede leer el GPIO inmediatamente después de detectar la señal INT.

---

# Limitación importante de las interrupciones

`MultiMCP23017` controla la configuración del MCP23017, pero no puede convertir automáticamente `INTA`/`INTB` en una interrupción del microcontrolador.

Las conexiones físicas deben realizarse externamente:

```text
MCP23017 INTA ----> GPIO interrupt del MCU
MCP23017 INTB ----> GPIO interrupt del MCU
```

La librería no necesita conocer qué GPIO del Arduino recibe físicamente la señal.

El programa de usuario debe configurar esa entrada mediante la API de interrupciones de su plataforma.

---

# Estado interno y caché

La librería mantiene en memoria solamente el estado que necesita para realizar modificaciones seguras:

```text
IODIRA
IODIRB

GPPUA
GPPUB

OLATA
OLATB

IPOLA
IPOLB

GPINTENA
GPINTENB

DEFVALA
DEFVALB

INTCONA
INTCONB
```

Esto permite realizar operaciones como:

```cpp
io.digitalWrite(3, HIGH);
```

sin tener que hacer previamente:

```text
I²C read
I²C modify
I²C write
```

La librería puede modificar directamente el bit conocido y efectuar una sola escritura.

El estado cacheado no pretende representar todos los registros físicos del MCP23017.

Por ejemplo, `GPIOA` es un registro de estado físico de entrada y se lee del dispositivo cuando se solicita.

---

# Rendimiento I²C

La prioridad de la librería es:

1. Corrección.
2. Seguridad.
3. Claridad.
4. Eficiencia I²C.

Ejemplos:

```cpp
io.writePortA(0, 0xAA);
```

utiliza una sola escritura de registro.

```cpp
io.write16(0, 0x55AA);
```

utiliza una transferencia secuencial de dos registros.

En cambio:

```cpp
io.digitalRead(0);
```

requiere una lectura del MCP23017 porque el estado solicitado corresponde al GPIO físico.

---

# Consideraciones eléctricas

El MCP23017 debe alimentarse dentro del rango especificado para la variante concreta del componente.

Microchip especifica para el dispositivo un rango de alimentación que llega hasta 5,5 V y, dependiendo de la condición/variante, parte desde 1,8 V o 2,7 V. Antes de diseñar el hardware se debe consultar la versión exacta del datasheet correspondiente al componente utilizado. 

No se deben asumir automáticamente los niveles eléctricos de un módulo comercial únicamente por llevar la etiqueta "MCP23017".

## I²C

SDA y SCL requieren las condiciones eléctricas apropiadas del bus I²C, incluyendo sus resistencias de pull-up.

La tensión de las pull-ups debe ser compatible con:

- el MCP23017;
- el microcontrolador;
- el resto de dispositivos conectados al bus.

No se deben conectar directamente dispositivos de 5 V a GPIO de 3,3 V sin verificar la compatibilidad eléctrica.

## GPIO

Los GPIO del MCP23017 no deben utilizarse para alimentar cargas que superen las especificaciones del dispositivo.

Aunque Microchip indica capacidad de sink/source de hasta 25 mA por I/O en sus características del producto, esto **no significa que todos los GPIO puedan operar simultáneamente a 25 mA sin considerar las limitaciones globales y las condiciones eléctricas del datasheet**. 
Para:

- motores;
- relés;
- solenoides;
- tiras LED;
- cargas de potencia;
- actuadores;

se recomienda utilizar transistores, MOSFETs, drivers o circuitos de interfaz apropiados.

---

# Módulos comerciales

Muchos módulos comerciales incorporan:

- MCP23017;
- resistencias pull-up I²C;
- jumpers A0/A1/A2;
- terminales de alimentación;
- conectores GPIO.

Hay que comprobar el esquema del módulo antes de conectar varios en paralelo.

En particular, deben revisarse:

- dirección;
- pull-ups I²C;
- tensión de alimentación;
- conexión de RESET;
- conexión de INTA/INTB;
- capacidad de las resistencias pull-up;
- compatibilidad de niveles lógicos.

---

# Compatibilidad

La librería está diseñada para utilizar:

```cpp
TwoWire
```

en lugar de asumir una implementación concreta del bus.

Por ello puede utilizarse en:

- Arduino AVR.
- ESP8266.
- ESP32.
- Otros microcontroladores compatibles con Arduino `Wire`.

Ejemplo:

```cpp
MultiMCP23017 io(3, 0x20, Wire);
```

---


# Limitaciones actuales

La versión inicial utiliza direcciones consecutivas:

```cpp
MultiMCP23017 io(8, 0x20);
```

Esto genera:

```text
0x20
0x21
0x22
0x23
0x24
0x25
0x26
0x27
```

El diseño interno mantiene las direcciones por módulo, por lo que una futura API de módulos con direcciones arbitrarias puede añadirse sin cambiar el modelo de GPIO virtual.

Por ejemplo, una futura API podría permitir:

```cpp
io.addModule(0x20);
io.addModule(0x23);
io.addModule(0x27);
```

---

# Roadmap

Posibles funciones futuras:

- Direcciones no consecutivas mediante `addModule()`.
- Configuración avanzada de interrupciones.
- API de lectura de ambos `INTCAP` en una sola operación.
- API para configurar polaridad.
- API para configurar `DEFVAL` por puerto.
- Operaciones de puertos con máscaras.
- Configuración explícita de `IOCON`.
- Métodos para consultar el estado de interrupción de un módulo.
- Optimización de operaciones múltiples.
- Tests automatizados con hardware real.
- Ejemplos específicos para ESP8266 y ESP32.

---

# Licencia

Este proyecto puede distribuirse bajo los términos de la licencia que se indica en el repositorio.

---

# Referencia técnica

La implementación se basa en el datasheet oficial:

**Microchip MCP23017/MCP23S17 — 16-Bit I/O Expander with Serial Interface, DS20001952.**

El mapa BANK=0, el funcionamiento de `SEQOP`, las operaciones I²C secuenciales y la arquitectura de registros utilizados por la librería están basados en la documentación de Microchip. 

Información oficial del dispositivo:

[Microchip MCP23017](https://www.microchip.com/en-us/product/mcp23017?utm_source=chatgpt.com)
