/* -----------------------------------------------------------------------------
    Fichier : levels_data.cpp
    Projet  : Wargame_Aka
    Objet   : Definition des 30 niveaux de la campagne (taille, terrain,
              unites ennemies, textes FR/EN, specialites tactiques)

    Structure :
        - Niveaux 1-10  : Acte 1 "La Horde Grise" (campagne d'origine)
        - Niveaux 11-20 : Acte 2 "La Main Cachee" (Malachar, le nécromant
          derriere l'invasion, et son lieutenant Vrask) — visible a toutes
          les difficultes, la campagne peut se terminer ici (fin "normale").
        - Niveaux 21-30 : Acte 3 "Les Terres Cendrees" — visible uniquement
          en difficulte Difficile (cf. difficulty.h), avec des unites
          ennemies nettement plus puissantes et la fin heroique complete.

    Specialites :
        - river_crossing : une riviere (Water) coupe la carte, seuls les
          Bridge (ponts) permettent de la franchir -> force le passage
          a des points precis, sous le feu de la garnison adverse.
        - castle_walls : un anneau de Wall (remparts) entoure la case
          Chateau, rendant l'approche du siege plus couteuse et plus
          dangereuse (bonus defensif eleve pour les defenseurs).
        - story_image : illustration optionnelle de briefing (cf.
          core/story_image.h) ; nullptr par defaut, seuls certains niveaux
          charnieres en ont une (cf. dossier story/ sur la carte SD).
----------------------------------------------------------------------------- */

#include "levels_data.h"
#include "../language.h"
#include "../difficulty.h"

namespace wg {

// Hash deterministe (meme carte a chaque partie, pas de RNG materiel)
static uint32_t hash2(int x, int y, int seed) {
    uint32_t h = (uint32_t)(x * 374761393 + y * 668265263 + seed * 2246822519u);
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

static const std::vector<LevelDef>& levels_table() {
    static std::vector<LevelDef> table = {

        // =====================================================================
        // Niveau 1 : Plaines de l'Aube
        // =====================================================================
        LevelDef{
            "Plaines de l'Aube", "Plains of Dawn",
            16, 12, 15, 10,
            {
                { UnitType::Infantry, {13, 4} },
                { UnitType::Infantry, {14, 7} },
                { UnitType::Archer,   {14, 3} },
            },
            { {0,3},{0,4},{0,5},{0,6},{1,3},{1,4},{1,5},{1,6} },
            30, 20,
            VictoryCondition::EliminateAll, 0,
            false, -1, {}, false,
            "Le royaume d'Aubevent vit en\n"
            "paix depuis des generations.\n"
            "Mais a l'aube, des eclaireurs\n"
            "de la Horde Grise ont franchi\n"
            "la frontiere nord. Vous menez\n"
            "la garnison locale pour\n"
            "repousser cette avant-garde\n"
            "avant qu'elle n'alerte le\n"
            "gros de l'armee ennemie.",
            "The realm of Aubevent has\n"
            "known peace for generations.\n"
            "But at dawn, scouts of the\n"
            "Grey Horde crossed the\n"
            "northern border. You lead the\n"
            "local garrison to repel this\n"
            "vanguard before it can warn\n"
            "the main enemy force.",
            "L'avant-garde est detruite.\n"
            "Mais les survivants fuient\n"
            "vers les collines... la Horde\n"
            "Grise sait desormais que vous\n"
            "etes pretes a vous battre.",
            "The vanguard is destroyed.\n"
            "But the survivors flee toward\n"
            "the hills... the Grey Horde\n"
            "now knows you are ready to\n"
            "fight back."
        },

        // =====================================================================
        // Niveau 2 : Col de la Colline Grise
        // =====================================================================
        LevelDef{
            "Col de la Colline Grise", "Grey Hill Pass",
            17, 13, 20, 35,
            {
                { UnitType::Infantry, {14, 4} },
                { UnitType::Infantry, {14, 8} },
                { UnitType::Archer,   {15, 5} },
                { UnitType::Cavalry,  {15, 7} },
            },
            { {0,3},{0,4},{0,5},{0,6},{0,7},{1,4},{1,5},{1,6} },
            40, 30,
            VictoryCondition::EliminateAll, 0,
            false, -1, {}, false,
            "En poursuivant les fuyards,\n"
            "votre armee atteint le Col de\n"
            "la Colline Grise. La Horde y\n"
            "a rassemble des cavaliers\n"
            "pour vous prendre en tenaille\n"
            "dans les passages etroits.\n"
            "Le terrain sera votre meilleur\n"
            "allie si vous savez l'utiliser.",
            "Pursuing the survivors, your\n"
            "army reaches the Grey Hill\n"
            "Pass. The Horde has gathered\n"
            "cavalry there to trap you in\n"
            "the narrow passes. The terrain\n"
            "will be your best ally if you\n"
            "know how to use it.",
            "La cavalerie ennemie est\n"
            "dispersee. Au loin, vous\n"
            "apercevez les feux d'un\n"
            "campement bien plus grand :\n"
            "la Horde Grise n'agissait pas\n"
            "seule.",
            "The enemy cavalry is\n"
            "scattered. In the distance,\n"
            "you spot the fires of a much\n"
            "larger camp: the Grey Horde\n"
            "was not acting alone."
        },

        // =====================================================================
        // Niveau 3 : Foret Noire
        // =====================================================================
        LevelDef{
            "Foret Noire", "The Dark Forest",
            18, 13, 50, 15,
            {
                { UnitType::Infantry, {15, 3} },
                { UnitType::Infantry, {15, 9} },
                { UnitType::Archer,   {16, 4} },
                { UnitType::Archer,   {16, 8} },
                { UnitType::Mage,     {16, 6} },
            },
            { {0,3},{0,4},{0,5},{0,6},{0,7},{0,8},{1,5},{1,6} },
            55, 40,
            VictoryCondition::EliminateAll, 0,
            false, -1, {}, false,
            "La piste s'enfonce dans la\n"
            "Foret Noire, un lieu que meme\n"
            "les anciens evitaient. Des\n"
            "mages corrompus s'y sont allies\n"
            "a la Horde Grise, attires par\n"
            "un pouvoir enfoui sous les\n"
            "racines. Mefiez-vous de leur\n"
            "magie : elle frappe a travers\n"
            "les armures.",
            "The trail leads into the Dark\n"
            "Forest, a place even the elders\n"
            "avoided. Corrupted mages have\n"
            "allied with the Grey Horde,\n"
            "drawn by a power buried beneath\n"
            "the roots. Beware their magic:\n"
            "it strikes through armor.",
            "Les mages sont vaincus, mais\n"
            "leurs grimoires parlent d'un\n"
            "seigneur de guerre attendant au\n"
            "Chateau Rouille, une forteresse\n"
            "tombee il y a des annees.",
            "The mages are defeated, but\n"
            "their grimoires speak of a\n"
            "warlord waiting at Rustfall\n"
            "Castle, a fortress that fell\n"
            "many years ago."
        },

        // =====================================================================
        // Niveau 4 : Siege du Chateau Rouille (SPECIALITE : siege de chateau)
        // =====================================================================
        LevelDef{
            "Siege du Chateau Rouille", "Siege of Rustfall Castle",
            19, 14, 25, 25,
            {
                { UnitType::Infantry,    {16, 4} },
                { UnitType::Infantry,    {16, 9} },
                { UnitType::Cavalry,     {17, 5} },
                { UnitType::Archer,      {17, 6} },
                { UnitType::Mage,        {17, 8} },
                { UnitType::SiegeEngine, {18, 6} },
            },
            { {0,4},{0,5},{0,6},{0,7},{0,8},{1,5},{1,6},{1,7} },
            70, 55,
            VictoryCondition::CaptureCastle, 0,
            false, -1, {}, true, // <-- remparts autour du chateau
            "Le Chateau Rouille, autrefois\n"
            "bastion du royaume, abrite\n"
            "maintenant l'etat-major de la\n"
            "Horde Grise. Les remparts\n"
            "ralentiront votre approche :\n"
            "amenez une unite jusqu'a la\n"
            "case Chateau pour reprendre\n"
            "la forteresse.",
            "Rustfall Castle, once a\n"
            "bastion of the realm, now\n"
            "houses the Grey Horde's\n"
            "command. Its walls will slow\n"
            "your approach: bring a unit\n"
            "onto the Castle tile to\n"
            "retake the fortress.",
            "L'etendard d'Aubevent flotte\n"
            "de nouveau sur le Chateau\n"
            "Rouille. Mais les survivants\n"
            "de la garnison fuient vers\n"
            "l'est, ou coule la riviere\n"
            "des Cendres. Vous devrez les\n"
            "poursuivre sans relache.",
            "Aubevent's banner flies over\n"
            "Rustfall Castle once more.\n"
            "But the surviving garrison\n"
            "flees east, toward the Ashen\n"
            "River. You must pursue them\n"
            "without rest."
        },

        // =====================================================================
        // Niveau 5 : Le Passage des Cendres (SPECIALITE : franchissement)
        // =====================================================================
        LevelDef{
            "Le Passage des Cendres", "The Ashen Crossing",
            20, 14, 20, 15,
            {
                { UnitType::Infantry, {10, 4} },
                { UnitType::Archer,   {11, 3} },
                { UnitType::Infantry, {10, 10} },
                { UnitType::Archer,   {11, 11} },
                { UnitType::Cavalry,  {15, 7} },
            },
            { {0,4},{0,5},{0,6},{0,7},{0,8},{1,5},{1,6},{1,7} },
            80, 60,
            VictoryCondition::EliminateAll, 0,
            true, 9, {3, 10}, false, // <-- riviere en colonne 9, ponts aux lignes 3 et 10
            "La garnison en fuite atteint\n"
            "la riviere des Cendres. Deux\n"
            "ponts de pierre permettent\n"
            "seuls de la franchir : le\n"
            "reste du courant est trop\n"
            "violent pour une armee en\n"
            "armure. L'ennemi tient les\n"
            "deux passages.",
            "The fleeing garrison reaches\n"
            "the Ashen River. Only two\n"
            "stone bridges allow crossing:\n"
            "the rest of the current is\n"
            "too violent for an armored\n"
            "force. The enemy holds both\n"
            "crossings.",
            "Les deux ponts sont conquis.\n"
            "Au-dela de la riviere, les\n"
            "fuyards se regroupent plus\n"
            "loin, pres d'un gue plus\n"
            "ancien et mieux defendu.",
            "Both bridges are taken.\n"
            "Beyond the river, the\n"
            "fugitives regroup further on,\n"
            "near an older, better\n"
            "defended ford."
        },

        // =====================================================================
        // Niveau 6 : Le Gue de la Riviere Argent (SPECIALITE : franchissement)
        // =====================================================================
        LevelDef{
            "Le Gue de la Riviere Argent", "The Silverford Crossing",
            21, 15, 20, 20,
            {
                { UnitType::Infantry,    {10, 2} },
                { UnitType::Infantry,    {10, 12} },
                { UnitType::Archer,      {11, 6} },
                { UnitType::Archer,      {11, 8} },
                { UnitType::Mage,        {12, 7} },
                { UnitType::SiegeEngine, {13, 7} },
                { UnitType::Griffon,     {16, 9} },
            },
            { {0,5},{0,6},{0,7},{0,8},{0,9},{1,6},{1,7},{1,8} },
            90, 65,
            VictoryCondition::EliminateAll, 0,
            true, 10, {2, 7, 12}, false, // <-- riviere en colonne 10, trois ponts
            "La Riviere Argent est plus\n"
            "large encore, mais trois gues\n"
            "y sont praticables. La Horde\n"
            "y a place une catapulte qui\n"
            "balaie chaque passage : avancer\n"
            "a decouvert sera couteux.",
            "The Silver River is wider\n"
            "still, but three fords cross\n"
            "it. The Horde has placed a\n"
            "siege engine that sweeps\n"
            "every crossing: advancing in\n"
            "the open will be costly.",
            "Les gues sont pris, la\n"
            "catapulte ennemie reduite au\n"
            "silence. Au-dela, les eclaireurs\n"
            "signalent un second bastion :\n"
            "le fort de Pierregrise.",
            "The fords are taken, the\n"
            "enemy siege engine silenced.\n"
            "Beyond, scouts report a\n"
            "second stronghold: the fort\n"
            "of Greystone."
        },

        // =====================================================================
        // Niveau 7 : Bastion de Pierregrise (SPECIALITE : siege de chateau)
        // =====================================================================
        LevelDef{
            "Bastion de Pierregrise", "Greystone Bastion",
            22, 15, 25, 30,
            {
                { UnitType::Infantry,    {18, 3} },
                { UnitType::Infantry,    {18, 11} },
                { UnitType::Cavalry,     {19, 5} },
                { UnitType::Cavalry,     {19, 9} },
                { UnitType::Archer,      {19, 7} },
                { UnitType::Mage,        {20, 6} },
                { UnitType::SiegeEngine, {20, 8} },
            },
            { {0,5},{0,6},{0,7},{0,8},{0,9},{1,6},{1,7},{1,8} },
            105, 75,
            VictoryCondition::CaptureCastle, 0,
            false, -1, {}, true, // <-- remparts epais autour du bastion
            "Le bastion de Pierregrise\n"
            "abrite une garnison bien plus\n"
            "nombreuse que celle du\n"
            "Chateau Rouille. Ses remparts\n"
            "sont hauts et bien gardes :\n"
            "chaque approche sera payee\n"
            "au prix du sang.",
            "Greystone Bastion holds a\n"
            "garrison far larger than\n"
            "Rustfall's. Its walls are\n"
            "high and well guarded: every\n"
            "approach will be paid for in\n"
            "blood.",
            "Pierregrise tombe a son tour.\n"
            "Dans les geoles, un prisonnier\n"
            "parle d'un capitaine\n"
            "mercenaire qui garde un pont\n"
            "strategique plus a l'est,\n"
            "vendu au plus offrant.",
            "Greystone falls in turn. In\n"
            "the dungeons, a prisoner\n"
            "speaks of a mercenary captain\n"
            "guarding a strategic bridge\n"
            "further east, loyal only to\n"
            "the highest bidder."
        },

        // =====================================================================
        // Niveau 8 : Le Pont des Trahisons (SPECIALITE : franchissement)
        // =====================================================================
        LevelDef{
            "Le Pont des Trahisons", "The Bridge of Betrayal",
            23, 16, 20, 20,
            {
                { UnitType::Infantry, {11, 3} },
                { UnitType::Infantry, {11, 12} },
                { UnitType::Cavalry,  {12, 6} },
                { UnitType::Cavalry,  {12, 9} },
                { UnitType::Archer,   {13, 5} },
                { UnitType::Archer,   {13, 10} },
                { UnitType::Mage,     {14, 7} },
            },
            { {0,5},{0,6},{0,7},{0,8},{0,9},{0,10},{1,7},{1,8} },
            115, 80,
            VictoryCondition::EliminateAll, 0,
            true, 11, {3, 8, 13}, false, // <-- riviere en colonne 11, trois ponts
            "Le capitaine mercenaire n'a\n"
            "pas hesite : il vend le\n"
            "passage du fleuve a la Horde\n"
            "Grise. Ses hommes tiennent les\n"
            "trois ponts avec une\n"
            "discipline qui n'a rien\n"
            "d'improvise.",
            "The mercenary captain did not\n"
            "hesitate: he sold the river\n"
            "crossing to the Grey Horde.\n"
            "His men hold all three\n"
            "bridges with a discipline\n"
            "that is anything but\n"
            "improvised.",
            "Le capitaine mercenaire est\n"
            "tombe avec ses hommes. Au-dela\n"
            "du fleuve, les eclaireurs\n"
            "rapportent la presence d'un\n"
            "lieutenant du seigneur de\n"
            "guerre en personne, retranche\n"
            "dans la citadelle de Karnstein.",
            "The mercenary captain has\n"
            "fallen with his men. Beyond\n"
            "the river, scouts report a\n"
            "lieutenant of the warlord\n"
            "himself, entrenched within\n"
            "the citadel of Karnstein."
        },

        // =====================================================================
        // Niveau 9 : Chute de Karnstein (SPECIALITE : siege de chateau)
        // =====================================================================
        LevelDef{
            "Chute de Karnstein", "The Fall of Karnstein",
            24, 16, 25, 30,
            {
                { UnitType::Infantry,    {19, 3} },
                { UnitType::Infantry,    {19, 12} },
                { UnitType::Cavalry,     {20, 5} },
                { UnitType::Cavalry,     {20, 10} },
                { UnitType::Archer,      {21, 6} },
                { UnitType::Archer,      {21, 9} },
                { UnitType::Mage,        {21, 7} },
                { UnitType::SiegeEngine, {22, 8} },
                { UnitType::Hero,        {22, 5} },
                { UnitType::Sapper,      {21, 11} },
            },
            { {0,5},{0,6},{0,7},{0,8},{0,9},{0,10},{1,7},{1,8} },
            130, 90,
            VictoryCondition::CaptureCastle, 0,
            false, -1, {}, true, // <-- la plus grande forteresse, fortement fortifiee
            "Karnstein est la plus grande\n"
            "citadelle de la region. Le\n"
            "lieutenant du seigneur de\n"
            "guerre la defend en personne,\n"
            "soutenu par toute sa garde\n"
            "d'elite. C'est ici que se\n"
            "joue l'acces au Trone Brise.",
            "Karnstein is the largest\n"
            "citadel in the region. The\n"
            "warlord's lieutenant defends\n"
            "it himself, backed by his\n"
            "entire elite guard. The road\n"
            "to the Broken Throne is\n"
            "decided here.",
            "Karnstein s'effondre. Le\n"
            "lieutenant vaincu, avant de\n"
            "mourir, murmure que le\n"
            "seigneur de guerre vous\n"
            "attend deja, sur les ruines\n"
            "du Trone Brise.",
            "Karnstein falls. Defeated,\n"
            "the lieutenant whispers\n"
            "before dying that the\n"
            "warlord already awaits you,\n"
            "upon the ruins of the\n"
            "Broken Throne."
        },

        // =====================================================================
        // Niveau 10 : Bataille Finale - Le Trone Brise
        // =====================================================================
        LevelDef{
            "Bataille Finale - Le Trone Brise", "Final Battle - The Broken Throne",
            26, 17, 30, 30,
            {
                { UnitType::Infantry,    {22, 3} },
                { UnitType::Infantry,    {22, 13} },
                { UnitType::Cavalry,     {23, 5} },
                { UnitType::Cavalry,     {23, 11} },
                { UnitType::Archer,      {24, 6} },
                { UnitType::Archer,      {24, 10} },
                { UnitType::Mage,        {24, 8} },
                { UnitType::SiegeEngine, {25, 6} },
                { UnitType::Hero,        {25, 9} },
            },
            { {0,5},{0,6},{0,7},{0,8},{0,9},{0,10},{0,11},{1,7},{1,8},{1,9} },
            160, 100,
            VictoryCondition::EliminateAll, 0,
            false, -1, {}, false,
            "Sur les ruines d'un trone\n"
            "brise depuis des siecles, le\n"
            "seigneur de guerre de la\n"
            "Horde Grise rassemble ses\n"
            "dernieres forces. C'est ici,\n"
            "et nulle part ailleurs, que\n"
            "le sort d'Aubevent sera\n"
            "scelle. Abattez le seigneur\n"
            "de guerre et son armee pour\n"
            "mettre fin a l'invasion.",
            "On the ruins of a throne\n"
            "broken for centuries, the\n"
            "Grey Horde's warlord gathers\n"
            "his last forces. It is here,\n"
            "and nowhere else, that\n"
            "Aubevent's fate will be\n"
            "sealed. Strike down the\n"
            "warlord and his army to end\n"
            "the invasion.",
            "Le seigneur de guerre tombe.\n"
            "La Horde Grise se disperse\n"
            "dans le vent. Aubevent est\n"
            "sauve - du moins pour cette\n"
            "generation. Vous avez ecrit\n"
            "votre legende.",
            "The warlord falls. The Grey\n"
            "Horde scatters to the wind.\n"
            "Aubevent is saved - at least\n"
            "for this generation. You have\n"
            "written your legend."
        },

        // =====================================================================
        // Niveau 11 : Traque dans Sylvenoire
        // =====================================================================
        LevelDef{
            "Traque dans Sylvenoire", "Hunt in Sylvenoire",
            22, 15, 45, 15,
            {
                { UnitType::Infantry, {17, 4} },
                { UnitType::Infantry, {17, 10} },
                { UnitType::Archer, {18, 6} },
                { UnitType::Archer, {18, 8} },
                { UnitType::Cavalry, {19, 7} },
            },
            { {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7} },
            175, 110,
            VictoryCondition::EliminateAll, 0,
            false, -1, {}, false,
            "Les survivants de la Horde\n"
            "Grise fuient vers le nord,\n"
            "dans la foret de Sylvenoire.\n"
            "Avant de mourir, un eclaireur\n"
            "capture a murmure un nom :\n"
            "\"le Maitre nous attend deja\".\n"
            "Qui commandait donc vraiment\n"
            "cette invasion ?",
            "The Grey Horde survivors flee\n"
            "north into Sylvenoire forest.\n"
            "Before dying, a captured\n"
            "scout whispered a name:\n"
            "\"the Master already awaits\n"
            "us\". Who truly commanded\n"
            "this invasion?",
            "Les fuyards sont abattus.\n"
            "Sur l'un d'eux, un talisman\n"
            "grave d'un symbole inconnu:\n"
            "une couronne brisee entouree\n"
            "de flammes noires.",
            "The fugitives are cut down.\n"
            "On one, a talisman engraved\n"
            "with an unknown symbol: a\n"
            "broken crown wreathed in\n"
            "black flame.",
            "act2_sylvenoire"
        },

        // =====================================================================
        // Niveau 12 : Le Marais de Gorvenn
        // =====================================================================
        LevelDef{
            "Le Marais de Gorvenn", "The Gorvenn Marshes",
            23, 15, 20, 10,
            {
                { UnitType::Infantry, {17, 3} },
                { UnitType::Infantry, {17, 11} },
                { UnitType::Archer, {18, 5} },
                { UnitType::Archer, {18, 9} },
                { UnitType::Mage, {19, 7} },
                { UnitType::Cavalry, {20, 6} },
            },
            { {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {0,8}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7}, {1,8} },
            185, 115,
            VictoryCondition::EliminateAll, 0,
            true, 14, {4, 5, 9, 10}, false,
            "Le talisman mene au Marais de\n"
            "Gorvenn. Une riviere boueuse\n"
            "coupe le passage ; seuls\n"
            "deux gues sont praticables.\n"
            "Des mages en embuscade\n"
            "gardent l'autre rive.",
            "The talisman leads to the\n"
            "Gorvenn Marshes. A muddy\n"
            "river blocks the way; only\n"
            "two fords are passable.\n"
            "Mages lie in ambush on the\n"
            "far bank.",
            "Les gues sont conquis. Au\n"
            "loin, des ruines englouties\n"
            "emergent de la brume - les\n"
            "vestiges de Var Kessad,\n"
            "cite oubliee depuis des\n"
            "siecles.",
            "The fords are taken. In the\n"
            "distance, sunken ruins\n"
            "emerge from the mist - the\n"
            "remains of Var Kessad, a\n"
            "city forgotten for\n"
            "centuries.",
            nullptr
        },

        // =====================================================================
        // Niveau 13 : Ruines de Var Kessad
        // =====================================================================
        LevelDef{
            "Ruines de Var Kessad", "Ruins of Var Kessad",
            24, 16, 15, 35,
            {
                { UnitType::Infantry, {17, 4} },
                { UnitType::Infantry, {17, 12} },
                { UnitType::Archer, {18, 6} },
                { UnitType::Archer, {18, 10} },
                { UnitType::Mage, {19, 8} },
                { UnitType::Mage, {20, 5} },
                { UnitType::Sapper, {20, 11} },
            },
            { {0,4}, {0,5}, {0,6}, {0,7}, {0,8}, {1,4}, {1,5}, {1,6}, {1,7}, {1,8} },
            200, 125,
            VictoryCondition::EliminateAll, 0,
            false, -1, {}, false,
            "Var Kessad etait jadis une\n"
            "cite de mages, rasee il y a\n"
            "des generations. Des\n"
            "cultistes en robe noire\n"
            "y celebrent un rituel parmi\n"
            "les colonnes effondrees.",
            "Var Kessad was once a city\n"
            "of mages, razed generations\n"
            "ago. Black-robed cultists\n"
            "hold a ritual among the\n"
            "fallen columns.",
            "Le rituel est interrompu.\n"
            "Des inscriptions fraiches\n"
            "parlent d'un \"Maitre\" qui\n"
            "\"reclamera ce qui lui est du\"\n"
            "a Rustfall.",
            "The ritual is interrupted.\n"
            "Fresh inscriptions speak of\n"
            "a \"Master\" who \"will reclaim\n"
            "what is owed\" at Rustfall.",
            nullptr
        },

        // =====================================================================
        // Niveau 14 : Embuscade a la Passe des Ombres
        // =====================================================================
        LevelDef{
            "Embuscade a la Passe des Ombres", "Ambush at Shadow Pass",
            22, 14, 10, 45,
            {
                { UnitType::Cavalry, {16, 4} },
                { UnitType::Cavalry, {16, 10} },
                { UnitType::Cavalry, {17, 7} },
                { UnitType::Archer, {18, 5} },
                { UnitType::Archer, {18, 9} },
                { UnitType::Infantry, {19, 7} },
            },
            { {0,2}, {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {1,2}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7} },
            210, 130,
            VictoryCondition::EliminateAll, 0,
            false, -1, {}, false,
            "La route vers Rustfall passe\n"
            "par un col etroit. La\n"
            "cavalerie ennemie a choisi\n"
            "ce terrain pour vous prendre\n"
            "en tenaille des l'entree du\n"
            "passage.",
            "The road to Rustfall runs\n"
            "through a narrow pass. The\n"
            "enemy cavalry chose this\n"
            "ground to trap you as soon\n"
            "as you enter the passage.",
            "La passe est degagee. Au\n"
            "sud, les tours familieres de\n"
            "Rustfall se dressent - mais\n"
            "un etendard inconnu flotte\n"
            "desormais sur ses remparts.",
            "The pass is clear. To the\n"
            "south, Rustfall's familiar\n"
            "towers rise - but an unknown\n"
            "banner now flies over its\n"
            "walls.",
            nullptr
        },

        // =====================================================================
        // Niveau 15 : Cendrebourg en Flammes
        // =====================================================================
        LevelDef{
            "Cendrebourg en Flammes", "Cinderbourg Ablaze",
            22, 15, 20, 15,
            {
                { UnitType::Infantry, {16, 4} },
                { UnitType::Infantry, {16, 10} },
                { UnitType::Archer, {17, 7} },
                { UnitType::Sapper, {18, 5} },
                { UnitType::Sapper, {18, 9} },
            },
            { {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7} },
            220, 135,
            VictoryCondition::SurviveTurns, 8,
            false, -1, {}, false,
            "Le village de Cendrebourg\n"
            "brule. Des sapeurs ennemis\n"
            "detruisent les maisons une a\n"
            "une. Tenez la ligne jusqu'a\n"
            "l'evacuation complete des\n"
            "villageois - 8 tours.",
            "The village of Cinderbourg\n"
            "burns. Enemy sappers destroy\n"
            "the houses one by one. Hold\n"
            "the line until all villagers\n"
            "are evacuated - 8 turns.",
            "Les derniers villageois\n"
            "s'echappent par le chemin\n"
            "sud. Un vieillard vous\n"
            "remercie et murmure:\n"
            "\"Rustfall etait notre\n"
            "dernier espoir...\"",
            "The last villagers escape\n"
            "down the southern path. An\n"
            "old man thanks you and\n"
            "whispers: \"Rustfall was our\n"
            "last hope...\"",
            nullptr
        },

        // =====================================================================
        // Niveau 16 : La Crypte Oubliee
        // =====================================================================
        LevelDef{
            "La Crypte Oubliee", "The Forgotten Crypt",
            22, 15, 10, 20,
            {
                { UnitType::Infantry, {16, 4} },
                { UnitType::Infantry, {16, 10} },
                { UnitType::Archer, {17, 6} },
                { UnitType::Archer, {17, 8} },
                { UnitType::Mage, {18, 7} },
                { UnitType::Hero, {19, 7} },
            },
            { {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7} },
            235, 145,
            VictoryCondition::EliminateAll, 0,
            false, -1, {}, true,
            "Sous Cendrebourg s'ouvre une\n"
            "crypte scellee. Un capitaine\n"
            "d'elite du Maitre la defend,\n"
            "entoure de remparts\n"
            "improvises. Il semble en\n"
            "savoir long sur Rustfall.",
            "Beneath Cinderbourg lies a\n"
            "sealed crypt. One of the\n"
            "Master's elite captains\n"
            "defends it, behind hastily\n"
            "raised walls. He seems to\n"
            "know much about Rustfall.",
            "Le capitaine tombe en\n"
            "riant: \"Vrask vous attend\n"
            "a Rustfall... et lui ne\n"
            "rit jamais.\"",
            "The captain falls laughing:\n"
            "\"Vrask awaits you at\n"
            "Rustfall... and he never\n"
            "laughs.\"",
            nullptr
        },

        // =====================================================================
        // Niveau 17 : Trahison a Rustfall
        // =====================================================================
        LevelDef{
            "Trahison a Rustfall", "Betrayal at Rustfall",
            24, 16, 15, 25,
            {
                { UnitType::Infantry, {17, 4} },
                { UnitType::Infantry, {17, 12} },
                { UnitType::Cavalry, {18, 6} },
                { UnitType::Cavalry, {18, 10} },
                { UnitType::Archer, {19, 8} },
                { UnitType::Mage, {20, 7} },
                { UnitType::SiegeEngine, {20, 10} },
            },
            { {0,4}, {0,5}, {0,6}, {0,7}, {0,8}, {1,4}, {1,5}, {1,6}, {1,7}, {1,8} },
            250, 155,
            VictoryCondition::CaptureCastle, 0,
            false, -1, {}, false,
            "Rustfall, forteresse alliee\n"
            "depuis toujours, a ete prise\n"
            "par surprise - trahie de\n"
            "l'interieur. Reprenez le\n"
            "chateau avant que Vrask n'en\n"
            "fasse une place forte.",
            "Rustfall, a long-time allied\n"
            "fortress, was taken by\n"
            "surprise - betrayed from\n"
            "within. Retake the castle\n"
            "before Vrask turns it into\n"
            "a stronghold.",
            "Rustfall est reprise. Le\n"
            "traitre est demasque et\n"
            "capture. Vrask, lui, s'est\n"
            "deja enfui vers Var Kessad\n"
            "avec le gros de ses forces.",
            "Rustfall is retaken. The\n"
            "traitor is unmasked and\n"
            "captured. Vrask himself has\n"
            "already fled to Var Kessad\n"
            "with the bulk of his forces.",
            "act2_rustfall"
        },

        // =====================================================================
        // Niveau 18 : Le Lieutenant Vrask
        // =====================================================================
        LevelDef{
            "Le Lieutenant Vrask", "Lieutenant Vrask",
            24, 16, 20, 20,
            {
                { UnitType::Infantry, {17, 5} },
                { UnitType::Infantry, {17, 11} },
                { UnitType::Archer, {18, 7} },
                { UnitType::Archer, {18, 9} },
                { UnitType::Cavalry, {19, 8} },
                { UnitType::SiegeEngine, {20, 6} },
                { UnitType::Hero, {21, 8} },
            },
            { {0,4}, {0,5}, {0,6}, {0,7}, {0,8}, {1,4}, {1,5}, {1,6}, {1,7}, {1,8} },
            265, 165,
            VictoryCondition::EliminateAll, 0,
            false, -1, {}, false,
            "Vrask commande en personne\n"
            "l'arriere-garde du Maitre.\n"
            "Grand, silencieux, il ne\n"
            "porte aucune banniere -\n"
            "seulement une couronne\n"
            "brisee gravee sur son\n"
            "armure.",
            "Vrask personally commands\n"
            "the Master's rearguard.\n"
            "Tall and silent, he bears no\n"
            "banner - only a broken\n"
            "crown engraved upon his\n"
            "armor.",
            "Vrask s'effondre sans un\n"
            "mot. Sur son corps, une\n"
            "carte de Var Kessad indique\n"
            "un sanctuaire souterrain\n"
            "que meme les ruines\n"
            "ignoraient.",
            "Vrask falls without a word.\n"
            "On his body, a map of Var\n"
            "Kessad marks an underground\n"
            "sanctuary that even the\n"
            "ruins did not reveal.",
            nullptr
        },

        // =====================================================================
        // Niveau 19 : Le Siege de Var Kessad
        // =====================================================================
        LevelDef{
            "Le Siege de Var Kessad", "The Siege of Var Kessad",
            25, 16, 15, 30,
            {
                { UnitType::Infantry, {18, 4} },
                { UnitType::Infantry, {18, 12} },
                { UnitType::Cavalry, {19, 6} },
                { UnitType::Cavalry, {19, 10} },
                { UnitType::Archer, {20, 7} },
                { UnitType::Archer, {20, 9} },
                { UnitType::Mage, {21, 8} },
                { UnitType::SiegeEngine, {21, 5} },
                { UnitType::Sapper, {22, 11} },
            },
            { {0,4}, {0,5}, {0,6}, {0,7}, {0,8}, {0,9}, {1,4}, {1,5}, {1,6}, {1,7}, {1,8}, {1,9} },
            285, 175,
            VictoryCondition::CaptureCastle, 0,
            false, -1, {}, true,
            "Le sanctuaire est sous\n"
            "haute garde : remparts\n"
            "reconstruits, garnison\n"
            "entiere rappelee de tout le\n"
            "front. Le Maitre sait que\n"
            "vous venez.",
            "The sanctuary is heavily\n"
            "guarded: walls rebuilt, the\n"
            "entire garrison recalled\n"
            "from the front. The Master\n"
            "knows you are coming.",
            "Les remparts cedent. Sous\n"
            "les ruines, un escalier\n"
            "descend plus profond que\n"
            "quiconque n'a jamais creuse\n"
            "a Var Kessad.",
            "The walls give way. Beneath\n"
            "the ruins, a stairway\n"
            "descends deeper than anyone\n"
            "has ever dug at Var Kessad.",
            nullptr
        },

        // =====================================================================
        // Niveau 20 : Le Sanctuaire de Malachar
        // =====================================================================
        LevelDef{
            "Le Sanctuaire de Malachar", "Malachar's Sanctuary",
            26, 17, 10, 20,
            {
                { UnitType::Infantry, {19, 4} },
                { UnitType::Infantry, {19, 12} },
                { UnitType::Cavalry, {20, 6} },
                { UnitType::Cavalry, {20, 10} },
                { UnitType::Archer, {21, 7} },
                { UnitType::Archer, {21, 9} },
                { UnitType::Mage, {22, 5} },
                { UnitType::Mage, {22, 11} },
                { UnitType::SiegeEngine, {22, 8} },
                { UnitType::Hero, {24, 8} },
            },
            { {0,4}, {0,5}, {0,6}, {0,7}, {0,8}, {0,9}, {0,10}, {1,4}, {1,5}, {1,6}, {1,7}, {1,8}, {1,9}, {1,10} },
            325, 200,
            VictoryCondition::EliminateAll, 0,
            false, -1, {}, false,
            "Au bout de l'escalier: une\n"
            "salle circulaire, et enfin\n"
            "un visage pour le \"Maitre\" -\n"
            "Malachar, necromant banni\n"
            "il y a des siecles, que l'on\n"
            "croyait mort avec le premier\n"
            "Trone Brise.",
            "At the bottom of the stairs:\n"
            "a circular chamber, and at\n"
            "last a face for the\n"
            "\"Master\" - Malachar, a\n"
            "necromancer banished\n"
            "centuries ago, long thought\n"
            "dead with the first Broken\n"
            "Throne.",
            "Malachar chancelle, blesse a\n"
            "mort - et disparait dans un\n"
            "eclat de flammes noires\n"
            "avant le coup final.\n"
            "Aubevent est sauve, pour\n"
            "cette generation. Mais vous\n"
            "savez desormais qu'il\n"
            "reviendra.",
            "Malachar staggers, mortally\n"
            "wounded - and vanishes in a\n"
            "burst of black flame before\n"
            "the final blow lands.\n"
            "Aubevent is saved, for this\n"
            "generation. But you now know\n"
            "he will return.",
            "act2_malachar"
        },

        // =====================================================================
        // Niveau 21 : Le Passage Interdit
        // =====================================================================
        LevelDef{
            "Le Passage Interdit", "The Forbidden Passage",
            24, 15, 5, 15,
            {
                { UnitType::Infantry, {18, 4} },
                { UnitType::Infantry, {18, 10} },
                { UnitType::Archer, {19, 6} },
                { UnitType::Archer, {19, 8} },
                { UnitType::Cavalry, {20, 7} },
                { UnitType::Griffon, {21, 7} },
            },
            { {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7} },
            350, 220,
            VictoryCondition::EliminateAll, 0,
            false, -1, {}, false,
            "Le sillage de flammes\n"
            "noires de Malachar traverse\n"
            "une frontiere que nul n'a\n"
            "franchie depuis la chute du\n"
            "premier Trone. Au-dela:\n"
            "les Terres Cendrees.\n"
            "Seuls les plus vaillants\n"
            "osent suivre.",
            "Malachar's trail of black\n"
            "flame crosses a border none\n"
            "has crossed since the fall\n"
            "of the first Throne. Beyond\n"
            "it: the Ashen Lands. Only\n"
            "the bravest dare follow.",
            "Le passage est force. Le\n"
            "ciel lui-meme semble plus\n"
            "sombre ici - comme si la\n"
            "lumiere refusait d'y\n"
            "entrer.",
            "The passage is forced. The\n"
            "sky itself seems darker\n"
            "here - as if light itself\n"
            "refused to enter.",
            "act3_passage"
        },

        // =====================================================================
        // Niveau 22 : Les Plaines Cendrees
        // =====================================================================
        LevelDef{
            "Les Plaines Cendrees", "The Ashen Plains",
            25, 16, 5, 10,
            {
                { UnitType::Cavalry, {18, 5} },
                { UnitType::Cavalry, {18, 11} },
                { UnitType::Cavalry, {19, 8} },
                { UnitType::Archer, {20, 6} },
                { UnitType::Archer, {20, 10} },
                { UnitType::Mage, {21, 8} },
                { UnitType::Griffon, {22, 7} },
            },
            { {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {0,8}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7}, {1,8} },
            370, 230,
            VictoryCondition::EliminateAll, 0,
            false, -1, {}, false,
            "A perte de vue, une plaine\n"
            "de cendres grises. La\n"
            "cavalerie de Malachar y\n"
            "regne sans partage,\n"
            "profitant du terrain\n"
            "degage.",
            "As far as the eye can see, a\n"
            "plain of grey ash. Malachar's\n"
            "cavalry rules unchallenged\n"
            "here, exploiting the open\n"
            "ground.",
            "La cavalerie est dispersee.\n"
            "Sur l'horizon, une foret\n"
            "petrifiee - des arbres\n"
            "changes en pierre en un\n"
            "seul instant, disent les\n"
            "legendes.",
            "The cavalry is scattered. On\n"
            "the horizon, a petrified\n"
            "forest - trees turned to\n"
            "stone in a single instant,\n"
            "the legends say.",
            nullptr
        },

        // =====================================================================
        // Niveau 23 : La Foret Petrifiee
        // =====================================================================
        LevelDef{
            "La Foret Petrifiee", "The Petrified Forest",
            25, 16, 55, 10,
            {
                { UnitType::Infantry, {18, 4} },
                { UnitType::Infantry, {18, 12} },
                { UnitType::Archer, {19, 6} },
                { UnitType::Archer, {19, 10} },
                { UnitType::Mage, {20, 8} },
                { UnitType::Mage, {21, 5} },
                { UnitType::Sapper, {21, 11} },
            },
            { {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {0,8}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7}, {1,8} },
            390, 240,
            VictoryCondition::EliminateAll, 0,
            false, -1, {}, false,
            "Les arbres de pierre\n"
            "etouffent la lumiere. Des\n"
            "mages de Malachar s'y\n"
            "cachent, invisibles jusqu'a\n"
            "ce qu'ils frappent.",
            "The stone trees choke out\n"
            "the light. Malachar's mages\n"
            "hide among them, unseen\n"
            "until they strike.",
            "La foret redevient\n"
            "silencieuse. Plus loin,\n"
            "le bruit de l'eau: une\n"
            "riviere noire, et un pont\n"
            "d'ossements qui la\n"
            "traverse.",
            "The forest falls silent\n"
            "again. Further on, the\n"
            "sound of water: a black\n"
            "river, and a bridge of bones\n"
            "crossing it.",
            nullptr
        },

        // =====================================================================
        // Niveau 24 : Le Pont des Ames
        // =====================================================================
        LevelDef{
            "Le Pont des Ames", "The Bridge of Souls",
            24, 15, 15, 10,
            {
                { UnitType::Infantry, {18, 4} },
                { UnitType::Infantry, {18, 10} },
                { UnitType::Archer, {19, 6} },
                { UnitType::Archer, {19, 8} },
                { UnitType::Mage, {20, 7} },
                { UnitType::SiegeEngine, {20, 5} },
                { UnitType::Hero, {21, 9} },
            },
            { {0,2}, {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {1,2}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7} },
            410, 250,
            VictoryCondition::EliminateAll, 0,
            true, 13, {7, 8}, false,
            "Un pont d'ossements est le\n"
            "seul passage sur la riviere\n"
            "noire. Un des capitaines de\n"
            "Malachar le tient, certain\n"
            "que nul n'osera le\n"
            "traverser.",
            "A bridge of bones is the\n"
            "only crossing over the black\n"
            "river. One of Malachar's\n"
            "captains holds it, certain\n"
            "none will dare cross.",
            "Le pont est a vous. Sur\n"
            "l'autre rive se dresse enfin\n"
            "la Garnison de l'Ombre -\n"
            "porte des dernieres\n"
            "defenses de Malachar.",
            "The bridge is yours. On the\n"
            "far bank stands at last the\n"
            "Shadow Garrison - gateway to\n"
            "Malachar's final defenses.",
            nullptr
        },

        // =====================================================================
        // Niveau 25 : La Garnison de l'Ombre
        // =====================================================================
        LevelDef{
            "La Garnison de l'Ombre", "The Shadow Garrison",
            25, 16, 15, 25,
            {
                { UnitType::Infantry, {18, 4} },
                { UnitType::Infantry, {18, 12} },
                { UnitType::Cavalry, {19, 6} },
                { UnitType::Archer, {19, 10} },
                { UnitType::Archer, {20, 8} },
                { UnitType::Mage, {21, 7} },
                { UnitType::Griffon, {21, 9} },
                { UnitType::Griffon, {22, 5} },
                { UnitType::SiegeEngine, {22, 11} },
            },
            { {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {0,8}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7}, {1,8} },
            430, 260,
            VictoryCondition::EliminateAll, 0,
            false, -1, {}, true,
            "Des griffons montes patrouil-\n"
            "lent au-dessus des remparts\n"
            "de la Garnison de l'Ombre.\n"
            "Rien de ce qui approche au\n"
            "sol ne leur echappe.",
            "Mounted griffons patrol\n"
            "above the Shadow Garrison's\n"
            "walls. Nothing approaching\n"
            "on the ground escapes their\n"
            "sight.",
            "Les griffons s'abattent. Au-\n"
            "dela des remparts en ruine,\n"
            "une citadelle s'eleve a\n"
            "flanc de montagne - le coeur\n"
            "du domaine de Malachar.",
            "The griffons fall. Beyond\n"
            "the ruined walls, a citadel\n"
            "rises against the\n"
            "mountainside - the heart of\n"
            "Malachar's domain.",
            nullptr
        },

        // =====================================================================
        // Niveau 26 : L'Ascension de la Citadelle
        // =====================================================================
        LevelDef{
            "L'Ascension de la Citadelle", "Ascent of the Citadel",
            26, 17, 10, 50,
            {
                { UnitType::Infantry, {19, 4} },
                { UnitType::Infantry, {19, 12} },
                { UnitType::Archer, {20, 6} },
                { UnitType::Archer, {20, 10} },
                { UnitType::Cavalry, {21, 8} },
                { UnitType::SiegeEngine, {22, 5} },
                { UnitType::SiegeEngine, {22, 11} },
                { UnitType::Hero, {23, 8} },
            },
            { {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {0,8}, {0,9}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7}, {1,8}, {1,9} },
            450, 270,
            VictoryCondition::EliminateAll, 0,
            false, -1, {}, false,
            "La route grimpe a flanc de\n"
            "montagne. Des catapultes\n"
            "ennemies pilonnent le chemin\n"
            "depuis les hauteurs -\n"
            "avancer vite est aussi\n"
            "dangereux que reculer.",
            "The road climbs the\n"
            "mountainside. Enemy\n"
            "catapults pound the path\n"
            "from the heights - advancing\n"
            "fast is as dangerous as\n"
            "retreating.",
            "Les catapultes sont\n"
            "reduites au silence. Au\n"
            "sommet, une garde d'honneur\n"
            "immobile attend - les\n"
            "Gardiens Eternels de\n"
            "Malachar.",
            "The catapults fall silent.\n"
            "At the summit, a motionless\n"
            "honor guard awaits -\n"
            "Malachar's Eternal\n"
            "Guardians.",
            nullptr
        },

        // =====================================================================
        // Niveau 27 : Les Gardiens Eternels
        // =====================================================================
        LevelDef{
            "Les Gardiens Eternels", "The Eternal Guardians",
            26, 17, 10, 20,
            {
                { UnitType::Infantry, {19, 4} },
                { UnitType::Infantry, {19, 12} },
                { UnitType::Archer, {20, 6} },
                { UnitType::Archer, {20, 10} },
                { UnitType::Mage, {21, 8} },
                { UnitType::Hero, {22, 6} },
                { UnitType::Hero, {22, 10} },
                { UnitType::Griffon, {23, 8} },
            },
            { {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {0,8}, {0,9}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7}, {1,8}, {1,9} },
            470, 280,
            VictoryCondition::EliminateAll, 0,
            false, -1, {}, false,
            "Deux capitaines d'elite,\n"
            "lies a Malachar par un\n"
            "serment ancien, gardent\n"
            "l'antichambre du trone. Ils\n"
            "ne connaissent ni la peur\n"
            "ni la fatigue.",
            "Two elite captains, bound to\n"
            "Malachar by an ancient oath,\n"
            "guard the throne's\n"
            "antechamber. They know\n"
            "neither fear nor fatigue.",
            "Le serment se brise avec\n"
            "eux. Le chemin vers\n"
            "l'antichambre du trone\n"
            "est enfin ouvert.",
            "The oath breaks with them.\n"
            "The way to the throne's\n"
            "antechamber lies open at\n"
            "last.",
            nullptr
        },

        // =====================================================================
        // Niveau 28 : La Faille des Cendres
        // =====================================================================
        LevelDef{
            "La Faille des Cendres", "The Ashen Rift",
            27, 17, 10, 15,
            {
                { UnitType::Infantry, {20, 4} },
                { UnitType::Infantry, {20, 12} },
                { UnitType::Cavalry, {21, 6} },
                { UnitType::Cavalry, {21, 10} },
                { UnitType::Archer, {22, 7} },
                { UnitType::Archer, {22, 9} },
                { UnitType::Mage, {23, 8} },
                { UnitType::SiegeEngine, {23, 5} },
                { UnitType::Griffon, {24, 11} },
                { UnitType::Hero, {24, 5} },
            },
            { {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {0,8}, {0,9}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7}, {1,8}, {1,9} },
            500, 300,
            VictoryCondition::EliminateAll, 0,
            false, -1, {}, false,
            "Une faille beante separe la\n"
            "citadelle du reste du monde.\n"
            "Malachar y a jete le plus\n"
            "gros de ce qui lui reste -\n"
            "tout, plutot que de vous\n"
            "laisser approcher.",
            "A gaping rift separates the\n"
            "citadel from the rest of the\n"
            "world. Malachar has thrown\n"
            "everything he has left into\n"
            "it - anything to keep you\n"
            "from approaching.",
            "La faille est franchie, au\n"
            "prix fort. L'antichambre du\n"
            "trone n'est plus qu'a\n"
            "quelques pas.",
            "The rift is crossed, at\n"
            "great cost. The throne's\n"
            "antechamber is only steps\n"
            "away.",
            nullptr
        },

        // =====================================================================
        // Niveau 29 : L'Antichambre du Trone
        // =====================================================================
        LevelDef{
            "L'Antichambre du Trone", "The Throne's Antechamber",
            27, 17, 10, 30,
            {
                { UnitType::Infantry, {20, 4} },
                { UnitType::Infantry, {20, 12} },
                { UnitType::Cavalry, {21, 6} },
                { UnitType::Archer, {21, 10} },
                { UnitType::Archer, {22, 8} },
                { UnitType::Mage, {23, 6} },
                { UnitType::Mage, {23, 10} },
                { UnitType::SiegeEngine, {23, 8} },
                { UnitType::Hero, {24, 6} },
                { UnitType::Hero, {24, 10} },
            },
            { {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {0,8}, {0,9}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7}, {1,8}, {1,9} },
            530, 320,
            VictoryCondition::CaptureCastle, 0,
            false, -1, {}, true,
            "Derniere ligne avant le\n"
            "trone. Malachar a rappele\n"
            "ici tout ce qui lui restait\n"
            "de fidele. Au-dela de ces\n"
            "portes, plus rien ne le\n"
            "protegera.",
            "The last line before the\n"
            "throne. Malachar has\n"
            "recalled here everything\n"
            "still loyal to him. Beyond\n"
            "these doors, nothing more\n"
            "will protect him.",
            "Les portes cedent. Un\n"
            "silence pesant regne dans\n"
            "la salle du trone. Malachar\n"
            "vous attend, seul,\n"
            "immobile.",
            "The doors give way. A heavy\n"
            "silence fills the throne\n"
            "room. Malachar awaits you,\n"
            "alone, unmoving.",
            "act3_antechamber"
        },

        // =====================================================================
        // Niveau 30 : Le Trone des Cendres
        // =====================================================================
        LevelDef{
            "Le Trone des Cendres", "The Throne of Ashes",
            28, 18, 10, 20,
            {
                { UnitType::Infantry, {21, 4} },
                { UnitType::Infantry, {21, 14} },
                { UnitType::Cavalry, {22, 6} },
                { UnitType::Cavalry, {22, 12} },
                { UnitType::Archer, {23, 8} },
                { UnitType::Archer, {23, 10} },
                { UnitType::Mage, {24, 7} },
                { UnitType::Mage, {24, 11} },
                { UnitType::SiegeEngine, {24, 9} },
                { UnitType::Griffon, {25, 5} },
                { UnitType::Griffon, {25, 13} },
                { UnitType::Hero, {26, 8} },
                { UnitType::Hero, {26, 10} },
            },
            { {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {0,8}, {0,9}, {0,10}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7}, {1,8}, {1,9}, {1,10} },
            600, 350,
            VictoryCondition::EliminateAll, 0,
            false, -1, {}, false,
            "Malachar se dresse enfin\n"
            "devant vous, sur un trone\n"
            "d'os et de cendres - le\n"
            "premier, le vrai, dont\n"
            "celui d'Aubevent n'etait\n"
            "qu'un pale echo. Sa garde\n"
            "est la plus puissante que\n"
            "vous ayez jamais affrontee.",
            "Malachar stands before you\n"
            "at last, upon a throne of\n"
            "bone and ash - the first,\n"
            "the true one, of which\n"
            "Aubevent's was only a pale\n"
            "echo. His guard is the most\n"
            "powerful you have ever\n"
            "faced.",
            "Malachar tombe pour de bon\n"
            "cette fois, et son trone\n"
            "d'os s'effondre en poussiere.\n"
            "Aux confins des Terres\n"
            "Cendrees, la lumiere du jour\n"
            "se leve pour la premiere\n"
            "fois depuis des siecles.\n"
            "Aubevent - et le monde -\n"
            "est enfin libre. Vous avez\n"
            "ecrit une legende que nul\n"
            "\"Maitre\" ne reecrira jamais.",
            "Malachar falls for good this\n"
            "time, and his throne of bone\n"
            "crumbles to dust. At the edge\n"
            "of the Ashen Lands, daylight\n"
            "rises for the first time in\n"
            "centuries. Aubevent - and the\n"
            "world - is finally free. You\n"
            "have written a legend that no\n"
            "\"Master\" will ever rewrite\n"
            "again.",
            "act3_finale"
        },
    };
    return table;
}

// ---------------------------------------------------------
// Fonctions utilitaires
// ---------------------------------------------------------
const char* level_name(const LevelDef& def) {
    return (language_get() == Language::FR) ? def.name_fr : def.name_en;
}
const char* level_briefing(const LevelDef& def) {
    return (language_get() == Language::FR) ? def.briefing_fr : def.briefing_en;
}
const char* level_victory_text(const LevelDef& def) {
    return (language_get() == Language::FR) ? def.victory_text_fr : def.victory_text_en;
}

// Le tableau complet fait 30 cartes (cf. levels_table()), mais les 10
// dernières (l'arc "Hard" et sa fin heroique) ne sont visibles/jouables
// qu'en difficulte Difficile -> level_count() plafonne a 20 sinon.
static constexpr int kNormalCampaignLength = 20;

int level_count() {
    int total = (int)levels_table().size();
    if (difficulty_get() == Difficulty::Hard) return total;
    return (kNormalCampaignLength < total) ? kNormalCampaignLength : total;
}

const LevelDef& level_get(int index) {
    const auto& t = levels_table();
    if (index < 0) index = 0;
    if (index >= (int)t.size()) index = (int)t.size() - 1;
    return t[index];
}

// ---------------------------------------------------------
// level_build_map()
// ---------------------------------------------------------
// Construit la carte :
//   1) terrain de base (foret / colline selon seeds)
//   2) route horizontale centrale
//   3) riviere + ponts (si river_crossing)
//   4) chateau ennemi (objectif)
//   5) remparts autour du chateau (si castle_walls)
//   6) instanciation des unites ennemies
// ---------------------------------------------------------
void level_build_map(const LevelDef& def, Map& out_map) {
    out_map.tiles.clear();
    out_map.units.clear();
    out_map.width = def.width;
    out_map.height = def.height;

    // 1) Terrain de base
    for (int q = 0; q < def.width; q++) {
        for (int r = 0; r < def.height; r++) {
            Terrain t = Terrain::Plain;
            uint32_t h1 = hash2(q, r, def.forest_seed) % 100;
            uint32_t h2 = hash2(q, r, def.hill_seed + 999) % 100;
            if ((int)h1 < def.forest_seed) t = Terrain::Forest;
            else if ((int)h2 < def.hill_seed) t = Terrain::Hill;

            // 2) route horizontale centrale
            if (r == def.height / 2) t = Terrain::Road;

            out_map.set_tile({q, r}, t);
        }
    }

    // 3) Riviere + ponts (specialite "franchissement")
    if (def.river_crossing && def.river_col >= 0 && def.river_col < def.width) {
        for (int r = 0; r < def.height; r++) {
            bool is_bridge = false;
            for (int br : def.bridge_rows)
                if (br == r) { is_bridge = true; break; }
            out_map.set_tile({def.river_col, r}, is_bridge ? Terrain::Bridge : Terrain::Water);
        }
    }

    // 4) Chateau ennemi (objectif)
    Hex castle_pos{ def.width - 1, def.height / 2 };
    out_map.set_tile(castle_pos, Terrain::Castle);

    // 5) Remparts autour du chateau (specialite "siege de chateau")
    if (def.castle_walls) {
        for (const auto& n : hex_neighbors(castle_pos)) {
            if (!out_map.in_bounds(n)) continue;
            out_map.set_tile(n, Terrain::Wall);
        }
    }

    // 6) Placement des unites ennemies
    uint32_t spawn_id = 1000;
    for (const auto& e : def.enemies) {
        Unit u;
        u.type = e.type;
        u.faction = Faction::Enemy;
        u.pos = e.pos;
        u.hp = unit_stats(e.type).hp_max;
        u.moves_left = unit_stats(e.type).move_range;
        u.action_used = false;
        u.alive = true;
        u.id = spawn_id++;
        out_map.units.push_back(u);
    }
}

} // namespace wg
