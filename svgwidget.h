#ifndef SVGWIDGET_H
#define SVGWIDGET_H

#pragma once

#include <QWidget>
#include <QSvgRenderer>
#include <QVector>
#include <QString>
#include <QMovie>
#include <QSvgRenderer>
#include <QTimer>
#include <QVector>

class SvgWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SvgWidget(QWidget *parent = nullptr);
    void setText(const QString &text);
    void play();
    void pause();
    void reset();

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;

private slots:
    void showNextGif();

private:
    struct CharacterRenderer
    {
        QString baseName;
        QMovie *movie = nullptr;
        QSvgRenderer *svg = nullptr;
    };

    QRectF targetRect(int index, const QSizeF &sourceSize) const;

    QVector<CharacterRenderer> m_characters;

    QTimer m_gifTimer;
    int m_currentGif = -1;
    bool m_showingGifs = false;
    bool m_playing = false;

    QString m_text;
};

#endif // SVGWIDGET_H

    //explicit StrokeAnimationWidget(QWidget *parent = nullptr);

