#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QListWidget>
#include <QChar>

#include <QTableWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class svgWidget; //the widget for drawing the makemeahanzi animated svg files

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    //method to load the dictionary
    void loadDictionaryFile(const QString &filePath, const QString &filePath2);

signals:
    void dictionaryLoaded(int entryCount);

private slots:
    // Connected to the buttons of the

    void on_searchButton_clicked();
    void on_clearButton_clicked();

    //the two buttons of the character svg animation
    void on_animationStartButton_clicked();
    void on_animationStopButton_clicked();

    //the two radio buttons
    void on_simplifiedRadio_clicked();
    void on_traditionalRadio_clicked();


    void on_TableSelectionChanged();

    void on_actionAbout_triggered(); // name must match your Designer action objectName; calling the about window from the addressbar

    void on_actionPreferences_triggered();

    void on_diacriticsFeatureChanged(bool enabled);
    void on_pinyinColorChanged(bool enabled);
    void on_HSKChanged(bool enabled);


    QChar toneMarkVowel(QChar base, int tone);
    QString numberedPinyinToDiacritics(QString s);
    QString convertToColoredSpan(QString s, int n);
    bool isValidPinyin(QString s);
    QVector<QPair<QString, int>> loadCsv_col1Text_col2Int(const QString& filePath);
    QString getHSKNumber(QString simp);


private:
    Ui::MainWindow *ui;
    svgWidget *hanziWidget;

    QListWidgetItem *currentItem;
    QString dictFilePath = "";

    QVector<QPair<QString, int>> HSKTableRows;

    bool diacriticsEnabled = false;
    bool colorPinyinEnabled = false;
    bool HSKLevelEnabled = false;
    //the two variables for the two radio buttons controlling whether the simplified or the traditional SVG files are displayed
    bool simplifiedDisplayed = true;
    bool traditionalDisplayed = false;
};
#endif // MAINWINDOW_H
