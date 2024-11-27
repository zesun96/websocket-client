# websocket-client

websocket-client is a standalone implementation of WebSockets in C++17 with C bindings for POSIX platforms (including GNU/Linux, Android, FreeBSD, Apple macOS and iOS) and Microsoft Windows.

It is a subset of [libdatachannel](https://github.com/paullouisageneau/libdatachannel).

It is very easy to integrate using source code.

## Support Platforms

- [x] Microsoft Windows(vs2019+cmake)
- [ ] GNU/Linux
- [ ] Apple macOS
- [ ] iOS
- [ ] Android
- [ ] FreeBSD

more platforms will be test.

## Dependencies

- [GnuTLS](https://www.gnutls.org/), [Mbed TLS](https://www.trustedfirmware.org/projects/mbed-tls/), or [OpenSSL](https://www.openssl.org/)

## Examples

See [examples](https://github.com/zesun96/websocket-client/tree/master/examples/) for complete usage examples with client (under MPL 2.0).

## Thanks

- [libdatachannel](https://github.com/paullouisageneau/libdatachannel)
