#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

LOCAL_DKP_DIR="$WORKSPACE_DIR/devkitpro-local"
LOCAL_PACMAN="$LOCAL_DKP_DIR/bootstrap/devkitpro/pacman/bin/pacman"
LOCAL_CONF="$LOCAL_DKP_DIR/pacman-local.conf"
LOCAL_ROOT="$LOCAL_DKP_DIR/root"
LOCAL_DB="$LOCAL_DKP_DIR/db"
LOCAL_CACHE="$LOCAL_DKP_DIR/cache"
LOCAL_LOG="$LOCAL_DKP_DIR/log/pacman.log"

echo "[1/3] Instalando toolchain DS (nds-dev)..."

if command -v dkp-pacman >/dev/null 2>&1; then
  sudo dkp-pacman -Syu --noconfirm
  sudo dkp-pacman -S --needed --noconfirm nds-dev
  export DEVKITPRO=/opt/devkitpro
  export DEVKITARM=/opt/devkitpro/devkitARM
elif [[ -x "$LOCAL_PACMAN" && -f "$LOCAL_CONF" ]]; then
  sudo "$LOCAL_PACMAN" \
    --config "$LOCAL_CONF" \
    --root "$LOCAL_ROOT" \
    --dbpath "$LOCAL_DB" \
    --cachedir "$LOCAL_CACHE" \
    --logfile "$LOCAL_LOG" \
    --noconfirm -Syu nds-dev

  export DEVKITPRO="$LOCAL_ROOT/opt/devkitpro"
  export DEVKITARM="$DEVKITPRO/devkitARM"
else
  echo "No se encontró dkp-pacman ni instalación local usable en $LOCAL_DKP_DIR"
  echo "Instala devkitPro: https://devkitpro.org/wiki/Getting_Started"
  exit 1
fi

echo "[2/3] Configurando entorno..."

if [[ ! -f "$DEVKITPRO/ds_rules" ]]; then
  echo "No se encontró $DEVKITPRO/ds_rules. La instalación no quedó completa."
  exit 1
fi

echo "[3/3] Compilando ROM..."
make clean
make

echo "ROM generada: $(pwd)/super_pou_bros_ds.nds"
