// preferencesdialog.h
#pragma once
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class PreferencesDialog;
}
QT_END_NAMESPACE

class PreferencesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PreferencesDialog(QWidget *parent = nullptr, bool diacriticValue = false);
    ~PreferencesDialog();

    bool diacriticPinyinEnabled() const;

signals:
    void diacriticPinyinEnabledChanged(bool enabled);

private slots:
    void onDiacriticCheckboxToggled(bool checked);

private:
    Ui::PreferencesDialog *ui;
};
