#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "preferencesdialog.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QtWidgets/QApplication>
#include <QtWidgets/QTableWidget>
#include <QTableWidget>
#include <QListWidget>
#include <QWidget>
#include <QAbstractScrollArea>
#include <QMessageBox>
//#include <QApplication>

//themes to be downloaded here: https://qt-material.readthedocs.io/en/latest/
//https://github.com/hubenchang0515/QtTheme
//https://qss-stock.devsecstudio.com/documentation.php


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->searchButton, &QPushButton::clicked, this, &MainWindow::on_searchButton_clicked);
    connect(ui->loadFileButton, &QPushButton::clicked, this, &MainWindow::on_loadFileButton_clicked);
    //this is for when a row of the table was selected
    connect(ui->tableWidget, &QTableWidget::itemSelectionChanged,
            this, &MainWindow::on_TableSelectionChanged);

    ui->lineEdit->setText("");

    // Setup Table Widget (Better than QListWidget for structured data)
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setColumnCount(4);  //there are 4 columns needed

    // Optional: Set horizontal headers
    /*QStringList headers;
    headers << "Traditional" << "Simplified" << "Pinyin" << "Definition";
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->setVisible(false);  // Hide row numbers*/

    QStringList headers = {"Traditional", "Simplified", "Pinyin", "Definition"};
    ui->tableWidget->setHorizontalHeaderLabels(headers);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->setVisible(false);  // Hide row numbers
}

MainWindow::~MainWindow()
{
    delete ui;
}

static bool parseCedictLine(const QString &line,
                            QString &traditional,
                            QString &simplified,
                            QString &pinyin,
                            QStringList &meanings)
{
    if (line.isEmpty() || line.startsWith('#'))
        return false;

    int openBracket = line.indexOf('[');
    int closeBracket = line.indexOf(']');
    int firstSlash = line.indexOf('/');
    int lastSlash = line.lastIndexOf('/');

    if (openBracket < 0 || closeBracket < 0 || firstSlash < 0 || lastSlash <= firstSlash)
        return false;

    QString head = line.left(openBracket).trimmed();
    QStringList headParts = head.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (headParts.size() < 2)
        return false;

    traditional = headParts[0];
    simplified  = headParts[1];
    pinyin      = line.mid(openBracket + 1, closeBracket - openBracket - 1).trimmed();

    QString meaningBlock = line.mid(firstSlash + 1, lastSlash - firstSlash - 1);
    meanings = meaningBlock.split('/', Qt::SkipEmptyParts);

    for (QString &m : meanings)
        m = m.trimmed();

    return true;
}

void MainWindow::loadDictionaryFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot open dictionary file!");
        return;
    }

    QTextStream in(&file);
    //converting everything to Unicode UTF-8
    in.setEncoding(QStringConverter::Utf8);

    ui->tableWidget->clear();

    //line added for the implementation
    ui->tableWidget->setRowCount(0);

    QList<QTableWidgetItem*> items = ui->tableWidget->selectedItems();
    foreach (QTableWidgetItem *item, items) {
        item->setSelected(false);
    }

    while (!in.atEnd()) {
        QString line = in.readLine();

        QString traditional, simplified, pinyin;
        QStringList meanings;

        // Skip comments and empty lines
        if (line.trimmed().startsWith("#") || line.trimmed().isEmpty()) {
            continue;
        }

        //if the line does not contain the required format and is therefore not parsable,
        //, continue
        if (!parseCedictLine(line, traditional, simplified, pinyin, meanings))
            continue;


        // Parse CC-CEDICT format. Since CC-CEDICT has no delimiters,

        //the order is: traditional, simplified, pinyin in brackets and definition in slashes

        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);

        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(traditional));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(simplified));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(pinyin));
        ui->tableWidget->setItem(row, 3, new QTableWidgetItem(meanings.join("; ")));
    }

    //setting the headers again, otherwise the headers would get replaced by the default numbers
    QStringList headers = {"Traditional", "Simplified", "Pinyin", "Definition"};
    ui->tableWidget->setHorizontalHeaderLabels(headers);

    file.close();

    emit dictionaryLoaded(ui->tableWidget->rowCount());

}

void MainWindow::on_searchButton_clicked()
{
    //if there has been a search made already, lines are hidden
    //unhiding all hidden rows
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i)
        ui->tableWidget->showRow(i);

    QString query = ui->lineEdit->text().trimmed();

    if (query.isEmpty()) {
        QMessageBox::information(this, "Info", "Please enter a search term.");
        return;
    }


    // Clear previous selection before new search
    QList<QTableWidgetItem*> selectedItems = ui->tableWidget->selectedItems();
    foreach (QTableWidgetItem *item, selectedItems) {
        item->setSelected(false);
    }

    //int currentIndex = ui->tableWidget->currentIndex();
    bool found = false;

    // Since we are reading from memory, search is fast for standard RAM usage
    // If you want to highlight the query inside the words:
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        for(int j = 0; j < ui->tableWidget->columnCount(); ++j)
        {

        }
        QTableWidgetItem *w = ui->tableWidget->item(i, 0);

        if (w && w->text().contains(query, Qt::CaseInsensitive)) {
            // Select the entire row when matching word is found
            for (int c = 0; c < 4; ++c) {  // Select all columns in matched row
                QTableWidgetItem *rowItem = ui->tableWidget->item(i, c);
                if (rowItem) {
                    //rowItem->setSelected(true);
                    found = true;
                }
            }

            // scroll to match
            ui->tableWidget->scrollToItem(ui->tableWidget->item(i, 0));
        }

        //hide the row if it doesn't contain the search term
        else
        {
            ui->tableWidget->hideRow(i);
        }
    }

    // Show info if no matches or list empty
    if (ui->tableWidget->rowCount() == 0) {
        QMessageBox::information(this, "Info", "Dictionary not loaded. Load a file first.");
    } else if (!found && query.size() > 0) {
        QMessageBox::information(this, "Info", "No matching entries found.");
    }
}


void MainWindow::on_clearButton_clicked()
{
    //unhiding all hidden rows
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i)
        ui->tableWidget->showRow(i);

    ui->lineEdit->clear();

    QList<QTableWidgetItem*> selectedItems = ui->tableWidget->selectedItems();
    foreach (QTableWidgetItem *item, selectedItems) {
        item->setSelected(false);
    }
}


void MainWindow::on_loadFileButton_clicked()
{
    // Open file dialog to select CC-CEDICT file
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select CC-CEDICT Dictionary File",
        "",
        "Text Files (*.txt)");

    if (!filePath.isEmpty()) {
        loadDictionaryFile(filePath);
        dictFilePath = filePath; // Store path for future reference
    }
}

void MainWindow::on_TableSelectionChanged()
{
    //clear the labels if there is nothing is selected
    if (!ui->tableWidget->selectionModel()->hasSelection()) {
        ui->definitionEng->clear();
        ui->definitionPinyin->clear();
        ui->definitionSimp->clear();
        ui->definitionTrad->clear();
        return;
    }

    // get all selected indexes; require exactly one cell selected (optional)
    QModelIndexList indexes = ui->tableWidget->selectionModel()->selectedIndexes();
    if (indexes.isEmpty()) {
        ui->definitionEng->clear();
        ui->definitionPinyin->clear();
        ui->definitionSimp->clear();
        ui->definitionTrad->clear();
        return;
    }

    // use row of the first selected index
    int row = indexes.first().row();

    auto textAt = [this, row](int col) -> QString {
        QTableWidgetItem *it = ui->tableWidget->item(row, col);
        return it ? it->text() : QString();
    };

    ui->definitionEng->setText(textAt(0));
    ui->definitionPinyin->setText(textAt(1));
    ui->definitionSimp->setText(textAt(2));
    ui->definitionTrad->setText(textAt(3));
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About"),
                       tr("CeDict GUI\nVersion 1.0\n\nShort description..."));
}

void MainWindow::on_actionPreferences_triggered()
{
    PreferencesDialog dlg(this);
    dlg.exec(); // modal
}