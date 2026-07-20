#ifndef LOADINGDIALOG_H
#define LOADINGDIALOG_H

// LoadingDialog.h
#include <QDialog>

class QLabel;

class LoadingDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoadingDialog(QWidget* parent = nullptr);
    void setProgressText(const QString& text);
private:
    QLabel* label_;
};


#endif // LOADINGDIALOG_H
