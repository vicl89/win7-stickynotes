#include "notebridge.h"
#include <QTextDocument>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QCursor>

NoteBridge::NoteBridge(QObject *parent) : QObject(parent) {}

static QTextCursor getCursor(QQuickTextDocument *qdoc, int selStart, int selEnd, int cursorPos) {
    if (!qdoc || !qdoc->textDocument()) return QTextCursor();
    QTextCursor cursor(qdoc->textDocument());
    if (selStart != selEnd) {
        cursor.setPosition(qMin(selStart, selEnd), QTextCursor::MoveAnchor);
        cursor.setPosition(qMax(selStart, selEnd), QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(cursorPos, QTextCursor::MoveAnchor);
        cursor.select(QTextCursor::WordUnderCursor);
    }
    return cursor;
}

void NoteBridge::toggleBold(QQuickTextDocument *qdoc, int selStart, int selEnd, int cursorPos) {
    QTextCursor cursor = getCursor(qdoc, selStart, selEnd, cursorPos);
    if (cursor.isNull()) return;
    QTextCharFormat fmt;
    bool isBold = cursor.charFormat().fontWeight() > QFont::Normal;
    fmt.setFontWeight(isBold ? QFont::Normal : QFont::Bold);
    cursor.mergeCharFormat(fmt);
}

void NoteBridge::toggleItalic(QQuickTextDocument *qdoc, int selStart, int selEnd, int cursorPos) {
    QTextCursor cursor = getCursor(qdoc, selStart, selEnd, cursorPos);
    if (cursor.isNull()) return;
    QTextCharFormat fmt;
    fmt.setFontItalic(!cursor.charFormat().fontItalic());
    cursor.mergeCharFormat(fmt);
}

void NoteBridge::toggleUnderline(QQuickTextDocument *qdoc, int selStart, int selEnd, int cursorPos) {
    QTextCursor cursor = getCursor(qdoc, selStart, selEnd, cursorPos);
    if (cursor.isNull()) return;
    QTextCharFormat fmt;
    fmt.setFontUnderline(!cursor.charFormat().fontUnderline());
    cursor.mergeCharFormat(fmt);
}

void NoteBridge::toggleBulletList(QQuickTextDocument *qdoc, int selStart, int selEnd, int cursorPos) {
    if (!qdoc || !qdoc->textDocument()) return;
    QTextCursor cursor(qdoc->textDocument());
    cursor.setPosition(qMin(selStart, selEnd), QTextCursor::MoveAnchor);
    cursor.setPosition(qMax(selStart, selEnd), QTextCursor::KeepAnchor);

    QTextList *currentList = cursor.currentList();
    if (currentList) {
        QTextBlockFormat blockFmt = cursor.blockFormat();
        blockFmt.setObjectIndex(-1);
        cursor.setBlockFormat(blockFmt);
    } else {
        QTextListFormat listFmt;
        listFmt.setStyle(QTextListFormat::ListDisc);
        cursor.createList(listFmt);
    }
}

void NoteBridge::toggleChecklist(QQuickTextDocument *qdoc, int selStart, int selEnd, int cursorPos) {
    if (!qdoc || !qdoc->textDocument()) return;
    QTextDocument *doc = qdoc->textDocument();

    int startPos = (selStart != selEnd) ? qMin(selStart, selEnd) : cursorPos;
    int endPos   = (selStart != selEnd) ? qMax(selStart, selEnd) : cursorPos;

    QTextCursor cursor(doc);
    cursor.beginEditBlock();

    QTextBlock startBlock = doc->findBlock(startPos);
    QTextBlock endBlock   = doc->findBlock(endPos);

    for (QTextBlock block = startBlock; block.isValid(); block = block.next()) {
        QTextCursor blockCursor(block);
        blockCursor.movePosition(QTextCursor::StartOfBlock);
        QString text = block.text();

        if (text.startsWith(QStringLiteral("☐ ")) || text.startsWith(QStringLiteral("☑ "))) {
            blockCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
            blockCursor.removeSelectedText();

            blockCursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            QTextCharFormat fmt;
            fmt.setFontStrikeOut(false);
            blockCursor.mergeCharFormat(fmt);
        } else {
            QTextCharFormat normalFmt;
            normalFmt.setFontStrikeOut(false);
            blockCursor.insertText(QStringLiteral("☐ "), normalFmt);
        }

        if (block == endBlock) break;
    }

    cursor.endEditBlock();
}

bool NoteBridge::handleChecklistClick(QQuickTextDocument *qdoc, int charPos) {
    if (!qdoc || !qdoc->textDocument() || charPos < 0) return false;
    QTextDocument *doc = qdoc->textDocument();
    QTextBlock block = doc->findBlock(charPos);
    if (!block.isValid()) return false;

    int relPos = charPos - block.position();
    QString text = block.text();

    // Si el clic ocurrió sobre la casilla al inicio de la línea (primeros 2 caracteres)
    if (relPos >= 0 && relPos <= 2 &&
        (text.startsWith(QStringLiteral("☐ ")) || text.startsWith(QStringLiteral("☑ ")))) {
        bool markDone = text.startsWith(QStringLiteral("☐ "));

        QTextCursor cursor(doc);
        cursor.beginEditBlock();

        // Conservar formato de fuente de la casilla sin tachar el icono en sí
        cursor.setPosition(block.position() + 1);
        QTextCharFormat boxFmt = cursor.charFormat();
        boxFmt.setFontStrikeOut(false);

        cursor.setPosition(block.position());
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
        cursor.insertText(markDone ? QStringLiteral("☑") : QStringLiteral("☐"), boxFmt);

        // Tachar o destachar el texto del pendiente en ese bloque
        cursor.setPosition(block.position() + 2);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        QTextCharFormat textFmt;
        textFmt.setFontStrikeOut(markDone);
        cursor.mergeCharFormat(textFmt);

        cursor.endEditBlock();
        return true;
    }
    return false;
}

QQuickItem* NoteBridge::findAppletContainer(QQuickItem *item) const {
    if (!item) return nullptr;
    QQuickItem *curr = item;
    QQuickItem *candidate = nullptr;

    while (curr->parentItem()) {
        QString cls = QString::fromLatin1(curr->metaObject()->className());
        if (cls.contains(QStringLiteral("AppletContainer")) ||
            cls.contains(QStringLiteral("ItemContainer"))) {
            return curr;
        }
        if (curr->parentItem()->width() > item->width() + 200 &&
            curr->parentItem()->height() > item->height() + 200) {
            candidate = curr;
            break;
        }
        curr = curr->parentItem();
    }
    return candidate;
}

void NoteBridge::beginMove(QQuickItem *appletItem) {
    m_targetContainer = findAppletContainer(appletItem);
    if (!m_targetContainer) return;
    m_startCursorPos = QCursor::pos();
    m_startContainerPos = m_targetContainer->position();
}

void NoteBridge::updateMove() {
    if (!m_targetContainer) return;
    QPoint delta = QCursor::pos() - m_startCursorPos;
    m_targetContainer->setPosition(m_startContainerPos + QPointF(delta.x(), delta.y()));
}

QPointF NoteBridge::endMove(QQuickItem *appletItem) {
    QQuickItem *container = m_targetContainer ? m_targetContainer : findAppletContainer(appletItem);
    if (!container) return QPointF(-1, -1);
    return container->position();
}

bool NoteBridge::restorePosition(QQuickItem *appletItem, int x, int y) {
    if (x < 0 || y < 0) return false;
    QQuickItem *container = findAppletContainer(appletItem);
    if (!container) return false;
    container->setPosition(QPointF(x, y));
    return true;
}

void NoteBridge::beginResize(QQuickItem *appletItem) {
    m_targetContainer = findAppletContainer(appletItem);
    if (!m_targetContainer) return;
    m_startCursorPos = QCursor::pos();
    m_startContainerSize = m_targetContainer->size();
}

QSizeF NoteBridge::updateResize(qreal minW, qreal minH) {
    if (!m_targetContainer) return QSizeF(minW, minH);
    QPoint delta = QCursor::pos() - m_startCursorPos;
    qreal newW = qMax(minW, m_startContainerSize.width() + delta.x());
    qreal newH = qMax(minH, m_startContainerSize.height() + delta.y());
    m_targetContainer->setSize(QSizeF(newW, newH));
    return QSizeF(newW, newH);
}

QRectF NoteBridge::nextCascadeGeometry(QQuickItem *appletItem) const {
    QQuickItem *container = findAppletContainer(appletItem);
    if (!container) return QRectF(100, 100, 260, 250);
    return QRectF(
        container->x() + 28,
        container->y() + 28,
        container->width(),
        container->height()
    );
}

void NoteBridge::spawnNewNoteFallback() {
    QDBusInterface iface(
        QStringLiteral("org.kde.plasmashell"),
        QStringLiteral("/PlasmaShell"),
        QStringLiteral("org.kde.PlasmaShell"),
        QDBusConnection::sessionBus()
    );
    if (iface.isValid()) {
        QString script = QStringLiteral(
            "var d = desktops()[0];"
            "d.addWidget('org.kde.plasma.win7stickynotes');"
        );
        iface.asyncCall(QStringLiteral("evaluateScript"), script);
    }
}

QStringList NoteBridge::systemFonts() const {
    static const QStringList cachedFonts = QFontDatabase::families();
    return cachedFonts;
}
