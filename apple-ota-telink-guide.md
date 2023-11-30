# Telink Matter Apple OTA guide

This guide explains how to create and verify non-Production Matter certificates for use in testing. The types of certificates are:

The Certification Declaration (CD)
The Product Attestation Intermediate Certificate (PAI)
The Device Attestation Certificate (DAC)

https://developers.home.google.com/matter/test/certificates?authuser=0


```bash
sudo apt-get install xxd
```


### Generating CD

For Telink Lighting:
```bash
 cd ../..
 export VID=1141
 export PID=8005
```

```bash
src/credentials/out/chip-cert gen-cd \
  --key credentials/test/certification-declaration/Chip-Test-CD-Signing-Key.pem \
  --cert credentials/test/certification-declaration/Chip-Test-CD-Signing-Cert.pem \
  --out credentials/test/certification-declaration/Chip-Test-CD-${VID}-${PID}.der \
  --format-version "1" \
  --vendor-id "${VID}" \
  --product-id "${PID}" \
  --device-type-id "0x100" \
  --certificate-id "ZIG20141ZB330001-24" \
  --security-level "0" \
  --security-info "0" \
  --version-number "2" \
  --certification-type "0"
```

Verify the CD. Make sure it contains your VID/PID (*in decimal format*):

```bash
src/credentials/out/chip-cert print-cd credentials/test/certification-declaration/Chip-Test-CD-${VID}-${PID}.der
```

Result: created file `connectedhomeip/credentials/test/certification-declaration/Chip-Test-CD-1141-8005.der`

My output:
```sh
matter@matter-1:~/repos/connectedhomeip$ src/credentials/out/chip-cert print-cd credentials/test/certification-declaration/Chip-Test-CD-${VID}-
${PID}.der
SignerKeyId value: hex:62FA823359ACFAA9963E1CFA140ADDF504F37160
0x01, tag[Anonymous]: 0x100, type: Structure (0x15), container: 
0x04,     tag[Context Specific]: 0x0, type: Unsigned Fixed Point (0x04), value: 1
0x08,     tag[Context Specific]: 0x1, type: Unsigned Fixed Point (0x04), value: 4417
0x0A,     tag[Context Specific]: 0x2, type: Array (0x16), container: 
0x0D,         tag[Anonymous]: 0x100, type: Unsigned Fixed Point (0x04), value: 32773
0x12,     tag[Context Specific]: 0x3, type: Unsigned Fixed Point (0x04), value: 256
0x15,     tag[Context Specific]: 0x4, type: UTF-8 String (0x0c), length: 19, value: "ZIG20141ZB330001-24"
0x2B,     tag[Context Specific]: 0x5, type: Unsigned Fixed Point (0x04), value: 0
0x2E,     tag[Context Specific]: 0x6, type: Unsigned Fixed Point (0x04), value: 0
0x31,     tag[Context Specific]: 0x7, type: Unsigned Fixed Point (0x04), value: 2
0x34,     tag[Context Specific]: 0x8, type: Unsigned Fixed Point (0x04), value: 0
```

### Generate the PAI using the PAA

```bash
$ src/credentials/out/chip-cert gen-att-cert --type i \
  --subject-cn "Matter Test PAI" \
  --subject-vid "${VID}" \
  --valid-from "2023-11-29 18:10:00" \
  --lifetime "4294967295" \
  --ca-key credentials/test/attestation/Chip-Test-PAA-NoVID-Key.pem \
  --ca-cert credentials/test/attestation/Chip-Test-PAA-NoVID-Cert.pem \
  --out-key credentials/test/attestation/"test-PAI-${VID}-key".pem \
  --out credentials/test/attestation/"test-PAI-${VID}-cert".pem
```

  Result: created files 
  connectedhomeip/credentials/test/attestation/test-PAI-1141-cert.pem
  connectedhomeip/credentials/test/attestation/test-PAI-1141-key.pem

### Generate the DAC using the PAI:
```bash
  $ src/credentials/out/chip-cert gen-att-cert --type d \
  --subject-cn "Matter Test DAC 0" \
  --subject-vid "${VID}" \
  --subject-pid "${PID}" \
  --valid-from "2023-11-29 18:10:00" \
  --lifetime "4294967295" \
  --ca-key credentials/test/attestation/"test-PAI-${VID}-key".pem \
  --ca-cert credentials/test/attestation/"test-PAI-${VID}-cert".pem \
  --out-key credentials/test/attestation/"test-DAC-${VID}-${PID}-key".pem \
  --out credentials/test/attestation/"test-DAC-${VID}-${PID}-cert".pem
```

Result: created files
credentials/test/attestation/test-DAC-1141-8005-cert.pem
credentials/test/attestation/test-DAC-1141-8005-key.pem

### Verify the DAC, PAI and PAA chain
If no errors appear in the output, it means that the certificate attestation chain is successfully verified

```bash 
src/credentials/out/chip-cert validate-att-cert \
--dac credentials/test/attestation/"test-DAC-${VID}-${PID}-cert".pem \
--pai credentials/test/attestation/"test-PAI-${VID}-cert".pem \
--paa credentials/test/attestation/Chip-Test-PAA-NoVID-Cert.pem
```

### You can inspect your keys using openssl:
```bash
openssl ec -noout -text -in \
>   credentials/test/attestation/test-DAC-${VID}-${PID}-key.pem
read EC key
```

Example output:
```bash
Private-Key: (256 bit)
priv:
    51:a4:94:fb:84:ce:cf:1f:63:3e:46:ee:a2:e2:a6:
    40:93:09:73:0d:62:50:10:bb:71:48:29:86:0e:df:
    c9:28
pub:
    04:e0:25:06:62:4c:e8:2b:87:86:8e:73:2f:1e:cc:
    b7:20:f9:15:55:21:fe:28:eb:77:a0:3d:db:ca:e0:
    e7:80:22:65:dd:f7:d0:6e:44:fc:8e:70:93:5f:73:
    30:28:d7:41:15:cc:12:b5:23:55:75:24:de:a5:52:
    f6:89:e3:77:64
ASN1 OID: prime256v1
NIST CURVE: P-256
```

### You may also use openssl to inspect your generated certificates:

```bash
matter@matter-1:~/repos/connectedhomeip$ openssl x509 -noout -text -in \
>   credentials/test/attestation/test-DAC-${VID}-${PID}-cert.pem
Certificate:
    Data:
        Version: 3 (0x2)
        Serial Number: 8147941706030535880 (0x71134dcfde8a54c8)
        Signature Algorithm: ecdsa-with-SHA256
        Issuer: CN = Matter Test PAI, 1.3.6.1.4.1.37244.2.1 = 1141
        Validity
            Not Before: Nov 29 18:10:00 2023 GMT
            Not After : Dec 31 23:59:59 9999 GMT
        Subject: CN = Matter Test DAC 0, 1.3.6.1.4.1.37244.2.1 = 1141, 1.3.6.1.4.1.37244.2.2 = 8005
        Subject Public Key Info:
            Public Key Algorithm: id-ecPublicKey
                Public-Key: (256 bit)
                pub:
                    04:e0:25:06:62:4c:e8:2b:87:86:8e:73:2f:1e:cc:
                    b7:20:f9:15:55:21:fe:28:eb:77:a0:3d:db:ca:e0:
                    e7:80:22:65:dd:f7:d0:6e:44:fc:8e:70:93:5f:73:
                    30:28:d7:41:15:cc:12:b5:23:55:75:24:de:a5:52:
                    f6:89:e3:77:64
                ASN1 OID: prime256v1
                NIST CURVE: P-256
        X509v3 extensions:
            X509v3 Basic Constraints: critical
                CA:FALSE
            X509v3 Key Usage: critical
                Digital Signature
            X509v3 Subject Key Identifier: 
                AA:27:8A:91:B0:24:0D:CA:46:94:9A:33:D7:56:BC:4F:A2:7B:D5:B0
            X509v3 Authority Key Identifier: 
                keyid:48:E4:C5:61:0D:E0:E9:59:36:3F:66:68:7E:C1:E5:B7:2A:67:35:81

    Signature Algorithm: ecdsa-with-SHA256
         30:45:02:20:74:9e:5f:f1:da:b8:e6:87:ce:3f:da:c8:b8:89:
         64:54:4f:97:02:b6:b8:4c:12:bd:65:89:2b:80:6f:8f:4e:1d:
         02:21:00:98:d0:a8:b7:f3:b9:62:f8:da:69:d5:39:d1:00:b9:
         47:77:7c:13:d3:fe:66:8e:13:62:64:34:45:3a:50:01:14
```

## Replace the certificates

Create `connectedhomeip/generate-embeddable-certs.sh`
```bash
#bin/bash...
```

### If you are not yet using Factory Data, you might want to place your PAI in src/credentials/examples/ExampleDACs.cpp. 

In this case, append the resulting generated code to your ExampleDACs.cpp file:
```
ByteSpan kDacCert       = ByteSpan(kDevelopmentDAC_Cert_FFF1_801F);
ByteSpan kDacPrivateKey = ByteSpan(kDevelopmentDAC_PrivateKey_FFF1_801F);
ByteSpan kDacPublicKey  = ByteSpan(kDevelopmentDAC_PublicKey_FFF1_801F);
#endif
} // namespace DevelopmentCerts
} // namespace chip

/* ------------------------------------------ */
/* current end-of-file                        */
/* ------------------------------------------ */

/* ------------------------------------------ */
/* output of creds-codelab.sh script          */
/* ------------------------------------------ */

namespace chip {
namespace DevelopmentCerts {

#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == hexProductId

...

ByteSpan kDacPrivateKey = ByteSpan(Dac_PrivateKey_Array);

#endif // CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID
} // namespace DevelopmentCerts
} // namespace chip
```

Result in connectedhomeip/src/credentials/examples/ExampleDACs.cpp

### Replace the CD

Extract a text representation of the contents of your CD file using xxd:

```bash
xxd -i credentials/test/certification-declaration/Chip-Test-CD-${VID}-${PID}.der
```

If you are using the credentials examples, you probably want to replace the contents of kCdForAllExamples in ExampleDACProvider::GetCertificationDeclaration, in `src/credentials/examples/DeviceAttestationCredsExample.cpp`

### Build the target
Changes:
```bash
# # 4417 == 0x1141 (telink vendor ID)
CONFIG_CHIP_DEVICE_VENDOR_ID=4417
# # 256 == 0x0100 (example lighting-app)
CONFIG_CHIP_DEVICE_TYPE=256

# Disable Matter OTA DFU
CONFIG_CHIP_OTA_REQUESTOR=y
CONFIG_CHIP_DEVICE_SOFTWARE_VERSION=1
CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING="2023.11.27"
```
And then

```bash
west build -b tlsr9518adk80d
```

After building version 1 change CONFIG_CHIP_DEVICE_SOFTWARE_VERSION and CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING, for ex:
```bash
CONFIG_CHIP_DEVICE_SOFTWARE_VERSION=2
CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING="2023.11.27-1"
``` 
then rebuild and extract `zephyr-ota.bin` file, thats the OTA file to be uploaded.

### Verify metadata
```bash
src/app/ota_image_tool.py show examples/lighting-app/telink/build/zephyr/zephyr-ota.bin
```
Example output:

```bash
Magic: 1beef11e
Total Size: 915568
Header Size: 72
Header TLV:
  [0] Vendor Id: 4417 (0x1141)
  [1] Product Id: 32773 (0x8005)
  [2] Version: 2 (0x2)
  [3] Version String: 2023.11.27-2
  [4] Payload Size: 915480 (0xdf818)
  [8] Digest Type: 1 (0x1)
  [9] Digest: dd2b90807a169045e0ca5f18cd8b9c2916ca186125a59482bcc5afbd77210396
```

### Create link to V2 on GoogleDrive (for ex.)

https://drive.google.com/uc?export=download&id=1ksHiCn37LWsWeSLCMGABriul9ADOyz--