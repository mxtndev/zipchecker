# Имя исполняемого файла
TARGET = zipchecker

# Компилятор
CC = gcc

# Флаги компиляции
CFLAGS = -Wall -Wextra -Wpedantic -std=c11

# Исходный файл
SRCS = zipchecker.c

# Объектный файл
OBJS = $(SRCS:.c=.o)

# Цель по умолчанию
all: $(TARGET)

# Сборка исполняемого файла
$(TARGET): $(OBJS)
    $(CC) $(CFLAGS) -o $@ $^

# Компиляция исходного файла в объектный
%.o: %.c
    $(CC) $(CFLAGS) -c $< -o $@

# Очистка временных файлов
clean:
    rm -f $(OBJS) $(TARGET)

# Удаление всех артефактов (включая исполняемый файл)
distclean: clean

.PHONY: all clean distclean