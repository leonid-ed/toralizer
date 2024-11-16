# Linux version

## Install

In `linux` subdirectory run the command:

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

## Testing

```bash

# Test via wget
$ ./toralize.sh wget "91.201.60.19" -O -
TORALIZE_LIB: the library has been loaded
--2024-11-03 09:20:11--  http://91.201.60.19/
Connecting to 91.201.60.19:80... TORALIZE_LIB: original socket: 3, IP: 91.201.60.19, port: 80
TORALIZE_LIB: created proxy socket 4
TORALIZE_LIB: connecting to SOCKS4 proxy server 192.168.0.101:61298 ... connected
TORALIZE_LIB: waiting for response from the proxy ... ready
TORALIZE_LIB: successfully connected through the proxy
TORALIZE_LIB: deallocate the original socket 3 in favour of the proxy one 4
connected.
HTTP request sent, awaiting response... 200 OK
Length: 142 [text/html]
Saving to: ‘STDOUT’

-                                                0%[                                                                                                     ]       0  --.-KB/s               <html>
    <head>
        <meta content="0;URL=/cgi-sys/defaultwebpage.cgi" http-equiv="refresh"/>
    </head>
    <body>
    </body>
</html>
-                                              100%[====================================================================================================>]     142  --.-KB/s    in 0s

2024-11-03 09:20:12 (3.09 MB/s) - written to stdout [142/142]

# Test via curl
$ ./toralize.sh curl "91.201.60.19"
TORALIZE_LIB: the library has been loaded
TORALIZE_LIB: original socket: 5, IP: 91.201.60.19, port: 80
TORALIZE_LIB: created proxy socket 6
TORALIZE_LIB: connecting to SOCKS4 proxy server 192.168.0.101:61298 ... connected
TORALIZE_LIB: waiting for response from the proxy ... ready
TORALIZE_LIB: successfully connected through the proxy
TORALIZE_LIB: deallocate the original socket 5 in favour of the proxy one 6
<html>
    <head>
        <meta content="0;URL=/cgi-sys/defaultwebpage.cgi" http-equiv="refresh"/>
    </head>
    <body>
    </body>
</html>
```
