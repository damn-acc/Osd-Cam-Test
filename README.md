-----

# OSD Camera & Control System

## Збірка

### 1\. Встановлення залежностей

Виконайте наступні команди в терміналі для встановлення інструментів розробки, OpenCV та інших системних бібліотек:

```bash
sudo apt update
sudo apt install -y build-essential cmake libopencv-dev nlohmann-json3-dev psmisc
sudo apt install -y python3-pip python3-venv
```

### 2\. Налаштування Python середовища

У директорії проєкту рекомендується створити віртуальне середовище, після цього необхідно встановити у нього бібліотеку Flask

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install flask
```

### 3\. Компіляція C++ додатка

```bash
cmake -S . -B build
cmake --build build
```

*(Або у VS Code через Task Runner (Build))*

-----

## Запуск

Система складається з двох модулів, які мають працювати одночасно.

### 1\. Відео-модуль (C++)

Запуск обробки відео з накладанням OSD:

```bash
# Стандартний запуск
./build/cam_app --camera=/dev/video0

# Запуск із реверсом компаса
./build/cam_app --camera=/dev/video0 -cr
```

*(Або у VS Code: Terminal -\> Run Task -\> **Run(cw)** або **Run(ccw)**)*

### 2\. Веб-інтерфейс (Python)

Запуск панелі керування параметрами:

```bash
source .venv/bin/activate
python3 web.py
```

Адреса у браузері: `http://127.0.0.1:5000`
Логін: `admin`

-----
