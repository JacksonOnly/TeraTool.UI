#pragma once
#include <QtWidgets>

class ThemeManager : public QObject {
 Q_OBJECT
public:
 explicit ThemeManager(QWidget *root);
 bool isDark() const { return dark; }
 void setDark(bool enabled,bool persist=true);
private:
 QWidget *root;
 bool dark=true;
 QCheckBox *darkToggle=nullptr;
 QComboBox *selector=nullptr;
 QHash<QWidget*,QString> originalStyles;
 QHash<QAbstractButton*,QIcon> originalIcons;
 QHash<QTextEdit*,QString> originalHtml;
 void synchronizeControls();
};
