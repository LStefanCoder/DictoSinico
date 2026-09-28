#include "svgwidget.h"

#include <QPainter>
#include <QFileInfo>
#include <QDir>

SvgWidget::SvgWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(300, 160);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
}

void SvgWidget::setText(const QString &text)
{
    for (const CharacterRenderer &character : m_characters) {
        delete character.renderer;
    }

    m_characters.clear();

    m_text = text;

    //making sure that only the first seven characters get rendered, if the word is longer
    int characterPos = 0;

    for (const QChar character : text) {
        const QString codePoint =
            QString::number(character.unicode(), 10);

        //the path to the corresponding character file
        const QString resourcePath =
            QStringLiteral(":/svgs/%1.svg").arg(codePoint);

        //skipping the character, if it doesn't have a corresponding SVG file . It will skip for any characters which are not hanzi.
        if (!QFile::exists(resourcePath))
        {
            continue;
        }

        auto *renderer = new QSvgRenderer(resourcePath, this);

        connect(renderer, &QSvgRenderer::repaintNeeded,
                this, QOverload<>::of(&SvgWidget::update));

        m_characters.append({renderer});
    }

    update();
}

void SvgWidget::play()
{
for (const CharacterRenderer &character : m_characters)
    character.renderer->setAnimationEnabled(true);

update();
}

void SvgWidget::pause()
{
for (const CharacterRenderer &character : m_characters)
    character.renderer->setAnimationEnabled(false);
}

void SvgWidget::reset()
{
    for (const CharacterRenderer &character : m_characters) {
        QSvgRenderer *renderer = character.renderer;

        if (renderer->animated()) {
            renderer->setCurrentFrame(0);
        }
    }

    update();
}

/*void SvgWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_characters.isEmpty())
        return;

    const int spacing = 12;
    const int characterWidth =
        (width() - spacing * (m_characters.size() - 1)) /
        (m_characters.size() * 2);

    const QRectF targetArea(
        0,
        0,
        characterWidth,
        height() / 2
        );

    for (int i = 0; i < m_characters.size(); ++i) {
        const int x = i * (characterWidth + spacing);

        QRectF target(
            x,
            0,
            characterWidth,
            height() / 2
            );

        m_characters[i].renderer->render(&painter, target);
    }
}*/

/*void SvgWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (m_characters.isEmpty())
        return;

    const int spacing = 12;
    const int count = m_characters.size();

    const qreal cellWidth =
        (width() - spacing * (count - 1)) / qreal(count);

    const qreal cellHeight = height();

    for (int i = 0; i < count; ++i) {
        QSvgRenderer *renderer = m_characters[i].renderer;
        if (!renderer || !renderer->isValid())
            continue;

        QRectF cell(
            i * (cellWidth + spacing),
            0,
            cellWidth,
            cellHeight
            );

        // Get the SVGs intrinsic dimensions.
        QSizeF svgSize = renderer->defaultSize();

        if (!svgSize.isValid() || svgSize.isEmpty())
            svgSize = renderer->viewBoxF().size();

        if (!svgSize.isValid() || svgSize.isEmpty())
            continue;

        // Scale uniformly so the entire SVG fits in the cell.
        const qreal scale = qMin(
            cell.width()  / svgSize.width(),
            cell.height() / svgSize.height()
            );

        const QSizeF renderedSize = svgSize * scale * 0.2;

        // Center the SVG inside the cell.
        QRectF target(
            //cell.center().x() - renderedSize.width() / 2.0,
            cell.left(),
            cell.center().y() - renderedSize.height() / 2.0 - 20,
            renderedSize.width(),
            renderedSize.height()
            );

        renderer->render(&painter, target);
    }
}*/

void SvgWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int count = m_characters.size();
    if (count == 0)
        return;

    const qreal margin = 0.0;
    const qreal availableWidth = width() - 2.0 * margin;
    const qreal availableHeight = height();

    QVector<QSizeF> renderedSizes;
    renderedSizes.reserve(count);

    qreal totalSvgWidth = 0.0;

    // Calculate the size of every SVG while preserving its aspect ratio.
    for (const auto &character : m_characters) {
        QSvgRenderer *renderer = character.renderer;

        if (!renderer || !renderer->isValid()) {
            renderedSizes.append(QSizeF());
            continue;
        }

        QSizeF svgSize = renderer->defaultSize();

        if (!svgSize.isValid() || svgSize.isEmpty())
            svgSize = renderer->viewBoxF().size();

        if (!svgSize.isValid() || svgSize.isEmpty()) {
            renderedSizes.append(QSizeF());
            continue;
        }

        const qreal scale = qMin(
            availableHeight / svgSize.height(),
            availableWidth / svgSize.width()
            );

        QSizeF renderedSize = svgSize * scale * 0.35;
        renderedSizes.append(renderedSize);
        totalSvgWidth += renderedSize.width();
    }

    // Equal gap between the visible SVGs.
    /*const qreal gap = count > 1
                          ? qMax(0.0, (availableWidth - totalSvgWidth) / (count - 1))
                          : 0.0;*/
    const qreal gap = width() / 20;

    qreal x = margin;

    for (int i = 0; i < count; ++i) {
        const QSizeF size = renderedSizes[i];

        if (size.isEmpty())
            continue;

        // Vertically center each SVG.
        const qreal y = (height() - size.height()) / 5.0;

        QRectF target(x, y, size.width(), size.height());
        m_characters[i].renderer->render(&painter, target);

        x += size.width() + gap;
    }
}

QSize SvgWidget::sizeHint() const
{
    return QSize(500, 220);
}