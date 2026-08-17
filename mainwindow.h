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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    //method to load the dictionary
    void loadDictionaryFile(const QString &filePath);

signals:
    void dictionaryLoaded(int entryCount);

private slots:
    // Connected to UI Designer buttons (defined below)

    void on_searchButton_clicked();

    void on_clearButton_clicked();

    void on_loadFileButton_clicked();

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

private:
    Ui::MainWindow *ui;

    QListWidgetItem *currentItem;
    QString dictFilePath = "";

    bool diacriticsEnabled = false;
    bool colorPinyinEnabled = false;
    bool HSKLevelEnabled = false;
};
#endif // MAINWINDOW_H
