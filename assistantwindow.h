#ifndef ASSISTANTWINDOW_H
#define ASSISTANTWINDOW_H

#include <QMainWindow>
#include <QDate>
#include <QHash>
#include <QList>
#include <QMap>
#include <QStringList>

class QCalendarWidget;
class QComboBox;
class QLabel;
class QListWidget;
class QNetworkAccessManager;
class QNetworkReply;
class QLineEdit;
class QPushButton;
class QToolButton;
class QVBoxLayout;

struct CalendarEvent
{
    QString title;
    QDate date;
    QString category;
    int demandScore = 0;
    double expectedSales = 0.0;
};

class AssistantWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AssistantWindow(QWidget *parent = nullptr);

private slots:
    void onDateChanged();
    void onCategoryChanged();
    void onThemeToggled();
    void onApiReplyFinished(QNetworkReply *reply);
    void loadCalendarFromApi();

private:
    QCalendarWidget *m_calendar = nullptr;
    QLabel *m_monthSummaryLabel = nullptr;
    QLabel *m_notificationsLabel = nullptr;
    QLabel *m_predictedSalesLabel = nullptr;
    QLabel *m_apiStatusLabel = nullptr;
    QListWidget *m_recommendationsList = nullptr;
    QComboBox *m_categoryFilter = nullptr;
    QLineEdit *m_apiUrlInput = nullptr;
    QPushButton *m_syncApiButton = nullptr;
    QToolButton *m_themeButton = nullptr;
    QNetworkAccessManager *m_networkManager = nullptr;

    QList<CalendarEvent> m_events;
    QMap<QDate, double> m_pastSalesByDate;
    QHash<QDate, int> m_busyScoreByDate;
    QStringList m_categories;
    bool m_isDarkMode = false;

    void buildUi();
    void initializeData();
    void simulateBusySeasons(const QDate &monthDate);
    void refreshCalendarAppearance();
    void refreshInsights();
    void updateRecommendations();
    void updateNotifications();
    void applyTheme(bool darkMode);
    double predictSales(const QDate &date, const QString &category) const;
    bool eventMatchesCurrentCategory(const CalendarEvent &event) const;
};

#endif // ASSISTANTWINDOW_H
