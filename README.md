# EmbUI
Embedded WebUI Interface

Фреймворк построения web-интерфейса и элементов управления для проектов под Arduino 
## Поддерживаемые платформы
 - ESP8266 Arduino Core
 - ESP32 Arduino Core

## Возможности
 - автопубликация контроллера в локальной сети через [mDNS](https://en.wikipedia.org/wiki/Multicast_DNS)/[ZeroConf](https://en.wikipedia.org/wiki/Zero-configuration_networking)
 - возможность обнаружения устройства:
    - [Service Browser](https://play.google.com/store/apps/details?id=com.druk.servicebrowser) Android
    - [SSDP](https://en.wikipedia.org/wiki/Simple_Service_Discovery_Protocol) for Windows
    - [Bonjour](https://en.wikipedia.org/wiki/Bonjour_(software)) iOS/MacOS
 - обмен данными с браузером через WebSocket
 - поддержка нескольких параллельных подключений, интерфейс обновляется одновременно на всех устройствах
 - self-hosted - нет зависимостей от внешних ресурсов/CDN/Cloud сервисов
 - встроенный WiFi менеджер, автопереключение в режим AP при потере клиентского соединения
 - полная поддержка всех существующих Временных Зон, автоматический переход на летнее/зимнее время, корректная калькуляция дат/временных интервалов
 - OTA, обновление прошивки/образа ФС через браузер
 - возможность подгружать данные/элементы интерфейса через AJAX

## Проекты на EmbUI
 - [FireLamp_JeeUI](https://github.com/DmytroKorniienko/FireLamp_JeeUI/tree/dev) - огненная лампа на светодиодной матрице ws2812
 - [ESPEM](https://github.com/vortigont/espem) - энергометр на основе измерителя PZEM-004


## Примеры построения интерфейсов

<img src="https://raw.githubusercontent.com/vortigont/espem/master/examples/espemembui.png" alt="espem ui" width="30%"/>

<img src="https://raw.githubusercontent.com/vortigont/espem/master/examples/espemembui_setup.png" alt="espem opts" width="30%"/>

## Использование
Для работы WebUI необходимо залить в контроллер образ фаловой системы LittleFS с web-ресурсами.
Подготовленные ресурсы для создания образа можно развернуть [из архива](https://github.com/DmytroKorniienko/EmbUI/raw/main/resources/data.zip).
В [Platformio](https://platformio.org/) это, обычно, каталог *data* в корне проекта.

## Depends

Projects           |                     URL                                    | Remarks
------------------ | :---------------------------------------------------------:| --------------
ArduinoJson        |  https://github.com/bblanchon/ArduinoJson.git              |
AsyncWebServer-mod |  https://github.com/DmytroKorniienko/ESPAsyncWebServer.git | manual install, fork
AsyncMqttClient    |  https://github.com/marvinroger/async-mqtt-client.git      | manual install
TaskScheduler      |  https://github.com/arkhipenko/TaskScheduler.git           |
LittleFS_esp32     |  https://github.com/lorol/LITTLEFS.git                     |
FtpClientServer    |  https://github.com/charno/FTPClientServer.git             | manual install, fork, esp32
ESP32SSDP          |  https://github.com/luc-github/ESP32SSDP.git               | manual install, esp32
