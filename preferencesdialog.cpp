// preferencesdialog.cpp
#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

PreferencesDialog::PreferencesDialog(QWidget *parent, bool diacriticValue)
    : QDialog(parent)
    , ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);

    //setting the value of the checkbox to what is supposed to be when the preferences window is called again
    ui->diacriticBox->setChecked(diacriticValue);

    connect(ui->diacriticBox, &QCheckBox::toggled,
            this, [this](bool on){
        emit diacriticPinyinEnabledChanged(on);
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

void PreferencesDialog::onDiacriticCheckboxToggled(bool checked)
{
    emit diacriticPinyinEnabledChanged(checked);
}