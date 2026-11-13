#include <Arduino.h>
#include <LittleFS.h>

// ---------- Вспомогательные функции логирования ----------

void printSeparator(const char* title) {
    Serial0.println();
    Serial0.println("========================================");
    Serial0.printf("[TEST] %s\n", title);
    Serial0.println("========================================");
}

// Рекурсивный вывод структуры каталогов и файлов
void listDir(const char* dirname, uint8_t levels) {
    Serial0.printf("[FS] Listing directory: %s\n", dirname);

    File root = LittleFS.open(dirname);
    if (!root) {
        Serial0.printf("[FS] ERROR: failed to open directory: %s\n", dirname);
        return;
    }
    if (!root.isDirectory()) {
        Serial0.printf("[FS] ERROR: not a directory: %s\n", dirname);
        root.close();
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial0.printf("[FS]   DIR  : %s (size=%u)\n", file.name(), (unsigned)file.size());
            if (levels > 0) {
                listDir(file.path(), levels - 1);
            }
        } else {
            Serial0.printf("[FS]   FILE : %s (size=%u)\n", file.name(), (unsigned)file.size());
        }
        file = root.openNextFile();
    }
    root.close();
}

// Вывод общей информации о ФС
void printFsInfo() {
    size_t total = LittleFS.totalBytes();
    size_t used  = LittleFS.usedBytes();
    Serial0.printf("[FS] total=%u used=%u free=%u\n",
                  (unsigned)total, (unsigned)used, (unsigned)(total - used));
}

// ---------- Основной тест ----------

void setup() {
    Serial0.begin(115200);
    delay(1500);
    Serial0.println();
    Serial0.println("########## LittleFS TEST START ##########");

    // 1. Монтирование
    printSeparator("MOUNT LittleFS");
    if (!LittleFS.begin(true)) {   // true = formatOnFail
        Serial0.println("[FS] MOUNT FAILED");
        return;
    }
    Serial0.println("[FS] MOUNT OK");
    printFsInfo();

    // 2. Структура до теста
    printSeparator("STRUCTURE BEFORE TEST");
    listDir("/", 3);

    // 3. Создание каталога и файла
    const char* dirPath  = "/test";
    const char* filePath = "/test/hello.txt";
    const char* phrase   = "Privet, eto testovaya stroka dlya LittleFS!";

    printSeparator("CREATE DIRECTORY");
    if (!LittleFS.exists(dirPath)) {
        if (LittleFS.mkdir(dirPath)) {
            Serial0.printf("[FS] mkdir OK: %s\n", dirPath);
        } else {
            Serial0.printf("[FS] mkdir FAILED: %s\n", dirPath);
            return;
        }
    } else {
        Serial0.printf("[FS] directory already exists: %s\n", dirPath);
    }

    // 4. Запись фразы
    printSeparator("WRITE FILE");
    Serial0.printf("[FS] opening for write: %s\n", filePath);
    File f = LittleFS.open(filePath, "w");
    if (!f) {
        Serial0.printf("[FS] ERROR: cannot open for write: %s\n", filePath);
        return;
    }
    size_t written = f.print(phrase);
    Serial0.printf("[FS] wrote %u bytes\n", (unsigned)written);
    f.close();
    Serial0.println("[FS] file closed after write");

    // 5. Проверка размера
    printSeparator("VERIFY SIZE");
    File chk = LittleFS.open(filePath, "r");
    if (!chk) {
        Serial0.printf("[FS] ERROR: cannot open for read: %s\n", filePath);
        return;
    }
    Serial0.printf("[FS] size on disk: %u bytes\n", (unsigned)chk.size());
    chk.close();

    // 6. Повторное открытие и чтение
    printSeparator("READ FILE");
    File r = LittleFS.open(filePath, "r");
    if (!r) {
        Serial0.printf("[FS] ERROR: cannot open for read: %s\n", filePath);
        return;
    }
    String content = r.readString();
    r.close();
    Serial0.printf("[FS] read %u bytes\n", (unsigned)content.length());
    Serial0.printf("[FS] CONTENT: \"%s\"\n", content.c_str());

    // 7. Сравнение
    printSeparator("COMPARE");
    if (content == phrase) {
        Serial0.println("[TEST] MATCH: content equals written phrase");
    } else {
        Serial0.println("[TEST] MISMATCH!");
        Serial0.printf("[TEST] expected: \"%s\"\n", phrase);
        Serial0.printf("[TEST] actual  : \"%s\"\n", content.c_str());
    }

    // 8. Структура после теста
    printSeparator("STRUCTURE AFTER TEST");
    listDir("/", 3);
    printFsInfo();

    Serial0.println();
    Serial0.println("########## LittleFS TEST DONE ##########");
}

void loop() {
    // ничего
}