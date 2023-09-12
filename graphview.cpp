#include <QDebug>
#include <QResizeEvent>
#include <QPaintEvent>
#include <algorithm>
#include <limits>
#include <QGraphicsTextItem>
#include <thread>
#include <cmath>
#include "graphview.h"

GraphView::GraphView(QGraphicsScene* scene, QWidget* parent): QGraphicsView(scene, parent), expr(Expression::invalid)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::HighQualityAntialiasing);
    resize(500, 500);
}

void GraphView::resizeEvent(QResizeEvent *event)
{

    QGraphicsView::resizeEvent(event);
}

void GraphView::paintEvent(QPaintEvent *event)
{
    scene()->clear();
    pixelsPerStep = size().width() / 100 - 4;
    if(pixelsPerStep < 1) {
        pixelsPerStep = 1;
    }

    auto points = calculate();

    double max = -std::numeric_limits<double>::infinity();
    double min = std::numeric_limits<double>::infinity();
    for(int i = 0; i < points.size(); ++i) {
        if(points[i].y() > max) {
            max = points[i].y();
        }

        if(points[i].y() < min) {
            min = points[i].y();
        }
    }
    top = min;
    bottom = max;

    drawGrid();

    auto transformed = QVector<QPoint>(points.size());
    auto firstTarget = [&points, &transformed, this]() {
        std::transform(points.begin(), points.begin() + points.size() / 4, transformed.begin(), [this](auto point) { return mapToScreen(point); });
    };
    auto secondTarget = [&points, &transformed, this]() {
        std::transform(points.begin() + points.size() / 4, points.begin() + points.size() / 4 * 2, transformed.begin() + transformed.size() / 4, [this](auto point) { return mapToScreen(point); });
    };
    auto thirdTarget = [&points, &transformed, this]() {
        std::transform(points.begin() + points.size() / 4 * 2, points.begin() + points.size() / 4 * 3, transformed.begin() + transformed.size() / 4 * 2, [this](auto point) { return mapToScreen(point); });
    };
    auto fourthTarget = [&points, &transformed, this]() {
        std::transform(points.begin() + points.size() / 4 * 3, points.end(), transformed.begin() + transformed.size() / 4 * 3, [this](auto point) { return mapToScreen(point); });
    };
    std::thread first(firstTarget);
    std::thread second(secondTarget);
    std::thread third(thirdTarget);
    std::thread fourth(fourthTarget);

    //std::transform(points.begin(), points.end(), transformed.begin(), [this](auto point) { return mapToScreen(point); });

    first.join();
    second.join();
    third.join();
    fourth.join();

    for(int i = 0; i < transformed.size() - 1; ++i) {
        drawLine(transformed[i], transformed[i + 1]);
    }

    scene()->setSceneRect(0, 0, size().width(), size().height());
    QGraphicsView::paintEvent(event);
}

QVector<QPointF> GraphView::calculate()
{
    if(!expr.isValid()) {
        return {};
    }
    QVector<QPointF> result;
    result.reserve(size().width());
    auto steps = mapFromScreen(marginedWidth(), size().height());
    auto xStep = steps.x();

    for(double i = _left; i < _right; i += xStep * pixelsPerStep) {
        result.push_back(QPointF(i, expr.evaluate(i)));
    }
    result.push_back(QPointF(_right, expr.evaluate(_right)));

    return result;
}

QPointF GraphView::mapFromScreen(int x, int y)
{
    return {(_right - _left) / x, coordsRangeY() / y};
}

QPoint GraphView::mapToScreen(QPointF point)
{
    return {(int)((point.x() - _left) / coordsRangeX() * marginedWidth()) + leftMargin, (int)((bottom - point.y()) / coordsRangeY() * marginedHeight()) + topMargin};
}

void GraphView::drawLine(QPoint begin, QPoint end)
{
    QPen pen;
    pen.setWidth(2);
    pen.setJoinStyle(Qt::PenJoinStyle::RoundJoin);
    scene()->addLine(begin.x(), begin.y() - 1, end.x(), end.y() - 1, pen);
}

double round(double value, int places) {
    if(places == 0) {
        return (int)value;
    }
    double multiplier = pow(10, places);

    return ceil(value * multiplier) / multiplier;
}

void GraphView::drawGrid()
{
    auto stepPixels = 50;


    for(int i = (size().height() - bottomMargin) / 50; i > 0; --i) {
        drawHorizontalLine(stepPixels * i);
    }

    leftMargin = 50;

    for(int i = (size().width() - rightMargin) / 50; i > 0; --i) {
        drawVerticalLine(stepPixels * i);
    }

    drawHorizontalLine(topMargin);
    drawHorizontalLine(size().height() - bottomMargin);
    drawVerticalLine(size().width() - rightMargin);
}

void GraphView::drawHorizontalLine(int y)
{
    QPen pen;
    pen.setWidth(2);

    auto line = scene()->addLine(leftMargin, y, size().width() - rightMargin, y, pen);
    line->setOpacity(0.4);
    auto yPosPixels = line->boundingRect().y() - topMargin + 1;
    auto yText = round(((marginedHeight() - yPosPixels) / marginedHeight() * coordsRangeY() + top), 2);

    QFont font;
    font.setPointSize(10);
    font.setBold(true);
    QGraphicsTextItem* text = scene()->addText(QString::number(yText), font);
    text->setPos(0, y - text->boundingRect().height() / 2);
    text->setOpacity(0.7);
}

void GraphView::drawVerticalLine(int x)
{
    QPen pen;
    pen.setWidth(2);

    auto line = scene()->addLine(x, 0, x, marginedHeight(), pen);
    line->setOpacity(0.4);
    auto xPosPixels = line->boundingRect().x() - leftMargin + 1;
    auto xText = round(((xPosPixels - marginedWidth()) / marginedWidth() * coordsRangeX() + _right), 2);

    QFont font;
    font.setPointSize(10);
    font.setBold(true);
    QGraphicsTextItem* text = scene()->addText(QString::number(xText), font);
    auto textRect = text->boundingRect();
    line->moveBy(0, textRect.height() + 5);
    text->setPos(x - textRect.width() / 2, 0);
    text->setOpacity(0.7);
    topMargin = textRect.height() + 5;
}
