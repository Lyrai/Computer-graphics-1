#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QObject>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <functional>
#include <QMap>
#include "expressionparser.h"

class GraphView : public QGraphicsView
{
    Q_OBJECT
public:
    GraphView(QGraphicsScene* scene, QWidget* parent = nullptr);

    void setLeft(double value) { _left = value; }
    void setRight(double value) { _right = value; }
    void setExpr(Expression exp) { expr = exp; repaint(); };

    double left() { return _left; }
    double right() { return _right; }

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void calculate();
    QPointF mapFromScreen(int x, int y);
    QPoint mapToScreen(QPointF point);
    void drawLine(QPoint start, QPoint end);
    void drawGrid();
    double coordsRangeY() { return bottom - top; }
    double coordsRangeX() { return _right - _left; }
    int marginedWidth() { return size().width() - leftMargin - rightMargin; }
    int marginedHeight() { return size().height() - topMargin - bottomMargin; }
    void drawHorizontalLine(int y);
    void drawVerticalLine(int x);

private:
    double _left = -10;
    double _right = 10;
    double top = -10;
    double bottom = 10;
    int pixelsPerStep = 1;
    int topMargin = 0;
    int leftMargin = 0;
    static constexpr int rightMargin = 30;
    static constexpr int bottomMargin = 30;
    Expression expr;
    QVector<QPointF> points;
};

#endif // GRAPHVIEW_H
