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
    explicit PreferencesDialog(QWidget *parent = nullptr, bool diacriticValue = false, bool colorPinyinValue = false, bool HSKValue = false);
    ~PreferencesDialog();

    bool diacriticPinyinEnabled() const;
    bool colorPinyinEnabled() const;
    bool HSKEnabled() const;

signals:
    void diacriticPinyinEnabledChanged(bool enabled);
    void colorPinyinEnabledChanged(bool enabled);
    void HSKEnabledChanged(bool enabled);

private slots:
    void onDiacriticCheckboxToggled(bool checked);
    void onColorCheckboxToggled(bool checked);
    void onHSKCheckboxToggled(bool checked);

private:
    Ui::PreferencesDialog *ui;
};
