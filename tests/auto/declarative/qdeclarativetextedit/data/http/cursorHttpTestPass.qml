import Qt 4.6
import "http://localhost:42332"

Rectangle { width: 300; height: 300; color: "white"
    resources: [ 
        Component { id:cursorWait; WaitItem { objectName: "delegateSlow" } },
        Component { id:cursorNorm; NormItem { objectName: "delegateOkay" } }
    ] 
    TextEdit {
        cursorDelegate: cursorWait
        text: "Hello"
    }
    TextEdit {
        objectName: "textEditObject"
        cursorDelegate: cursorNorm
        focus: true;
        text: "Hello"
    }
}