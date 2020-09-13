#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <argp.h>

#define NTP_TIMESTAMP_DELTA 2208988800ull

// NTP packet struct
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

// Input arguments
struct arguments {
    char *hostname;
    int port;
};

// Version
const char *argp_program_version = "tinyntp 1.0";

// Email for bugs
const char *argp_program_bug_address = "<kiky.tokamuro@yandex.ru>";

// Doc string
static char doc[] = "tinyntp - Tiny NTP client";

// Options
static struct argp_option options[] = { 
    {"hostname", 'h', "HOSTNAME", 0, "Hostname of NTP server (default \"0.europe.pool.ntp.org\")"},
    {"port", 'p', "PORT", 0, "Port of NTP server (default: 123)"},
    {0} 
};

// Arguments parser
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
    
    switch (key) {
        case 'h': arguments->hostname = arg; break;
        case 'p': arguments->port = atoi(arg); break;
        default: return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static struct argp argp = {options, parse_opt, 0, doc};

// Print error and exit
void err(char *message) {
    perror(message);
    exit(1);
}


int main(int argc, char **argv) {
    struct arguments arguments;

    // Default arguments
    arguments.hostname = "0.europe.pool.ntp.org";
    arguments.port = 123;

    // Arguments parse
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    // Creating NTP packet
    ntp_packet ntp_p = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    memset(&ntp_p, 0, sizeof(ntp_packet));

    // Set li = 0 (00), vn = 3 (011), mode = 3 (011)
    // 0x1b = 00011011
    *((char *) &ntp_p + 0) = 0x1b;

    struct sockaddr_in addr;
    struct hostent *ip;

    // Creating socket
    int socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_fd < 0)
        err("ERROR: Can't opening socket");

    // Geting hostname
    ip = gethostbyname(arguments.hostname);
    if (ip == NULL)
        err("ERROR: Can't finding hostname");

    // Zeroing address struct
    bzero((char *) &addr, sizeof(addr));

    addr.sin_family = AF_INET;

    // Copying IP to address struct
    bcopy((char *) ip->h_addr_list[0], (char *) &addr.sin_addr.s_addr, ip->h_length);

    // Saving port to address struct
    addr.sin_port = htons(arguments.port);

    // Connecting
    if (connect(socket_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
        err("ERROR: Can't connect");

    // Sending NTP packet
    if (write(socket_fd, (char *) &ntp_p, sizeof(ntp_packet)) < 0)
        err("ERROR: Can't writing to socket");

    // Reading responce
    if (read(socket_fd, (char *) &ntp_p, sizeof(ntp_packet)) < 0)
        err("ERROR: Can't reading to socket");

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