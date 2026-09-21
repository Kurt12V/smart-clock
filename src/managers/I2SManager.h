#pragma once

#include <Arduino.h>
#include <driver/i2s.h>


class I2SManager
{
public:

    // ========================================================
    // PORTS
    // ========================================================

    static constexpr i2s_port_t MICROPHONE_PORT =
        I2S_NUM_0;

    static constexpr i2s_port_t SPEAKER_PORT =
        I2S_NUM_1;


    // ========================================================
    // CONSTRUCTOR
    // ========================================================

    I2SManager();


    // ========================================================
    // BEGIN / END
    // ========================================================

    bool begin();

    void end();


    // ========================================================
    // MICROPHONE
    // ========================================================

    bool beginMicrophone(
        uint32_t sampleRate = 16000
    );

    void endMicrophone();

    bool isMicrophoneInitialized() const;


    // ========================================================
    // SPEAKER
    // ========================================================

    bool beginSpeaker(
        uint32_t sampleRate = 44100
    );

    void endSpeaker();

    bool isSpeakerInitialized() const;


    // ========================================================
    // STATUS
    // ========================================================

    bool isInitialized() const;

    bool isPortInstalled(
        i2s_port_t port
    ) const;


    // ========================================================
    // ACCESS
    // ========================================================

    i2s_port_t microphonePort() const;

    i2s_port_t speakerPort() const;


private:

    bool initialized;

    bool microphoneInitialized;

    bool speakerInitialized;


    // ========================================================
    // INTERNAL
    // ========================================================

    bool installMicrophoneDriver(
        uint32_t sampleRate
    );

    bool installSpeakerDriver(
        uint32_t sampleRate
    );
};
