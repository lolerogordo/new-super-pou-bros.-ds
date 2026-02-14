# Super Pou Bros DS (Homebrew)

Mini plataforma 2D estilo New Super Mario Bros DS, con Pou como personaje principal.

## Controles (Nintendo DS)

- `LEFT/RIGHT`: mover
- `A`: saltar
- `START`: reiniciar cuando ganas o pierdes

## Requisitos

- devkitPro con `devkitARM`, `libnds` y `ndstool`
- Variables de entorno configuradas (`DEVKITPRO`, `DEVKITARM`)

## Compilar

```bash
cd /Users/jacobovaliente/Downloads/stupidnesscode/super_pou_bros_ds
make
```

Salida esperada:

- `super_pou_bros_ds.nds`

## Ejecutar

- En emulador DS (melonDS, DeSmuME) o flashcart.

## Nota legal

Este proyecto es un juego homebrew original inspirado en plataformas clásicas, sin usar assets de Nintendo.

## Build sin sudo local (GitHub Actions)

Si no tienes permisos de admin en tu PC, puedes compilar en GitHub:

1. Sube esta carpeta a un repo de GitHub.
2. Ve a `Actions` -> `Build Super Pou Bros DS` -> `Run workflow`.
3. Descarga el artefacto `super_pou_bros_ds`.

El workflow está en:

- `.github/workflows/build-nds.yml`
