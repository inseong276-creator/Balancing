#!/usr/bin/env bash
set -euo pipefail

# 기본값: bin/*.elf 중 최신 파일
ELF="${1:-}"
if [[ -z "${ELF}" ]]; then
  ELF="$(ls -t bin/*.elf 2>/dev/null | head -n1 || true)"
fi
if [[ -z "${ELF}" || ! -f "${ELF}" ]]; then
  echo "ELF가 없습니다. 사용법: $0 <path/to/firmware.elf>"; exit 1
fi

# 보드 cfg는 환경변수로도 변경 가능 (기본: Nucleo-F103RB)
BOARD_CFG="${BOARD_CFG:-board/st_nucleo_f103rb.cfg}"

# OpenOCD 스크립트 경로를 별도 설치 위치에 뒀다면(선택)
# export OPENOCD_SCRIPTS=/path/to/openocd/scripts

echo "Flashing: ${ELF}"
openocd -f "${BOARD_CFG}" \
  -c "init; reset halt; program ${ELF} verify reset exit"

