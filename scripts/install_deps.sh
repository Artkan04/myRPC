#!/bin/bash
# Установка зависимостей для myRPC

set -e

echo "=== Установка зависимостей myRPC ==="

# Проверка прав
if [ "$EUID" -ne 0 ]; then
    echo "Запустите с правами root: sudo $0"
    exit 1
fi

# Обновление пакетов
echo "[1/3] Обновление списка пакетов..."
apt-get update -qq

# Установка из списка
echo "[2/3] Установка зависимостей..."
if [ -f requirements.list ]; then
    xargs -a requirements.list apt-get install -y -qq
else
    echo "ОШИБКА: requirements.list не найден"
    exit 1
fi

echo "[3/3] Проверка установки..."
echo ""
echo "✓ Все зависимости установлены"
echo ""

# Проверка наличия ключевых программ
echo "Проверка компилятора:"
gcc --version | head -1
echo ""

echo "Проверка make:"
make --version | head -1
echo ""

echo "Проверка dpkg-deb:"
dpkg-deb --version | head -1
