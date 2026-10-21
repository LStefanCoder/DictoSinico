// preferencesdialog.cpp
#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

PreferencesDialog::PreferencesDialog(QWidget *parent, bool diacriticValue, bool colorPinyinValue, bool HSKValue)
    : QDialog(parent)
    , ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);

    //just like the main window, it shouldn't be resized to a smaller than original size
    this->setMinimumSize(this->size());
    //in this version, the window can't be enlarged either
    this->setMaximumSize(this->size());
    //giving a window title
    this->setWindowTitle("Preferences");

    //setting the value of the checkbox to what is supposed to be when the preferences window is called again
    ui->diacriticBox->setChecked(diacriticValue);

    connect(ui->diacriticBox, &QCheckBox::toggled,
            this, [this](bool on){
        emit diacriticPinyinEnabledChanged(on);
    });

    ui->colorPinyinBox->setChecked(colorPinyinValue);

    connect(ui->colorPinyinBox, &QCheckBox::toggled,
            this, [this](bool on){
        emit colorPinyinEnabledChanged(on);
    });

    ui->HSKLevelBox->setChecked(HSKValue);

    connect(ui->HSKLevelBox, &QCheckBox::toggled,
            this, [this](bool on){
        emit HSKEnabledChanged(on);
    });

}


PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

bool PreferencesDialog::diacriticPinyinEnabled() const
{
    return ui->diacriticBox->isChecked();
}

bool PreferencesDialog::colorPinyinEnabled() const
{
    return ui->colorPinyinBox->isChecked();
}

bool PreferencesDialog::HSKEnabled() const
{
    return ui->HSKLevelBox->isChecked();
}

void PreferencesDialog::onDiacriticCheckboxToggled(bool checked)
{
    emit diacriticPinyinEnabledChanged(checked);
}

void PreferencesDialog::onColorCheckboxToggled(bool checked)
{
    emit colorPinyinEnabledChanged(checked);
}

void PreferencesDialog::onHSKCheckboxToggled(bool checked)
{
    emit HSKEnabledChanged(checked);
}