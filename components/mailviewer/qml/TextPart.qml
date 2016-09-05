import QtQuick 2.4

Text  {
    width: delegateRoot.width

    text: model.textContent
    wrapMode: Text.WordWrap

    color: embeded ? "grey" : "black"
}
