# Automatizirano kuhalo za kavu – ESP32

![shema](https://github.com/user-attachments/assets/2889451e-04c3-4aca-8e70-01342b01d59e)


Projekt predstavlja simulaciju automatiziranog kuhala za kavu pomoću ESP32 mikrokontrolera. Sustav provjerava razinu vode, prisutnost kave, mjeri temperaturu, prikazuje status putem LED-ica i koristi energetski učinkovite režime rada.

## Opis rada

Uređaj koristi ultrazvučni senzor za mjerenje razine vode, tipkalo za detekciju kave u filtru i NTC termistor za mjerenje temperature. Ovisno o uvjetima, uređaj odlučuje hoće li započeti proces kuhanja. Grijanje traje dok temperatura ne dosegne 80°C. Nakon toga sustav ulazi u režim niske potrošnje.

## Funkcionalnosti

- Mjerenje razine vode (HC-SR04)
- Detekcija prisutnosti kave (tipkalo)
- Mjerenje temperature (NTC termistor)
- Prikaz statusa putem LED-ica:
  - 3 LED-ice za prikaz razine vode (nisko, srednje, visoko)
  - Crvena LED za upozorenje na nedostatak vode
  - Crvena LED za upozorenje na nedostatak kave
  - Zelena LED za prikaz da je kuhanje u tijeku
- Energetski učinkovito ponašanje:
  - Light sleep tijekom grijanja
  - Deep sleep nakon završenog ciklusa

## Korištene komponente

- ESP32 DevKit V1
- HC-SR04 ultrazvučni senzor
- NTC termistor
- Tipkalo (GPIO 33)
- Višebojne LED-ice (RGB)

## Logika rada

1. Pokretanje ili buđenje iz mirovanja
2. Mjerenje udaljenosti (razina vode)
3. Provjera prisutnosti kave
4. Ako su uvjeti zadovoljeni:
    - Započinje grijanje
    - Očitava temperaturu svakih 3 sekunde
    - Završava kada temperatura dosegne 80°C
5. Nakon završetka:
    - Sve LED-ice se gase
    - Uređaj ulazi u deep sleep
6. Novi ciklus se pokreće pritiskom na tipkalo

## FAST_TEST način

Za potrebe brže simulacije u Wokwi simulatoru koristi se FAST_TEST način:

- Očitavanje temperature svakih 0.3 sekunde
- Deep sleep je onemogućen (simulacija se ne prekida)
- Omogućava brzu provjeru funkcionalnosti

Uključuje se u kodu postavljanjem:

```cpp
#define FAST_TEST true

