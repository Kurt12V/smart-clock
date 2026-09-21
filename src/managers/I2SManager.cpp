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
// BEGIN
// ============================================================

bool I2SManager::begin()
{
    if (_initialized)
    {
        return true;
    }

    Serial.println();
    Serial.println("[I2S] ========================================");
    Serial.println("[I2S] I2S Manager");
    Serial.println("[I2S] ========================================");

    _initialized = true;

    Serial.println("[I2S] Manager initialized");

    return true;
}

// ============================================================
// END
// ============================================================

void I2SManager::end()
{
    if (!_initialized)
    {
        return;
    }

    endMicrophone();
    endSpeaker();

    _initialized = false;

    Serial.println("[I2S] Manager stopped");
}

// ============================================================
// STATE
// ============================================================

bool I2SManager::isInitialized() const
{
    return _initialized;
}

// ============================================================
// MICROPHONE
// ============================================================

bool I2SManager::beginMicrophone(
    uint32_t sampleRate
)
{
    if (_microphoneInitialized)
    {
        return true;
    }

    if (!_initialized)
    {
        if (!begin())
        {
            return false;
        }
    }

    Serial.println();
    Serial.println("[I2S] Initializing microphone");
    Serial.printf(
        "[I2S] Port: I2S_NUM_%d\n",
        MICROPHONE_PORT
    );
    Serial.printf(
        "[I2S] Sample rate: %lu Hz\n",
        static_cast<unsigned long>(sampleRate)
    );

    if (!installMicrophoneDriver(sampleRate))
    {
        Serial.println(
            "[I2S] ERROR: microphone driver installation failed"
        );

        return false;
    }

    _microphoneSampleRate = sampleRate;
    _microphoneInitialized = true;

    Serial.println(
        "[I2S] Microphone initialized"
    );

    return true;
}

// ============================================================
// INSTALL MICROPHONE DRIVER
// ============================================================

bool I2SManager::installMicrophoneDriver(
    uint32_t sampleRate
)
{
    const i2s_port_t port =
        static_cast<i2s_port_t>(MICROPHONE_PORT);

    // --------------------------------------------------------
    // CONFIG
    // --------------------------------------------------------

    const i2s_config_t config =
    {
        .mode =
            static_cast<i2s_mode_t>(
                I2S_MODE_MASTER |
                I2S_MODE_RX
            ),

        .sample_rate = sampleRate,

        .bits_per_sample =
            I2S_BITS_PER_SAMPLE_32BIT,

        .channel_format =
            I2S_CHANNEL_FMT_ONLY_LEFT,

        .communication_format =
            I2S_COMM_FORMAT_STAND_I2S,

        .intr_alloc_flags = 0,

        .dma_buf_count = 8,

        .dma_buf_len = 256,

        .use_apll = false,

        .tx_desc_auto_clear = false,

        .fixed_mclk = 0
    };

    // --------------------------------------------------------
    // INSTALL
    // --------------------------------------------------------

    esp_err_t result =
        i2s_driver_install(
            port,
            &config,
            0,
            nullptr
        );

    if (result != ESP_OK)
    {
        Serial.printf(
            "[I2S] ERROR: i2s_driver_install(MIC) failed: %s\n",
            esp_err_to_name(result)
        );

        return false;
    }

    // --------------------------------------------------------
    // PINS
    // --------------------------------------------------------

    const i2s_pin_config_t pins =
    {
        .bck_io_num = PIN_INMP_SCK,

        .ws_io_num = PIN_INMP_WS,

        .data_out_num = I2S_PIN_NO_CHANGE,

        .data_in_num = PIN_INMP_SD
    };

    result =
        i2s_set_pin(
            port,
            &pins
        );

    if (result != ESP_OK)
    {
        Serial.printf(
            "[I2S] ERROR: i2s_set_pin(MIC) failed: %s\n",
            esp_err_to_name(result)
        );

        i2s_driver_uninstall(port);

        return false;
    }

    // --------------------------------------------------------
    // CLEAR DMA
    // --------------------------------------------------------

    i2s_zero_dma_buffer(port);

    return true;
}

// ============================================================
// END MICROPHONE
// ============================================================

void I2SManager::endMicrophone()
{
    if (!_microphoneInitialized)
    {
        return;
    }

    const i2s_port_t port =
        static_cast<i2s_port_t>(MICROPHONE_PORT);

    i2s_stop(port);

    i2s_driver_uninstall(port);

    _microphoneInitialized = false;

    Serial.println(
        "[I2S] Microphone stopped"
    );
}

// ============================================================
// MICROPHONE STATE
// ============================================================

bool I2SManager::isMicrophoneInitialized() const
{
    return _microphoneInitialized;
}

int I2SManager::microphonePort() const
{
    return MICROPHONE_PORT;
}

// ============================================================
// CLEAR MICROPHONE
// ============================================================

bool I2SManager::clearMicrophone()
{
    if (!_microphoneInitialized)
    {
        return false;
    }

    const i2s_port_t port =
        static_cast<i2s_port_t>(MICROPHONE_PORT);

    const esp_err_t result =
        i2s_zero_dma_buffer(port);

    if (result != ESP_OK)
    {
        Serial.printf(
            "[I2S] ERROR: clear microphone failed: %s\n",
            esp_err_to_name(result)
        );

        return false;
    }

    return true;
}

// ============================================================
// READ MICROPHONE
// ============================================================

bool I2SManager::readMicrophone(
    void* buffer,
    size_t size,
    size_t& bytesRead,
    uint32_t timeoutMs
)
{
    bytesRead = 0;

    if (!_microphoneInitialized)
    {
        return false;
    }

    if (buffer == nullptr)
    {
        return false;
    }

    if (size == 0)
    {
        return false;
    }

    const i2s_port_t port =
        static_cast<i2s_port_t>(MICROPHONE_PORT);

    const esp_err_t result =
        i2s_read(
            port,
            buffer,
            size,
            &bytesRead,
            pdMS_TO_TICKS(timeoutMs)
        );

    if (result != ESP_OK)
    {
        Serial.printf(
            "[I2S] ERROR: microphone read failed: %s\n",
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

bool I2SManager::beginSpeaker(
    uint32_t sampleRate
)
{
    // Если уже установлен на той же частоте —
    // ничего делать не нужно.
    if (_speakerInitialized)
    {
        if (_speakerSampleRate == sampleRate)
        {
            return true;
        }

        // Для WAV с другой частотой переустанавливаем драйвер.
        endSpeaker();
    }

    if (!_initialized)
    {
        if (!begin())
        {
            return false;
        }
    }

    Serial.println();
    Serial.println("[I2S] Initializing speaker");
    Serial.printf(
        "[I2S] Port: I2S_NUM_%d\n",
        SPEAKER_PORT
    );
    Serial.printf(
        "[I2S] Sample rate: %lu Hz\n",
        static_cast<unsigned long>(sampleRate)
    );

    if (!installSpeakerDriver(sampleRate))
    {
        Serial.println(
            "[I2S] ERROR: speaker driver installation failed"
        );

        return false;
    }

    _speakerSampleRate = sampleRate;
    _speakerInitialized = true;

    Serial.println(
        "[I2S] Speaker initialized"
    );

    return true;
}

// ============================================================
// INSTALL SPEAKER DRIVER
// ============================================================

bool I2SManager::installSpeakerDriver(
    uint32_t sampleRate
)
{
    const i2s_port_t port =
        static_cast<i2s_port_t>(SPEAKER_PORT);

    // --------------------------------------------------------
    // CONFIG
    // --------------------------------------------------------

    const i2s_config_t config =
    {
        .mode =
            static_cast<i2s_mode_t>(
                I2S_MODE_MASTER |
                I2S_MODE_TX
            ),

        .sample_rate = sampleRate,

        .bits_per_sample =
            I2S_BITS_PER_SAMPLE_16BIT,

        .channel_format =
            I2S_CHANNEL_FMT_ONLY_LEFT,

        .communication_format =
            I2S_COMM_FORMAT_STAND_I2S,

        .intr_alloc_flags = 0,

        .dma_buf_count = 8,

        .dma_buf_len = 256,

        .use_apll = false,

        .tx_desc_auto_clear = true,

        .fixed_mclk = 0
    };

    // --------------------------------------------------------
    // INSTALL
    // --------------------------------------------------------

    esp_err_t result =
        i2s_driver_install(
            port,
            &config,
            0,
            nullptr
        );

    if (result != ESP_OK)
    {
        Serial.printf(
            "[I2S] ERROR: i2s_driver_install(SPK) failed: %s\n",
            esp_err_to_name(result)
        );

        return false;
    }

    // --------------------------------------------------------
    // PINS
    // --------------------------------------------------------

    const i2s_pin_config_t pins =
    {
        .bck_io_num = PIN_I2S_BCLK,

        .ws_io_num = PIN_I2S_LRCLK,

        .data_out_num = PIN_I2S_DIN,

        .data_in_num = I2S_PIN_NO_CHANGE
    };

    result =
        i2s_set_pin(
            port,
            &pins
        );

    if (result != ESP_OK)
    {
        Serial.printf(
            "[I2S] ERROR: i2s_set_pin(SPK) failed: %s\n",
            esp_err_to_name(result)
        );

        i2s_driver_uninstall(port);

        return false;
    }

    i2s_zero_dma_buffer(port);

    return true;
}

// ============================================================
// END SPEAKER
// ============================================================

void I2SManager::endSpeaker()
{
    if (!_speakerInitialized)
    {
        return;
    }

    const i2s_port_t port =
        static_cast<i2s_port_t>(SPEAKER_PORT);

    i2s_stop(port);

    i2s_driver_uninstall(port);

    _speakerInitialized = false;

    Serial.println(
        "[I2S] Speaker stopped"
    );
}

// ============================================================
// SPEAKER STATE
// ============================================================

bool I2SManager::isSpeakerInitialized() const
{
    return _speakerInitialized;
}

int I2SManager::speakerPort() const
{
    return SPEAKER_PORT;
}

// ============================================================
// START SPEAKER
// ============================================================

bool I2SManager::startSpeaker()
{
    if (!_speakerInitialized)
    {
        return false;
    }

    const i2s_port_t port =
        static_cast<i2s_port_t>(SPEAKER_PORT);

    return i2s_start(port) == ESP_OK;
}

// ============================================================
// STOP SPEAKER
// ============================================================

bool I2SManager::stopSpeaker()
{
    if (!_speakerInitialized)
    {
        return false;
    }

    const i2s_port_t port =
        static_cast<i2s_port_t>(SPEAKER_PORT);

    return i2s_stop(port) == ESP_OK;
}

// ============================================================
// CLEAR SPEAKER
// ============================================================

bool I2SManager::clearSpeaker()
{
    if (!_speakerInitialized)
    {
        return false;
    }

    const i2s_port_t port =
        static_cast<i2s_port_t>(SPEAKER_PORT);

    return i2s_zero_dma_buffer(port) == ESP_OK;
}

// ============================================================
// WRITE SPEAKER
// ============================================================

bool I2SManager::writeSpeaker(
    const uint8_t* data,
    size_t bytes,
    size_t& bytesWritten,
    uint32_t timeoutMs
)
{
    bytesWritten = 0;

    if (!_speakerInitialized)
    {
        return false;
    }

    if (data == nullptr)
    {
        return false;
    }

    if (bytes == 0)
    {
        return false;
    }

    const i2s_port_t port =
        static_cast<i2s_port_t>(SPEAKER_PORT);

    const esp_err_t result =
        i2s_write(
            port,
            data,
            bytes,
            &bytesWritten,
            pdMS_TO_TICKS(timeoutMs)
        );

    if (result != ESP_OK)
    {
        Serial.printf(
            "[I2S] ERROR: speaker write failed: %s\n",
            esp_err_to_name(result)
        );

        bytesWritten = 0;

        return false;
    }

    return bytesWritten > 0;
}

// ============================================================
// PORT INSTALLED
// ============================================================

bool I2SManager::isPortInstalled(
    int port
) const
{
    if (port == MICROPHONE_PORT)
    {
        return _microphoneInitialized;
    }

    if (port == SPEAKER_PORT)
    {
        return _speakerInitialized;
    }

    return false;
}