# SiFive HiFive 1 Rev B - Amazon FreeRTOS

## Information

### Develop base on amazon-freertos repo
- Clone from commit ID: 343f604888471ef445dc602beb9eac6ccbad0fd4 
```
$ git clone https://github.com/aws/amazon-freertos.git --recurse-submodules
$ git checkout 343f604888471ef445dc602beb9eac6ccbad0fd4 
```

### amazon-freertos\.git\config
```
[remote "origin"]
	url = https://github.com/aws/amazon-freertos.git
	fetch = +refs/heads/*:refs/remotes/origin/*
[branch "master"]
	remote = origin
	merge = refs/heads/master
[submodule "freertos_kernel"]
	url = https://github.com/FreeRTOS/FreeRTOS-Kernel.git
[submodule "libraries/3rdparty/CMock"]
	url = https://github.com/ThrowTheSwitch/CMock.git
[submodule "http-parser"]
	url = https://github.com/nodejs/http-parser.git
[submodule "lwip"]
	url = https://github.com/lwip-tcpip/lwip.git
[submodule "mbedtls"]
	url = https://github.com/ARMmbed/mbedtls.git
[submodule "pkcs11"]
	url = https://github.com/amazon-freertos/pkcs11.git
[submodule "unity"]
	url = https://github.com/ThrowTheSwitch/Unity.git
[submodule "libraries/abstractions/pkcs11/psa"]
	url = https://github.com/Linaro/freertos-pkcs11-psa.git
[submodule "vendors/espressif/esp-idf"]
	url = https://github.com/espressif/esp-afr-sdk.git
```
