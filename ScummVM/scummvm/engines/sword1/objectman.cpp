/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */


#include "common/textconsole.h"
#include "common/util.h"

#include "sword1/objectman.h"
#include "sword1/sworddefs.h"
#include "sword1/swordres.h"
#include "sword1/sword1.h"

namespace Sword1 {

ObjectMan::ObjectMan(ResMan *pResourceMan) {
	_resMan = pResourceMan;
}

void ObjectMan::initialize() {
	uint16 cnt;
	for (cnt = 0; cnt < TOTAL_SECTIONS; cnt++)
		_liveList[cnt] = 0; // we don't need to close the files here. When this routine is
	                        // called, the memory was flushed() anyways, so these resources
	                        // already *are* closed.

	_liveList[128] = _liveList[129] = _liveList[130] = _liveList[131] = _liveList[133] =
	                                      _liveList[134] = _liveList[145] = _liveList[146] = _liveList[TEXT_sect] = 1;

	for (cnt = 0; cnt < TOTAL_SECTIONS; cnt++) {
		if (_liveList[cnt])
			_cptData[cnt] = (uint8 *)_resMan->cptResOpen(_objectList[cnt]) + sizeof(Header);
		else
			_cptData[cnt] = NULL;
	}
}

ObjectMan::~ObjectMan() {
	for (uint16 cnt = 0; cnt < TOTAL_SECTIONS; cnt++)
		if (_liveList[cnt])
			_resMan->resClose(_objectList[cnt]);
}

bool ObjectMan::sectionAlive(uint16 section) {
	return (_liveList[section] > 0);
}

void ObjectMan::megaEntering(uint16 section) {
	_liveList[section]++;
	if (_liveList[section] == 1)
		_cptData[section] = ((uint8 *)_resMan->cptResOpen(_objectList[section])) + sizeof(Header);
}

void ObjectMan::megaLeaving(uint16 section, int id) {
	if (_liveList[section] == 0)
		error("mega %d is leaving empty section %d", id, section);
	_liveList[section]--;
	if ((_liveList[section] == 0) && (id != PLAYER)) {
		_resMan->resClose(_objectList[section]);
		_cptData[section] = NULL;
	}
	/* if the player is leaving the section then we have to close the resources after
	   mainloop ends, because the screen will still need the resources*/
}

uint8 ObjectMan::fnCheckForTextLine(uint32 textId) {
	uint8 retVal = 0;
	if (!_textList[textId / ITM_PER_SEC][0])
		return 0; // section does not exist

	uint8 lang = SwordEngine::_systemVars.language;
	uint32 *textData = (uint32 *)((uint8 *)_resMan->openFetchRes(_textList[textId / ITM_PER_SEC][lang]) + sizeof(Header));
	if ((textId & ITM_ID) < _resMan->readUint32(textData)) {
		textData++;
		if (textData[textId & ITM_ID])
			retVal = 1;
	}
	_resMan->resClose(_textList[textId / ITM_PER_SEC][lang]);
	return retVal;
}

char *ObjectMan::lockText(uint32 textId) {
	uint8 lang = SwordEngine::_systemVars.language;
	char *text = lockText(textId, lang);
	if (text == 0) {
		if (lang != BS1_ENGLISH) {
			text = lockText(textId, BS1_ENGLISH);
			if (text != 0)
				warning("Missing translation for textId %u (\"%s\")", textId, text);
			unlockText(textId, BS1_ENGLISH);
		}

		return _missingSubTitleStr;
	}
	return text;
}

char *ObjectMan::lockText(uint32 textId, uint8 lang) {
	char *addr = (char *)_resMan->openFetchRes(_textList[textId / ITM_PER_SEC][lang]);
	if (addr == 0)
		return NULL;
	addr += sizeof(Header);
	if ((textId & ITM_ID) >= _resMan->readUint32(addr)) {
		// Workaround for missing sentences in some langages in the demo.
		switch(textId) {
		case 8455194:
			return const_cast<char *>(_translationId8455194[lang]);
		case 8455195:
			return const_cast<char *>(_translationId8455195[lang]);
		case 8455196:
			return const_cast<char *>(_translationId8455196[lang]);
		case 8455197:
			return const_cast<char *>(_translationId8455197[lang]);
		case 8455198:
			return const_cast<char *>(_translationId8455198[lang]);
		case 8455199:
			return const_cast<char *>(_translationId8455199[lang]);
		case 8455200:
			return const_cast<char *>(_translationId8455200[lang]);
		case 8455201:
			return const_cast<char *>(_translationId8455201[lang]);
		case 8455202:
			return const_cast<char *>(_translationId8455202[lang]);
		case 8455203:
			return const_cast<char *>(_translationId8455203[lang]);
		case 8455204:
			return const_cast<char *>(_translationId8455204[lang]);
		case 8455205:
			return const_cast<char *>(_translationId8455205[lang]);
		case 6488080:
			return const_cast<char *>(_translationId6488080[lang]);
		case 6488081:
			return const_cast<char *>(_translationId6488081[lang]);
		case 6488082:
			return const_cast<char *>(_translationId6488082[lang]);
		case 6488083:
			return const_cast<char *>(_translationId6488083[lang]);
		}

		warning("ObjectMan::lockText(%d): only %d texts in file", textId & ITM_ID, _resMan->readUint32(addr));
		return NULL;
	}
	uint32 offset = _resMan->readUint32(addr + ((textId & ITM_ID) + 1) * 4);
	if (offset == 0) {
		// Workaround bug for missing sentence in some langages in Syria (see bug #1977094).
		// We use the hardcoded text in this case.
		if (textId == 2950145)
			return const_cast<char *>(_translationId2950145[lang]);

		warning("ObjectMan::lockText(%d): text number has no text lines", textId);
		return NULL;
	}
	return addr + offset;
}

void ObjectMan::unlockText(uint32 textId) {
	unlockText(textId, SwordEngine::_systemVars.language);
}

void ObjectMan::unlockText(uint32 textId, uint8 lang) {
	_resMan->resClose(_textList[textId / ITM_PER_SEC][lang]);
}

uint32 ObjectMan::lastTextNumber(int section) {
	uint8 *data = (uint8 *)_resMan->openFetchRes(_textList[section][SwordEngine::_systemVars.language]) + sizeof(Header);
	uint32 result = _resMan->readUint32(data) - 1;
	_resMan->resClose(_textList[section][SwordEngine::_systemVars.language]);
	return result;
}

Object *ObjectMan::fetchObject(uint32 id) {
	uint8 *addr = _cptData[id / ITM_PER_SEC];
	if (!addr)
		error("fetchObject: section %d is not open", id / ITM_PER_SEC);
	id &= ITM_ID;
	// DON'T do endian conversion here. it's already done.
	return (Object *)(addr + * (uint32 *)(addr + (id + 1) * 4));
}

uint32 ObjectMan::fetchNoObjects(int section) {
	if (_cptData[section] == NULL)
		error("fetchNoObjects: section %d is not open", section);
	return *(uint32 *)_cptData[section];
}

void ObjectMan::closeSection(uint32 screen) {
	if (_liveList[screen] == 0)  // close the section that PLAYER has just left, if it's empty now
		_resMan->resClose(_objectList[screen]);
}

void ObjectMan::loadLiveList(uint16 *src) {
	for (uint16 cnt = 0; cnt < TOTAL_SECTIONS; cnt++) {
		if (_liveList[cnt]) {
			_resMan->resClose(_objectList[cnt]);
			_cptData[cnt] = NULL;
		}
		_liveList[cnt] = src[cnt];
		if (_liveList[cnt])
			_cptData[cnt] = ((uint8 *)_resMan->cptResOpen(_objectList[cnt])) + sizeof(Header);
	}
}

void ObjectMan::saveLiveList(uint16 *dest) {
	memcpy(dest, _liveList, TOTAL_SECTIONS * sizeof(uint16));
}

// String displayed when a subtitle sentence is missing in the cluster file.
// It happens with at least one sentence in Syria in some langages (see bug
// #1977094).
// Note: an empty string or a null pointer causes a crash.

char ObjectMan::_missingSubTitleStr[] = " ";

// Missing translation for textId 2950145 (see bug #1977094).
// Currently text is missing for Portuguese languages. (It's possible that it
// is not needed. The English version of the game does not include Portuguese
// so I cannot check.)

const char *const ObjectMan::_translationId2950145[7] = {
	"Oh?",     // English (not needed)
	"Quoi?",   // French
	"Oh?",     // German
	"Eh?",     // Italian
	"\277Eh?", // Spanish
	"Ano?",    // Czech
	NULL       // Portuguese
};

// The translations for the next texts are missing in the demo but are present
// in the full game. The translations were therefore extracted from the full game.

// Missing translation for textId 8455194 (in the demo).
const char *const ObjectMan::_translationId8455194[7] = {
	NULL, // "Who was the guy you were supposed to meet?",              // English (not needed)
	"Qui \351tait l'homme que vous deviez rencontrer?",                 // French
	"Wer war der Typ, den Du treffen wolltest?",                        // German
	"Chi dovevi incontrare?",                                           // Italian
	"\277Qui\351n era el hombre con el que ten\355as que encontrarte?", // Spanish
	NULL,                                                               // Czech
	NULL                                                                // Portuguese
};

// Missing translation for textId 8455195 (in the demo).
const char *const ObjectMan::_translationId8455195[7] = {
	NULL, // "His name was Plantard. I didn't know him, but he called me last night.",                    // English (not needed)
	"Son nom \351tait Plantard. Je ne le connaissais pas, mais il m'avait t\351l\351phon\351 la veille.", // French
	"Sein Name war Plantard. Ich kannte ihn nicht, aber er hat mich letzte Nacht angerufen.",             // German
	"Si chiamava Plantard. Mi ha chiamato ieri sera, ma non lo conoscevo.",                               // Italian
	"Su nombre era Plantard. Yo no lo conoc\355a pero \351l me llam\363 ayer por la noche.",              // Spanish
	NULL,                                                                                                 // Czech
	NULL                                                                                                  // Portuguese
};

// Missing translation for textId 8455196 (in the demo).
const char *const ObjectMan::_translationId8455196[7] = {
	NULL, // "He said he had a story which would interest me.",           // English (not needed)
	"Il a dit qu'il avait une histoire qui devrait m'int\351resser.",     // French
	"Er sagte, er h\344tte eine Story, die mich interessieren w\374rde.", // German
	"Mi disse che aveva una storia che mi poteva interessare.",           // Italian
	"Dijo que ten\355a una historia que me interesar\355a.",              // Spanish
	NULL,                                                                 // Czech
	NULL                                                                  // Portuguese
};

// Missing translation for textId 8455197 (in the demo).
const char *const ObjectMan::_translationId8455197[7] = {
	NULL, // "He asked me to meet him at the caf\351.",          // English (not needed)
	"Il m'a demand\351 de le rencontrer au caf\351.",            // French
	"Er fragte mich, ob wir uns im Caf\351 treffen k\366nnten.", // German
	"Mi chiese di incontrarci al bar.",                          // Italian
	"Me pidi\363 que nos encontr\341ramos en el caf\351.",       // Spanish
	NULL,                                                        // Czech
	NULL                                                         // Portuguese
};

// Missing translation for textId 8455198 (in the demo).
const char *const ObjectMan::_translationId8455198[7] = {
	NULL, // "I guess I'll never know what he wanted to tell me...",  // English (not needed)
	"Je suppose que je ne saurai jamais ce qu'il voulait me dire...", // French
	"Ich werde wohl nie erfahren, was er mir sagen wollte...",        // German
	"Penso che non sapr\362 mai che cosa voleva dirmi...",            // Italian
	"Supongo que nunca sabr\351 qu\351 me quer\355a contar...",       // Spanish
	NULL,                                                             // Czech
	NULL                                                              // Portuguese
};

// Missing translation for textId 8455199 (in the demo).
const char *const ObjectMan::_translationId8455199[7] = {
	NULL, // "Not unless you have Rosso's gift for psychic interrogation.",           // English (not needed)
	"Non, \340 moins d'avoir les dons de Rosso pour les interrogatoires psychiques.", // French
	"Es sei denn, Du h\344ttest Rosso's Gabe der parapsychologischen Befragung.",     // German
	"A meno che tu non riesca a fare interrogatori telepatici come Rosso.",           // Italian
	"A no ser que tengas el don de Rosso para la interrogaci\363n ps\355quica.",      // Spanish
	NULL,                                                                             // Czech
	NULL                                                                              // Portuguese
};

// Missing translation for textId 8455200 (in the demo).
const char *const ObjectMan::_translationId8455200[7] = {
	NULL, // "How did Plantard get your name?",     // English (not needed)
	"Comment Plantard a-t-il obtenu votre nom?",    // French
	"Woher hat Plantard Deinen Namen?",             // German
	"Come ha fatto Plantard a sapere il tuo nome?", // Italian
	"\277C\363mo consigui\363 Plantard tu nombre?", // Spanish
	NULL,                                           // Czech
	NULL                                            // Portuguese
};

// Missing translation for textId 8455201 (in the demo).
const char *const ObjectMan::_translationId8455201[7] = {
	NULL, // "Through the newspaper - La Libert\351.", // English (not needed)
	"Par mon journal... La Libert\351.",      // French
	"\334ber die Zeitung - La Libert\351.",   // German
	"Tramite il giornale La Libert\351.",     // Italian
	"Por el peri\363dico - La Libert\351.",   // Spanish
	NULL,                                     // Czech
	NULL                                      // Portuguese
};

// Missing translation for textId 8455202 (in the demo).
const char *const ObjectMan::_translationId8455202[7] = {
	NULL, // "I'd written an article linking two unsolved murders, one in Italy, the other in Japan.",                                                    // English (not needed)
	"J'ai \351crit un article o\371 je faisais le lien entre deux meurtres inexpliqu\351s, en Italie et au japon.",                                       // French
	"Ich habe einen Artikel geschrieben, in dem ich zwei ungel\366ste Morde miteinander in Verbindung bringe, einen in Italien, einen anderen in Japan.", // German
	"Ho scritto un articolo che metteva in collegamento due omicidi insoluti in Italia e Giappone.",                                                      // Italian
	"Yo hab\355a escrito un art\355culo conectando dos asesinatos sin resolver, uno en Italia, el otro en Jap\363n.",                                     // Spanish
	NULL,                                                                                                                                                 // Czech
	NULL                                                                                                                                                  // Portuguese
};

// Missing translation for textId 8455203 (in the demo).
const char *const ObjectMan::_translationId8455203[7] = {
	NULL, // "The cases were remarkably similar...",      // English (not needed)
	"Les affaires \351taient quasiment identiques...",    // French
	"Die F\344lle sind sich bemerkenswert \344hnlich...", // German
	"I casi erano sorprendentemente uguali...",           // Italian
	"Los casos eran incre\355blemente parecidos...",      // Spanish
	NULL,                                                 // Czech
	NULL                                                  // Portuguese
};

// Missing translation for textId 8455204 (in the demo).
const char *const ObjectMan::_translationId8455204[7] = {
	NULL, // "...a wealthy victim, no apparent motive, and a costumed killer.",              // English (not needed)
	"...une victime riche, pas de motif apparent, et un tueur d\351guis\351.",              // French
	"...ein wohlhabendes Opfer, kein offensichtliches Motiv, und ein verkleideter Killer.", // German
	"...una vittima ricca, nessun motivo apparente e un assassino in costume.",             // Italian
	"...una v\355ctima rica, sin motivo aparente, y un asesino disfrazado.",                // Spanish
	NULL,                                                                                   // Czech
	NULL                                                                                    // Portuguese
};

// Missing translation for textId 8455205 (in the demo).
const char *const ObjectMan::_translationId8455205[7] = {
	NULL, // "Plantard said he could supply me with more information.",        // English (not needed)
	"Plantard m'a dit qu'il pourrait me fournir des renseignements.",          // French
	"Plantard sagte, er k\366nne mir weitere Informationen beschaffen.",       // German
	"Plantard mi disse che mi avrebbe fornito ulteriori informazioni.",        // Italian
	"Plantard dijo que \351l me pod\355a proporcionar m\341s informaci\363n.", // Spanish
	NULL,                                                                      // Czech
	NULL                                                                       // Portuguese
};

// Missing translation for textId 6488080 (in the demo).
const char *const ObjectMan::_translationId6488080[7] = {
	NULL, // "I wasn't going to head off all over Paris until I'd investigated some more.", // English (not needed)
	"Je ferais mieux d'enqu\351ter un peu par ici avant d'aller me promener ailleurs.",     // French
	"Ich durchquere nicht ganz Paris, bevor ich etwas mehr herausgefunden habe.",           // German
	"Non mi sarei incamminato per tutta Parigi prima di ulteriori indagini.",               // Italian
	"No iba a ponerme a recorrer Par\355s sin haber investigado un poco m\341s.",           // Spanish
	NULL,                                                                                   // Czech
	NULL                                                                                    // Portuguese
};

// The next three sentences are specific to the demo and only the english text is present.
// The translations were provided by:
// French: Thierry Crozat
// German: Simon Sawatzki
// Italian: Matteo Angelino
// Spanish: Tomás Maidagan

// Missing translation for textId 6488081 (in the demo).
const char *const ObjectMan::_translationId6488081[7] = {
	NULL, // "I wasn't sure what I was going to do when I caught up with that clown...", // English (not needed)
	"Je ne savais pas ce que je ferais quand je rattraperais le clown...",               // French
	"Ich wu\337te nicht, worauf ich mich einlie\337, als ich dem Clown nachjagte...",    // German
	"Non sapevo cosa avrei fatto una volta raggiunto quel clown...",                     // Italian
	"No sab\355a muy bien qu\351 es lo que har\355a cuando alcanzara al payaso...",      // Spanish
	NULL,                                                                                // Czech
	NULL                                                                                 // Portuguese
};

// Missing translation for textId 6488082 (in the demo).
const char *const ObjectMan::_translationId6488082[7] = {
	NULL, // "...but before I knew it, I was drawn into a desperate race between two ruthless enemies.",                             // English (not needed)
	"...mais avant de m'en rendre compte je me retrouvais happ\351 dans une course effr\351n\351e entre deux ennemis impitoyables.", // French
	"... doch bevor ich mich versah, war ich inmitten eines Wettlaufs von zwei r\374cksichtslosen Feinden.",                         // German
	"... ma prima che me ne rendessi conto, fui trascinato in una corsa disperata con due spietati nemici.",                         // Italian
	"...pero sin darme cuenta, acab\351 en medio de una desesperada carrera entre dos despiadados enemigos.",                        // Spanish
	NULL,                                                                                                                            // Czech
	NULL                                                                                                                             // Portuguese
};

// Missing translation for textId 6488083 (in the demo).
const char *const ObjectMan::_translationId6488083[7] = {
	NULL, // "The goal: the mysterious power of the Broken Sword.",    // English (not needed)
	"Le but: les pouvoirs myst\351rieux de l'\351p\351e bris\351e.",   // French
	"Das Ziel: die geheimnisvolle Macht des zerbrochenen Schwertes.",  // German
	"Obiettivo: il misterioso potere della Spada spezzata.",           // Italian
	"El objetivo: el misterioso poder de la Espada Rota.",             // Spanish
	NULL,                                                              // Czech
	NULL                                                               // Portuguese
};

} // End of namespace Sword1
