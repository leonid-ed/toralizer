# macOS version

## Install

In `macos` subdirectory run the command:

```bash
$ make build
```

## Setting up

In the file `toralizer.h` you can find the default proxy address and port which
are customizable via the environment variables `SOCKS_VER`, `PROXY_ADDRESS` and
`PROXY_PORT`.

```c
#define ENV_PROXY_ADDRESS "PROXY_ADDRESS"
#define ENV_PROXY_PORT "PROXY_PORT"
#define ENV_SOCKS_VER "SOCKS_VER"
#define DEFAULT_PROXY_ADDRESS "127.0.0.1"
#define DEFAULT_PROXY_PORT 9050
#define DEFAULT_SOCKS_VER 5
```

Go to the bash script `toralize.sh` and update it accordingly, e.g.:

```bash
export SOCKS_VER=5
export PROXY_ADDRESS=<proxy_ip_address>
export PROXY_PORT=<proxy_port>
export DYLD_INSERT_LIBRARIES=<full_path_to_the_subdirectory>/toralize.dylib
```

## Take into account macOS protection

I'm using macOS version Somona 14.6.1.

Nowadays, macOS has a security subsystem called `System Integrity Protection`
and it prevents injecting code via `DYLD_INSERT_LIBRARIES` (the macOS equivalent
of `LD_PRELOAD`) into any binary in `/bin`, `/sbin`, `/usr/bin` and `/usr/sbin`.

```bash
# Check the location of the utilities wget and curl
$ where wget curl
/usr/local/bin/wget
/usr/bin/cur
```

That's why I will test macOS version only with `wget`.

## Testing

```bash
# Test via wget
$ ./toralize.sh wget "https://ifconfig.co/city"  -O -
TORALIZE_LIB: the library has been loaded
--2024-11-16 08:27:51--  https://ifconfig.co/city
Resolving ifconfig.co (ifconfig.co)... 104.21.54.91, 172.67.168.106
Connecting to ifconfig.co (ifconfig.co)|104.21.54.91|:443... TORALIZE_LIB: original socket: 6, IPv4: 104.21.54.91, port: 443
TORALIZE_LIB: created proxy socket 7
TORALIZE_LIB: connecting to SOCKS5 proxy server 127.0.0.1:62494 ... connected
TORALIZE_LIB: waiting for response1 from the proxy ... ready
TORALIZE_LIB: waiting for response2 from the proxy ... ready
TORALIZE_LIB: successfully connected through the proxy
TORALIZE_LIB: deallocate the original socket 6 in favour of the proxy one 7
connected.
HTTP request sent, awaiting response... 200 OK
Length: 10 [text/plain]
Saving to: ‘STDOUT’

-                           0%[                                     ]       0  --.-KB/s               Amsterdam
-                         100%[====================================>]      10  --.-KB/s    in 0s

2024-11-16 08:27:52 (698 KB/s) - written to stdout [10/10]
```