#include "supplierchartwidgets.h"

#include <QPainter>

namespace {
static const QColor kBrown(88, 41, 0);
static const QColor kBarFill(180, 120, 70);
static const QColor kBg(252, 248, 240);

static QVector<QColor> pieColors()
{
    return {QColor(139, 90, 43), QColor(205, 133, 63), QColor(222, 184, 135), QColor(160, 82, 45),
            QColor(210, 105, 30), QColor(184, 134, 11), QColor(139, 69, 19),  QColor(205, 92, 92)};
}
} // namespace

SupplierBarChartWidget::SupplierBarChartWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumHeight(120);
}

void SupplierBarChartWidget::setBars(const QVector<QPair<QString, int>> &items)
{
    m_items = items;
    update();
}

void SupplierBarChartWidget::setChartTitle(const QString &title)
{
    m_title = title;
    update();
}

void SupplierBarChartWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), kBg);
    const int w = width(), h = height();
    const int top = 8, titleH = 22, bottomLab = 36, marginX = 10;
    p.setPen(kBrown);
    p.setFont(QFont(font().family(), 10, QFont::Bold));
    p.drawText(QRect(marginX, top, w - 2 * marginX, titleH), Qt::AlignLeft | Qt::AlignVCenter,
               m_title.isEmpty() ? QStringLiteral("Commandes par fournisseur") : m_title);
    const int chartTop = top + titleH + 4;
    const int chartBottom = h - bottomLab;
    const int chartH = qMax(20, chartBottom - chartTop);
    if (m_items.isEmpty()) {
        p.setFont(QFont(font().family(), 9));
        p.drawText(rect(), Qt::AlignCenter, QStringLiteral("Aucune donnee"));
        return;
    }
    int maxV = 1;
    for (const auto &pr : m_items) maxV = qMax(maxV, pr.second);
    const int n = m_items.size();
    const double slot = double(w - 2 * marginX) / double(qMax(1, n));
    const double barW = qMax(6.0, slot * 0.55);
    const double gap = (slot - barW) * 0.5;
    p.setFont(QFont(font().family(), 8));
    for (int i = 0; i < n; ++i) {
        const double x0 = marginX + i * slot + gap;
        const int v = m_items[i].second;
        const double bh = chartH * (double(v) / double(maxV));
        const double y0 = chartBottom - bh;
        p.setPen(kBrown);
        p.setBrush(kBarFill);
        p.drawRoundedRect(QRectF(x0, y0, barW, bh), 3, 3);
        QString lab = m_items[i].first;
        if (lab.size() > 10) lab = lab.left(9) + QChar(0x2026);
        p.setPen(kBrown);
        p.drawText(QRect(int(x0 - 4), chartBottom + 2, int(barW + 8), bottomLab - 4),
                   Qt::AlignHCenter | Qt::TextWordWrap, lab);
    }
    p.setPen(QColor(200, 180, 160));
    p.drawLine(marginX, chartBottom, w - marginX, chartBottom);
}

SupplierPieChartWidget::SupplierPieChartWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumHeight(120);
}

void SupplierPieChartWidget::setSlices(const QVector<QPair<QString, int>> &slices)
{
    m_slices = slices;
    update();
}

void SupplierPieChartWidget::setChartTitle(const QString &title)
{
    m_title = title;
    update();
}

void SupplierPieChartWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), kBg);
    const int w = width(), h = height();
    const int margin = 8, titleH = 22;
    p.setPen(kBrown);
    p.setFont(QFont(font().family(), 10, QFont::Bold));
    p.drawText(QRect(margin, margin, w - 2 * margin, titleH), Qt::AlignLeft | Qt::AlignVCenter,
               m_title.isEmpty() ? QStringLiteral("Repartition des commandes") : m_title);
    int sum = 0;
    for (const auto &pr : m_slices) sum += pr.second;
    const int chartTop = margin + titleH + 4;
    const QRect full(margin, chartTop, w - 2 * margin, h - chartTop - margin);
    const int pieSide = qMin(full.height(), full.width() * 5 / 9);
    const QRect pieRect(full.left(), full.top(), pieSide, pieSide);
    if (sum <= 0 || m_slices.isEmpty()) {
        p.setFont(QFont(font().family(), 9));
        p.drawText(pieRect, Qt::AlignCenter, QStringLiteral("Sans commandes"));
        return;
    }
    const QVector<QColor> cols = pieColors();
    qreal angle = 0;
    for (int i = 0; i < m_slices.size(); ++i) {
        const int v = m_slices[i].second;
        if (v <= 0) continue;
        const qreal span = 360.0 * qreal(v) / qreal(sum);
        p.setBrush(cols[i % cols.size()]);
        p.setPen(Qt::white);
        p.drawPie(pieRect, int(angle * 16.0), int(span * 16.0));
        angle += span;
    }
}
