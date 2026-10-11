#include "mainwindow.h"
#include "theme_manager.h"
#include "ui_mainwindow.h"

UiReplica::UiReplica() = default;
UiReplica::~UiReplica(){delete root;}
bool UiReplica::has(QString id,const char *base) const {return data.value(id)["bases"].toArray().contains(base);}
QWidget *UiReplica::widget(QString id){return qobject_cast<QWidget*>(objects.value(id));}
void UiReplica::revealPage(QWidget *page){for(QWidget *w=page;w;w=w->parentWidget())if(auto s=qobject_cast<QStackedWidget*>(w->parentWidget()))s->setCurrentWidget(w);}
bool UiReplica::load(){
 qApp->setStyle("Fusion");
 auto window=new CentralWidget;
 root=window;form.reset(new Ui::TeraToolWindow);form->setupUi(window);
 root->setWindowFlags(Qt::Window|Qt::FramelessWindowHint);
 QList<QObject*> all=root->findChildren<QObject*>();all.prepend(root);
 for(auto o:all){
  if(!qobject_cast<QWidget*>(o)&&!qobject_cast<QLayout*>(o))continue;
  QString id=o->objectName();if(id.isEmpty())id=QString("internal_%1").arg(quintptr(o),0,16);
  while(objects.contains(id))id+="_";
  objects[id]=o;QJsonArray bases;for(auto m=o->metaObject();m;m=m->superClass())bases.append(m->className());
  data[id]=QJsonObject{{"class",o->metaObject()->className()},{"name",o->objectName()},{"bases",bases}};
  if(o->objectName()=="qt_tabwidget_stackedwidget")automatic.insert(id);
  if(auto w=qobject_cast<QWidget*>(o)){
   // Designer has no visibility property; inactive page visibility is Qt-owned.
   if(w->property("initiallyHidden").toBool())w->hide();
   if(w->property("mouseTransparent").toBool())w->setAttribute(Qt::WA_TransparentForMouseEvents);
   if(auto b=qobject_cast<QTextBrowser*>(w))b->setOpenExternalLinks(false);
   if(auto l=qobject_cast<QLabel*>(w))l->setOpenExternalLinks(false);
  }
 }
 wireNavigation();theme=new ThemeManager(root);root->resize(1200,800);root->show();return true;
}
QJsonObject UiReplica::audit() const {
 int widgets=0,layouts=0;for(auto o:objects){widgets+=qobject_cast<QWidget*>(o)!=nullptr;layouts+=qobject_cast<QLayout*>(o)!=nullptr;}
 QJsonArray captions;for(auto c:root->findChildren<CaptionButton*>()){auto p=c->mapTo(root,QPoint());captions.append(QJsonObject{{"kind",c->kind},{"visible",c->isVisible()},{"geometry",QJsonArray{p.x(),p.y(),c->width(),c->height()}}});}
 return QJsonObject{{"uiSource","ui/mainwindow.ui"},{"runtimeSnapshotRequired",false},{"widgetsMapped",widgets},{"layoutsMapped",layouts},{"captions",captions},{"unmapped",QJsonArray()},{"diagnostics",QJsonArray()}};
}

void UiReplica::wireNavigation(){
  auto find=[&](QString n){for(auto o:objects)if(o&&o->objectName()==n)return o;return static_cast<QObject*>(nullptr);};
  auto loginPage=qobject_cast<QWidget*>(find("Login_Page"));
  auto registerForm=qobject_cast<QWidget*>(find("LogIn_2"));
  auto brandsPage=qobject_cast<QWidget*>(find("page_11"));
  auto settingsPage=qobject_cast<QWidget*>(find("page_5"));
  auto mainStack=qobject_cast<QStackedWidget*>(find("stackedWidget"));
  auto loginHost=qobject_cast<QWidget*>(find("LOGIN"));
  auto toolbar=qobject_cast<QWidget*>(find("horizontalFrame"));
  if(mainStack&&loginHost&&toolbar){
   auto updateToolbar=[mainStack,loginHost,toolbar]{toolbar->setVisible(mainStack->currentWidget()!=loginHost);};
   QObject::connect(mainStack,&QStackedWidget::currentChanged,toolbar,[updateToolbar](int){updateToolbar();});
   updateToolbar();
  }
  // Only presentation navigation is reconstructed: no authentication, devices,
  // serial ports, downloads, subprocesses, or network code exists in this build.
  for(auto i=data.begin();i!=data.end();++i)if(auto b=qobject_cast<QAbstractButton*>(objects.value(i.key()))){
   QString n=b->objectName(),text=b->text().trimmed();
   QWidget *target=nullptr;
   if(text=="Register now")target=registerForm;
   else if(text=="Already have account?"||text=="Log out")target=loginPage;
   else if(n=="LogIn"||text=="Back")target=brandsPage;
   else if(n=="btn_settings")target=settingsPage;
   if(target)QObject::connect(b,&QAbstractButton::clicked,target,[this,target]{revealPage(target);});
  }
  auto menu=[this]{QMenu m(root);for(auto i=data.begin();i!=data.end();++i)if(auto t=qobject_cast<QTabWidget*>(widget(i.key()))){for(int n=0;n<t->count();n++){auto a=m.addAction(t->objectName()+" / "+t->tabText(n));QObject::connect(a,&QAction::triggered,t,[this,t,n]{revealPage(t);t->setCurrentIndex(n);});}}else if(auto s=qobject_cast<QStackedWidget*>(widget(i.key()))){if(automatic.contains(i.key()))continue;for(int n=0;n<s->count();n++){auto a=m.addAction(s->objectName()+" / "+s->widget(n)->objectName());QObject::connect(a,&QAction::triggered,s,[this,s,n]{revealPage(s);s->setCurrentIndex(n);});}}m.exec(QCursor::pos());};
  auto shortcut=new QShortcut(QKeySequence(Qt::Key_F6),root);QObject::connect(shortcut,&QShortcut::activated,root,menu);
 }
