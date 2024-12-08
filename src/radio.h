#ifndef RADIO_H_
#define RADIO_H_

namespace RADIO
{

    int setupAFSK(float frequency);
    int setupLoRa(float frequency, String speed);
    bool setup();
    void startupTone();

    void sendAFSK();
    void sendLoRa();

    void reset();

}

#endif