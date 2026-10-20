import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import Qt5Compat.GraphicalEffects
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.win7stickynotes.backend 1.0

PlasmoidItem {
    id: root
    width: Plasmoid.configuration.noteWidth
    height: Plasmoid.configuration.noteHeight
    Layout.preferredWidth: Plasmoid.configuration.noteWidth
    Layout.preferredHeight: Plasmoid.configuration.noteHeight
    Layout.minimumWidth: 180
    Layout.minimumHeight: 160

    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    NoteBridge {
        id: bridge
    }

    function syncNoteText() {
        Plasmoid.configuration.noteText = editor.text
        if (typeof Plasmoid.configuration.writeConfig === "function") {
            Plasmoid.configuration.writeConfig()
        }
    }

    // Sincronizador inicial con el contenedor del escritorio
    Timer {
        id: posRestoreTimer
        interval: 250
        repeat: true
        property int attempts: 0
        onTriggered: {
            attempts += 1
            var bound = bridge.syncGeometry(
                root,
                Plasmoid.configuration.posX,
                Plasmoid.configuration.posY,
                Plasmoid.configuration.noteWidth,
                Plasmoid.configuration.noteHeight
            )
            // Mantener verificación durante los primeros 6 segundos de arranque de sesión
            if (bound && attempts >= 24) {
                stop()
            } else if (attempts >= 40) {
                stop()
            }
        }
    }

    Component.onCompleted: {
        posRestoreTimer.start()
    }

    function createNewStickyNote() {
        var geom = bridge.nextCascadeGeometry(root)
        if (Plasmoid.containment && typeof Plasmoid.containment.createApplet === "function") {
            Plasmoid.containment.createApplet("org.kde.plasma.win7stickynotes", [], geom)
        } else {
            bridge.spawnNewNoteFallback()
        }
    }

    readonly property var themes: [
        { name: "Verde",    body: "#ccf7c8", header: "#b5e6b0", accent: "#224d1e", btnBg: "#9ed698" },
        { name: "Azul",     body: "#cde9ff", header: "#b3dbfa", accent: "#183e5e", btnBg: "#9acbf2" },
        { name: "Amarillo", body: "#fdfdca", header: "#f2f2a7", accent: "#4d4612", btnBg: "#e3e388" },
        { name: "Rosa",     body: "#f6d5f7", header: "#eab8ec", accent: "#521d54", btnBg: "#dca0de" },
        { name: "Morado",   body: "#dcd5fe", header: "#c5bafa", accent: "#2e2066", btnBg: "#b0a2f2" },
        { name: "Blanco",   body: "#f5f5f5", header: "#e0e0e0", accent: "#2b2b2b", btnBg: "#cccccc" }
    ]

    property var currentTheme: themes[Plasmoid.configuration.colorTheme] || themes[0]
    property bool showToolbar: false

    DropShadow {
        anchors.fill: noteContainer
        horizontalOffset: 2
        verticalOffset: 4
        radius: 10.0
        samples: 17
        color: "#66000000"
        source: noteContainer
    }

    Rectangle {
        id: noteContainer
        anchors.fill: parent
        anchors.margins: 6
        color: root.currentTheme.body
        border.color: "#44000000"
        border.width: 1

        HoverHandler {
            id: noteHover
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            // Barra superior (Header de Windows 7)
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                color: root.currentTheme.header

                MouseArea {
                    id: dragArea
                    anchors.fill: parent
                    cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                    onPressed: bridge.beginMove(root)
                    onPositionChanged: {
                        if (pressed) bridge.updateMove()
                    }
                    onReleased: {
                        var finalPos = bridge.endMove(root)
                        if (finalPos.x > -10000 && finalPos.y > -10000) {
                            Plasmoid.configuration.posX = Math.round(finalPos.x)
                            Plasmoid.configuration.posY = Math.round(finalPos.y)
                            if (typeof Plasmoid.configuration.writeConfig === "function") {
                                Plasmoid.configuration.writeConfig()
                            }
                        }
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    opacity: (noteHover.hovered || editor.activeFocus || root.showToolbar) ? 1.0 : 0.0
                    Behavior on opacity { NumberAnimation { duration: 150 } }

                    // Botón "+" (Nueva nota)
                    Rectangle {
                        width: 22; height: 22
                        radius: 3
                        color: plusHover.containsMouse ? root.currentTheme.btnBg : "transparent"
                        Text {
                            anchors.centerIn: parent
                            text: "+"
                            font.pixelSize: 19
                            font.bold: true
                            color: root.currentTheme.accent
                        }
                        MouseArea {
                            id: plusHover
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.createNewStickyNote()
                        }
                    }

                    Item { Layout.fillWidth: true }

                    // Botón "Aa" (Mostrar/Ocultar barra de formato)
                    Rectangle {
                        width: 26; height: 22
                        radius: 3
                        color: (root.showToolbar || aaHover.containsMouse) ? root.currentTheme.btnBg : "transparent"
                        Text {
                            anchors.centerIn: parent
                            text: "Aa"
                            font.pixelSize: 13
                            font.bold: true
                            color: root.currentTheme.accent
                        }
                        MouseArea {
                            id: aaHover
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.showToolbar = !root.showToolbar
                        }
                    }

                    // Botón "✕" (Eliminar nota)
                    Rectangle {
                        width: 22; height: 22
                        radius: 3
                        color: closeHover.containsMouse ? "#e81123" : "transparent"
                        Text {
                            anchors.centerIn: parent
                            text: "✕"
                            font.pixelSize: 13
                            font.bold: true
                            color: closeHover.containsMouse ? "#ffffff" : root.currentTheme.accent
                        }
                        MouseArea {
                            id: closeHover
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: Plasmoid.internalAction("remove").trigger()
                        }
                    }
                }
            }

            // Barra de herramientas desplegable
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: root.showToolbar ? 66 : 0
                clip: true
                color: Qt.darker(root.currentTheme.header, 1.04)
                Behavior on Layout.preferredHeight { NumberAnimation { duration: 120 } }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    spacing: 5

                    // Fila 1: Botones de formato y colores
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Repeater {
                            model: [
                                { label: "B",  bold: true,  italic: false, underline: false, action: "bold" },
                                { label: "I",  bold: false, italic: true,  underline: false, action: "italic" },
                                { label: "U",  bold: false, italic: false, underline: true,  action: "underline" },
                                { label: "•≡", bold: true,  italic: false, underline: false, action: "bullet" },
                                { label: "☑",  bold: false, italic: false, underline: false, action: "check" }
                            ]

                            Rectangle {
                                width: 24; height: 24
                                radius: 3
                                color: fmtMouse.pressed ? Qt.darker(root.currentTheme.btnBg, 1.15)
                                     : (fmtMouse.containsMouse ? root.currentTheme.btnBg : "#33ffffff")
                                border.color: root.currentTheme.accent
                                border.width: 1

                                Text {
                                    anchors.centerIn: parent
                                    text: modelData.label
                                    font.pixelSize: 13
                                    font.bold: modelData.bold
                                    font.italic: modelData.italic
                                    font.underline: modelData.underline
                                    color: root.currentTheme.accent
                                }

                                MouseArea {
                                    id: fmtMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        var sStart = editor.selectionStart
                                        var sEnd   = editor.selectionEnd
                                        var cPos   = editor.cursorPosition

                                        if (modelData.action === "bold")
                                            bridge.toggleBold(editor.textDocument, sStart, sEnd, cPos)
                                        else if (modelData.action === "italic")
                                            bridge.toggleItalic(editor.textDocument, sStart, sEnd, cPos)
                                        else if (modelData.action === "underline")
                                            bridge.toggleUnderline(editor.textDocument, sStart, sEnd, cPos)
                                        else if (modelData.action === "bullet")
                                            bridge.toggleBulletList(editor.textDocument, sStart, sEnd, cPos)
                                        else if (modelData.action === "check")
                                            bridge.toggleChecklist(editor.textDocument, sStart, sEnd, cPos)

                                        root.syncNoteText()
                                        editor.forceActiveFocus()
                                    }
                                }
                            }
                        }

                        Item { Layout.fillWidth: true }

                        Repeater {
                            model: root.themes
                            Rectangle {
                                width: 14; height: 14
                                radius: 3
                                color: modelData.body
                                border.color: Plasmoid.configuration.colorTheme === index ? root.currentTheme.accent : "#66000000"
                                border.width: Plasmoid.configuration.colorTheme === index ? 2 : 1
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: Plasmoid.configuration.colorTheme = index
                                }
                            }
                        }
                    }

                    // Fila 2: Selector de tipografía propio y selector de tamaño
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 5

                        Rectangle {
                            id: fontSelector
                            Layout.fillWidth: true
                            Layout.preferredHeight: 26
                            color: fontMouse.containsMouse ? root.currentTheme.btnBg : "#55ffffff"
                            border.color: root.currentTheme.accent
                            border.width: 1
                            radius: 3

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 8
                                anchors.rightMargin: 6
                                spacing: 4

                                Text {
                                    Layout.fillWidth: true
                                    text: Plasmoid.configuration.fontFamily
                                    font.family: Plasmoid.configuration.fontFamily
                                    font.pixelSize: 12
                                    color: root.currentTheme.accent
                                    elide: Text.ElideRight
                                    verticalAlignment: Text.AlignVCenter
                                }

                                Text {
                                    text: "▾"
                                    font.pixelSize: 12
                                    color: root.currentTheme.accent
                                }
                            }

                            MouseArea {
                                id: fontMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (fontListView.model.length === 0) {
                                        fontListView.model = bridge.systemFonts()
                                    }
                                    fontPopup.open()
                                }
                            }

                            QQC2.Popup {
                                id: fontPopup
                                y: fontSelector.height + 2
                                width: fontSelector.width
                                height: 180
                                padding: 2

                                background: Rectangle {
                                    color: root.currentTheme.body
                                    border.color: root.currentTheme.accent
                                    border.width: 1
                                    radius: 3
                                }

                                contentItem: ListView {
                                    id: fontListView
                                    clip: true
                                    model: []
                                    delegate: Rectangle {
                                        width: fontListView.width
                                        height: 24
                                        color: itemMouse.containsMouse ? root.currentTheme.btnBg : "transparent"

                                        Text {
                                            anchors.fill: parent
                                            anchors.leftMargin: 6
                                            verticalAlignment: Text.AlignVCenter
                                            text: modelData
                                            font.family: modelData
                                            font.pixelSize: 12
                                            color: root.currentTheme.accent
                                            elide: Text.ElideRight
                                        }

                                        MouseArea {
                                            id: itemMouse
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            onClicked: {
                                                Plasmoid.configuration.fontFamily = modelData
                                                bridge.setFontFamily(editor.textDocument, modelData)
                                                root.syncNoteText()
                                                fontPopup.close()
                                                editor.forceActiveFocus()
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        Rectangle {
                            Layout.preferredWidth: 74
                            Layout.preferredHeight: 26
                            color: "#55ffffff"
                            border.color: root.currentTheme.accent
                            border.width: 1
                            radius: 3

                            RowLayout {
                                anchors.fill: parent
                                spacing: 0

                                Rectangle {
                                    Layout.preferredWidth: 22
                                    Layout.fillHeight: true
                                    color: decMouse.containsMouse ? root.currentTheme.btnBg : "transparent"
                                    radius: 3
                                    Text {
                                        anchors.centerIn: parent
                                        text: "−"
                                        font.pixelSize: 14
                                        font.bold: true
                                        color: root.currentTheme.accent
                                    }
                                    MouseArea {
                                        id: decMouse
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            if (Plasmoid.configuration.fontSize > 8) {
                                                var newSize = Plasmoid.configuration.fontSize - 1
                                                Plasmoid.configuration.fontSize = newSize
                                                bridge.setFontSize(editor.textDocument, newSize)
                                                root.syncNoteText()
                                                editor.forceActiveFocus()
                                            }
                                        }
                                    }
                                }

                                Text {
                                    Layout.fillWidth: true
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    text: Plasmoid.configuration.fontSize
                                    font.pixelSize: 12
                                    font.bold: true
                                    color: root.currentTheme.accent
                                }

                                Rectangle {
                                    Layout.preferredWidth: 22
                                    Layout.fillHeight: true
                                    color: incMouse.containsMouse ? root.currentTheme.btnBg : "transparent"
                                    radius: 3
                                    Text {
                                        anchors.centerIn: parent
                                        text: "+"
                                        font.pixelSize: 14
                                        font.bold: true
                                        color: root.currentTheme.accent
                                    }
                                    MouseArea {
                                        id: incMouse
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            if (Plasmoid.configuration.fontSize < 42) {
                                                var newSize = Plasmoid.configuration.fontSize + 1
                                                Plasmoid.configuration.fontSize = newSize
                                                bridge.setFontSize(editor.textDocument, newSize)
                                                root.syncNoteText()
                                                editor.forceActiveFocus()
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Área de edición de texto enriquecido
            QQC2.ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                QQC2.TextArea {
                    id: editor
                    width: parent.width
                    textFormat: TextEdit.RichText
                    wrapMode: TextEdit.Wrap
                    selectByMouse: true
                    persistentSelection: true
                    color: "#1a1a1a"
                    font.family: Plasmoid.configuration.fontFamily
                    font.pixelSize: Plasmoid.configuration.fontSize
                    background: null
                    padding: 10

                    property point pressPoint: Qt.point(0, 0)

                    onPressed: (event) => {
                        pressPoint = Qt.point(event.x, event.y)
                    }

                    onReleased: (event) => {
                        var dx = Math.abs(event.x - pressPoint.x)
                        var dy = Math.abs(event.y - pressPoint.y)
                        if (dx < 6 && dy < 6 && editor.selectionStart === editor.selectionEnd) {
                            var pos = editor.positionAt(event.x, event.y)
                            if (bridge.handleChecklistClick(editor.textDocument, pos)) {
                                root.syncNoteText()
                            }
                        }
                    }

                    Component.onCompleted: {
                        if (Plasmoid.configuration.noteText !== "") {
                            text = Plasmoid.configuration.noteText
                        }
                    }

                    onTextChanged: {
                        Plasmoid.configuration.noteText = text
                    }

                    Keys.onPressed: (event) => {
                        if (event.modifiers & Qt.ControlModifier) {
                            if (event.key === Qt.Key_B) {
                                bridge.toggleBold(editor.textDocument, editor.selectionStart, editor.selectionEnd, editor.cursorPosition)
                                root.syncNoteText()
                                event.accepted = true
                            } else if (event.key === Qt.Key_I) {
                                bridge.toggleItalic(editor.textDocument, editor.selectionStart, editor.selectionEnd, editor.cursorPosition)
                                root.syncNoteText()
                                event.accepted = true
                            } else if (event.key === Qt.Key_U) {
                                bridge.toggleUnderline(editor.textDocument, editor.selectionStart, editor.selectionEnd, editor.cursorPosition)
                                root.syncNoteText()
                                event.accepted = true
                            } else if ((event.modifiers & Qt.ShiftModifier) && event.key === Qt.Key_L) {
                                bridge.toggleBulletList(editor.textDocument, editor.selectionStart, editor.selectionEnd, editor.cursorPosition)
                                root.syncNoteText()
                                event.accepted = true
                            } else if ((event.modifiers & Qt.ShiftModifier) && event.key === Qt.Key_C) {
                                bridge.toggleChecklist(editor.textDocument, editor.selectionStart, editor.selectionEnd, editor.cursorPosition)
                                root.syncNoteText()
                                event.accepted = true
                            }
                        }
                    }
                }
            }
        }

        // Esquina inferior derecha: Redimensionado libre de parpadeo
        Canvas {
            id: resizeGrip
            width: 16
            height: 16
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 3

            onPaint: {
                var ctx = getContext("2d");
                ctx.clearRect(0, 0, width, height);
                ctx.fillStyle = root.currentTheme.accent;
                var step = 3.5;
                for (var r = 0; r < 4; r++) {
                    for (var c = 3 - r; c < 4; c++) {
                        ctx.fillRect(c * step, r * step, 1.8, 1.8);
                    }
                }
            }

            Connections {
                target: root
                function onCurrentThemeChanged() { resizeGrip.requestPaint() }
            }

            MouseArea {
                anchors.fill: parent
                anchors.margins: -4
                cursorShape: Qt.SizeFDiagCursor
                property size lastSize: Qt.size(root.width, root.height)

                onPressed: bridge.beginResize(root)
                onPositionChanged: {
                    if (pressed) lastSize = bridge.updateResize(180, 160)
                }
                onReleased: {
                    var finalSize = bridge.endResize(root)
                    Plasmoid.configuration.noteWidth = Math.round(finalSize.width)
                    Plasmoid.configuration.noteHeight = Math.round(finalSize.height)
                    if (typeof Plasmoid.configuration.writeConfig === "function") {
                        Plasmoid.configuration.writeConfig()
                    }
                }
            }
        }
    }
}
