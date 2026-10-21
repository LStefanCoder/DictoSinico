#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "preferencesdialog.h"
#include "loadingdialog.h"
#include "svgwidget.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QChar>
#include <QMessageBox>
#include <QFileDialog>
#include <QtWidgets/QApplication>
#include <QtWidgets/QTableWidget>
#include <QTableWidget>
#include <QListWidget>
#include <QWidget>
#include <QAbstractScrollArea>
#include <QMessageBox>
#include <QVector>

#include <QDir>
#include <QFileInfo>
#include <QVBoxLayout>

//for the loading window
#include <QFutureWatcher>
#include <QtConcurrent>

#include <QFont>
#include <QFontDatabase>

//themes can be downloaded here: https://qt-material.readthedocs.io/en/latest/
//https://github.com/hubenchang0515/QtTheme
//https://qss-stock.devsecstudio.com/documentation.php


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->searchButton, &QPushButton::clicked, this, &MainWindow::on_searchButton_clicked);
    //connecting the svg view buttons
    connect(ui->animationStartButton, &QPushButton::clicked, this, &MainWindow::on_animationStartButton_clicked);
    connect(ui->animationStopButton, &QPushButton::clicked, this, &MainWindow::on_animationStopButton_clicked);
    //the two radio buttons
    connect(ui->simplifiedRadioButton, &QRadioButton::toggled, this, &MainWindow::on_simplifiedRadio_clicked);
    //making sure that there is already an option selected at startup
    ui->simplifiedRadioButton->setChecked(true);
    connect(ui->traditionalRadioButton, &QRadioButton::toggled, this, &MainWindow::on_traditionalRadio_clicked);


    //this is for when a row of the table was selected
    connect(ui->tableWidget, &QTableWidget::itemSelectionChanged,
            this, &MainWindow::on_tableSelection_changed);

    ui->searchField->setText("");

    // Setup Table Widget (Better than QListWidget for structured data)
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setColumnCount(4);  //there are 4 columns needed

    //setting headers
    QStringList headers = {"Traditional", "Simplified", "Pinyin", "Definition"};
    ui->tableWidget->setHorizontalHeaderLabels(headers);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->setVisible(false);  // Hide row numbers

    //searching when pressing enter
    connect(ui->searchField, &QLineEdit::returnPressed, this, &MainWindow::on_searchButton_clicked);

    //loading the fonts
    //setting font types
    QFont fontLatin(familyLatin);
    QFont fontSimplified(familySimplified);
    QFont fontTraditional(familyTraditional);
    //because the definiton sometimes has Chinese words in it, and the other font looks ugly, it is set to the Chinese font
    //a separate QFont is needed due to the different point sizes
    QFont fontEnglishDefinition(familySimplified);

    fontLatin.setPointSize(14);
    fontSimplified.setPointSize(20);
    fontTraditional.setPointSize(20);
    fontEnglishDefinition.setPointSize(14);

    ui->definitionEng->setFont(fontEnglishDefinition);
    ui->definitionPinyin->setFont(fontLatin);
    ui->definitionSimp->setFont(fontSimplified);
    ui->definitionTrad->setFont(fontTraditional);
    ui->hskValue->setFont(fontLatin);

    ui->definitionEng->setWordWrap(true);
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

void MainWindow::loadDictionaryFile(const QString &filePath, const QString &filePath2)
{
    //creating the logic for the loading window to show during the loading of the dictionary
    auto* loading = new LoadingDialog(this);

    //showing the dialog
    loading->show();

    //keeping the dialog alive until work finishes
    auto* watcher = new QFutureWatcher<LoadResult>(this);

    connect(watcher, &QFutureWatcher<LoadResult>::finished, this, [this, watcher, loading]() {
        const LoadResult result = watcher->result();

        //HSKTableRows = result.hskRows; // assign on the GUI thread

        // Parse result.data1 / result.data2 and update widgets here.

        loading->close();
        loading->deleteLater();
        watcher->deleteLater();
    });

    QFile file(filePath);

    watcher->setFuture(QtConcurrent::run([this, filePath, filePath2]() {
        LoadResult result;
        result.hskRows = this->loadCsv_col1Text_col2Int(filePath2);
        HSKTableRows = result.hskRows; // assign on the GUI thread
        return result;
    }));

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

        //skip comments and empty lines
        if (line.trimmed().startsWith("#") || line.trimmed().isEmpty()) {
            continue;
        }

        //continue if the line cannot be parsed
        if (!parseCedictLine(line, traditional, simplified, pinyin, meanings))
            continue;


        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);

        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(traditional));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(simplified));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(pinyin));
        ui->tableWidget->setItem(row, 3, new QTableWidgetItem(meanings.join("; ")));

        //setting the fonts of the table
        QFont fontLatin(familyLatin);
        QFont fontSimplified(familySimplified);
        QFont fontTraditional(familyTraditional);

        fontLatin.setPointSize(11);
        fontSimplified.setPointSize(11);
        fontTraditional.setPointSize(11);

        QTableWidgetItem *w = ui->tableWidget->item(row, 0);
        QTableWidgetItem *w2 = ui->tableWidget->item(row, 1);
        QTableWidgetItem *w3 = ui->tableWidget->item(row, 2);
        QTableWidgetItem *w4 = ui->tableWidget->item(row, 3);

        w->setFont(fontTraditional);
        w2->setFont(fontSimplified);
        w3->setFont(fontLatin);
        w4->setFont(fontLatin);

        //clearing the bold from the heading of the first column
        QFont headerFont = ui->tableWidget->horizontalHeader()->font();
        headerFont.setBold(false);
        ui->tableWidget->horizontalHeader()->setFont(headerFont);
    }

    //setting the headers again, otherwise the headers would get replaced by the default numbers
    QStringList headers = {"Traditional", "Simplified", "Pinyin", "Definition"};
    ui->tableWidget->setHorizontalHeaderLabels(headers);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    file.close();

    emit dictionaryLoaded(ui->tableWidget->rowCount());

}

void MainWindow::on_searchButton_clicked()
{
    //if there has been a search made already, lines are hidden
    //unhiding all hidden rows
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i)
        ui->tableWidget->showRow(i);

    QString query = ui->searchField->text().trimmed();

    if (query.isEmpty()) {
        QMessageBox::information(this, "Info", "Please enter a search term.");
        return;
    }

    // Clear previous selection before new search
    QList<QTableWidgetItem*> selectedItems = ui->tableWidget->selectedItems();
    foreach (QTableWidgetItem *item, selectedItems) {
        item->setSelected(false);
    }

    bool found = false;

    // Since we are reading from memory, search is fast for standard RAM usage
    // if you want to highlight the query inside the words:
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {

        QTableWidgetItem *w = ui->tableWidget->item(i, 0);
        QTableWidgetItem *w2 = ui->tableWidget->item(i, 1);
        QTableWidgetItem *w3 = ui->tableWidget->item(i, 2);
        QTableWidgetItem *w4 = ui->tableWidget->item(i, 3);

        if ((w && w->text().contains(query, Qt::CaseInsensitive)) || (w2 && w2->text().contains(query, Qt::CaseInsensitive)) || (w3 && w3->text().contains(query, Qt::CaseInsensitive)) || (w4 && w4->text().contains(query, Qt::CaseInsensitive))) {
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

    ui->searchField->clear();

    QList<QTableWidgetItem*> selectedItems = ui->tableWidget->selectedItems();
    foreach (QTableWidgetItem *item, selectedItems) {
        item->setSelected(false);
    }
}


void MainWindow::on_tableSelection_changed()
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

    ui->definitionEng->setText(textAt(3));
    if (diacriticsEnabled == false)
    {
        if (colorPinyinEnabled == false)
        {
            ui->definitionPinyin->setText(textAt(2));
        }
        else
        {
            //QString pinyinString = textAt(2);
            QStringList pinyinParts = textAt(2).split(QChar(' '));
            QStringList coloredPinyinParts = {};
            for (int i=0; i < pinyinParts.length(); i++)
            {
                //checking each syllable for a valid pinyin string
                if (isValidPinyin(pinyinParts[i]) == true)
                {
                    //getting the last character of the syllable and converting it to an integer
                    QChar intAtTheEnd = pinyinParts[i].back();
                    //qDebug() << intAtTheEnd;
                    int lastDigit = intAtTheEnd.digitValue();

                    coloredPinyinParts.append(convertToColoredSpan(pinyinParts[i], lastDigit));
                }

                else
                {
                    coloredPinyinParts.append(pinyinParts[i]);
                }

            }

            //unifying the parts of the stringlist into one string
            QString coloredPinyinTogether = coloredPinyinParts.join(", ");
            ui->definitionPinyin->setText(coloredPinyinTogether);
        }

    }
    else
    {
        if (colorPinyinEnabled == false)
        {
            //getting all the syllables in the string, converting them to the diacritics format and concatenating them back to a string
            QString pinyinString = textAt(2);
            QStringList pinyinParts = textAt(2).split(QChar(' ')); //splitting on the space character to get the individual syllables
            QStringList diacriticPinyinParts = {};

            //iterating over the split strings
            for (const QString& syllable : pinyinParts) {
                diacriticPinyinParts.append(numberedPinyinToDiacritics(syllable));
            }

            //joining the QStringList to a QString with no separator
            QString diacriticsPinyin = diacriticPinyinParts.join(' ');
            ui->definitionPinyin->setText(diacriticsPinyin);
        }
        else
        {
            QStringList pinyinParts = textAt(2).split(QChar(' '));
            QStringList allPinyinParts = {};
            for (int i=0; i < pinyinParts.length(); i++)
            {
                //checking each syllable for a valid pinyin string
                if (isValidPinyin(pinyinParts[i]) == true)
                {
                    //getting the last character of the syllable and converting it to an integer
                    QChar intAtTheEnd = pinyinParts[i].back();
                    int lastDigit = intAtTheEnd.digitValue();

                    //since the second function can take two arguments, it is convenient to use them in this order
                    QString diacriticSyllable = numberedPinyinToDiacritics(pinyinParts[i]);

                    allPinyinParts.append(convertToColoredSpan(diacriticSyllable, lastDigit));
                }

                else
                {
                    allPinyinParts.append(pinyinParts[i]);
                }

            }

            //unifying the parts of the stringlist into one string
            QString allPinyinTogether = allPinyinParts.join(", ");
            ui->definitionPinyin->setText(allPinyinTogether);
        }
    }

    //the HSK level text
    if (HSKLevelEnabled == true)
    {
        QString HSKTest;
        HSKTest = MainWindow::getHSKNumber(textAt(1));

        ui->hskValue->setText(HSKTest);
        //reducing the font size of the HSK label
        //ui->hskValue->setPointSize(11);
    }
    else
    {
        //empty label
        ui->hskValue->setText(" ");
    }

    ui->definitionSimp->setText(textAt(1));
    ui->definitionTrad->setText(textAt(0));

    //setting the svg files
    if (simplifiedDisplayed == true)
    {
        displayedText = textAt(1);
        ui->characterWidget->setText(textAt(1));
    }
    else
    {
        displayedText = textAt(0);
        ui->characterWidget->setText(textAt(0));
    }

}

void MainWindow::on_actionAbout_triggered()
{
    //breaking up each line for e
    QMessageBox::about(this, tr("About"),
                       tr("DictoSinico v. 0.1 \n\nA GUI for the free Chinese-English dictionary CC-CEDICT \nReleased under GNU GPL v.3.0 "
                          "\n\nSee latest version at https://github.com/LStefanCoder/dictosinico"
                          "\n\nCredits: CC-CEDICT under CC-BY-SA 2.0 \nCSV file with HSK values from a Kaggle dataset by Rudd Fawcett under ODbL v.1.0. "
                          "\nAll fonts are from Google Fonts and under the SIL OFL. \nThe animated characters and SVG files are from Makemeahanzi, under the Arphic Public License."
                          "\nCombinear theme from DevSec Studio under the MIT License."
                          "\nAll links found in the readme on Github."
                          ));
}

void MainWindow::on_actionPreferences_triggered()
{
    PreferencesDialog dlg(this, diacriticsEnabled, colorPinyinEnabled, HSKLevelEnabled);
    //connecting the settings in the preferences window here, before the .exec() function
    connect(&dlg, &PreferencesDialog::diacriticPinyinEnabledChanged,
            this, &MainWindow::on_diacriticsFeatureChanged);
    connect(&dlg, &PreferencesDialog::colorPinyinEnabledChanged,
            this, &MainWindow::on_pinyinColorChanged);
    connect(&dlg, &PreferencesDialog::HSKEnabledChanged,
            this, &MainWindow::on_HSKChanged);
    dlg.exec(); // modal
}

void MainWindow::on_diacriticsFeatureChanged(bool enabled)
{
    //setting the global variable diacriticsEnabled to the state of the checkbox
    diacriticsEnabled = enabled;
}

void MainWindow::on_pinyinColorChanged(bool enabled)
{
    colorPinyinEnabled = enabled;
}

void MainWindow::on_HSKChanged(bool enabled)
{
    HSKLevelEnabled = enabled;
}

//this function gives an appropriate HTML string for a colored pinyin representation
QString MainWindow::convertToColoredSpan(QString s, int n)
{
    //going through all four tones, and returning the input string if the tone is 5 and colored HTML code if it is one of the other tones
    if (n == 1)
    {
        return "<span style='color:#ff0000;'>" + s + "</span>";
    }

    else if (n == 2)
    {
        return "<span style='color:#ffa500;'>" + s + "</span>";
    }

    else if (n == 3)
    {
        return "<span style='color:#008000;'>" + s + "</span>";
    }

    else if (n == 4)
    {
        return "<span style='color:#0000ff;'>" + s + "</span>";
    }

    else
    {
        return s;
    }
}

//helper functions
//AI-generated

QChar MainWindow::toneMarkVowel(QChar base, int tone)
{
    if (tone == 0 || tone == 5) return base;

    // Tone: 1=ā/á/ǎ/à, 2=..., 3=..., 4=...
    // Index 0..3 correspond to tone 1..4
    auto map = [&](QChar lower) -> const QChar* {
        static const QChar a[4] = { u'ā', u'á', u'ǎ', u'à' };
        static const QChar e[4] = { u'ē', u'é', u'ě', u'è' };
        static const QChar i[4] = { u'ī', u'í', u'ǐ', u'ì' };
        static const QChar o[4] = { u'ō', u'ó', u'ǒ', u'ò' };
        static const QChar u_[4] = { u'ū', u'ú', u'ǔ', u'ù' };
        static const QChar v[4] = { u'ǖ', u'ǘ', u'ǚ', u'ǜ' };

        // Also support uppercase by returning uppercase sets
        static const QChar A[4] = { u'Ā', u'Á', u'Ǎ', u'À' };
        static const QChar E[4] = { u'Ē', u'É', u'Ě', u'È' };
        static const QChar I[4] = { u'Ī', u'Í', u'Ǐ', u'Ì' };
        static const QChar O[4] = { u'Ō', u'Ó', u'Ǒ', u'Ò' };
        static const QChar U[4] = { u'Ū', u'Ú', u'Ǔ', u'Ù' };
        static const QChar V[4] = { u'Ǖ', u'Ǘ', u'Ǚ', u'Ǜ' };

        // lower might already be uppercase depending on how you call it,
        // so handle exact cases
        if (lower == u'a') return a;
        if (lower == u'e') return e;
        if (lower == u'i') return i;
        if (lower == u'o') return o;
        if (lower == u'u') return u_;
        if (lower == u'ü') return v;

        if (lower == u'A') return A;
        if (lower == u'E') return E;
        if (lower == u'I') return I;
        if (lower == u'O') return O;
        if (lower == u'U') return U;
        if (lower == u'Ü') return V;

        return nullptr;
    };

    const QChar* arr = map(base);
    if (!arr) return base;

    // tone is 1..4
    int idx = tone - 1;
    return arr[idx];
}

//AI generated
// Converts one syllable like "hai2" -> "hái"
QString MainWindow::numberedPinyinToDiacritics(QString s)
{
    s = s.trimmed();
    if (s.isEmpty()) return s;

    // If you sometimes use u: instead of ü, convert first
    s.replace("u:", "ü");

    // Tone digit at end
    QChar last = s.back();
    if (!last.isDigit()) return s;

    int tone = last.digitValue(); // 0..5 expected
    if (tone < 0 || tone > 5) return s;

    s.chop(1); // remove tone digit

    // Choose which vowel to mark:
    // 1) a, 2) e, 3) if "ou" then mark o, 4) otherwise mark i/u/ü
    int markPos = -1;

    auto find = [&](QChar ch) -> int {
        return s.indexOf(ch);
    };

    markPos = find(QChar(u'a'));
    if (markPos == -1) markPos = find(QChar(u'A'));

    if (markPos == -1) markPos = find(QChar(u'e'));
    if (markPos == -1) markPos = find(QChar(u'E'));

    if (markPos == -1) {
        int ouPos = s.indexOf("ou");
        if (ouPos != -1) markPos = ouPos;
    }

    if (markPos == -1) markPos = find(QChar(u'i'));
    if (markPos == -1) markPos = find(QChar(u'I'));

    if (markPos == -1) markPos = find(QChar(u'u'));
    if (markPos == -1) markPos = find(QChar(u'U'));

    if (markPos == -1) markPos = find(QChar(u'ü'));
    if (markPos == -1) markPos = find(QChar(u'Ü'));

    if (markPos == -1) return s; // no vowel found

    QChar base = s[markPos];
    s[markPos] = MainWindow::toneMarkVowel(base, tone);
    return s;
}

bool MainWindow::isValidPinyin(QString input)
{
    // Normalize case and keyboard-style ü forms:
    // v, u:, and u: -> ü
    input = input.toLower();

    QString normalized;
    normalized.reserve(input.size());

    for (int i = 0; i < input.size(); ++i) {
        const QChar ch = input.at(i);

        if (ch == QLatin1Char('u') &&
            i + 1 < input.size() &&
            input.at(i + 1) == QLatin1Char(':')) {
            normalized.append(QChar(0x00FC)); // ü
            ++i;
        }
        else if (ch == QLatin1Char('v')) {
            normalized.append(QChar(0x00FC)); // ü
        }
        else {
            normalized.append(ch);
        }
    }

    if (normalized.isEmpty())
        return false;

    QString core = normalized;

    // A tone number is mandatory.
    // Accept only trailing digits 1–5.
    if (core.isEmpty())
        return false;

    const QChar last = core.back();

    if (last < QLatin1Char('1') ||
        last > QLatin1Char('5')) {
        return false;
    }

    // Remove the mandatory tone number.
    core.chop(1);

    if (core.isEmpty())
        return false;

    // Only allow ASCII letters and ü.
    const QChar umlautU(0x00FC);

    for (const QChar ch : core) {
        const bool isAsciiLetter =
            (ch >= QLatin1Char('a') &&
             ch <= QLatin1Char('z'));

        if (!isAsciiLetter && ch != umlautU)
            return false;
    }

    // Initials must be checked longest first.
    static const QStringList initials = {
        QStringLiteral("zh"),
        QStringLiteral("ch"),
        QStringLiteral("sh"),

        QStringLiteral("b"),
        QStringLiteral("p"),
        QStringLiteral("m"),
        QStringLiteral("f"),
        QStringLiteral("d"),
        QStringLiteral("t"),
        QStringLiteral("n"),
        QStringLiteral("l"),
        QStringLiteral("g"),
        QStringLiteral("k"),
        QStringLiteral("h"),
        QStringLiteral("j"),
        QStringLiteral("q"),
        QStringLiteral("x"),
        QStringLiteral("r"),
        QStringLiteral("z"),
        QStringLiteral("c"),
        QStringLiteral("s"),
        QStringLiteral("y"),
        QStringLiteral("w")
    };

    // These are spelling finals, including the special "i" used in
    // zhi, chi, shi, ri, zi, ci, and si.
    static const QSet<QString> finals = {
        // a finals
        QStringLiteral("a"),
        QStringLiteral("ai"),
        QStringLiteral("ao"),
        QStringLiteral("an"),
        QStringLiteral("ang"),

        // e finals
        QStringLiteral("e"),
        QStringLiteral("ei"),
        QStringLiteral("en"),
        QStringLiteral("eng"),
        QStringLiteral("er"),

        // o finals
        QStringLiteral("o"),
        QStringLiteral("ou"),
        QStringLiteral("ong"),

        // i finals
        QStringLiteral("i"),
        QStringLiteral("ia"),
        QStringLiteral("iao"),
        QStringLiteral("ie"),
        QStringLiteral("iu"),
        QStringLiteral("ian"),
        QStringLiteral("iang"),
        QStringLiteral("in"),
        QStringLiteral("ing"),
        QStringLiteral("iong"),

        // u finals
        QStringLiteral("u"),
        QStringLiteral("ua"),
        QStringLiteral("uo"),
        QStringLiteral("uai"),
        QStringLiteral("ui"),
        QStringLiteral("uan"),
        QStringLiteral("uang"),
        QStringLiteral("un"),
        QStringLiteral("uo"),
        QStringLiteral("ueng"),

        // ü finals
        QStringLiteral("ü"),
        QStringLiteral("üe"),
        QStringLiteral("üan"),
        QStringLiteral("ün")
    };

    // Find the longest matching initial.
    QString finalPart = core;
    int longestInitialLength = 0;

    for (const QString &initial : initials) {
        if (core.startsWith(initial) &&
            initial.size() > longestInitialLength) {
            longestInitialLength = initial.size();
        }
    }

    if (longestInitialLength > 0) {
        finalPart = core.mid(longestInitialLength);
    }

    if (finalPart.isEmpty())
        return false;

    return finals.contains(finalPart);
}

//function reading the CSV with the HSK values
QVector<QPair<QString, int>> MainWindow::loadCsv_col1Text_col2Int(const QString& filePath)
{
    QVector<QPair<QString, int>> rows;

    QFile file(filePath);
    //the following function is needed, otherwise the function won't work
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        qDebug() << "wrong";
        //return rows;

    QTextStream in(&file);

    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        // Simple (unquoted) CSV split
        const QStringList parts = line.split(',', Qt::KeepEmptyParts);
        if (parts.size() < 2) continue;

        const QString col1 = parts.at(0).trimmed();

        bool ok = false;
        const int col2 = parts.at(1).trimmed().toInt(&ok);
        if (!ok) continue; // skip rows where col2 isn't an integer

        rows.push_back({col1, col2});
    }

    //there should also be a global variable
    return rows;
}

//getting the correct HSK number
QString MainWindow::getHSKNumber(QString simp)
{
    //HSKTableRows was declared as a private variable of MainWindow
    int HSKNumber = 0;


    //looping through all the characters of the simplified word, looking for the simplified character in the HSK table
    for (QChar ch : simp) {
        for (const auto& [character, number] : HSKTableRows)
        {
            //the vector contains QStrings, not QChars

            if(character == QString(ch))
            {
                if(int(number) > HSKNumber)
                {
                    HSKNumber = int(number);
                }
            }
        }
    }

    if (HSKNumber == 0)
    {
        return "HSK+";
    }
    else
    {
        QString returnString = "HSK" + QString::number(HSKNumber);
        return returnString;
    }

}

//the logic of the two buttons controlling the animation
void MainWindow::on_animationStartButton_clicked()
{
    if(!displayedText.isEmpty())
    {
        ui->characterWidget->setText(displayedText);
        ui->characterWidget->play();
    }
}

void MainWindow::on_animationStopButton_clicked()
{
    ui->characterWidget->stop();
}

//the code for the two radio buttons controlling whether simplified or traditional character SVG files are displayed
void MainWindow::on_simplifiedRadio_clicked()
{
    simplifiedDisplayed = true;
    traditionalDisplayed = false;
}

void MainWindow::on_traditionalRadio_clicked()
{
    traditionalDisplayed = true;
    simplifiedDisplayed = false;
}