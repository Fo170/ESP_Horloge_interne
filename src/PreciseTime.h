#ifndef PRECISE_TIME_H
#define PRECISE_TIME_H

#include <Arduino.h>

#if defined(ESP32)
#include "driver/timer.h"
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#endif

class PreciseTime {
private:
    // Variables statiques
    static bool initialized;
    
#if defined(ESP32)
    // Variables spécifiques ESP32
    static hw_timer_t* timer;
    static volatile uint64_t time_fct_micros;
    static portMUX_TYPE timerMux;
    
    // Interruption timer ESP32
    static void IRAM_ATTR timerISR() {
        portENTER_CRITICAL_ISR(&timerMux);
        time_fct_micros++;
        portEXIT_CRITICAL_ISR(&timerMux);
    }
    
#elif defined(ESP8266)
    // Variables spécifiques ESP8266
    static volatile uint32_t overflow_counter;
    static uint32_t last_micros;
    static uint64_t total_micros;
    
    // Interruption débordement ESP8266
    static void IRAM_ATTR microsOverflowISR() {
        overflow_counter++;
    }
    
    // Mise à jour du temps ESP8266
    static void updateTime() {
        if (!initialized) return;
        
        uint32_t current_micros = micros();
        uint32_t high_part = overflow_counter;
        
        // Vérifier si un débordement s'est produit
        if (current_micros < last_micros) {
            high_part++;
        }
        
        // Calculer le temps total en microsecondes (64 bits)
        total_micros = ((uint64_t)high_part << 32) | current_micros;
        last_micros = current_micros;
    }
    
#else
    // Variables pour autres architectures
    static uint32_t last_millis;
    static uint64_t total_millis;
    
    // Mise à jour logicielle
    static void updateSoftwareTimer() {
        if (!initialized) return;
        
        uint32_t current = millis();
        uint32_t delta;
        
        if (current >= last_millis) {
            delta = current - last_millis;
        } else {
            // Gestion du débordement de millis()
            delta = (UINT32_MAX - last_millis) + current + 1;
        }
        
        total_millis += delta;
        last_millis = current;
    }
#endif

public:
    // Initialise le système de temps précis
    static void begin() {
        if (initialized) return;
        
#if defined(ESP32)
        // Configuration du timer ESP32 (1MHz = 1µs par tick)
        timer = timerBegin(0, 80, true); // Timer 0, prescaler 80 (80MHz/80=1MHz)
        timerAttachInterrupt(timer, &timerISR, true);
        timerAlarmWrite(timer, 1, true); // Déclenchement toutes les 1µs
        timerAlarmEnable(timer);
        
        Serial.println("PreciseTime: Timer ESP32 initialisé (1µs résolution)");
        
#elif defined(ESP8266)
        // Configuration pour ESP8266
        last_micros = micros();
        total_micros = 0;
        overflow_counter = 0;
        
        // Attacher une interruption au débordement de micros()
        timer1_isr_init();
        timer1_attachInterrupt(microsOverflowISR);
        timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
        
        Serial.println("PreciseTime: Système ESP8266 initialisé");
        
#else
        // Méthode logicielle pour autres architectures
        last_millis = millis();
        total_millis = 0;
        
        Serial.println("PreciseTime: Timer logiciel initialisé (1ms résolution)");
#endif
        
        initialized = true;
        Serial.printf("PreciseTime: Débordement dans %.2f années\n", getOverflowYears());
    }
    
    // Retourne le temps en microsecondes depuis le début
    static uint64_t getMicroseconds() {
#if defined(ESP32)
        if (!initialized) return 0;
        uint64_t time;
        portENTER_CRITICAL(&timerMux);
        time = time_fct_micros;
        portEXIT_CRITICAL(&timerMux);
        return time;
        
#elif defined(ESP8266)
        if (!initialized) return 0;
        updateTime();
        return total_micros;
        
#else
        if (!initialized) return 0;
        updateSoftwareTimer();
        return total_millis * 1000ULL; // Conversion ms → µs
#endif
    }
    
    // Retourne le temps en millisecondes depuis le début
    static uint64_t getMilliseconds() {
        return getMicroseconds() / 1000ULL;
    }
    
    // Retourne le temps en secondes depuis le début
    static uint64_t getSeconds() {
        return getMicroseconds() / 1000000ULL;
    }
    
    // Retourne le temps en secondes avec décimales (précision microsecondes)
    static double getSecondsPrecise() {
        return (double)getMicroseconds() / 1000000.0;
    }
    
    // Retourne le temps formaté: jours, heures, minutes, secondes
    static void getFormattedTime(uint64_t &days, uint32_t &hours, 
                                 uint32_t &minutes, uint32_t &seconds) {
        uint64_t total_seconds = getSeconds();
        
        days = total_seconds / 86400ULL;
        uint64_t remaining = total_seconds % 86400ULL;
        
        hours = remaining / 3600ULL;
        remaining %= 3600ULL;
        
        minutes = remaining / 60ULL;
        seconds = remaining % 60ULL;
    }
    
    // Retourne une chaîne formatée du temps écoulé
    static String getFormattedString() {
        uint64_t days;
        uint32_t hours, minutes, seconds;
        
        getFormattedTime(days, hours, minutes, seconds);
        
        char buffer[64];
        if (days > 0) {
            snprintf(buffer, sizeof(buffer), "%llu jours, %02lu:%02lu:%02lu", 
                     days, hours, minutes, seconds);
        } else {
            snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", 
                     hours, minutes, seconds);
        }
        
        return String(buffer);
    }
    
    // Retourne le temps avant débordement en années (64 bits)
    static double getOverflowYears() {
        // Calcul du temps avant débordement d'un uint64_t en années
        // 2^64 microsecondes = 18446744073709551616 µs
        // Conversion en années: /1000000/3600/24/365
        return (pow(2, 64) / 1000000.0 / 3600.0 / 24.0 / 365.0);
    }
    
    // Mise à jour manuelle (pour les architectures sans interruption)
    static void update() {
#if defined(ESP8266)
        updateTime();
#elif !defined(ESP32)
        updateSoftwareTimer();
#endif
    }
    
    // Vérifie si le système est initialisé
    static bool isInitialized() {
        return initialized;
    }
    
    // Réinitialise le compteur de temps
    static void reset() {
#if defined(ESP32)
        portENTER_CRITICAL(&timerMux);
        time_fct_micros = 0;
        portEXIT_CRITICAL(&timerMux);
        
#elif defined(ESP8266)
        last_micros = micros();
        total_micros = 0;
        overflow_counter = 0;
        
#else
        last_millis = millis();
        total_millis = 0;
#endif
        Serial.println("PreciseTime: Compteur réinitialisé");
    }
};

// Initialisation des variables statiques
bool PreciseTime::initialized = false;

#if defined(ESP32)
hw_timer_t* PreciseTime::timer = nullptr;
volatile uint64_t PreciseTime::time_fct_micros = 0;
portMUX_TYPE PreciseTime::timerMux = portMUX_INITIALIZER_UNLOCKED;

#elif defined(ESP8266)
volatile uint32_t PreciseTime::overflow_counter = 0;
uint32_t PreciseTime::last_micros = 0;
uint64_t PreciseTime::total_micros = 0;

#else
uint32_t PreciseTime::last_millis = 0;
uint64_t PreciseTime::total_millis = 0;
#endif

#endif // PRECISE_TIME_H