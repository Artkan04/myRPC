#!/bin/bash
# Основной скрипт установки myRPC

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "========================================"
echo "  Установка myRPC v1.0.0"
echo "========================================"
echo ""

# Проверка прав
if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}Запустите с правами root: sudo $0${NC}"
    exit 1
fi

# Шаг 1: Установка зависимостей
echo -e "${YELLOW}[1/5] Установка зависимостей...${NC}"
if [ -f "$(dirname "$0")/install_deps.sh" ]; then
    bash "$(dirname "$0")/install_deps.sh"
else
    echo "Установка зависимостей напрямую..."
    apt-get update -qq
    apt-get install -y -qq build-essential gcc make dpkg-dev
fi
echo ""

# Шаг 2: Сборка проекта
echo -e "${YELLOW}[2/5] Сборка проекта...${NC}"
cd "$(dirname "$0")/.."
make clean 2>/dev/null || true
if make all; then
    echo -e "${GREEN}✓ Сборка выполнена успешно${NC}"
else
    echo -e "${RED}✗ Ошибка сборки${NC}"
    exit 1
fi
echo ""

# Шаг 3: Сборка deb-пакетов
echo -e "${YELLOW}[3/5] Сборка deb-пакетов...${NC}"
if make deb; then
    echo -e "${GREEN}✓ DEB-пакеты созданы${NC}"
else
    echo -e "${RED}✗ Ошибка создания пакетов${NC}"
    exit 1
fi
echo ""

# Шаг 4: Установка пакетов
echo -e "${YELLOW}[4/5] Установка пакетов...${NC}"
DEB_CLIENT=$(ls myrpc-client_*.deb 2>/dev/null | head -1)
DEB_SERVER=$(ls myrpc-server_*.deb 2>/dev/null | head -1)

if [ -n "$DEB_CLIENT" ]; then
    dpkg -i "$DEB_CLIENT"
    echo -e "${GREEN}✓ Клиент установлен${NC}"
fi

if [ -n "$DEB_SERVER" ]; then
    dpkg -i "$DEB_SERVER"
    echo -e "${GREEN}✓ Сервер установлен${NC}"
fi
echo ""

# Шаг 5: Проверка установки
echo -e "${YELLOW}[5/5] Проверка установки...${NC}"
ERRORS=0

# Проверка клиента
if command -v myRPC-client &> /dev/null; then
    echo -e "${GREEN}✓ myRPC-client найден в PATH${NC}"
    myRPC-client --help > /dev/null 2>&1 && echo -e "${GREEN}✓ myRPC-client работает${NC}" || echo -e "${RED}✗ myRPC-client не запускается${NC}"
else
    echo -e "${RED}✗ myRPC-client не найден${NC}"
    ERRORS=$((ERRORS + 1))
fi

# Проверка сервера
if command -v myRPC-server &> /dev/null; then
    echo -e "${GREEN}✓ myRPC-server найден в PATH${NC}"
else
    echo -e "${RED}✗ myRPC-server не найден${NC}"
    ERRORS=$((ERRORS + 1))
fi

# Проверка конфигов
if [ -f /etc/myRPC/myRPC.conf ]; then
    echo -e "${GREEN}✓ Конфигурация сервера установлена${NC}"
else
    echo -e "${YELLOW}⚠ Конфигурация не найдена в /etc/myRPC/${NC}"
fi

if [ -f /etc/myRPC/users.conf ]; then
    echo -e "${GREEN}✓ Файл пользователей установлен${NC}"
else
    echo -e "${YELLOW}⚠ Файл пользователей не найден${NC}"
fi

# Проверка systemd сервиса
if [ -f /lib/systemd/system/myRPC-server.service ]; then
    echo -e "${GREEN}✓ Systemd сервис установлен${NC}"
else
    echo -e "${YELLOW}⚠ Systemd сервис не установлен (опционально)${NC}"
fi

echo ""
echo "========================================"
if [ $ERRORS -eq 0 ]; then
    echo -e "${GREEN}  Установка завершена успешно!${NC}"
else
    echo -e "${YELLOW}  Установка завершена с $ERRORS предупреждениями${NC}"
fi
echo "========================================"
echo ""
echo "Настройка сервера:"
echo "  1. Отредактируйте /etc/myRPC/users.conf (добавьте пользователей)"
echo "  2. Отредактируйте /etc/myRPC/myRPC.conf (настройте порт)"
echo "  3. Запустите сервер: systemctl start myRPC-server"
echo ""
echo "Использование клиента:"
echo "  myRPC-client -h АДРЕС -p ПОРТ -s -c \"КОМАНДА\""
echo ""
