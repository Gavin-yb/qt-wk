/*
    This file is part of the WebKit open source project.
    This file has been generated by generate-bindings.pl. DO NOT MODIFY!

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"

#if ENABLE(SVG_ANIMATION)

#include "JSSVGAnimateColorElement.h"

#include "SVGAnimateColorElement.h"
#include <wtf/GetPtr.h>

using namespace JSC;

namespace WebCore {

ASSERT_CLASS_FITS_IN_CELL(JSSVGAnimateColorElement);

/* Hash table */

static const HashTableValue JSSVGAnimateColorElementTableValues[2] =
{
    { "constructor", DontEnum|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGAnimateColorElementConstructor), (intptr_t)0 },
    { 0, 0, 0, 0 }
};

static JSC_CONST_HASHTABLE HashTable JSSVGAnimateColorElementTable =
#if ENABLE(PERFECT_HASH_SIZE)
    { 0, JSSVGAnimateColorElementTableValues, 0 };
#else
    { 2, 1, JSSVGAnimateColorElementTableValues, 0 };
#endif

/* Hash table for constructor */

static const HashTableValue JSSVGAnimateColorElementConstructorTableValues[1] =
{
    { 0, 0, 0, 0 }
};

static JSC_CONST_HASHTABLE HashTable JSSVGAnimateColorElementConstructorTable =
#if ENABLE(PERFECT_HASH_SIZE)
    { 0, JSSVGAnimateColorElementConstructorTableValues, 0 };
#else
    { 1, 0, JSSVGAnimateColorElementConstructorTableValues, 0 };
#endif

class JSSVGAnimateColorElementConstructor : public DOMConstructorObject {
public:
    JSSVGAnimateColorElementConstructor(ExecState* exec, JSDOMGlobalObject* globalObject)
        : DOMConstructorObject(JSSVGAnimateColorElementConstructor::createStructure(globalObject->objectPrototype()), globalObject)
    {
        putDirect(exec->propertyNames().prototype, JSSVGAnimateColorElementPrototype::self(exec, globalObject), None);
    }
    virtual bool getOwnPropertySlot(ExecState*, const Identifier&, PropertySlot&);
    virtual bool getOwnPropertyDescriptor(ExecState*, const Identifier&, PropertyDescriptor&);
    virtual const ClassInfo* classInfo() const { return &s_info; }
    static const ClassInfo s_info;

    static PassRefPtr<Structure> createStructure(JSValue proto) 
    { 
        return Structure::create(proto, TypeInfo(ObjectType, StructureFlags), AnonymousSlotCount); 
    }
    
protected:
    static const unsigned StructureFlags = OverridesGetOwnPropertySlot | ImplementsHasInstance | DOMConstructorObject::StructureFlags;
};

const ClassInfo JSSVGAnimateColorElementConstructor::s_info = { "SVGAnimateColorElementConstructor", 0, &JSSVGAnimateColorElementConstructorTable, 0 };

bool JSSVGAnimateColorElementConstructor::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<JSSVGAnimateColorElementConstructor, DOMObject>(exec, &JSSVGAnimateColorElementConstructorTable, this, propertyName, slot);
}

bool JSSVGAnimateColorElementConstructor::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    return getStaticValueDescriptor<JSSVGAnimateColorElementConstructor, DOMObject>(exec, &JSSVGAnimateColorElementConstructorTable, this, propertyName, descriptor);
}

/* Hash table for prototype */

static const HashTableValue JSSVGAnimateColorElementPrototypeTableValues[1] =
{
    { 0, 0, 0, 0 }
};

static JSC_CONST_HASHTABLE HashTable JSSVGAnimateColorElementPrototypeTable =
#if ENABLE(PERFECT_HASH_SIZE)
    { 0, JSSVGAnimateColorElementPrototypeTableValues, 0 };
#else
    { 1, 0, JSSVGAnimateColorElementPrototypeTableValues, 0 };
#endif

const ClassInfo JSSVGAnimateColorElementPrototype::s_info = { "SVGAnimateColorElementPrototype", 0, &JSSVGAnimateColorElementPrototypeTable, 0 };

JSObject* JSSVGAnimateColorElementPrototype::self(ExecState* exec, JSGlobalObject* globalObject)
{
    return getDOMPrototype<JSSVGAnimateColorElement>(exec, globalObject);
}

const ClassInfo JSSVGAnimateColorElement::s_info = { "SVGAnimateColorElement", &JSSVGAnimationElement::s_info, &JSSVGAnimateColorElementTable, 0 };

JSSVGAnimateColorElement::JSSVGAnimateColorElement(NonNullPassRefPtr<Structure> structure, JSDOMGlobalObject* globalObject, PassRefPtr<SVGAnimateColorElement> impl)
    : JSSVGAnimationElement(structure, globalObject, impl)
{
}

JSObject* JSSVGAnimateColorElement::createPrototype(ExecState* exec, JSGlobalObject* globalObject)
{
    return new (exec) JSSVGAnimateColorElementPrototype(JSSVGAnimateColorElementPrototype::createStructure(JSSVGAnimationElementPrototype::self(exec, globalObject)));
}

bool JSSVGAnimateColorElement::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<JSSVGAnimateColorElement, Base>(exec, &JSSVGAnimateColorElementTable, this, propertyName, slot);
}

bool JSSVGAnimateColorElement::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    return getStaticValueDescriptor<JSSVGAnimateColorElement, Base>(exec, &JSSVGAnimateColorElementTable, this, propertyName, descriptor);
}

JSValue jsSVGAnimateColorElementConstructor(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGAnimateColorElement* domObject = static_cast<JSSVGAnimateColorElement*>(asObject(slotBase));
    return JSSVGAnimateColorElement::getConstructor(exec, domObject->globalObject());
}
JSValue JSSVGAnimateColorElement::getConstructor(ExecState* exec, JSGlobalObject* globalObject)
{
    return getDOMConstructor<JSSVGAnimateColorElementConstructor>(exec, static_cast<JSDOMGlobalObject*>(globalObject));
}


}

#endif // ENABLE(SVG_ANIMATION)
