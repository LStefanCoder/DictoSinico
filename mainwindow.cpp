#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "preferencesdialog.h"
#include "loadingdialog.h"

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

//for the loading window
#include <QFutureWatcher>
#include <QtConcurrent>

//themes to be downloaded here: https://qt-material.readthedocs.io/en/latest/
//https://github.com/hubenchang0515/QtTheme
//https://qss-stock.devsecstudio.com/documentation.php


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->searchButton, &QPushButton::clicked, this, &MainWindow::on_searchButton_clicked);
    //connect(ui->loadFileButton, &QPushButton::clicked, this, &MainWindow::on_loadFileButton_clicked);
    //this is for when a row of the table was selected
    connect(ui->tableWidget, &QTableWidget::itemSelectionChanged,
            this, &MainWindow::on_TableSelectionChanged);

    ui->lineEdit->setText("");

    // Setup Table Widget (Better than QListWidget for structured data)
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setColumnCount(4);  //there are 4 columns needed

    //setting headers
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

void MainWindow::loadDictionaryFile(const QString &filePath, const QString &filePath2)
{
    qDebug() << filePath;
    qDebug() << filePath2;
    //creating the logic for the loading window to show during the loading of the dictionary
    auto* loading = new LoadingDialog(this);

    // Show immediately
    loading->show();

    // Keep the dialog alive until work finishes
    auto* watcher = new QFutureWatcher<std::pair<QByteArray, QByteArray>>(this);

    connect(watcher, &QFutureWatcher<QByteArray>::finished, this, [=]() {
        // Retrieve results from the future and update your UI
        auto [data1, data2] = watcher->result();
        // ... parse data / update editor ...

        loading->close();
        loading->deleteLater();
        watcher->deleteLater();
    });

    QFile file(filePath);

    watcher->setFuture(QtConcurrent::run([filePath, filePath2]() -> QPair<QByteArray, QByteArray> {
        auto readAll = [](const QString& path) -> QByteArray {
            QFile f(path);
            if (!f.open(QIODevice::ReadOnly)) return {};
            return f.readAll();
        };

        return { readAll(filePath), readAll(filePath2) };
    }));

    /*watcher->setFuture(QtConcurrent::run([filePath]() -> QByteArray {
        QFile f(filePath);
        if (!f.open(QIODevice::ReadOnly)) return {};
        return f.readAll();
    }));*/



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

    HSKTableRows = MainWindow::loadCsv_col1Text_col2Int(filePath2);

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

    ui->lineEdit->clear();

    QList<QTableWidgetItem*> selectedItems = ui->tableWidget->selectedItems();
    foreach (QTableWidgetItem *item, selectedItems) {
        item->setSelected(false);
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

    ui->definitionEng->setText(textAt(3));
    if (diacriticsEnabled == false)
    {
        if (colorPinyinEnabled == false)
        {
            ui->definitionPinyin->setText(textAt(2));
        }
        else
        {
            QString pinyinString = textAt(2);
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
                    //qDebug() << 'false';
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

    //the HSK level text

    if (HSKLevelEnabled == true)
    {
        QString HSKTest;
        HSKTest = MainWindow::getHSKNumber(textAt(1));

        ui->hskValue->setText(HSKTest);
    }
    else
    {
        //empty label
        ui->hskValue->setText(" ");
    }

    ui->definitionSimp->setText(textAt(1));
    ui->definitionTrad->setText(textAt(0));
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About"),
                       tr("CeDict GUI\nVersion 1.0\n\nShort description..."));
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
    //going through all four tones, and returning the input string if the tone is 5
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
    // --- normalize: lowercase + keyboard forms u: / v -> ü ---
    input = input.toLower();
    QString s;
    s.reserve(input.size());

    for (int i = 0; i < input.size(); ++i) {
        const QChar ch = input[i];
        if (ch == 'u' && i + 1 < input.size() && input[i + 1] == ':') {
            s.push_back(QChar(0x00FC)); // ü
            ++i; // skip ':'
        } else if (ch == 'v') {
            s.push_back(QChar(0x00FC)); // ü
        } else {
            s.push_back(ch);
        }
    }

    // --- strip optional trailing tone digit (1-5), default tone=5 (neutral/no digit) ---
    QString core = s;
    int tone = 5;

    if (core.isEmpty())
        return false;

    const QChar last = core.back();
    if (last.isDigit()) {
        int d = last.digitValue();
        if (d < 1 || d > 5)
            return false;
        tone = d;
        core.chop(1);
    }

    if (core.isEmpty())
        return false;

    // --- validate characters: only a-z and ü ---
    auto isUmlautU = [](QChar c) -> bool { return c == QChar(0x00FC); };

    for (QChar ch : core) {
        const ushort u = ch.unicode();
        bool ok = (u >= 'a' && u <= 'z') || isUmlautU(ch);
        if (!ok)
            return false;
        if (ch.isDigit())
            return false;
    }

    // --- initials (single letters) ---
    static const std::vector<QString> initials = {
        "b","p","m","f","d","t","n","l","g","k","h","j","q","x","r","z","c","s"
    };

    // --- finals (pragmatic set) ---
    static const std::unordered_set<std::string> finals = {
        // a-row
        "a","ai","ao","an","ang","ia","uan",
        // o-row
        "o","ou","ong",
        // e-row
        "e","ei","en","eng","er",
        // i-row
        "i","ia","ie","in","ing","iong",
        // u-row
        "u","ua","uo","uai","ui","uan","uang","ueng","un","uong",
        // ü-row
        "ü","üe","üan","ün"
    };

    // --- split into initial (if any) + final ---
    QChar first = core[0];
    bool hasInitial = false;
    for (const QString &iniCand : initials) {
        if (iniCand.size() == 1 && iniCand[0] == first) {
            hasInitial = true;
            break;
        }
    }

    QString fin = hasInitial ? core.mid(1) : core;
    if (fin.isEmpty())
        return false;

    // --- final must be in our finals set ---
    // Convert fin (UTF-16) to UTF-8 for lookup strings (expects ü as UTF-8)
    std::string finStd = fin.toUtf8().constData();
    if (finals.find(finStd) == finals.end())
        return false;

    // --- must contain at least one vowel-like character ---
    bool hasVowel = false;
    for (QChar c : fin) {
        if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' || isUmlautU(c)) {
            hasVowel = true;
            break;
        }
    }
    return hasVowel;
}

QVector<QPair<QString, int>> MainWindow::loadCsv_col1Text_col2Int(const QString& filePath)
{
    qDebug() << filePath;
    QVector<QPair<QString, int>> rows;

    QFile file(filePath);
    //if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
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
    //HSKTableRows was declared as a private variable of MainWindo
    QVector<QPair<QString, int>> HSKTable = HSKTableRows;
    int HSKNumber = 0;

    //looping through all the characters of the simplified word, looking for the simplified character in the HSK table
    for (QChar ch : simp) {
        for (const auto& [character, number] : HSKTable)
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