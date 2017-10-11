// Copyright (c) 2005 - 2017 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#ifndef GAMEOBJECT_H_INCLUDED
#define GAMEOBJECT_H_INCLUDED

#pragma once

#include "gameTypes/GO_Type.h"
#include <string>

class SerializedGameData;
class GameWorldGame;
class EventManager;
class PostMsg;

/// Basisklasse für alle Spielobjekte
class GameObject
{
public:
    /// ID for an invalid GameObject
    static const unsigned INVALID_ID = 0xFFFFFFFF;

    GameObject();
    GameObject(SerializedGameData& sgd, const unsigned obj_id);
    GameObject(const GameObject& go);
    virtual ~GameObject();

    /// zerstört das Objekt.
    virtual void Destroy() = 0;

    /// Benachrichtigen, wenn neuer GF erreicht wurde.
    virtual void HandleEvent(const unsigned /*id*/) {}

    /// Gibt Objekt-ID zurück.
    unsigned GetObjId() const { return objId; }

    /// Serialisierungsfunktion.
    virtual void Serialize(SerializedGameData& sgd) const = 0;
    /// Liefert den GOT (siehe oben)
    virtual GO_Type GetGOT() const = 0;

    virtual std::string ToString() const;

protected:
    /// Serialisierungsfunktion.
    void Serialize_GameObject(SerializedGameData& /*sgd*/) const {}
    // Following are some "sandbox methods". They avoid dependencies of subclasses to commonly used functions
    EventManager& GetEvMgr() const;
    /// Send the msg to given player
    void SendPostMessage(unsigned player, PostMsg* msg);

private:
    unsigned objId; /// eindeutige Objekt-ID

    // Static members
public:
    /// Setzt Pointer auf GameWorld und EventManager
    static void SetPointers(GameWorldGame* const gameWorld) { GameObject::gwg = gameWorld; }
    /// setzt den Objekt und Objekt-ID-Counter zurück
    static void ResetCounter()
    {
        objIdCounter_ = 1;
        objCounter_ = 0;
    };
    /// Gibt Anzahl Objekte zurück.
    static unsigned GetObjCount() { return objCounter_; }
    /// Setzt Anzahl der Objekte (NUR FÜR DAS LADEN!)
    static void SetObjCount(const unsigned obj_count) { objCounter_ = obj_count; }
    /// Gibt Obj-ID-Counter zurück (NUR FÜR DAS SPEICHERN!)
    static unsigned GetObjIDCounter() { return objIdCounter_; }
    /// Setzt Counter (NUR FÜR DAS LADEN!)
    static void SetObjIDCounter(const unsigned obj_id_counter) { objIdCounter_ = obj_id_counter; }

protected:
    /// Zugriff auf übrige Spielwelt
    static GameWorldGame* gwg;

private:
    static unsigned objIdCounter_; /// Objekt-ID-Counter
    static unsigned objCounter_;   /// Objekt-Counter
};

/// Calls destroy on a GameObject and then deletes it setting the ptr to NULL
template<typename T>
void destroyAndDelete(T*& obj)
{
    obj->Destroy();
    deletePtr(obj);
}

#endif /// GAMEOBJECT_H_INCLUDED
