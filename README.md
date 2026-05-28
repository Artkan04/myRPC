# myRPC Project
```test
ЗвОС, Ставцев Алексей Вячеславович
Кан А.Е, студент группы ККСО-06-22
````
### Компоненты системы:

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

---

# Сборка проекта

Для сборки всех компонентов выполните в корне проекта:

```bash
make
```

---

# Установка

```bash
# Клонируйте репозиторий
git clone https://github.com/Artkan04/myRPC
cd myRPC

# Запустите установку от root
sudo bash scripts/install.sh
```

---

После установки `myRPC-client` и `myRPC-server` будут доступны в `/usr/bin`.

Логи пишутся в:

```text
/var/log/myrpc-server.log
```

---

# Конфигурация

Создайте папку и конфигурационные файлы:

```bash
sudo mkdir -p /etc/myRPC
```

## `/etc/myRPC/myRPC.conf`

```text
# Порт для соединения
port = 8642

# Тип сокета: stream или dgram
socket_type = stream

# Режим: daemon или console
mode = daemon
```

## `/etc/myRPC/users.conf`

```text
# Список разрешенных пользователей (по одному в строке)
root
```

---

# Пример использования

## Запуск сервера

```bash
sudo myRPC-server /etc/myRPC/myRPC.conf
```

## Запуск клиента (TCP)

```bash
myRPC-client --host 127.0.0.1 --port 8642 --stream --command "ls -l /tmp"
```

## Запуск клиента (UDP)

```bash
myRPC-client --host 127.0.0.1 --port 8642 --dgram --command "whoami"
```

---

# Протокол (JSON)

## Запрос клиента

```json
{
  "login": "имя_пользователя",
  "command": "bash-команда"
}
```

## Ответ сервера

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

# Очистка

```bash
make clean
```

Удалит все `.o`, бинарники, временные и сборочные файлы.

---

---

# Требования
* GCC, make
* Astra Linux SE 1.7 (или другой Linux)
* Права на запуск и чтение конфигов (`sudo`)
---
