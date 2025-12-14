#ifndef ESP_HORLOGE_INTERNAL_LICENSE_HEADER
#define ESP_HORLOGE_INTERNAL_LICENSE_HEADER
/**
 * ESP_Horloge_interne
 * Copyright (C) 2025 Fo170
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#endif

#include <Arduino.h>
#include "PreciseTime.h"

// Définir cette variable pour tester différentes périodes d'affichage
#define DISPLAY_INTERVAL_MS 1000  // Affichage toutes les secondes
#define BLINK_INTERVAL_MS 500     // Clignotement LED toutes les 500ms

// Variables pour la démonstration
uint64_t lastDisplayTime = 0;
uint64_t lastBlinkTime = 0;
bool ledState = false;
int taskCounter = 0;

// Fonction pour démontrer la mesure de temps d'exécution
void demonstrateTaskTiming() {
    uint64_t startTime = PreciseTime::getMicroseconds();
    
    // Simuler une tâche qui prend du temps
    volatile long sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += i * i;
        delayMicroseconds(10); // Simuler un petit délai
    }
    
    uint64_t endTime = PreciseTime::getMicroseconds();
    uint64_t duration = endTime - startTime;
    
    // Afficher occasionnellement la durée
    taskCounter++;
    if (taskCounter % 5 == 0) {
        Serial.printf("[Task %d] Durée: %llu µs (%.3f ms)\n", 
                     taskCounter, duration, duration / 1000.0);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000); // Attente pour la stabilisation du port série
    
    Serial.println("\n════════════════════════════════════════");
    Serial.println("   Système de Temps Précis - Démonstration");
    Serial.println("════════════════════════════════════════");
    
    // Initialiser le système de temps
    PreciseTime::begin();
    
    // Afficher les informations sur l'architecture
    Serial.println("\n📊 Informations système:");
    Serial.print("  Architecture: ");
    #if defined(ESP32)
        Serial.println("ESP32");
    #elif defined(ESP8266)
        Serial.println("ESP8266");
    #else
        Serial.println("Autre (Arduino)");
    #endif
    
    Serial.print("  Résolution temporelle: ");
    #if defined(ESP32)
        Serial.println("1 microseconde");
    #elif defined(ESP8266)
        Serial.println("~4 microsecondes");
    #else
        Serial.println("1 milliseconde");
    #endif
    
    Serial.printf("  Débordement prévu dans: %.1f années\n", 
                  PreciseTime::getOverflowYears());
    Serial.println("════════════════════════════════════════");
    
    // Configurer la LED intégrée
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    
    Serial.println("\n✅ Système prêt!");
    Serial.println("  - LED clignote toutes les 500ms");
    Serial.println("  - Affichage du temps toutes les secondes");
    Serial.println("  - Mesure de temps de tâches en arrière-plan");
    Serial.println("════════════════════════════════════════\n");
}

void loop() {
    uint64_t currentTime = PreciseTime::getMilliseconds();
    
    // 1. Clignotement de la LED
    if (currentTime - lastBlinkTime >= BLINK_INTERVAL_MS) {
        lastBlinkTime = currentTime;
        ledState = !ledState;
        digitalWrite(LED_BUILTIN, ledState);
    }
    
    // 2. Affichage périodique des informations
    if (currentTime - lastDisplayTime >= DISPLAY_INTERVAL_MS) {
        lastDisplayTime = currentTime;
        
        // Récupérer différentes représentations du temps
        String formatted = PreciseTime::getFormattedString();
        uint64_t micros = PreciseTime::getMicroseconds();
        uint64_t millis = PreciseTime::getMilliseconds();
        uint64_t seconds = PreciseTime::getSeconds();
        double preciseSeconds = PreciseTime::getSecondsPrecise();
        
        // Afficher les informations
        Serial.println("🕒 === TEMPS ÉCOULÉ ===");
        Serial.printf("  Formaté:    %s\n", formatted.c_str());
        Serial.printf("  Microsecondes: %llu µs\n", micros);
        Serial.printf("  Millisecondes: %llu ms\n", millis);
        Serial.printf("  Secondes:      %llu s\n", seconds);
        Serial.printf("  Secondes précises: %.6f s\n", preciseSeconds);
        
        // Calcul du pourcentage d'utilisation avant débordement
        double percent = (preciseSeconds / 3600.0 / 24.0 / 365.0) / 
                         PreciseTime::getOverflowYears() * 100.0;
        Serial.printf("  Utilisation: %.10f%% de la durée maximale\n", percent);
        Serial.println();
    }
    
    // 3. Exécuter une tâche avec mesure de temps
    demonstrateTaskTiming();
    
    // 4. Démontrer la fonction update() si nécessaire
    #if defined(ESP8266) || !defined(ESP32)
    // Pour les architectures sans interruption hardware
    PreciseTime::update();
    #endif
    
    // Petit délai pour éviter la surcharge
    delay(500);
}

// Fonction pour démontrer la réinitialisation (à appeler si nécessaire)
void resetTimer() {
    Serial.println("\n🔄 Réinitialisation du compteur de temps...");
    PreciseTime::reset();
    Serial.println("✅ Compteur réinitialisé à zéro");
}