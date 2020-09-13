# tinyntp
Tiny NTP client


### Compile:
```bash
$ cc tinyntp.c -o tinyntp
```

### Run:
```
$ ./tinyntp -h "ntp3.stratum2.ru" -p 123
Sun Sep 13 15:39:04 2020

$ ./tinyntp
Sun Sep 13 15:39:08 2020
```

### Help:
```
Usage: tinyntp [OPTION...]
tinyntp - Tiny NTP client

  -h, --hostname=HOSTNAME    Hostname of NTP server (default "0.europe.pool.ntp.org")
  -p, --port=PORT            Port of NTP server (default: 123)
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to <kiky.tokamuro@yandex.ru>
```
