#include "notebridge.h"
#include <QTextDocument>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QCursor>

NoteBridge::NoteBridge(QObject *parent) : QObject(parent) {}

static QTextCursor getCursor(QQuickTextDocument *qdoc, int cursorPos, int anchorPos) {
    if (!qdoc || !qdoc->textDocument()) return QTextCursor();
    QTextCursor cursor(qdoc->textDocument());
    cursor.setPosition(anchorPos);
    cursor.setPosition(cursorPos, QTextCursor::KeepAnchor);
    return cursor;
}

void NoteBridge::toggleBold(QQuickTextDocument *qdoc, int cursorPos, int anchorPos) {
    QTextCursor cursor = getCursor(qdoc, cursorPos, anchorPos);
    if (cursor.isNull()) return;
    QTextCharFormat fmt;
    bool isBold = cursor.charFormat().fontWeight() > QFont::Normal;
    fmt.setFontWeight(isBold ? QFont::Normal : QFont::Bold);
    cursor.mergeCharFormat(fmt);
}

void NoteBridge::toggleItalic(QQuickTextDocument *qdoc, int cursorPos, int anchorPos) {
    QTextCursor cursor = getCursor(qdoc, cursorPos, anchorPos);
    if (cursor.isNull()) return;
    QTextCharFormat fmt;
    fmt.setFontItalic(!cursor.charFormat().fontItalic());
    cursor.mergeCharFormat(fmt);
}

void NoteBridge::toggleUnderline(QQuickTextDocument *qdoc, int cursorPos, int anchorPos) {
    QTextCursor cursor = getCursor(qdoc, cursorPos, anchorPos);
    if (cursor.isNull()) return;
    QTextCharFormat fmt;
    fmt.setFontUnderline(!cursor.charFormat().fontUnderline());
    cursor.mergeCharFormat(fmt);
}

void NoteBridge::toggleBulletList(QQuickTextDocument *qdoc, int cursorPos, int anchorPos) {
    QTextCursor cursor = getCursor(qdoc, cursorPos, anchorPos);
    if (cursor.isNull()) return;

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

QQuickItem* NoteBridge::findAppletContainer(QQuickItem *item) const {
    if (!item) return nullptr;
    QQuickItem *curr = item;
    QQuickItem *candidate = item;

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
    // Caché estática en memoria: evita re-escanear las fuentes del sistema al crear cada nota
    static const QStringList cachedFonts = QFontDatabase::families();
    return cachedFonts;
}
