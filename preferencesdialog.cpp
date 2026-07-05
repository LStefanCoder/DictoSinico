// preferencesdialog.cpp
#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);

    // Placeholder: set initial slider values
    //if (ui->horizontalSliderVolume) ui->horizontalSliderVolume->setValue(50);
    //if (ui->horizontalSliderBrightness) ui->horizontalSliderBrightness->setValue(50);
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}
