#ifndef RADIO_H_
#define RADIO_H_

namespace RADIO
{

    int setupAFSK(int frequency);
    int setupLoRa(int frequency, bool isFast);
    bool setup();
    void startupTone();

    void sendAFSK();
    void sendLoRa();

    void reset();

}

#endif