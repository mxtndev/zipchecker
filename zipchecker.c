#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h> // Для ntohs

// Сигнатура начала ZIP-архива
#define ZIP_SIGNATURE "\x50\x4B\x03\x04"
#define ZIP_SIGNATURE_LEN 4

// Сигнатура центрального каталога ZIP
#define CENTRAL_DIR_SIGNATURE "\x50\x4B\x01\x02"
#define CENTRAL_DIR_SIGNATURE_LEN 4

// Максимальный размер для чтения заголовков
#define MAX_HEADER_SIZE 1024

// Макрос для минимального значения
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// Функция для поиска сигнатуры в буфере
int find_signature(const uint8_t *data, size_t data_len, const uint8_t *signature, size_t signature_len) {
    for (size_t i = 0; i <= data_len - signature_len; ++i) {
        if (memcmp(data + i, signature, signature_len) == 0) {
            return i;
        }
    }
    return -1;
}

// Функция для чтения файла в память
uint8_t *read_file(const char *filename, size_t *file_size) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Ошибка открытия файла");
        return NULL;
    }

    // Определяем размер файла
    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    rewind(file);

    // Читаем файл в память
    uint8_t *buffer = malloc(*file_size);
    if (!buffer) {
        perror("Ошибка выделения памяти");
        fclose(file);
        return NULL;
    }

    if (fread(buffer, 1, *file_size, file) != *file_size) {
        perror("Ошибка чтения файла");
        free(buffer);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return buffer;
}

// Функция для анализа ZIP-архива и вывода списка файлов
void parse_zip_archive(const uint8_t *data, size_t data_len) {
    // Поиск начала центрального каталога
    int central_dir_pos = find_signature(data, data_len, (const uint8_t *)CENTRAL_DIR_SIGNATURE, CENTRAL_DIR_SIGNATURE_LEN);
    if (central_dir_pos == -1) {
        printf("Центральный каталог ZIP не найден или поврежден.\n");
        return;
    }

    printf("Список файлов в архиве:\n");

    size_t offset = central_dir_pos;
    while (offset + CENTRAL_DIR_SIGNATURE_LEN < data_len) {
        // Проверяем сигнатуру центрального каталога
        if (memcmp(data + offset, CENTRAL_DIR_SIGNATURE, CENTRAL_DIR_SIGNATURE_LEN) != 0) {
            break;
        }

        // Парсим заголовок центрального каталога
        uint16_t filename_length = ntohs(*(uint16_t *)(data + offset + 28)); // Смещение имени файла
        uint16_t extra_field_length = ntohs(*(uint16_t *)(data + offset + 30)); // Длина дополнительного поля
        uint16_t comment_length = ntohs(*(uint16_t *)(data + offset + 32)); // Длина комментария

        // Проверяем границы
        if (offset + 46 + filename_length + extra_field_length + comment_length > data_len) {
            printf("Ошибка: Некорректная структура ZIP.\n");
            return;
        }

        // Имя файла начинается после заголовка
        const char *filename = (const char *)(data + offset + 46);
        printf("- %.*s\n", filename_length, filename);

        // Переходим к следующей записи
        offset += 46 + filename_length + extra_field_length + comment_length;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <файл>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];
    size_t file_size;
    uint8_t *file_data = read_file(filename, &file_size);
    if (!file_data) {
        return EXIT_FAILURE;
    }

    // Поиск сигнатуры ZIP в конце файла
    size_t search_start = (file_size > MAX_HEADER_SIZE) ? file_size - MAX_HEADER_SIZE : 0;
    int zip_start = find_signature(file_data + search_start, file_size - search_start,
                                   (const uint8_t *)ZIP_SIGNATURE, ZIP_SIGNATURE_LEN);
    if (zip_start == -1) {
        printf("Файл не содержит ZIP-архив или он поврежден.\n");
        free(file_data);
        return EXIT_SUCCESS;
    }

    printf("ZIP-архив найден в конце файла.\n");
    parse_zip_archive(file_data + search_start + zip_start, file_size - (search_start + zip_start));

    free(file_data);
    return EXIT_SUCCESS;
}