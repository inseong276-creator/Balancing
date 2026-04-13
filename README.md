# Nucleo-F103RB 예제 프로젝트

Nucleo-F103RB 보드에서 기본 LED(PC13)를 깜박이는 최소 bare-metal 프로젝트입니다. CMSIS/HAL 같은 외부 라이브러리에 의존하지 않고, 필수 레지스터만 정의하여 동작합니다.

## 구성 요소

- `src/main.c` : PC13 LED 토글 예제
- `src/system_stm32f103xb.c` : 클럭 초기화 및 `SystemCoreClock` 관리
- `startup/startup_stm32f103xb.s` : 벡터 테이블과 초기화 루틴
- `include/stm32f103xb.h` : 최소한의 레지스터 정의
- `linker/stm32f103xb.ld` : 링크 스크립트(128KB Flash, 20KB SRAM 가정)
- `Makefile` : `arm-none-eabi-gcc` 기반 빌드 스크립트

## 필요 도구

- GNU Arm Embedded Toolchain (`arm-none-eabi-gcc`, `arm-none-eabi-objcopy`, `arm-none-eabi-size`)
- OpenOCD (선택 사항, `make flash` 사용 시 필요)

## 빌드 방법

```bash
make
```

빌드가 완료되면 `bin/` 디렉토리에 `*.elf`, `*.hex`, `*.bin` 파일이 생성됩니다.

## 보드에 다운로드

OpenOCD를 사용한다면 다음 명령으로 바로 다운로드할 수 있습니다.

```bash
make flash
```

OpenOCD 설정 파일은 `board/st_nucleo_f103rb.cfg`를 사용하도록 되어 있습니다. 다른 도구(예: ST-LINK CLI, STM32CubeProgrammer)를 사용한다면 `bin/nucleo-f103rb.hex` 또는 `bin/nucleo-f103rb.bin`을 직접 지정하여 다운로드하세요.

## 참고

- 프로젝트는 외부 HSE(8MHz)를 PLL ×9로 승수해 72MHz 시스템 클럭을 구성합니다.
- LED 토글 딜레이는 단순 루프 기반으로, 빌드 최적화 옵션에 따라 주기가 달라질 수 있습니다.
- 필요에 따라 `include/stm32f103xb.h`에 다른 주변장치 레지스터를 추가하면서 확장해 사용할 수 있습니다.
