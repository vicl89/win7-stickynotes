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

    Q_INVOKABLE void toggleBold(QQuickTextDocument *qdoc, int cursorPos, int anchorPos);
    Q_INVOKABLE void toggleItalic(QQuickTextDocument *qdoc, int cursorPos, int anchorPos);
    Q_INVOKABLE void toggleUnderline(QQuickTextDocument *qdoc, int cursorPos, int anchorPos);
    Q_INVOKABLE void toggleBulletList(QQuickTextDocument *qdoc, int cursorPos, int anchorPos);

    Q_INVOKABLE void beginMove(QQuickItem *appletItem);
    Q_INVOKABLE void updateMove();

    Q_INVOKABLE void beginResize(QQuickItem *appletItem);
    Q_INVOKABLE QSizeF updateResize(qreal minW, qreal minH);

    // Obtiene el rectángulo desplazado (+24px, +24px) para crear la nueva nota al instante
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
