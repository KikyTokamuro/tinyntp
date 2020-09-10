#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netdb.h>

#define NTP_TIMESTAMP_DELTA 2208988800ull

typedef struct {
    uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
                             // li.   Two bits.   Leap indicator.
                             // vn.   Three bits. Version number of the protocol.
                             // mode. Three bits. Client will pick mode 3 for client.

    uint8_t stratum;         // Eight bits. Stratum level of the local clock.
    uint8_t poll;            // Eight bits. Maximum interval between successive messages.
    uint8_t precision;       // Eight bits. Precision of the local clock.

    uint32_t rootDelay;      // 32 bits. Total round trip delay time.
    uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
    uint32_t refId;          // 32 bits. Reference clock identifier.

    uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
    uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

    uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
    uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

    uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
    uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

    uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
    uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.

} ntp_packet;                // Total: 384 bits or 48 bytes.


int main(int argc, char **argv) {
    if (argc < 2) {
        perror("ERROR: NTP hostname not setting");
        exit(0);
    }

    int socket_fd, n;

    // Creating NTP packet
    ntp_packet ntp_p = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    memset(&ntp_p, 0, sizeof(ntp_packet));

    // Set li = 0 (00), vn = 3 (011), mode = 3 (011)
    // 0x1b = 00011011
    *((char *) &ntp_p + 0) = 0x1b;

    struct sockaddr_in addr;
    struct hostent *ip;

    // Creating socket
    socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_fd < 0) {
        perror("ERROR: Can't opening socket");
        exit(0);
    }

    // Geting hostname
    ip = gethostbyname(argv[1]);
    if (ip == NULL) {
        perror("ERROR: Can't finding hostname");
        exit(0);
    }

    // Zeroing address struct
    bzero((char *) &addr, sizeof(addr));

    addr.sin_family = AF_INET;

    // Copying IP to address struct
    bcopy((char *) ip->h_addr, (char *) &addr.sin_addr.s_addr, ip->h_length);

    // Saving port to address struct
    addr.sin_port = htons(123);

    // Connecting
    if (connect(socket_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("ERROR: Can't connect");
        exit(0);
    }

    // Sending NTP packet
    n = write(socket_fd, (char *) &ntp_p, sizeof(ntp_packet));
    if (n < 0) {
        perror("ERROR: Can't writing to socket");
        exit(0);
    }

    // Reading responce
    n = read(socket_fd, (char *) &ntp_p, sizeof(ntp_packet));
    if (n < 0) {
        perror("ERROR: Can't reading to socket");
        exit(0);
    }

    // Seconds
    ntp_p.txTm_s = htonl(ntp_p.txTm_s);
    // Fraction
    ntp_p.txTm_f = htonl(ntp_p.txTm_f);

    // Convert to time
    time_t time = (time_t)(ntp_p.txTm_s - NTP_TIMESTAMP_DELTA);
    char * time_str = ctime((time_t *) &time);

    printf("%s", time_str);

    return 0;
}