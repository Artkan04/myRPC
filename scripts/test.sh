#!/bin/bash
# Комплексное тестирование myRPC

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

PASSED=0
FAILED=0
SERVER_PID=""

cleanup() {
    if [ -n "$SERVER_PID" ]; then
        kill $SERVER_PID 2>/dev/null
        wait $SERVER_PID 2>/dev/null
    fi
    rm -f /tmp/test_output.txt /tmp/test_output2.txt
}
trap cleanup EXIT

echo "========================================"
echo "  Тестирование myRPC"
echo "========================================"
echo ""

# Тест 1: Проверка бинарников
echo "--- Тест 1: Наличие исполняемых файлов ---"
if command -v myRPC-client &> /dev/null; then
    echo -e "${GREEN}✓ myRPC-client найден${NC}"
    PASSED=$((PASSED + 1))
else
    echo -e "${RED}✗ myRPC-client не найден${NC}"
    FAILED=$((FAILED + 1))
fi

if command -v myRPC-server &> /dev/null; then
    echo -e "${GREEN}✓ myRPC-server найден${NC}"
    PASSED=$((PASSED + 1))
else
    echo -e "${RED}✗ myRPC-server не найден${NC}"
    FAILED=$((FAILED + 1))
fi
echo ""

# Тест 2: Вывод справки клиента
echo "--- Тест 2: Вывод справки ---"
if myRPC-client --help 2>&1 | grep -q "Использование"; then
    echo -e "${GREEN}✓ Справка выводится${NC}"
    PASSED=$((PASSED + 1))
else
    echo -e "${RED}✗ Справка не работает${NC}"
    FAILED=$((FAILED + 1))
fi
echo ""

# Тест 3: Запуск сервера в консольном режиме
echo "--- Тест 3: Запуск сервера ---"
# Временно меняем режим на консольный
sed -i 's/mode = daemon/mode = console/' /etc/myRPC/myRPC.conf 2>/dev/null || true

myRPC-server /etc/myRPC/myRPC.conf &
SERVER_PID=$!
sleep 1

if kill -0 $SERVER_PID 2>/dev/null; then
    echo -e "${GREEN}✓ Сервер запущен (PID: $SERVER_PID)${NC}"
    PASSED=$((PASSED + 1))
else
    echo -e "${RED}✗ Сервер не запустился${NC}"
    FAILED=$((FAILED + 1))
fi
echo ""

# Тест 4: Отправка команды
echo "--- Тест 4: Выполнение команды ---"
if myRPC-client -h 127.0.0.1 -p 8642 -s -c "echo TEST_OK" > /tmp/test_output.txt 2>&1; then
    if grep -q "TEST_OK" /tmp/test_output.txt; then
        echo -e "${GREEN}✓ Команда выполнена успешно${NC}"
        PASSED=$((PASSED + 1))
    else
        echo -e "${YELLOW}⚠ Ответ получен, но результат не совпадает${NC}"
        FAILED=$((FAILED + 1))
    fi
else
    echo -e "${RED}✗ Не удалось выполнить команду${NC}"
    FAILED=$((FAILED + 1))
fi
echo ""

# Тест 5: Проверка конфигурации
echo "--- Тест 5: Конфигурационные файлы ---"
if [ -f /etc/myRPC/myRPC.conf ]; then
    echo -e "${GREEN}✓ myRPC.conf существует${NC}"
    PASSED=$((PASSED + 1))
else
    echo -e "${RED}✗ myRPC.conf отсутствует${NC}"
    FAILED=$((FAILED + 1))
fi

if [ -f /etc/myRPC/users.conf ]; then
    echo -e "${GREEN}✓ users.conf существует${NC}"
    PASSED=$((PASSED + 1))
else
    echo -e "${RED}✗ users.conf отсутствует${NC}"
    FAILED=$((FAILED + 1))
fi
echo ""

# Остановка сервера
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null
SERVER_PID=""

# Возвращаем режим демона
sed -i 's/mode = console/mode = daemon/' /etc/myRPC/myRPC.conf 2>/dev/null || true

# Итоги
echo "========================================"
echo "  Результаты тестирования"
echo "========================================"
echo -e "Пройдено: ${GREEN}$PASSED${NC}"
echo -e "Провалено: ${RED}$FAILED${NC}"
echo ""

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}Все тесты пройдены успешно!${NC}"
    exit 0
else
    echo -e "${RED}Некоторые тесты не пройдены${NC}"
    exit 1
fi
