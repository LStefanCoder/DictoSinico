// LoadingDialog.cpp
#include "loadingdialog.h"
#include <QLabel>
#include <QVBoxLayout>

LoadingDialog::LoadingDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Loading...");
    setModal(true);
    label_ = new QLabel("Loading, please wait...", this);
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(label_);
}
void LoadingDialog::setProgressText(const QString& text) { label_->setText(text); }
