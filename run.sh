#!/bin/bash

echo "[*] Компіляція проєкту..."
cd "$(dirname "$0")" || exit
mkdir -p build
cd build || exit
cmake .. -DCMAKE_POLICY_VERSION_MINIMUM=3.5
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "[!] Помилка компіляції."
    exit 1
fi

echo "======================================"
echo "Оберіть режим запуску:"
echo "1) Запустити як СЕРВЕР (Порт 8080)"
echo "2) Швидкий запуск як КЛІЄНТ (127.0.0.1:8080)"
read -p "Твій вибір (1 або 2): " script_choice

if [ "$script_choice" == "1" ]; then
    ./broadcast --server
elif [ "$script_choice" == "2" ]; then
    ./broadcast --fast-client
else
    echo "[!] Невірний вибір. Вихід."
    exit 1
fi