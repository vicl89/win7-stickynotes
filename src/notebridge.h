#pragma once

#include <QObject>
#include <QQuickItem>
#include <QQuickTextDocument>
#include <QTextCursor>
#include <QTextListFormat>
#include <QTextList>
#include <QFontDatabase>
#include <QPointF>
#include <QSizeF>
#include <QRectF>
#include <QtQml/qqmlregistration.h>

class NoteBridge : public QObject {
    Q_OBJECT
    QML_ELEMENT

public:
    explicit NoteBridge(QObject *parent = nullptr);

    Q_INVOKABLE void toggleBold(QQuickTextDocument *qdoc, int selStart, int selEnd, int cursorPos);
    Q_INVOKABLE void toggleItalic(QQuickTextDocument *qdoc, int selStart, int selEnd, int cursorPos);
    Q_INVOKABLE void toggleUnderline(QQuickTextDocument *qdoc, int selStart, int selEnd, int cursorPos);
    Q_INVOKABLE void toggleBulletList(QQuickTextDocument *qdoc, int selStart, int selEnd, int cursorPos);
    Q_INVOKABLE void toggleChecklist(QQuickTextDocument *qdoc, int selStart, int selEnd, int cursorPos);
    Q_INVOKABLE bool handleChecklistClick(QQuickTextDocument *qdoc, int charPos);

    Q_INVOKABLE void beginMove(QQuickItem *appletItem);
    Q_INVOKABLE void updateMove();
    Q_INVOKABLE QPointF endMove(QQuickItem *appletItem);
    Q_INVOKABLE bool restorePosition(QQuickItem *appletItem, int x, int y);

    Q_INVOKABLE void beginResize(QQuickItem *appletItem);
    Q_INVOKABLE QSizeF updateResize(qreal minW, qreal minH);

    // Obtiene el rectángulo desplazado (+28px, +28px) para crear la nueva nota al instante
    Q_INVOKABLE QRectF nextCascadeGeometry(QQuickItem *appletItem) const;

    // Respaldo por D-Bus solo si se ejecuta en plasmoidviewer aislado
    Q_INVOKABLE void spawnNewNoteFallback();

    Q_INVOKABLE QStringList systemFonts() const;

private:
    QQuickItem* findAppletContainer(QQuickItem *item) const;
    QQuickItem *m_targetContainer = nullptr;
    QPoint m_startCursorPos;
    QPointF m_startContainerPos;
    QSizeF m_startContainerSize;
};
