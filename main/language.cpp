/*
===============================================================================
  language.cpp — Tables de traduction et API langue (FR / EN)
===============================================================================
*/
#include "language.h"
#include <nvs_flash.h>
#include <nvs.h>

namespace wg {

static Language g_lang = Language::FR;

// IMPORTANT : ordre strict = ordre de l'enum TextID dans language.h
static const char* FR[] = {
    // Écran titre
    "WARGAME MEDIEVAL FANTASTIQUE",
    "Gamebuino AKA Edition",
    "Carte %d/%d - PV de campagne: %d",
    "Appuyez sur A pour commencer",
    "Langue",
    "</> : changer de langue",
    "Difficulte",
    "Haut/Bas : changer de difficulte",
    "Verrouillee (debut de campagne)",
    "Maintenir L1+R1 : deverrouiller (debug)",

    // Briefing
    "A : engager la bataille",

    // HUD bataille
    "PV:%d  Joueurs:%d  Ennemis:%d",
    "Case: %s (cout:%d def:+%d%%)",
    "Case: %s (infranchissable)",
    "A: selection   B: unite suivante",
    "A: valider dep.   B: annuler",
    "A: attaquer   B: annuler",
    "Aucune action ici   B: annuler",
    "C: fin du tour",
    "%s PV:%d/%d ATQ:%d DEF:%d POR:%d",
    "Objectif: eliminer toute l'armee ennemie",
    "Objectif: amener une unite sur le CHATEAU",
    "Objectif: survivre %d tours (tour %d)",

    // Aide
    "AIDE - COMMANDES",
    "D-pad : deplacer le curseur",
    "A : selectionner / valider",
    "  (case bleue=unite, cyan=",
    "   se deplacer, rouge=attaquer)",
    "B : annuler la selection",
    "C : terminer le tour",
    "L1 / R1 : dezoomer / zoomer",
    "Pastille verte = peut agir",
    "Pastille jaune = a moitie",
    "Terrains :",

    // Victoire / Défaite / Campagne
    "VICTOIRE !",
    "Points de victoire : %d",
    "A : Acceder aux renforts",
    "DEFAITE...",
    "Votre armee a ete vaincue",
    "A : Retenter la bataille",
    "CAMPAGNE TERMINEE !",
    "Score final : %d points de victoire",

    // Boutique
    "RENFORTS",
    "PV disponibles : %d",
    "MON ARMEE",
    "RECRUTEMENT",
    "A: soigner +%d PV (cout %d)",
    "Unite au complet",
    "A: recruter (cout %d)",
    "PV insuffisants",
    "Aucune unite",
    "</> : changer de colonne",
    "C: partir vers la carte suivante",

    // Terrains
    "Plaine",
    "Foret",
    "Colline",
    "Eau",
    "Route",
    "Chateau",

    // Unités
    "Infanterie",
    "Archers",
    "Cavalerie",
    "Mage",
    "Catapulte",
    "Heros",
};

static const char* EN[] = {
    // Title screen
    "MEDIEVAL FANTASY WARGAME",
    "Gamebuino AKA Edition",
    "Map %d/%d - Campaign VP: %d",
    "Press A to start",
    "Language",
    "</> : change language",
    "Difficulty",
    "Up/Down: change difficulty",
    "Locked (start of campaign)",
    "Hold L1+R1: unlock (debug)",

    // Briefing
    "A : start the battle",

    // Battle HUD
    "VP:%d  Allies:%d  Enemies:%d",
    "Tile: %s (cost:%d def:+%d%%)",
    "Tile: %s (impassable)",
    "A: select unit   B: next unit",
    "A: confirm move   B: cancel",
    "A: attack   B: cancel",
    "No action here   B: cancel",
    "C: end turn",
    "%s HP:%d/%d ATK:%d DEF:%d RNG:%d",
    "Objective: eliminate the entire enemy army",
    "Objective: bring a unit onto the CASTLE",
    "Objective: survive %d turns (turn %d)",

    // Help
    "HELP - CONTROLS",
    "D-pad : move the cursor",
    "A : select / confirm",
    "  (blue tile=unit, cyan=",
    "   move, red=attack)",
    "B : cancel selection",
    "C : end turn",
    "L1 / R1 : zoom out / in",
    "Green dot = can still act",
    "Yellow dot = half used",
    "Terrain types:",

    // Victory / Defeat / Campaign
    "VICTORY !",
    "Victory points: %d",
    "A : Go to reinforcements",
    "DEFEAT...",
    "Your army has been defeated",
    "A : Retry this battle",
    "CAMPAIGN COMPLETE !",
    "Final score: %d victory points",

    // Shop
    "REINFORCEMENTS",
    "VP available: %d",
    "MY ARMY",
    "RECRUITMENT",
    "A: heal +%d HP (cost %d)",
    "Unit at full health",
    "A: recruit (cost %d)",
    "Not enough VP",
    "No units",
    "</> : switch column",
    "C: move on to the next map",

    // Terrain
    "Plain",
    "Forest",
    "Hill",
    "Water",
    "Road",
    "Castle",

    // Units
    "Infantry",
    "Archers",
    "Cavalry",
    "Mage",
    "Siege Engine",
    "Hero",
};

static const char** get_table(Language lang) {
    switch (lang) {
        case Language::FR: return FR;
        case Language::EN: return EN;
        case Language::COUNT: break;
    }
    return EN;
}

void language_set(Language lang) {
    if ((int)lang < 0 || (int)lang >= (int)Language::COUNT)
        lang = Language::EN;
    g_lang = lang;
    language_save_to_nvs();
}

Language language_get() { return g_lang; }

void language_next() {
    int v = ((int)g_lang + 1) % (int)Language::COUNT;
    language_set((Language)v);
}

void language_prev() {
    int v = ((int)g_lang - 1 + (int)Language::COUNT) % (int)Language::COUNT;
    language_set((Language)v);
}

const char* tr(TextID id) {
    return get_table(g_lang)[(int)id];
}

const char* language_name(Language lang) {
    switch (lang) {
        case Language::FR: return "Francais";
        case Language::EN: return "English";
        case Language::COUNT: break;
    }
    return "???";
}

void language_load_from_nvs() {
    g_lang = Language::FR;
    nvs_handle_t h;
    if (nvs_open("settings", NVS_READONLY, &h) == ESP_OK) {
        uint8_t v = (uint8_t)Language::FR;
        if (nvs_get_u8(h, "lang", &v) == ESP_OK && v < (uint8_t)Language::COUNT)
            g_lang = (Language)v;
        nvs_close(h);
    }
}

void language_save_to_nvs() {
    nvs_handle_t h;
    if (nvs_open("settings", NVS_READWRITE, &h) == ESP_OK) {
        nvs_set_u8(h, "lang", (uint8_t)g_lang);
        nvs_commit(h);
        nvs_close(h);
    }
}

} // namespace wg
