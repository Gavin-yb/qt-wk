import Qt 4.7

Item {
    property int test: myObject.object.a

    Item {
        id: myObject
        property QtObject object;
        object: QtObject {
            property int a: 10
        }
    }
}