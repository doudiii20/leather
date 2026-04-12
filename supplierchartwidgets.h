#ifndef SUPPLIERCHARTWIDGETS_H
#define SUPPLIERCHARTWIDGETS_H

#include <QPair>
#include <QVector>
#include <QWidget>

class SupplierBarChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SupplierBarChartWidget(QWidget *parent = nullptr);
    void setBars(const QVector<QPair<QString, int>> &items);
    void setChartTitle(const QString &title);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<QPair<QString, int>> m_items;
    QString m_title;
};

class SupplierPieChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SupplierPieChartWidget(QWidget *parent = nullptr);
    void setSlices(const QVector<QPair<QString, int>> &slices);
    void setChartTitle(const QString &title);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<QPair<QString, int>> m_slices;
    QString m_title;
};

#endif // SUPPLIERCHARTWIDGETS_H
