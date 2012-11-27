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

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

/*
    BS_RenderObject
    ---------------
    Dieses ist die Klasse die s�mtliche sichtbaren Objekte beschreibt. Alle anderen sichtbaren Objekte m�ssen von ihr abgeleitet werden.
    Diese Klasse erledigt Aufgaben wie: minimales Neuzeichnen, Renderreihenfolge, Objekthierachie.
    Alle BS_RenderObject Instanzen werden von einem BS_RenderObjectManager in einem Baum verwaltet.

    Autor: Malte Thiesen
*/

#ifndef SWORD25_RENDEROBJECT_H
#define SWORD25_RENDEROBJECT_H

#include "sword25/kernel/common.h"
#include "sword25/kernel/persistable.h"
#include "common/rect.h"
#include "sword25/gfx/renderobjectptr.h"

#include "common/list.h"

namespace Sword25 {

class Kernel;
class RenderObjectManager;
class Bitmap;
class Animation;
class AnimationTemplate;
class Panel;
class Text;

// Klassendefinition
/**
    @brief  Dieses ist die Klasse die s�mtliche sichtbaren Objekte beschreibt.

    Alle anderen sichtbaren Objekte m�ssen von ihr abgeleitet werden.
    Diese Klasse erledigt Aufgaben wie: minimales Neuzeichnen, Renderreihenfolge, Objekthierachie.
    Alle BS_RenderObject Instanzen werden von einem BS_RenderObjektManager in einem Baum verwaltet.
 */
class RenderObject {
public:
	// Konstanten
	// ----------
	enum TYPES {
		/// Das Wurzelobjekt. Siehe BS_RenderObjectManager
		TYPE_ROOT,
		/// Ein Image. Siehe BS_Bitmap.
		TYPE_STATICBITMAP,
		TYPE_DYNAMICBITMAP,
		/// Eine Animation. Siehe BS_Animation.
		TYPE_ANIMATION,
		/// Eine farbige Fl�che. Siehe BS_Panel.
		TYPE_PANEL,
		/// Ein Text. Siehe BS_Text.
		TYPE_TEXT,
		/// Ein unbekannter Objekttyp. Diesen Typ sollte kein Renderobjekt annehmen.
		TYPE_UNKNOWN
	};

	// Add-Methoden
	// ------------

	/**
	    @brief Erzeugt ein Bitmap als Kinderobjekt des Renderobjektes.
	    @param FileName der Dateiname der Quellbilddatei
	    @return Gibt einen BS_RenderObjectPtr auf das erzeugte Objekt zur�ck.<br>
	            Falls ein Fehler aufgetreten ist wird ein ung�ltiger BS_RenderObjectPtr zur�ckgegeben.
	*/
	RenderObjectPtr<Bitmap> addBitmap(const Common::String &fileName);
	/**
	    @brief Erzeugt ein ver�nderbares Bitmap als Kinderobjekt des Renderobjektes.
	    @param Width die Breite des Bitmaps
	    @param Height die H�he des Bitmaps
	    @return Gibt einen BS_RenderObjectPtr auf das erzeugte Objekt zur�ck.<br>
	            Falls ein Fehler aufgetreten ist wird ein ung�ltiger BS_RenderObjectPtr zur�ckgegeben.
	*/
	RenderObjectPtr<Bitmap> addDynamicBitmap(uint width, uint height);
	/**
	    @brief Erzeugt eine Animation auf Basis einer Animationsdatei als Kinderobjekt des Renderobjektes.
	    @param FileName der Dateiname der Quelldatei
	    @return Gibt einen BS_RenderObjectPtr auf das erzeugte Objekt zur�ck.<br>
	            Falls ein Fehler aufgetreten ist wird ein ung�ltiger BS_RenderObjectPtr zur�ckgegeben.
	*/
	RenderObjectPtr<Animation> addAnimation(const Common::String &fileName);
	/**
	    @brief Erzeugt eine Animation auf Basis eines Animationstemplate als Kinderobjekt des Renderobjektes.
	    @param pAnimationTemplate ein Pointer auf das Animationstemplate
	    @return Gibt einen BS_RenderObjectPtr auf das erzeugte Objekt zur�ck.<br>
	            Falls ein Fehler aufgetreten ist wird ein ung�ltiger BS_RenderObjectPtr zur�ckgegeben.
	    @remark Das Renderobjekt �bernimmt die Verwaltung des Animationstemplate.
	*/
	RenderObjectPtr<Animation> addAnimation(const AnimationTemplate &animationTemplate);
	/**
	    @brief Erzeugt ein neues Farbpanel als Kinderobjekt des Renderobjektes.
	    @param Width die Breite des Panels
	    @param Height die H�he des Panels
	    @param Color die Farbe des Panels.<br>
	           Der Standardwert ist Schwarz (BS_RGB(0, 0, 0)).
	    @return Gibt einen BS_RenderObjectPtr auf das erzeugte Objekt zur�ck.<br>
	            Falls ein Fehler aufgetreten ist wird ein ung�ltiger BS_RenderObjectPtr zur�ckgegeben.
	*/

	RenderObjectPtr<Panel> addPanel(int width, int height, uint color = 0xff000000);
	/**
	    @brief Erzeugt ein Textobjekt als Kinderobjekt des Renderobjektes.
	    @param Font der Dateiname des zu verwendenen Fonts
	    @param Text der anzuzeigende Text.<br>
	                Der Standardwert ist "".
	    @return Gibt einen BS_RenderObjectPtr auf das erzeugte Objekt zur�ck.<br>
	            Falls ein Fehler aufgetreten ist wird ein ung�ltiger BS_RenderObjectPtr zur�ckgegeben.
	*/
	RenderObjectPtr<Text> addText(const Common::String &font, const Common::String &text = "");

	// Cast-Methoden
	// -------------
	/**
	    @brief Castet das Objekt zu einem BS_Bitmap-Objekt wenn zul�ssig.
	    @return Gibt einen BS_RenderObjectPtr auf das Objekt zur�ck.<br>
	            Falls der Cast nicht zul�ssig ist, wird ein ung�ltiger BS_RenderObjectPtr zur�ckgegeben.
	*/
	RenderObjectPtr<Bitmap> toBitmap() {
		if (_type == TYPE_STATICBITMAP || _type == TYPE_DYNAMICBITMAP)
			return RenderObjectPtr<Bitmap>(this->getHandle());
		else
			return RenderObjectPtr<Bitmap>();
	}
	/**
	    @brief Castet das Objekt zu einem BS_Animation-Objekt wenn zul�ssig.
	    @return Gibt einen BS_RenderObjectPtr auf das Objekt zur�ck.<br>
	            Falls der Cast nicht zul�ssig ist, wird ein ung�ltiger BS_RenderObjectPtr zur�ckgegeben.
	*/
	RenderObjectPtr<Animation> toAnimation() {
		if (_type == TYPE_ANIMATION)
			return RenderObjectPtr<Animation>(this->getHandle());
		else
			return RenderObjectPtr<Animation>();
	}
	/**
	    @brief Castet das Objekt zu einem BS_Panel-Objekt wenn zul�ssig.
	    @return Gibt einen BS_RenderObjectPtr auf das Objekt zur�ck.<br>
	            Falls der Cast nicht zul�ssig ist, wird ein ung�ltiger BS_RenderObjectPtr zur�ckgegeben.
	*/
	RenderObjectPtr<Panel> toPanel() {
		if (_type == TYPE_PANEL)
			return RenderObjectPtr<Panel>(this->getHandle());
		else
			return RenderObjectPtr<Panel>();
	}
	/**
	    @brief Castet das Object zu einem BS_Text-Objekt wenn zul�ssig.
	    @return Gibt einen BS_RenderObjectPtr auf das Objekt zur�ck.<br>
	            Falls der Cast nicht zul�ssig ist, wird ein ung�ltiger BS_RenderObjectPtr zur�ckgegeben.
	*/
	RenderObjectPtr<Text> toText() {
		if (_type == TYPE_TEXT)
			return RenderObjectPtr<Text>(this->getHandle());
		else
			return RenderObjectPtr<Text>();
	}

	// Konstruktor / Desktruktor
	// -------------------------
	/**
	    @brief Erzeugt ein neues BS_RenderObject.
	    @param pKernel ein Pointer auf den Kernel
	    @param pParent ein Pointer auf das Elternobjekt des neuen Objektes im Objektbaum.<br>
	                   Der Pointer darf nicht NULL sein.
	    @param Type der Objekttyp<br>
	                Der Typ BS_RenderObject::TYPE_ROOT ist nicht zul�ssig. Wurzelknoten m�ssen mit dem alternativen Konstruktor erzeugt
	                werden.
	    @param Handle das Handle, welches dem Objekt zugewiesen werden soll.<br>
	                Dieser Parameter erzwingt ein bestimmtes Handle f�r das neue Objekt, oder w�hlt automatisch ein Handle, wenn der Parameter 0 ist.
	                Ist das gew�nschte Handle bereits vergeben, gibt GetInitSuccess() false zur�ck.<br>
	                Der Standardwert ist 0.
	    @remark Nach dem Aufruf des Konstruktors kann �ber die Methode GetInitSuccess() abgefragt werden, ob die Konstruktion erfolgreich war.<br>
	            Es ist nicht notwendig alle BS_RenderObject Instanzen einzeln zu l�schen. Dieses geschiet automatisch beim L�schen eines
	            Vorfahren oder beim L�schen des zust�ndigen BS_RenderObjectManager.
	 */
	RenderObject(RenderObjectPtr<RenderObject> pParent, TYPES type, uint handle = 0);
	virtual ~RenderObject();

	// Interface
	// ---------
	/**
	    @brief Rendert des Objekt und alle seine Unterobjekte.
	    @return Gibt false zur�ck, falls beim Rendern ein Fehler aufgetreten ist.
	    @remark Vor jedem Aufruf dieser Methode muss ein Aufruf von UpdateObjectState() erfolgt sein.
	            Dieses kann entweder direkt geschehen oder durch den Aufruf von UpdateObjectState() an einem Vorfahren-Objekt.<br>
	            Diese Methode darf nur von BS_RenderObjectManager aufgerufen werden.
	*/
	bool render();
	/**
	    @brief Bereitet das Objekt und alle seine Unterobjekte auf einen Rendervorgang vor.
	           Hierbei werden alle Dirty-Rectangles berechnet und die Renderreihenfolge aktualisiert.
	    @return Gibt false zur�ck, falls ein Fehler aufgetreten ist.
	    @remark Diese Methode darf nur von BS_RenderObjectManager aufgerufen werden.
	 */
	bool updateObjectState();
	/**
	    @brief L�scht alle Kinderobjekte.
	*/
	void deleteAllChildren();

	// Accessor-Methoden
	// -----------------
	/**
	    @brief Setzt die Position des Objektes.
	    @param X die neue X-Koordinate des Objektes relativ zum Elternobjekt.
	    @param Y die neue Y-Koordinate des Objektes relativ zum Elternobjekt.
	 */
	virtual void setPos(int x, int y);
	/**
	    @brief Setzt die Position des Objektes auf der X-Achse.
	    @param X die neue X-Koordinate des Objektes relativ zum Elternobjekt.
	 */
	virtual void setX(int x);
	/**
	    @brief Setzt die Position des Objektes auf der Y-Achse.
	    @param Y die neue Y-Koordinate des Objektes relativ zum Elternobjekt.
	 */
	virtual void setY(int y);
	/**
	    @brief Setzt den Z-Wert des Objektes.
	    @param Z der neue Z-Wert des Objektes relativ zum Elternobjekt<br>
	             Negative Z-Werte sind nicht zul�ssig.
	    @remark Der Z-Wert legt die Renderreihenfolge der Objekte fest. Objekte mit niedrigem Z-Wert werden vor Objekten mit h�herem
	            Z-Wert gezeichnet. Je h�her der Z-Wert desto weiter "vorne" liegt ein Objekt also.<br>
	            Wie alle andere Attribute ist auch dieses relativ zum Elternobjekt, ein Kinderobjekt kann also nie unter seinem
	            Elternobjekt liegen, auch wenn es einen Z-Wert von 0 hat.
	 */
	virtual void setZ(int z);
	/**
	    @brief Setzt die Sichtbarkeit eine Objektes.
	    @param Visible der neue Sichtbarkeits-Zustand des Objektes<br>
	                   true entspricht sichtbar, false entspricht unsichtbar.
	 */
	virtual void setVisible(bool visible);
	/**
	    @brief Gibt die Position des Objektes auf der X-Achse relativ zum Elternobjekt zur�ck.
	 */
	virtual int getX() const {
		return _x;
	}
	/**
	    @brief Gibt die Position des Objektes auf der Y-Achse relativ zum Elternobjekt zur�ck.
	 */
	virtual int getY() const {
		return _y;
	}
	/**
	    @brief Gibt die absolute Position des Objektes auf der X-Achse zur�ck.
	*/
	virtual int getAbsoluteX() const {
		return _absoluteX;
	}
	/**
	    @brief Gibt die absolute Position des Objektes auf der Y-Achse zur�ck.
	*/
	virtual int getAbsoluteY() const {
		return _absoluteY;
	}
	/**
	    @brief Gibt den Z-Wert des Objektes relativ zum Elternobjekt zur�ck.
	    @remark Der Z-Wert legt die Renderreihenfolge der Objekte fest. Objekte mit niedrigem Z-Wert werden vor Objekten mit h�herem
	            Z-Wert gezeichnet. Je h�her der Z-Wert desto weiter "vorne" liegt ein Objekt also.<br>
	            Wie alle andere Attribute ist auch dieses relativ zum Elternobjekt, ein Kinderobjekt kann also nie unter seinem
	            Elternobjekt liegen, auch wenn es einen Z-Wert von 0 hat.
	 */
	int         getZ() const {
		return _z;
	}
	/**
	    @brief Gibt die Breite des Objektes zur�ck.
	 */
	int         getWidth() const {
		return _width;
	}
	/**
	    @brief Gibt die H�he des Objektes zur�ck.
	 */
	int         getHeight() const {
		return _height;
	}
	/**
	    @brief Gibt den Sichtbarkeitszustand des Objektes zur�ck.
	    @return Gibt den Sichtbarkeitszustand des Objektes zur�ck.<br>
	            true entspricht sichtbar, false entspricht unsichtbar.
	 */
	bool        isVisible() const {
		return _visible;
	}
	/**
	    @brief Gibt den Typ des Objektes zur�ck.
	 */
	TYPES       getType() const {
		return _type;
	}
	/**
	    @brief Gibt zur�ck, ob das Objekt erfolgreich initialisiert wurde.
	    @remark H�sslicher Workaround um das Problem, dass Konstruktoren keine R�ckgabewerte haben.
	 */
	bool        getInitSuccess() const {
		return _initSuccess;
	}
	/**
	    @brief Gibt die Bounding-Box des Objektes zur�ck.
	    @remark Diese Angabe erfolgt ausnahmsweise in Bildschirmkoordianten und nicht relativ zum Elternobjekt.
	 */
	const Common::Rect  &getBbox() const {
		return _bbox;
	}
	/**
	    @brief Stellt sicher, dass das Objekt im n�chsten Frame neu gezeichnet wird.
	*/
	void forceRefresh() {
		_refreshForced = true;
	}
	/**
	    @brief Gibt das Handle des Objekte zur�ck.
	*/
	uint getHandle() const {
		return _handle;
	}

	// Persistenz-Methoden
	// -------------------
	virtual bool persist(OutputPersistenceBlock &writer);
	virtual bool unpersist(InputPersistenceBlock &reader);
	// TODO: Evtl. protected
	bool persistChildren(OutputPersistenceBlock &writer);
	bool unpersistChildren(InputPersistenceBlock &reader);
	// TODO: Evtl. private
	RenderObjectPtr<RenderObject> recreatePersistedRenderObject(InputPersistenceBlock &reader);

protected:
	// Typen
	// -----
	typedef Common::List<RenderObjectPtr<RenderObject> >          RENDEROBJECT_LIST;
	typedef Common::List<RenderObjectPtr<RenderObject> >::iterator    RENDEROBJECT_ITER;

	int         _x;            ///< Die X-Position des Objektes relativ zum Eltern-Objekt
	int         _y;            ///< Die Y-Position des Objektes relativ zum Eltern-Objekt
	int         _absoluteX;    ///< Die absolute X-Position des Objektes
	int         _absoluteY;    ///< Die absolute Y-Position des Objektes
	int         _z;            ///< Der Z-Wert des Objektes relativ zum Eltern-Objekt
	int         _width;        ///< Die Breite des Objektes
	int         _height;       ///< Die H�he des Objektes
	bool        _visible;      ///< Ist true, wenn das Objekt sichtbar ist
	bool        _childChanged; ///< Ist true, wenn sich ein Kinderobjekt ver�ndert hat
	TYPES       _type;         ///< Der Objekttyp
	bool        _initSuccess;  ///< Ist true, wenn Objekt erfolgreich intialisiert werden konnte
	Common::Rect _bbox;         ///< Die Bounding-Box des Objektes in Bildschirmkoordinaten

	// Kopien der Variablen, die f�r die Errechnung des Dirty-Rects und zur Bestimmung der Objektver�nderung notwendig sind
	Common::Rect     _oldBbox;
	int         _oldX;
	int         _oldY;
	int         _oldZ;
	bool        _oldVisible;

	/// Ein Pointer auf den BS_RenderObjektManager, der das Objekt verwaltet.
	RenderObjectManager *_managerPtr;

	// Render-Methode
	// --------------
	/**
	    @brief Einschubmethode, die den tats�chlichen Redervorgang durchf�hrt.

	    Diese Methode wird von Render() aufgerufen um das Objekt darzustellen.
	    Diese Methode sollte von allen Klassen implementiert werden, die von BS_RederObject erben, um das Zeichnen umzusetzen.

	    @return Gibt false zur�ck, falls das Rendern fehlgeschlagen ist.
	    @remark
	 */
	virtual bool doRender() = 0; // { return true; }

	// RenderObject-Baum Variablen
	// ---------------------------
	// Der Baum legt die hierachische Ordnung der BS_RenderObjects fest.
	// Alle Eigenschaften wie X, Y, Z und Visible eines BS_RenderObjects sind relativ zu denen seines Vaters.
	// Au�erdem sind die Objekte von links nach rechts in ihrer Renderreihenfolge sortiert.
	// Das prim�re Sortierkriterium ist hierbei der Z-Wert und das sekund�re der Y-Wert (von oben nach unten).
	// Beispiel:
	//                  Screen
	//                 /  |   \.
	//               /    |     \.
	//             /      |       \.
	//           /        |         \.
	//     Background Interface   Maus
	//      /   \      /  |   \.
	//    /      \    /   |    \.
	// George   T�r Icn1 Icn2 Icn3
	//
	// Wenn jetzt das Interface mit SetVisible() ausgeblendet w�rde, verschwinden auch die Icons, die sich im Interface
	// befinden.
	// Wenn der Hintergrund bewegt wird (Scrolling), bewegen sich auch die darauf befindlichen Gegenst�nde und Personen.

	/// Ein Pointer auf das Elternobjekt.
	RenderObjectPtr<RenderObject> _parentPtr;
	/// Die Liste der Kinderobjekte nach der Renderreihenfolge geordnet
	RENDEROBJECT_LIST           _children;

	/**
	    @brief Gibt einen Pointer auf den BS_RenderObjektManager zur�ck, der das Objekt verwaltet.
	 */
	RenderObjectManager *getManager() const {
		return _managerPtr;
	}
	/**
	    @brief F�gt dem Objekt ein neues Kinderobjekt hinzu.
	    @param pObject ein Pointer auf das einzuf�gende Objekt
	    @return Gibt false zur�ck, falls das Objekt nicht eingef�gt werden konnte.
	*/
	bool addObject(RenderObjectPtr<RenderObject> pObject);

private:
	/// Ist true, wenn das Objekt in n�chsten Frame neu gezeichnet werden soll
	bool _refreshForced;

	uint _handle;

	/**
	    @brief Entfernt ein Objekt aus der Kinderliste.
	    @param pObject ein Pointer auf das zu entfernende Objekt
	    @return Gibt false zur�ck, falls das zu entfernende Objekt nicht in der Liste gefunden werden konnte.
	 */
	bool detatchChildren(RenderObjectPtr<RenderObject> pObject);
	/**
	    @brief Berechnet die Bounding-Box und registriert das Dirty-Rect beim BS_RenderObjectManager.
	 */
	void updateBoxes();
	/**
	    @brief Berechnet die Bounding-Box des Objektes.
	    @return Gibt die Bounding-Box des Objektes in Bildschirmkoordinaten zur�ck.
	*/
	Common::Rect calcBoundingBox() const;
	/**
	    @brief Berechnet das Dirty-Rectangle des Objektes.
	    @return Gibt das Dirty-Rectangle des Objektes in Bildschirmkoordinaten zur�ck.
	*/
	Common::Rect calcDirtyRect() const;
	/**
	    @brief Berechnet die absolute Position des Objektes.
	*/
	void calcAbsolutePos(int &x, int &y) const;
	/**
	    @brief Berechnet die absolute Position des Objektes auf der X-Achse.
	*/
	int calcAbsoluteX() const;
	/**
	    @brief Berechnet die absolute Position des Objektes.
	*/
	int calcAbsoluteY() const;
	/**
	    @brief Sortiert alle Kinderobjekte nach ihrem Renderang.
	 */
	void sortRenderObjects();
	/**
	    @brief Validiert den Zustand eines Objektes nachdem die durch die Ver�nderung verursachten Folgen abgearbeitet wurden.
	 */
	void validateObject();
	/**
	    @brief Berechnet die absolute Position des Objektes und aller seiner Kinderobjekte neu.

	    Diese Methode muss aufgerufen werden, wann immer sich die Position des Objektes ver�ndert. Damit die Kinderobjekte immer die
	    richtige absolute Position haben.
	*/
	void updateAbsolutePos();
	/**
	    @brief Teilt dem Objekt mit, dass sich eines seiner Kinderobjekte dahingehend ver�ndert hat, die eine erneute Bestimmung der
	           Rendereihenfolge verlangt.
	*/
	void signalChildChange() {
		_childChanged = true;
	}
	/**
	    @brief Berechnet des Schnittrechteck der Bounding-Box des Objektes mit einem anderen Objekt.
	    @param pObjekt ein Pointer auf das Objekt mit dem geschnitten werden soll
	    @param Result das Ergebnisrechteck
	    @return Gibt false zur�ck, falls sich die Objekte gar nicht schneiden.
	 */
	bool getObjectIntersection(RenderObjectPtr<RenderObject> pObject, Common::Rect &result);
	/**
	    @brief Vergleichsoperator der auf Objektpointern basiert statt auf Objekten.
	    @remark Diese Methode wird f�rs Sortieren der Kinderliste nach der Rendereihenfolge benutzt.
	*/
	static bool greater(const RenderObjectPtr<RenderObject> lhs, const RenderObjectPtr<RenderObject> rhs);
};

} // End of namespace Sword25

#endif
