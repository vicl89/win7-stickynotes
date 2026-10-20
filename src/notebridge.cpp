#include "notebridge.h"
#include <QTextDocument>
#include <QTextBlock>
#include <QTextFragment>
#include <QTextCharFormat>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QCursor>
#include <QTimer>
#include <QtMath>

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
    int startPos = (selStart != selEnd) ? qMin(selStart, selEnd) : cursorPos;
    int endPos   = (selStart != selEnd) ? qMax(selStart, selEnd) : cursorPos;
    cursor.setPosition(startPos, QTextCursor::MoveAnchor);
    cursor.setPosition(endPos, QTextCursor::KeepAnchor);

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
            QTextCharFormat normalFmt = blockCursor.charFormat();
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

    if (relPos >= 0 && relPos <= 2 &&
        (text.startsWith(QStringLiteral("☐ ")) || text.startsWith(QStringLiteral("☑ ")))) {
        bool markDone = text.startsWith(QStringLiteral("☐ "));

        QTextCursor cursor(doc);
        cursor.beginEditBlock();

        cursor.setPosition(block.position() + 1);
        QTextCharFormat boxFmt = cursor.charFormat();
        boxFmt.setFontStrikeOut(false);

        cursor.setPosition(block.position());
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
        cursor.insertText(markDone ? QStringLiteral("☑") : QStringLiteral("☐"), boxFmt);

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

void NoteBridge::setFontFamily(QQuickTextDocument *qdoc, const QString &family) {
    if (!qdoc || !qdoc->textDocument() || family.isEmpty()) return;
    QTextDocument *doc = qdoc->textDocument();

    QFont defFont = doc->defaultFont();
    defFont.setFamily(family);
    doc->setDefaultFont(defFont);

    QTextCursor cursor(doc);
    cursor.beginEditBlock();

    cursor.select(QTextCursor::Document);
    QTextCharFormat mergeFmt;
    mergeFmt.setFontFamilies(QStringList{family});
    mergeFmt.setProperty(QTextFormat::FontFamily, family);
    cursor.mergeCharFormat(mergeFmt);

    for (QTextBlock block = doc->begin(); block.isValid(); block = block.next()) {
        QTextCursor blockCursor(block);
        QTextCharFormat bFmt = block.charFormat();
        bFmt.setFontFamilies(QStringList{family});
        bFmt.setProperty(QTextFormat::FontFamily, family);
        blockCursor.setBlockCharFormat(bFmt);

        for (auto it = block.begin(); !it.atEnd(); ++it) {
            QTextFragment frag = it.fragment();
            if (!frag.isValid()) continue;
            QTextCharFormat fmt = frag.charFormat();
            fmt.setFontFamilies(QStringList{family});
            fmt.setProperty(QTextFormat::FontFamily, family);
            cursor.setPosition(frag.position());
            cursor.setPosition(frag.position() + frag.length(), QTextCursor::KeepAnchor);
            cursor.setCharFormat(fmt);
        }
    }

    cursor.endEditBlock();
}

void NoteBridge::setFontSize(QQuickTextDocument *qdoc, int pixelSize) {
    if (!qdoc || !qdoc->textDocument() || pixelSize <= 0) return;
    QTextDocument *doc = qdoc->textDocument();

    QFont defFont = doc->defaultFont();
    defFont.setPixelSize(pixelSize);
    doc->setDefaultFont(defFont);

    QTextCursor cursor(doc);
    cursor.beginEditBlock();

    cursor.select(QTextCursor::Document);
    QTextCharFormat mergeFmt;
    mergeFmt.setProperty(QTextFormat::FontPixelSize, pixelSize);
    cursor.mergeCharFormat(mergeFmt);

    for (QTextBlock block = doc->begin(); block.isValid(); block = block.next()) {
        QTextCursor blockCursor(block);
        QTextCharFormat bFmt = block.charFormat();
        bFmt.clearProperty(QTextFormat::FontPointSize);
        bFmt.setProperty(QTextFormat::FontPixelSize, pixelSize);
        blockCursor.setBlockCharFormat(bFmt);

        for (auto it = block.begin(); !it.atEnd(); ++it) {
            QTextFragment frag = it.fragment();
            if (!frag.isValid()) continue;
            QTextCharFormat fmt = frag.charFormat();
            fmt.clearProperty(QTextFormat::FontPointSize);
            fmt.setProperty(QTextFormat::FontPixelSize, pixelSize);
            cursor.setPosition(frag.position());
            cursor.setPosition(frag.position() + frag.length(), QTextCursor::KeepAnchor);
            cursor.setCharFormat(fmt);
        }
    }

    cursor.endEditBlock();
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

void NoteBridge::bindContainerSignals(QQuickItem *container) {
    if (!container) return;

    if (m_boundContainer != container) {
        if (m_boundContainer) {
            disconnect(m_boundContainer, nullptr, this, nullptr);
        }
        m_boundContainer = container;
        connect(m_boundContainer, &QQuickItem::xChanged, this, &NoteBridge::onContainerGeometryChanged, Qt::UniqueConnection);
        connect(m_boundContainer, &QQuickItem::yChanged, this, &NoteBridge::onContainerGeometryChanged, Qt::UniqueConnection);
        connect(m_boundContainer, &QQuickItem::widthChanged, this, &NoteBridge::onContainerGeometryChanged, Qt::UniqueConnection);
        connect(m_boundContainer, &QQuickItem::heightChanged, this, &NoteBridge::onContainerGeometryChanged, Qt::UniqueConnection);
    }

    QQuickItem *layout = container->parentItem();
    if (layout && m_boundLayout != layout) {
        if (m_boundLayout) {
            disconnect(m_boundLayout, nullptr, this, nullptr);
        }
        m_boundLayout = layout;
        connect(m_boundLayout, &QQuickItem::widthChanged, this, &NoteBridge::onContainerGeometryChanged, Qt::UniqueConnection);
        connect(m_boundLayout, &QQuickItem::heightChanged, this, &NoteBridge::onContainerGeometryChanged, Qt::UniqueConnection);
    }
}

void NoteBridge::onContainerGeometryChanged() {
    if (m_isMoving || m_isResizing || m_RestoringGuard || !m_boundContainer || !m_hasSavedPos) {
        return;
    }

    // Si el usuario está moviendo el widget desde el modo de edición nativo de Plasma, respetarlo
    QVariant editProp = m_boundContainer->property("editMode");
    if (editProp.isValid() && editProp.toBool()) {
        m_savedPos = m_boundContainer->position();
        return;
    }

    const qreal dx = qAbs(m_boundContainer->x() - m_savedPos.x());
    const qreal dy = qAbs(m_boundContainer->y() - m_savedPos.y());
    if (dx > 1.0 || dy > 1.0) {
        QTimer::singleShot(10, this, &NoteBridge::applySavedGeometry);
    }
}

void NoteBridge::applySavedGeometry() {
    if (m_isMoving || m_isResizing || !m_boundContainer || !m_hasSavedPos) {
        return;
    }
    m_RestoringGuard = true;
    m_boundContainer->setPosition(m_savedPos);
    if (m_savedSize.width() >= 180 && m_savedSize.height() >= 160) {
        m_boundContainer->setSize(m_savedSize);
    }
    m_RestoringGuard = false;
}

bool NoteBridge::syncGeometry(QQuickItem *appletItem, int x, int y, int w, int h) {
    QQuickItem *container = findAppletContainer(appletItem);
    if (!container) return false;

    bindContainerSignals(container);

    if (w >= 180 && h >= 160) {
        m_savedSize = QSizeF(w, h);
    }
    if (x > -10000 && y > -10000) {
        m_savedPos = QPointF(x, y);
        m_hasSavedPos = true;
        applySavedGeometry();
    }
    return true;
}

void NoteBridge::beginMove(QQuickItem *appletItem) {
    m_targetContainer = findAppletContainer(appletItem);
    if (!m_targetContainer) return;
    bindContainerSignals(m_targetContainer);
    m_isMoving = true;
    m_startCursorPos = QCursor::pos();
    m_startContainerPos = m_targetContainer->position();
}

void NoteBridge::updateMove() {
    if (!m_targetContainer) return;
    QPoint delta = QCursor::pos() - m_startCursorPos;
    QPointF newPos = m_startContainerPos + QPointF(delta.x(), delta.y());
    m_targetContainer->setPosition(newPos);
    m_savedPos = newPos;
    m_hasSavedPos = true;
}

QPointF NoteBridge::endMove(QQuickItem *appletItem) {
    m_isMoving = false;
    QQuickItem *container = m_targetContainer ? m_targetContainer.data() : findAppletContainer(appletItem);
    if (!container) return QPointF(-99999, -99999);
    m_savedPos = container->position();
    m_hasSavedPos = true;
    return m_savedPos;
}

void NoteBridge::beginResize(QQuickItem *appletItem) {
    m_targetContainer = findAppletContainer(appletItem);
    if (!m_targetContainer) return;
    bindContainerSignals(m_targetContainer);
    m_isResizing = true;
    m_startCursorPos = QCursor::pos();
    m_startContainerSize = m_targetContainer->size();
}

QSizeF NoteBridge::updateResize(qreal minW, qreal minH) {
    if (!m_targetContainer) return QSizeF(minW, minH);
    QPoint delta = QCursor::pos() - m_startCursorPos;
    qreal newW = qMax(minW, m_startContainerSize.width() + delta.x());
    qreal newH = qMax(minH, m_startContainerSize.height() + delta.y());
    m_savedSize = QSizeF(newW, newH);
    m_targetContainer->setSize(m_savedSize);
    return m_savedSize;
}

QSizeF NoteBridge::endResize(QQuickItem *appletItem) {
    m_isResizing = false;
    QQuickItem *container = m_targetContainer ? m_targetContainer.data() : findAppletContainer(appletItem);
    if (!container) return m_savedSize;
    m_savedSize = container->size();
    return m_savedSize;
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
