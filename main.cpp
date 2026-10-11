#include "mainwindow.h"
#include "theme_manager.h"
#include "tests/navigation_checks.h"
#include "tests/theme_checks.h"

int main(int argc,char **argv){
 QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
 QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
 QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
 QApplication app(argc,argv);UiReplica ui;if(!ui.load())return 2;
 const auto args=app.arguments();
 if(args.contains("--audit")||args.contains("--render-all")||args.contains("--verify-navigation")||args.contains("--verify-theme")){
  QTimer::singleShot(200,&app,[&]{
   const int outputIndex=args.indexOf("--output");
   QString dir=outputIndex>=0?QDir(args.value(outputIndex+1)).absolutePath():QCoreApplication::applicationDirPath()+"/validation";
   QDir().mkpath(dir);
   const auto save=[&](const QString &name,QJsonObject report){QFile f(dir+"/"+name);if(!f.open(QIODevice::WriteOnly))return false;f.write(QJsonDocument(report).toJson());return true;};
   save("audit.json",ui.audit());ui.root->grab().save(dir+"/initial.png");
   if(args.contains("--verify-navigation")){const auto result=verifyNavigation(ui,dir);save("navigation.json",result);app.exit(result["failures"].toArray().isEmpty()?0:1);return;}
   if(args.contains("--verify-theme")){const auto result=verifyTheme(ui,dir);save("theme.json",result);app.exit(result["failures"].toArray().isEmpty()?0:1);return;}
   if(args.contains("--render-all"))for(auto i=ui.data.begin();i!=ui.data.end();++i){
    auto w=ui.widget(i.key());if(!w||ui.automatic.contains(i.key()))continue;
    QString n=w->objectName();
    if(auto tab=qobject_cast<QTabWidget*>(w)){ui.revealPage(tab);for(int p=0;p<tab->count();++p){tab->setCurrentIndex(p);app.processEvents();ui.root->grab().save(dir+"/"+n+"-"+QString::number(p)+".png");}}
    else if(auto stack=qobject_cast<QStackedWidget*>(w)){ui.revealPage(stack);for(int p=0;p<stack->count();++p){stack->setCurrentIndex(p);app.processEvents();ui.root->grab().save(dir+"/"+n+"-"+QString::number(p)+".png");}}
   }
   app.quit();
  });
 }
 return app.exec();
}
