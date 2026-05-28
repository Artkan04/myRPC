#!/bin/bash
# Скрипт удаления myRPC

set -e

if [ "$EUID" -ne 0 ]; then
    echo "Запустите с правами root: sudo $0"
    exit 1
fi

echo "=== Удаление myRPC ==="

# Остановка сервера
if systemctl is-active --quiet myRPC-server 2>/dev/null; then
    echo "Остановка сервера..."
    systemctl stop myRPC-server
fi

# Удаление пакетов
echo "Удаление пакетов..."
dpkg -r myrpc-client 2>/dev/null || echo "Клиент не установлен"
dpkg -r myrpc-server 2>/dev/null || echo "Сервер не установлен"

# Очистка конфигов (опционально)
read -p "Удалить конфигурационные файлы? [y/N] " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    rm -rf /etc/myRPC
    rm -f /lib/systemd/system/myRPC-server.service
    systemctl daemon-reload
    echo "Конфигурация удалена"
fi

echo ""
echo "✓ myRPC удален"
