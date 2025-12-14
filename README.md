# ESP_Horloge_interne

Projet PlatformIO pour une horloge interne basée sur ESP (ESP32/ESP8266).

## Description

Code source principal et bibliothèque minimale pour afficher une horloge interne.

## Prérequis

- PlatformIO (extension VS Code ou CLI)
- Un microcontrôleur ESP compatible

## Commandes utiles

Construire le projet :

```bash
platformio run
```

Téléverser sur la carte :

```bash
platformio run --target upload
```

Moniteur série :

```bash
platformio device monitor
```

## Structure

- `platformio.ini` : configuration PlatformIO
- `src/main.cpp` : code principal
- `src/PreciseTime.h` : en-têtes auxiliaires

## Dépôt GitHub

Poussé sur : https://github.com/Fo170/ESP_Horloge_interne

## Licence

MIT — modifiez selon votre choix.

---

Si vous voulez, je peux ajouter un badge PlatformIO, des instructions plus détaillées ou un guide de contribution.
