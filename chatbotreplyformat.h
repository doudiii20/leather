#pragma once

#include <QString>
#include <QStringList>

/// Insère des sauts de ligne avant les libellés « Vous : » / « Assistant : » que le modèle
/// colle souvent au paragraphe précédent (ex. « ...domaines.**Vous :** … **Assistant :** … »).
inline QString formatChatbotDialogueLineBreaks(QString t)
{
    const QStringList markers = {
        QStringLiteral("**Vous :**"),
        QStringLiteral("**Vous:**"),
        QStringLiteral("**Assistant :**"),
        QStringLiteral("**Assistant:**"),
        QStringLiteral("*Vous :*"),
        QStringLiteral("*Vous:*"),
        QStringLiteral("*Assistant :*"),
        QStringLiteral("*Assistant:*"),
    };
    for (const QString &m : markers) {
        int from = 0;
        while ((from = t.indexOf(m, from)) >= 0) {
            if (from > 0) {
                const QChar prev = t.at(from - 1);
                if (prev != QLatin1Char('\n') && prev != QLatin1Char('\r')) {
                    t.insert(from, QStringLiteral("\n\n"));
                    from += m.size() + 2;
                    continue;
                }
            }
            from += m.size();
        }
    }
    return t;
}
