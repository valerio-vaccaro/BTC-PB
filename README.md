# BTC-PB Bitcoin price broadcaster

Broadcast BTC Price using an ESP8266 board and wifi.

![Android](https://github.com/valerio-vaccaro/BTC-PB/blob/master/screenshots/1-android.jpeg "Android")

Based on:

- Arduino framework
- WiFiManager
- https://api.coindesk.com/v1/bpi/currentprice.json API

## Firmware builds

GitHub Actions builds every PlatformIO environment and publishes a firmware
artifact containing one `<version>_<board>/` directory per board and an
`index.json` manifest. The directory layout and manifest are compatible with
the browser flasher at https://valerio-vaccaro.github.io/diyflasher/.

To create the same package locally after building:

```bash
pio run
python tools/package_firmware.py --version dev --output dist
```
