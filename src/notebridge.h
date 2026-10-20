#pragma once

#include <QObject>
#include <QPointer>
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

    Q_INVOKABLE void setFontFamily(QQuickTextDocument *qdoc, const QString &family);
    Q_INVOKABLE void setFontSize(QQuickTextDocument *qdoc, int pixelSize);

    Q_INVOKABLE void beginMove(QQuickItem *appletItem);
    Q_INVOKABLE void updateMove();
    Q_INVOKABLE QPointF endMove(QQuickItem *appletItem);

    Q_INVOKABLE void beginResize(QQuickItem *appletItem);
    Q_INVOKABLE QSizeF updateResize(qreal minW, qreal minH);
    Q_INVOKABLE QSizeF endResize(QQuickItem *appletItem);

    // Vincula el contenedor de Plasma y mantiene bloqueada su posición/tamaño frente a reinicios de AppletsLayout
    Q_INVOKABLE bool syncGeometry(QQuickItem *appletItem, int x, int y, int w, int h);

    // Obtiene el rectángulo desplazado (+28px, +28px) para crear la nueva nota al instante
    Q_INVOKABLE QRectF nextCascadeGeometry(QQuickItem *appletItem) const;

    // Respaldo por D-Bus solo si se ejecuta en plasmoidviewer aislado
    Q_INVOKABLE void spawnNewNoteFallback();

    Q_INVOKABLE QStringList systemFonts() const;

private slots:
    void onContainerGeometryChanged();
    void applySavedGeometry();

private:
    QQuickItem* findAppletContainer(QQuickItem *item) const;
    void bindContainerSignals(QQuickItem *container);

    QPointer<QQuickItem> m_targetContainer;
    QPointer<QQuickItem> m_boundContainer;
    QPointer<QQuickItem> m_boundLayout;

    QPoint m_startCursorPos;
    QPointF m_startContainerPos;
    QSizeF m_startContainerSize;

    QPointF m_savedPos = QPointF(-99999, -99999);
    QSizeF m_savedSize = QSizeF(260, 250);
    bool m_hasSavedPos = false;
    bool m_isMoving = false;
    bool m_isResizing = false;
    bool m_RestoringGuard = false;
};
