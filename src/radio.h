#ifndef RADIO_H_
#define RADIO_H_

namespace RADIO
{

    void setupAFSK(int frequency);
    void setupLoRa(int frequency, bool isFast);

    void startupTone();

    void sendAFSK();
    void sendLoRa();

    void sendAFSKStatus(char *status);
    void sendLoRaStatus(char *status);

    void reset();

}

#endif