# Контрольная работа №2 — вариант 3 (все задания)

## Как запустить (Qt Creator)
1. Распакуйте архив → Qt Creator → **File → Open File or Project** → `CMakeLists.txt`
   (или `Control_Work_02_Polymorphism.pro`, если работаете с qmake).
2. Выберите любой Desktop-kit (Qt 5.15 или Qt 6.x) → **Run** (Ctrl+R).

Откроется окно-стенд, а вывод заданий 1–3 и пяти сценариев автопилота
появится одновременно в *Application Output* и в журнале внизу окна.

Запуск только с консольным выводом: аргумент `--console`
(Projects → Run → Command line arguments).

**Звук.** Синтезируется кодом (WAV во временной папке) через Qt Multimedia.
Если модуль не установлен, проект всё равно соберётся, а вместо звуков будет системный beep.
Для Qt 6 модуль ставится через Qt Maintenance Tool → Additional Libraries → Qt Multimedia.

## Что где
| Файл | Содержимое |
|---|---|
| `uav.h/.cpp` | Задание 1: `UAV` → `SurveyUAV`, `CargoUAV`, `ReconnaissanceUAV`, `execute_test_mission(UAV&)` |
| `sensor.h/.cpp` | Задание 2: `Sensor` → `Lidar`, `Radar`, `Camera`, `obstacle_is_near(Sensor&, double)` |
| `controller.h/.cpp` | Задание 3: `FlightController` → `PIDController`, `SimpleController`, `FuzzyController`, `test_controller(...)` |
| `navigation.h/.cpp` | `Position`, `NavigationSystem` → `GPSNavigation` (нужно для автопилота) |
| `autopilot.h/.cpp` | Интеграционное задание: `Autopilot::run_cycle()` |
| `tasks.cpp` | Консольные демонстрации + 5 обязательных сценариев |
| `simulation.*`, `widgets.*`, `dashboard.*`, `soundengine.*` | Визуализация: графики, радар, анимация полёта, звук |

## Стенд (визуализация)
- **Пробел** — старт/пауза, **R** — сброс, **1–5** — тестовые сценарии из задания.
- Слева анимированный вид полёта (дрон, след, линия цели, вектор тяги).
- В центре графики высоты, ошибки и управляющего воздействия, вкладка сравнения трёх регуляторов.
- Справа радар Lidar/Radar с порогами 10/20 м и карточки сенсоров (клик по карточке = отказ сенсора).
- Регулятор переключается, включая `nullptr`; можно имитировать отказ GPS и отрицательную высоту.
- Звуки: старт, гул моторов (громкость зависит от тяги), сирена при прерывании цикла,
  сигнал при препятствии ближе 10 м, колокольчик при достижении цели.
