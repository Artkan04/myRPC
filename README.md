# myRPC Project
```test
ЗвОС, Ставцев Алексей Вячеславович
Кан А.Е, студент группы ККСО-06-22
````
### Компоненты:

* **myRPC-server** — сервер-демон для обработки клиентских запросов;
* **myRPC-client** — консольная утилита для отправки команд на сервер;
* **libmysyslog** — библиотека для логирования действий клиента и сервера.

---

# Структура проекта

```text
myRPC/
├── common/                    # JSON протокол
│   ├── protocol.c
│   ├── protocol.h
│   └── README.md
├── config/                    # Конфигурационные файлы
│   ├── myRPC.conf
│   ├── users.conf
│   └── README.md
├── libmysyslog/               # Библиотека логирования
│   ├── mysyslog.h
│   ├── mysyslog.c
│   ├── Makefile
│   └── README.md
├── scripts/                   # Скрипты установки и тестирования
│   ├── install.sh
│   ├── install_deps.sh
│   ├── uninstall.sh
│   ├── test.sh
│   ├── myRPC-server.service
│   ├── requirements.list
│   └── README.md
├── src/                       # Исходный код
│   ├── client/
│   │   ├── myRPC-client.c
│   │   ├── myRPC-client.h
│   │   └── Makefile
│   ├── server/
│   │   ├── myRPC-server.c
│   │   ├── myRPC-server.h
│   │   └── Makefile
│   └── README.md
├── Makefile                  
└── README.md               
````

# Установка на сервере

```bash
# Клонируйте репозиторий
git clone https://github.com/Artkan04/myRPC.git
cd myRPC

# Запустите установку
sudo bash scripts/install.sh

```
# Настройка сервера

```bash
# Добавьте пользователей в whitelist (одна строка = пользователь)
sudo nano /etc/myRPC/users.conf

Пример:
root
user1
user2

# Запустите сервер
sudo systemctl start myRPC-server
```

# Установка на клиенте

```bash
# Клонируйте репозиторий
git clone https://github.com/Artkan04/myRPC.git
cd myRPC

# Установите зависимости
sudo apt update
sudo apt install -y build-essential gcc make

# Сборка клиента
make -C libmysyslog
make -C src/client

# Установка клиента
sudo cp src/client/myRPC-client /usr/local/bin/
```

### Отправка запроса с клиента

```bash
# Пример
myRPC-client -h 192.168.1.10 -p 8642 -s -c "whoami"
myRPC-client -h 192.168.1.10 -p 8642 -s -c "ls -la /tmp"
```

# Логировние
```bash
/var/log/myrpc-server.log
```

# Запрос клиента

```json
{
  "login": "имя_пользователя",
  "command": "bash-команда"
}
```

# Ответ сервера

```json
{
  "code": 0,
  "result": "Результат"
}
```

* `code = 0` — успех
* `code = 1` — ошибка

---

# Аргументы клиента

| Аргумент    | Назначение                  |
| ----------- | --------------------------- |
| `--host`    | IP-адрес сервера            |
| `--port`    | Порт сервера                |
| `--command` | Команда bash для выполнения |
| `--stream`  | Использовать TCP            |
| `--dgram`   | Использовать UDP            |
| `--help`    | Показать справку            |

---

---

# Требования
* GCC, make
* Astra Linux SE 1.7 (или другой Linux)
* Права на запуск и чтение конфигов (`sudo`)
---
