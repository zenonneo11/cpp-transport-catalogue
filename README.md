# transport catalogue
## Система для хранения транспортных маршрутов и обработки запросов к ней
Работа программы разделена на 2 стадии:
* Стадия make_base: создание базы транспортного справочника по запросам base_requests и её сериализация в файл.  
* Стадия process_requests: десериализация базы из файла и использование её для ответов на запросы stat_requests.

### Стадия make_base
На вход программе make_base через стандартный поток ввода подаётся JSON со следующими ключами:
* base_requests: запросы Bus и Stop на создание базы.
  * Bus X: описание маршрута - Запрос на добавление автобусного маршрута X
  * Stop X: latitude, longitude, D1m to stop1, D2m to stop2, ... - Добавляет информацию об остановке с названием X, после широты и долготы содержится список расстояний от этой остановки до соседних с ней остановок. 
* routing_settings: настройки маршрутизации.
* render_settings: настройки отрисовки.
* serialization_settings: настройки сериализации. В этот файл сохраняется сериализованная база.

Задача на стадии make_base — построить базу и сериализовать её в файл с указанным именем.

### Стадия process_requests
На вход программе process_requests подаётся файл с сериализованной базой (результат работы make_base), а также — через стандартный поток ввода — JSON со следующими ключами:
* stat_requests: запросы Bus, Stop, Map и Route к готовой базе.
  * Bus X - Вывести информацию об автобусном маршруте X
  * Stop - Вывести информацию об остановке.
  * Map - построить карту маршрутов в svg формате
  * Route - 
* serialization_settings: настройки сериализации в формате, аналогичном этой же секции на входе make_base. А именно, в ключе file указывается название файла, из которого нужно считать сериализованную базу.

Программа process_requests выводит JSON с ответами на запросы.

### Используемый стандарт языка
C++ 17

### Используемая версия Protobuf
3.21.12

### Планы по улучшению
* Использовать GTest для юнит тестирования  
* GUI
