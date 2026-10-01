#include "svgwidget.h"

#include <QPainter>
#include <QFileInfo>
#include <QDir>
#include <QMovie>
#include <QSvgRenderer>
#include <QTimer>
#include <QVector>
#include <QDebug>

SvgWidget::SvgWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(300, 160);
    setAttribute(Qt::WA_OpaquePaintEvent, false);

    m_gifTimer.setSingleShot(true);

    connect(&m_gifTimer,
            &QTimer::timeout,
            this,
            &SvgWidget::showNextGif);
}

void SvgWidget::setText(const QString &text)
{
    m_gifTimer.stop();

    for (CharacterRenderer &character : m_characters) {
        delete character.movie;
        delete character.svg;
    }

    qDebug() << "a";

    m_characters.clear();

    m_text = text;
    m_currentGif = -1;
    m_showingGifs = false;
    m_playing = false;

    int characterPos = 0;

    for (const QChar character : text) {
        if (characterPos >= 7)
            break;

        const QString codePoint =
            QString::number(character.unicode(), 10);

        const QString svgPath = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("svgs/%1-still.svg").arg(codePoint));
        //QStringLiteral(":/svgs/%1-still.svg").arg(codePoint);
        qDebug() << svgPath;

        const QString gifPath = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("gifs/%1.gif").arg(codePoint));
            //QStringLiteral(":/gifs/%1.gif").arg(codePoint);

        // Ignore characters for which neither file exists.
        if (!QFile::exists(svgPath) && !QFile::exists(gifPath))
            continue;

        auto *svg = QFile::exists(svgPath)
                        ? new QSvgRenderer(svgPath, this)
                        : nullptr;

        auto *movie = QFile::exists(gifPath)
                          ? new QMovie(gifPath, QByteArray(), this)
                          : nullptr;

        if (movie) {
            movie->setCacheMode(QMovie::CacheAll);

            connect(movie,
                    &QMovie::frameChanged,
                    this,
                    [this](int) {
                        update();
                    });
        }

        if (svg) {
            connect(svg,
                    &QSvgRenderer::repaintNeeded,
                    this,
                    QOverload<>::of(&SvgWidget::update));
        }

        m_characters.append({
            codePoint,
            movie,
            svg
        });

        ++characterPos;
    }

    update();
}

void SvgWidget::play()
{
    if (m_characters.isEmpty())
        return;

    m_playing = true;
    m_showingGifs = true;

    m_currentGif = -1;

    for (CharacterRenderer &character : m_characters) {
        if (character.movie) {
            character.movie->stop();
            character.movie->jumpToFrame(0);
        }

        if (character.svg) {
            character.svg->setAnimationEnabled(false);
            character.svg->setCurrentFrame(0);
        }
    }

    showNextGif();
}

void SvgWidget::showNextGif()
{
    if (!m_playing)
        return;

    ++m_currentGif;

    while (m_currentGif < m_characters.size() &&
           !m_characters[m_currentGif].movie) {
        ++m_currentGif;
    }

    // All GIFs have finished. Switch to the SVG display.
    if (m_currentGif >= m_characters.size()) {
        m_showingGifs = false;

        for (CharacterRenderer &character : m_characters) {
            if (character.movie)
                character.movie->stop();

            if (character.svg) {
                character.svg->setCurrentFrame(0);
                character.svg->setAnimationEnabled(true);
            }
        }

        update();
        return;
    }

    // Stop any previously active movie.
    for (int i = 0; i < m_characters.size(); ++i) {
        if (i != m_currentGif && m_characters[i].movie)
            m_characters[i].movie->stop();
    }

    QMovie *movie = m_characters[m_currentGif].movie;

    movie->jumpToFrame(0);
    movie->start();

    update();

    // Move to the next GIF after seven seconds.
    m_gifTimer.start(7000);
}

void SvgWidget::pause()
{
    m_playing = false;
    m_gifTimer.stop();

    for (CharacterRenderer &character : m_characters) {
        if (character.movie)
            character.movie->setPaused(true);

        if (character.svg)
            character.svg->setAnimationEnabled(false);
    }
}

void SvgWidget::reset()
{
    m_playing = false;
    m_gifTimer.stop();

    m_currentGif = -1;
    m_showingGifs = false;

    for (CharacterRenderer &character : m_characters) {
        if (character.movie) {
            character.movie->stop();
            character.movie->jumpToFrame(0);
        }

        if (character.svg) {
            character.svg->setAnimationEnabled(false);
            character.svg->setCurrentFrame(0);
        }
    }

    update();
}

QRectF SvgWidget::targetRect(int index, const QSizeF &sourceSize) const
{
    if (sourceSize.isEmpty())
        return {};

    const qreal margin = 0.0;
    const qreal gap = width() / 20.0;

    const qreal availableWidth =
        width() - 2.0 * margin;

    const qreal availableHeight =
        height();

    const qreal scale = qMin(
        availableHeight / sourceSize.height(),
        availableWidth / sourceSize.width()
        );

    const QSizeF renderedSize =
        sourceSize * scale * 0.35;

    qreal x = margin;

    for (int i = 0; i < index; ++i) {
        const CharacterRenderer &previous = m_characters[i];

        QSizeF previousSize;

        if (m_showingGifs && previous.movie) {
            previousSize = previous.movie->currentImage().size();
        } else if (previous.svg && previous.svg->isValid()) {
            previousSize = previous.svg->defaultSize();

            if (!previousSize.isValid() || previousSize.isEmpty())
                previousSize = previous.svg->viewBoxF().size();
        }

        if (!previousSize.isEmpty()) {
            const qreal previousScale = qMin(
                availableHeight / previousSize.height(),
                availableWidth / previousSize.width()
                );

            x += previousSize.width() *
                     previousScale *
                     0.35 +
                 gap;
        }
    }

    const qreal y =
        (height() - renderedSize.height()) / 5.0;

    return QRectF(x, y,
                  renderedSize.width(),
                  renderedSize.height());
}

void SvgWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (m_characters.isEmpty())
        return;

    if (m_showingGifs) {
        // Only the currently selected GIF is painted.
        if (m_currentGif < 0 ||
            m_currentGif >= m_characters.size()) {
            return;
        }

        QMovie *movie = m_characters[m_currentGif].movie;

        if (!movie || movie->currentImage().isNull())
            return;

        const QImage image = movie->currentImage();

        const QRectF target =
            targetRect(m_currentGif, image.size());

        painter.drawImage(target, image);
        return;
    }

    // SVG phase: paint all SVGs in their normal positions.
    for (int i = 0; i < m_characters.size(); ++i) {
        QSvgRenderer *renderer = m_characters[i].svg;

        if (!renderer || !renderer->isValid())
            continue;

        QSizeF svgSize = renderer->defaultSize();

        if (!svgSize.isValid() || svgSize.isEmpty())
            svgSize = renderer->viewBoxF().size();

        if (!svgSize.isValid() || svgSize.isEmpty())
            continue;

        const QRectF target = targetRect(i, svgSize);

        renderer->render(&painter, target);
    }
}

QSize SvgWidget::sizeHint() const
{
    return QSize(500, 220);
}