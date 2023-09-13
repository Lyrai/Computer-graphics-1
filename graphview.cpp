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
    setRenderHint(QPainter::Antialiasing);
    resize(500, 500);

    points.reserve(1920);
}

void GraphView::paintEvent(QPaintEvent *event)
{
    scene()->clear();
    scene()->setSceneRect(0, 0, size().width(), size().height());
    pixelsPerStep = size().width() / 100 - 4;
    if(pixelsPerStep < 1) {
        pixelsPerStep = 1;
    }

    calculate();

    drawGrid();

    auto transformed = QVector<QPoint>(points.size());
    auto firstTarget = [&transformed, this]() {
        std::transform(points.begin(), points.begin() + points.size() / 4, transformed.begin(), [this](auto point) { return mapToScreen(point); });
    };
    auto secondTarget = [&transformed, this]() {
        std::transform(points.begin() + points.size() / 4, points.begin() + points.size() / 4 * 2, transformed.begin() + transformed.size() / 4, [this](auto point) { return mapToScreen(point); });
    };
    auto thirdTarget = [&transformed, this]() {
        std::transform(points.begin() + points.size() / 4 * 2, points.begin() + points.size() / 4 * 3, transformed.begin() + transformed.size() / 4 * 2, [this](auto point) { return mapToScreen(point); });
    };
    auto fourthTarget = [&transformed, this]() {
        std::transform(points.begin() + points.size() / 4 * 3, points.end(), transformed.begin() + transformed.size() / 4 * 3, [this](auto point) { return mapToScreen(point); });
    };
    std::thread first(firstTarget);
    std::thread second(secondTarget);
    std::thread third(thirdTarget);
    std::thread fourth(fourthTarget);

    first.join();
    second.join();
    third.join();
    fourth.join();

    for(int i = 0; i < transformed.size() - 1; ++i) {
        drawLine(transformed[i], transformed[i + 1]);
    }

    QGraphicsView::paintEvent(event);
}

void GraphView::calculate()
{
    if(!expr.isValid()) {
        return;
    }
    points.resize(0);
    top = std::numeric_limits<double>::infinity();
    bottom = -std::numeric_limits<double>::infinity();
    auto steps = mapFromScreen(marginedWidth(), size().height());
    auto xStep = steps.x();

    for(double i = _left; i < _right; i += xStep * pixelsPerStep) {
        auto value = expr.evaluate(i);
        points.push_back(QPointF(i, value));
        if(value < top) {
            top = value;
        }

        if(value > bottom) {
            bottom = value;
        }
    }
    points.push_back(QPointF(_right, expr.evaluate(_right)));
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
    scene()->addLine(begin.x(), begin.y() - 1, end.x(), end.y() - 1, pen);
}

double round(double value, int places) {
    if(places == 0) {
        return (int)value;
    }
    int multiplier = 1;

    for(int i = 0; i < places; ++i) {
        multiplier *= 10;
    }

    return ceil(value * multiplier) / (double)multiplier;
}

void GraphView::drawGrid()
{
    constexpr auto stepPixels = 100;

    for(int i = (size().height() - bottomMargin) / stepPixels; i > 0; --i) {
        drawHorizontalLine(stepPixels * i);
    }

    leftMargin = 50;

    for(int i = (size().width() - rightMargin) / stepPixels; i > 0; --i) {
        drawVerticalLine(stepPixels * i);
    }

    drawHorizontalLine(topMargin);
    drawHorizontalLine(size().height() - bottomMargin);
    drawVerticalLine(leftMargin);
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
    auto text = scene()->addSimpleText(QString::number(yText), font);
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
    auto text = scene()->addSimpleText(QString::number(xText), font);
    auto textRect = text->boundingRect();
    line->moveBy(0, textRect.height() + 5);
    text->setPos(x - textRect.width() / 2, 0);
    text->setOpacity(0.7);
    topMargin = textRect.height() + 5;
}
