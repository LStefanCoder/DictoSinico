#ifndef SVGWIDGET_H
#define SVGWIDGET_H

#pragma once

#include <QWidget>
#include <QSvgRenderer>
#include <QVector>
#include <QString>

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

private:
    struct CharacterRenderer
    {
        QSvgRenderer *renderer = nullptr;
    };

    QVector<CharacterRenderer> m_characters;
    QString m_text;
};

#endif // SVGWIDGET_H

    //explicit StrokeAnimationWidget(QWidget *parent = nullptr);

