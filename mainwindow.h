#pragma once
#include "shell.h"
#include <memory>

namespace Ui { class TeraToolWindow; }
class ThemeManager;

// Editable ui/mainwindow.ui is compiled by uic; ui.json is not loaded.
class UiReplica {
public:
 UiReplica();
 ~UiReplica();
 bool load();
 void revealPage(QWidget *page);
 void wireNavigation();
 bool has(QString id,const char *base) const;
 QWidget *widget(QString id);
 QJsonObject audit() const;
 QHash<QString,QJsonObject> data;
 QHash<QString,QObject*> objects;
 QSet<QString> automatic;
 QWidget *root=nullptr;
 ThemeManager *theme=nullptr;
private:
 std::unique_ptr<Ui::TeraToolWindow> form;
};
