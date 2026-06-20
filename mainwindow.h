#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QListWidget>

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

    void onAboutClicked(); //calling the about window from the addressbar

private:
    Ui::MainWindow *ui;

    QListWidgetItem *currentItem;
    QString dictFilePath = "";
};
#endif // MAINWINDOW_H
