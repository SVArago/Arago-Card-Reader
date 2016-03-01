# Juliana2
Juliana2 is a daemon that polls a connected NFC-reader for tags, and broadcasts
these over a WebSocket. It is inspired by a [similar project](https://github.com/Inter-Actief/JulianaNFC_C)
by Inter-Actief.

## Key features
* Runs on any platform supported by libnfc and Qt, notably Linux and Windows
* Supports TLS on the websocket server
* Supports multiple parallel connections to the server

## Configuration
Juliana2 reads a juliana2.ini configuration file from the same directory as the
executable is placed in. It accepts the following settings:

### websocket section
* port: The port to listen on
* tls: Whether to enable or disable TLS (true/false)
* certificate: Path to a PEM-encoded TLS certificate
* key: Path to a PEM-encoded private key

### nfc
* device: libnfc connection string for the NFC reader to use

## Protocol format
Juliana2 announces each scanned card in a JSON-encoded format on the websocket,
with messages like these:

    {"atqa":"1234", "uid":"567890ab", "sak":"cd"}

The ATQA, UID and SAK are hexadecimal encoded.