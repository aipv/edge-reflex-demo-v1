# ESP32-S3 Verified Boot Example

An ESP32-S3 OTA firmware verification example using the embedded-firmware-verification library.

This project demonstrates:

- Signed firmware image creation
- SHA256 firmware integrity verification
- Ed25519 firmware signature verification
- Boot-time firmware validation
- MQTT-triggered OTA firmware updates
- HTTP OTA firmware delivery
- Tampered firmware rejection

The example integrates ESP-IDF, TweetNaCl-based Ed25519 verification, OTA firmware updates, and bootloader verification flow on ESP32-S3 devices.

```text
Firmware Build
      ↓
Signed Firmware Image
      ↓
Firmware Verification
      ↓
Local Flash / OTA Update
      ↓
Bootloader Verification
      ↓
Boot Verified Firmware
```

## Requirements

- ESP32-S3 development board
- ESP-IDF
- Python 3
- libsodium
- mosquitto-clients

## Build Firmware

Configure ESP-IDF:

```bash
source $IDF_PATH/export.sh
```

Configure WiFi and MQTT settings:

```bash
idf.py menuconfig 
```

Build firmware:

```bash
idf.py build
```

## Create Signed Firmware Image

Generate signed firmware image:

```bash
./tests/create_esp32-s3_image.sh
```

The script performs:

1. Firmware signing
2. Signed image verification

Generated image:

```text
build/signed_firmware.bin
```

## Local Flash Flow

Flash signed firmware image:

```bash
./tests/flash_esp32-s3_image.sh
```

The script:

1. Firmware signing
2. Verifies the signed firmware image
3. Flashes the ESP32-S3 application partition

Bootloader verification occurs automatically during boot.

## OTA Update Flow

Start OTA HTTP server:

```bash
./tests/ota_server_start.sh
```

Example OTA URL:

```text
http://192.168.0.100:8080/signed_firmware.bin
```

Trigger OTA update:

```bash
./tests/ota_update_trigger.sh
```

The OTA URL is published through MQTT:

```text
Topic: /esp32/ota
```

The ESP32-S3 device downloads the signed firmware image, performs OTA update, and verifies the firmware during boot.

## Bootloader Verification Sequence

During boot:

1. Firmware metadata is validated
2. Ed25519 signature is verified
3. SHA256 firmware hash is recalculated
4. Firmware integrity is verified
5. Verified firmware is booted

Tampered firmware images are rejected.

## Tamper Tests

Modify firmware payload:

```bash
./tests/data_tamper.sh
```

Modify firmware metadata:

```bash
./tests/meta_tamper.sh
```

Modify firmware signature:

```bash
./tests/sign_tamper.sh
```

The bootloader verification logic should reject all tampered firmware images.

## Security Scope

Currently implemented:

- Signed firmware verification
- OTA firmware authenticity verification
- OTA firmware integrity verification
- Boot-time firmware verification
- Tampered firmware detection

## Recommended Workflow

### Step 0: Build Signing and Verification Tools

Go to the embedded-firmware-verification folder:

```bash
mkdir build
cd build
cmake ..
make
```

### Step 2: Build and Flash Default Firmware

Go to the `examples/esp32-s3-secure-bootloader` folder:

```bash
idf.py build flash monitor
```

The default firmware image is rejected because it is not signed.

### Step 3: Create Signed Firmware Image and Flash

```bash
./tests/create_esp32-s3_image.sh
./tests/flash_esp32-s3_image.sh
```

The bootloader successfully verifies and boots the signed firmware image.

### Step 4: OTA Update Flow

Terminal 1: Start OTA HTTP Server

```bash
./tests/ota_server_start.sh
```

Terminal 2: Trigger OTA Update

```bash
./tests/ota_update_trigger.sh
```

The ESP32-S3 device downloads the signed firmware image and verifies the firmware during boot.