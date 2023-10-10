#include <SPI.h>
#include <SD.h>
#define SD_CS 36
#define SD_MOSI 34
#define SD_MISO 19
#define SD_SCLK 5

namespace SDCard
{
    void readFile(fs::FS &fs, const char *path)
    {
        Serial.printf("Reading file: %s\n", path);

        File file = fs.open(path);
        if (!file)
        {
            Serial.println("Failed to open file for reading");
            return;
        }

        Serial.print("Read from file: ");
        while (file.available())
        {
            Serial.write(file.read());
        }
        file.close();
    }
    void writeFile(fs::FS &fs, const char *path, const char *message)
    {
        Serial.printf("Writing file: %s\n", path);

        File file = fs.open(path, FILE_WRITE);
        if (!file)
        {
            Serial.println("Failed to open file for writing");
            return;
        }
        if (file.print(message))
        {
            Serial.println("File written");
        }
        else
        {
            Serial.println("Write failed");
        }
        file.close();
    }
    void appendFile(fs::FS &fs, const char *path, String message)
    {
        Serial.printf("Appending to file: %s\n", path);

        File file = fs.open(path, FILE_APPEND);
        if (!file)
        {
            Serial.println("Failed to open file for appending");
            return;
        }
        if (file.print(message))
        {
            Serial.println("Message appended");
        }
        else
        {
            Serial.println("Append failed");
        }
        file.close();
    }
    void deleteFile(fs::FS &fs, const char *path)
    {
        Serial.printf("Deleting file: %s\n", path);
        if (fs.remove(path))
        {
            Serial.println("File deleted");
        }
        else
        {
            Serial.println("Delete failed");
        }
    }
    void createDir(fs::FS &fs, const char *path)
    {
        Serial.printf("Creating Dir: %s\n", path);
        if (fs.mkdir(path))
        {
            Serial.println("Dir created");
        }
        else
        {
            Serial.println("mkdir failed");
        }
    }
    void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
    {
        Serial.printf("Listing directory: %s\n", dirname);

        File root = fs.open(dirname);
        if (!root)
        {
            Serial.println("Failed to open directory");
            return;
        }
        if (!root.isDirectory())
        {
            Serial.println("Not a directory");
            return;
        }
    }
}