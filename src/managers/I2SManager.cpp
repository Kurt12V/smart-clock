
#include "I2SManager.h"

#include <Arduino.h>
#include <driver/i2s.h>

#include "Pins.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

I2SManager::I2SManager()
    : _initialized(false),
      _microphoneInitialized(false),
      _speakerInitialized(false),
      _microphoneSampleRate(16000),
      _speakerSampleRate(44100)
{
}

// ============================================================
// BEGIN / END
// ============================================================

bool I2SManager::begin()
{
    if (_initialized)
        return true;

    Serial0.println();
    Serial0.println("[I2S] ========================================");
    Serial0.println("[I2S] I2S Manager");
    Serial0.println("[I2S] ========================================");

    _initialized = true;

    Serial0.println("[I2S] Manager initialized");

    return true;
}

void I2SManager::end()
{
    if (!_initialized)
        return;

    endMicrophone();
    endSpeaker();

    _initialized = false;

    Serial0.println("[I2S] Manager stopped");
}

bool I2SManager::isInitialized() const
{
    return _initialized;
}

// ============================================================
// MICROPHONE
// ============================================================

bool I2SManager::beginMicrophone(uint32_t sampleRate)
{
    if (_microphoneInitialized)
        return true;

    if (!_initialized && !begin())
        return false;

    Serial0.println("[I2S] Initializing microphone");

    Serial0.printf(
        "[I2S] Microphone port: %d\n",
        MICROPHONE_PORT
    );

    Serial0.printf(
        "[I2S] Microphone rate: %lu Hz\n",
        static_cast<unsigned long>(sampleRate)
    );

    if (!installMicrophoneDriver(sampleRate))
    {
        Serial0.println("[I2S][ERROR] Microphone driver installation failed");
        return false;
    }

    _microphoneSampleRate = sampleRate;
    _microphoneInitialized = true;

    Serial0.println("[I2S] Microphone initialized");

    return true;
}

bool I2SManager::installMicrophoneDriver(uint32_t sampleRate)
{
    const i2s_port_t port =
        static_cast<i2s_port_t>(MICROPHONE_PORT);

    const i2s_config_t config =
    {
        .mode = static_cast<i2s_mode_t>(
            I2S_MODE_MASTER | I2S_MODE_RX
        ),
        .sample_rate = sampleRate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = 256,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    esp_err_t result = i2s_driver_install(
        port,
        &config,
        0,
        nullptr
    );

    if (result != ESP_OK)
    {
        Serial0.printf(
            "[I2S][ERROR] MIC driver install: %s\n",
            esp_err_to_name(result)
        );
        return false;
    }

    const i2s_pin_config_t pins =
    {
        .bck_io_num = PIN_INMP_SCK,
        .ws_io_num = PIN_INMP_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = PIN_INMP_SD
    };

    result = i2s_set_pin(port, &pins);

    if (result != ESP_OK)
    {
        Serial0.printf(
            "[I2S][ERROR] MIC set pins: %s\n",
            esp_err_to_name(result)
        );

        i2s_driver_uninstall(port);
        return false;
    }

    i2s_zero_dma_buffer(port);

    return true;
}

void I2SManager::endMicrophone()
{
    if (!_microphoneInitialized)
        return;

    const i2s_port_t port =
        static_cast<i2s_port_t>(MICROPHONE_PORT);

    i2s_stop(port);
    i2s_driver_uninstall(port);

    _microphoneInitialized = false;

    Serial0.println("[I2S] Microphone stopped");
}

bool I2SManager::isMicrophoneInitialized() const
{
    return _microphoneInitialized;
}

int I2SManager::microphonePort() const
{
    return MICROPHONE_PORT;
}

bool I2SManager::clearMicrophone()
{
    if (!_microphoneInitialized)
        return false;

    const esp_err_t result = i2s_zero_dma_buffer(
        static_cast<i2s_port_t>(MICROPHONE_PORT)
    );

    if (result != ESP_OK)
    {
        Serial0.printf(
            "[I2S][ERROR] Clear MIC buffer: %s\n",
            esp_err_to_name(result)
        );
        return false;
    }

    return true;
}

bool I2SManager::readMicrophone(
    void* buffer,
    size_t size,
    size_t& bytesRead,
    uint32_t timeoutMs
)
{
    bytesRead = 0;

    if (!_microphoneInitialized ||
        buffer == nullptr ||
        size == 0)
    {
        return false;
    }

    const esp_err_t result = i2s_read(
        static_cast<i2s_port_t>(MICROPHONE_PORT),
        buffer,
        size,
        &bytesRead,
        pdMS_TO_TICKS(timeoutMs)
    );

    if (result != ESP_OK)
    {
        Serial0.printf(
            "[I2S][ERROR] MIC read: %s\n",
            esp_err_to_name(result)
        );

        bytesRead = 0;
        return false;
    }

    return bytesRead > 0;
}

// ============================================================
// SPEAKER
// ============================================================

bool I2SManager::beginSpeaker(uint32_t sampleRate)
{
    if (_speakerInitialized &&
        _speakerSampleRate == sampleRate)
    {
        return true;
    }

    if (_speakerInitialized)
        endSpeaker();

    if (!_initialized && !begin())
        return false;

    Serial0.println();
    Serial0.println("[I2S] Initializing speaker");

    Serial0.printf(
        "[I2S] Speaker port: %d\n",
        SPEAKER_PORT
    );

    Serial0.printf(
        "[I2S] Speaker sample rate: %lu Hz\n",
        static_cast<unsigned long>(sampleRate)
    );

    Serial0.printf(
        "[I2S] BCLK=%d LRCLK=%d DIN=%d\n",
        PIN_I2S_BCLK,
        PIN_I2S_LRCLK,
        PIN_I2S_DIN
    );

    if (!installSpeakerDriver(sampleRate))
    {
        Serial0.println("[I2S][ERROR] Speaker initialization failed");
        return false;
    }

    _speakerSampleRate = sampleRate;
    _speakerInitialized = true;

    Serial0.println("[I2S] Speaker initialized");

    return true;
}

bool I2SManager::installSpeakerDriver(uint32_t sampleRate)
{
    const i2s_port_t port =
        static_cast<i2s_port_t>(SPEAKER_PORT);

    const i2s_config_t config =
    {
        .mode = static_cast<i2s_mode_t>(
            I2S_MODE_MASTER | I2S_MODE_TX
        ),
        .sample_rate = sampleRate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,

        // Моно: передаём данные в левый слот.
        // Проверь, что SD_MODE/SEL на MAX98357A
        // настроен на приём именно левого канала.
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,

        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = 256,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    esp_err_t result = i2s_driver_install(
        port,
        &config,
        0,
        nullptr
    );

    if (result != ESP_OK)
    {
        Serial0.printf(
            "[I2S][ERROR] SPK driver install: %s\n",
            esp_err_to_name(result)
        );
        return false;
    }

    const i2s_pin_config_t pins =
    {
        .bck_io_num = PIN_I2S_BCLK,
        .ws_io_num = PIN_I2S_LRCLK,
        .data_out_num = PIN_I2S_DIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    result = i2s_set_pin(port, &pins);

    if (result != ESP_OK)
    {
        Serial0.printf(
            "[I2S][ERROR] SPK set pins: %s\n",
            esp_err_to_name(result)
        );

        i2s_driver_uninstall(port);
        return false;
    }

    result = i2s_zero_dma_buffer(port);

    if (result != ESP_OK)
    {
        Serial0.printf(
            "[I2S][ERROR] SPK clear DMA: %s\n",
            esp_err_to_name(result)
        );

        i2s_driver_uninstall(port);
        return false;
    }

    return true;
}

void I2SManager::endSpeaker()
{
    if (!_speakerInitialized)
        return;

    const i2s_port_t port =
        static_cast<i2s_port_t>(SPEAKER_PORT);

    i2s_stop(port);
    i2s_driver_uninstall(port);

    _speakerInitialized = false;

    Serial0.println("[I2S] Speaker stopped");
}

bool I2SManager::isSpeakerInitialized() const
{
    return _speakerInitialized;
}

int I2SManager::speakerPort() const
{
    return SPEAKER_PORT;
}

bool I2SManager::startSpeaker()
{
    if (!_speakerInitialized)
    {
        Serial0.println("[I2S][ERROR] startSpeaker: not initialized");
        return false;
    }

    const esp_err_t result = i2s_start(
        static_cast<i2s_port_t>(SPEAKER_PORT)
    );

    if (result != ESP_OK)
    {
        Serial0.printf(
            "[I2S][ERROR] Start speaker: %s\n",
            esp_err_to_name(result)
        );
        return false;
    }

    Serial0.println("[I2S] Speaker started");

    return true;
}

bool I2SManager::stopSpeaker()
{
    if (!_speakerInitialized)
        return false;

    const esp_err_t result = i2s_stop(
        static_cast<i2s_port_t>(SPEAKER_PORT)
    );

    if (result != ESP_OK)
    {
        Serial0.printf(
            "[I2S][ERROR] Stop speaker: %s\n",
            esp_err_to_name(result)
        );
        return false;
    }

    return true;
}

bool I2SManager::clearSpeaker()
{
    if (!_speakerInitialized)
        return false;

    const esp_err_t result = i2s_zero_dma_buffer(
        static_cast<i2s_port_t>(SPEAKER_PORT)
    );

    if (result != ESP_OK)
    {
        Serial0.printf(
            "[I2S][ERROR] Clear speaker DMA: %s\n",
            esp_err_to_name(result)
        );
        return false;
    }

    return true;
}

bool I2SManager::writeSpeaker(
    const uint8_t* data,
    size_t bytes,
    size_t& bytesWritten,
    uint32_t timeoutMs
)
{
    bytesWritten = 0;

    if (!_speakerInitialized ||
        data == nullptr ||
        bytes == 0)
    {
        Serial0.println(
            "[I2S][ERROR] writeSpeaker: invalid state or buffer"
        );
        return false;
    }

    const esp_err_t result = i2s_write(
        static_cast<i2s_port_t>(SPEAKER_PORT),
        data,
        bytes,
        &bytesWritten,
        pdMS_TO_TICKS(timeoutMs)
    );

    if (result != ESP_OK)
    {
        Serial0.printf(
            "[I2S][ERROR] Speaker write: %s\n",
            esp_err_to_name(result)
        );

        bytesWritten = 0;
        return false;
    }

    if (bytesWritten != bytes)
    {
        Serial0.printf(
            "[I2S][WARN] Partial write: %u/%u bytes\n",
            static_cast<unsigned>(bytesWritten),
            static_cast<unsigned>(bytes)
        );
    }

    return bytesWritten > 0;
}

bool I2SManager::isPortInstalled(int port) const
{
    if (port == MICROPHONE_PORT)
        return _microphoneInitialized;

    if (port == SPEAKER_PORT)
        return _speakerInitialized;

    return false;
}
